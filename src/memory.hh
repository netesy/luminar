#pragma once

#include "memory_analyzer.hh"
#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#ifdef _MSC_VER
#include <intrin.h>
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define TRACE_INFO() (std::string(__FUNCTION__) + " at line " + TOSTRING(__LINE__))

// Size classes for small allocations (in bytes)
constexpr std::array<size_t, 8> SMALL_SIZES = {16, 32, 64, 128, 256, 512, 1024, 2048};
constexpr size_t MAX_SMALL_SIZE = SMALL_SIZES.back();
constexpr size_t BLOCK_SIZE = 64 * 1024; // 64KB blocks
constexpr size_t MAX_BLOCKS_PER_CHUNK = 64;
constexpr size_t POOL_INITIAL_SIZE = 1024 * 1024; // 1MB initial pool size

class DefaultAllocator {
private:
    std::mutex mutex;
    static constexpr size_t TEMP_BUFFER_SIZE = 4096;
    static constexpr size_t POOL_GROWTH_FACTOR = 2;

    struct ObjectPool {
        std::mutex pool_mutex;
        std::vector<void*> free_objects;
        size_t object_size;
        size_t capacity;
        std::atomic<size_t> allocation_count{0};
        std::unordered_map<void*, bool> allocated_objects; // Track allocated objects

        ObjectPool(size_t size, size_t initial_capacity)
            : object_size(size)
            , capacity(initial_capacity)
        {
            expand(initial_capacity);
        }

        void expand(size_t additional_capacity) {
            std::lock_guard<std::mutex> lock(pool_mutex);
            try {
                size_t old_size = free_objects.size();
                free_objects.reserve(old_size + additional_capacity);

                uint8_t* memory = new uint8_t[object_size * additional_capacity];
                for (size_t i = 0; i < additional_capacity; ++i) {
                    free_objects.push_back(memory + (i * object_size));
                }
                capacity += additional_capacity;
            } catch (const std::bad_alloc& e) {
                // Log error and try to recover
                std::cerr << "Failed to expand pool: " << e.what() << std::endl;
                throw;
            }
        }

        void* allocate() {
            // std::lock_guard<std::mutex> lock(pool_mutex);
            // if (free_objects.empty()) {
            //     expand(capacity * POOL_GROWTH_FACTOR);
            // }

            // void* ptr = free_objects.back();
            // free_objects.pop_back();
            // allocation_count.fetch_add(1, std::memory_order_relaxed);
            // return ptr;
            std::lock_guard<std::mutex> lock(pool_mutex);
            if (free_objects.empty()) {
                expand(capacity * POOL_GROWTH_FACTOR);
            }

            void* ptr = free_objects.back();
            free_objects.pop_back();
            allocated_objects[ptr] = true;
            return ptr;
        }

        bool deallocate(void* ptr) {
            // std::lock_guard<std::mutex> lock(pool_mutex);
            // free_objects.push_back(ptr);
            // allocation_count.fetch_sub(1, std::memory_order_relaxed);
            std::lock_guard<std::mutex> lock(pool_mutex);
            auto it = allocated_objects.find(ptr);
            if (it != allocated_objects.end()) {
                free_objects.push_back(ptr);
                allocated_objects.erase(it);
                return true;
            }
            return false;
        }

        ~ObjectPool() {
            std::lock_guard<std::mutex> lock(pool_mutex);
            // Clean up all allocated memory
            std::unordered_set<uint8_t*> base_pointers;

            for (void* ptr : free_objects) {
                base_pointers.insert(static_cast<uint8_t*>(ptr) -
                                     ((reinterpret_cast<std::uintptr_t>(ptr) / object_size) * object_size));
            }

            for (uint8_t* ptr : base_pointers) {
                delete[] ptr;
            }
        }
    };

    // Thread-local cache with proper initialization
    struct ThreadCache {
        struct CacheEntry {
            std::vector<void*> objects;
            size_t size_class;
            size_t hit_count;  // Changed from atomic
            size_t miss_count; // Changed from atomic

            CacheEntry() : size_class(0), hit_count(0), miss_count(0) {}
            explicit CacheEntry(size_t sc) : size_class(sc), hit_count(0), miss_count(0) {}

            // Now we can safely copy and assign
            CacheEntry(const CacheEntry& other) = default;
            CacheEntry& operator=(const CacheEntry& other) = default;
        };

        std::array<CacheEntry, SMALL_SIZES.size()> small_cache;
        std::vector<std::pair<void*, size_t>> large_allocations;
        static constexpr size_t MAX_CACHE_OBJECTS = 32;
        std::mutex cache_mutex; // Added mutex for thread safety

        ThreadCache() {
            for (size_t i = 0; i < SMALL_SIZES.size(); ++i) {
                small_cache[i] = CacheEntry(SMALL_SIZES[i]);
            }
        }

        void* fetchFromCache(size_t size_class_index) {
            std::lock_guard<std::mutex> lock(cache_mutex);
            auto& cache = small_cache[size_class_index];
            if (!cache.objects.empty()) {
                void* ptr = cache.objects.back();
                cache.objects.pop_back();
                cache.hit_count++;
                return ptr;
            }
            cache.miss_count++;
            return nullptr;
        }

        void returnToCache(void* ptr, size_t size_class_index) {
            std::lock_guard<std::mutex> lock(cache_mutex);
            auto& cache = small_cache[size_class_index];
            if (cache.objects.size() < MAX_CACHE_OBJECTS) {
                cache.objects.push_back(ptr);
            } else {
                delete[] static_cast<uint8_t*>(ptr);
            }
        }

        ~ThreadCache() {
            std::lock_guard<std::mutex> lock(cache_mutex);
            // Clean up cache
            for (auto& cache : small_cache) {
                for (void* ptr : cache.objects) {
                    delete[] static_cast<uint8_t*>(ptr);
                }
            }

            // Clean up large allocations
            for (auto& [ptr, size] : large_allocations) {
                delete[] static_cast<uint8_t*>(ptr);
            }
        }
    };

    static thread_local ThreadCache thread_cache;
    std::mutex global_mutex;

    // Initialize object pools in constructor
    std::array<std::unique_ptr<ObjectPool>, SMALL_SIZES.size()> object_pools;

    void initializePools() {
        for (size_t i = 0; i < SMALL_SIZES.size(); ++i) {
            object_pools[i] = std::make_unique<ObjectPool>(
                SMALL_SIZES[i],
                POOL_INITIAL_SIZE / SMALL_SIZES[i]
                );
        }
    }

public:
    DefaultAllocator() {
        initializePools();
    }

    void* allocate(size_t size, size_t alignment) {
        // Handle alignment requirements
        size = std::max(size, alignment);

        if (size <= MAX_SMALL_SIZE) {
            size_t pool_index = get_size_class(size);
            if (pool_index < object_pools.size()) {
                return object_pools[pool_index]->allocate();
            }
        }

        // Fall back to regular allocation for large sizes
        try {
            return new uint8_t[size];
        } catch (const std::bad_alloc& e) {
            std::cerr << "Failed to allocate " << size << " bytes: " << e.what() << std::endl;
            throw;
        }
    }

    void deallocate(void* ptr) noexcept {
        if (!ptr) return;

        // Try to find the pointer in object pools
        for (size_t i = 0; i < object_pools.size(); ++i) {
            if (object_pools[i]->deallocate(ptr)) {
                return;
            }
        }

        // If not found in pools, it must be a large allocation
        delete[] static_cast<uint8_t*>(ptr);
    }

private:
    static size_t get_size_class(size_t size) {
        for (size_t i = 0; i < SMALL_SIZES.size(); ++i) {
            if (size <= SMALL_SIZES[i]) return i;
        }
        return static_cast<size_t>(-1);
    }
};

// Define the thread_local static member outside the class
//thread_local DefaultAllocator::ThreadCache DefaultAllocator::thread_cache;

struct AllocationInfo
{
    size_t size;
    std::chrono::steady_clock::time_point timestamp;
    std::string stackTrace;
    size_t generation;
};

class AllocationTracker
{
    std::map<size_t, std::vector<void *>> sizeTree;
    std::unordered_map<void *, std::unique_ptr<AllocationInfo>> skiplist;
    std::mutex mutex;

public:
    void add(void *ptr, size_t size, const std::string &stackTrace, size_t generation)
    {
        std::lock_guard lock(mutex);
        auto info = std::make_unique<AllocationInfo>(
            AllocationInfo{size, std::chrono::steady_clock::now(), stackTrace, generation});
        sizeTree[size].push_back(ptr);
        skiplist[ptr] = std::move(info);
    }

    void remove(void *ptr)
    {
        std::lock_guard lock(mutex);
        if (auto it = skiplist.find(ptr); it != skiplist.end()) {
            sizeTree[it->second->size].erase(std::remove(sizeTree[it->second->size].begin(),
                                                         sizeTree[it->second->size].end(),
                                                         ptr),
                                             sizeTree[it->second->size].end());
            if (sizeTree[it->second->size].empty())
                sizeTree.erase(it->second->size);
            skiplist.erase(it);
        }
    }

    AllocationInfo *get(void *ptr)
    {
        std::lock_guard lock(mutex);
        auto it = skiplist.find(ptr);
        return it != skiplist.end() ? it->second.get() : nullptr;
    }

    // Add these methods to make AllocationTracker iterable
    auto begin() const { return skiplist.begin(); }
    auto end() const { return skiplist.end(); }
    auto begin() { return skiplist.begin(); }
    auto end() { return skiplist.end(); }
};

template<typename Allocator = DefaultAllocator>
class MemoryManager
{
private:
    mutable std::ofstream logFile;
    mutable std::mutex logMutex;
    bool auditMode;
    Allocator allocator;
    AllocationTracker tracker;
    MemoryAnalyzer analyzer;

public:
    MemoryManager(bool enableAudit = false)
        : auditMode(enableAudit)
    {
    }

    ~MemoryManager()
    {
        analyzeMemoryUsage();
    }

    void setAuditMode(bool enable)
    {
        auditMode = enable;
    }

    void *allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        void *ptr = allocator.allocate(size, alignment);
        analyzer.recordAllocation(ptr, size, auditMode ? TRACE_INFO() : "");
        return ptr;
    }

    void deallocate(void *ptr)
    {
        analyzer.recordDeallocation(ptr);
        allocator.deallocate(ptr);
    }

    void analyzeMemoryUsage() const {
        std::cout << std::string(20, '-') << "\n";
        auto reports = analyzer.getMemoryUsage();
        analyzer.printMemoryUsageReport(reports);

        std::cout << std::string(20, '-') << "\n";
    }

    class Region
    {
    private:
        MemoryManager &manager;
        std::unordered_map<void *, size_t> objectGenerations;
        size_t currentGeneration;

    public:
        explicit Region(MemoryManager &mgr)
            : manager(mgr)
            , currentGeneration(0)
        {
        }

        ~Region()
        {
            for (const auto &[ptr, gen] : objectGenerations) {
                manager.deallocate(ptr);
            }
        }

        template<typename T, typename... Args>
        T *create(Args &&...args)
        {
            void *memory = manager.allocate(sizeof(T), alignof(T));
            T *obj = new (memory) T(std::forward<Args>(args)...);
            size_t generation = ++currentGeneration;
            objectGenerations[memory] = generation;
            return obj;
        }

        void deallocate(void *ptr)
        {
            auto it = objectGenerations.find(ptr);
            if (it != objectGenerations.end()) {
                manager.deallocate(ptr);
                objectGenerations.erase(it);
            }
        }

        size_t getGeneration(void *ptr) const
        {
            auto it = objectGenerations.find(ptr);
            return (it != objectGenerations.end()) ? it->second : 0;
        }
    };

    template<typename T>
    class Linear
    {
    private:
        T *ptr;
        Region *region;
        bool ownsResource;
        MemoryManager &manager;

    public:
        explicit Linear(Region &r, T *p, MemoryManager &mgr)
            : ptr(p)
            , region(&r)
            , ownsResource(true)
            , manager(mgr)
        {
        }

        Linear(const Linear &) = delete;
        Linear &operator=(const Linear &) = delete;

        Linear(Linear &&other) noexcept
            : ptr(other.ptr)
            , region(other.region)
            , ownsResource(other.ownsResource)
            , manager(other.manager)
        {
            other.ptr = nullptr;
            other.region = nullptr;
            other.ownsResource = false;
        }

        Linear &operator=(Linear &&other) noexcept
        {
            if (this != &other) {
                release();
                ptr = other.ptr;
                region = other.region;
                ownsResource = other.ownsResource;
                other.ptr = nullptr;
                other.region = nullptr;
                other.ownsResource = false;
            }
            return *this;
        }

        ~Linear() { release(); }

        T *operator->() const { return ptr; }
        T &operator*() const { return *ptr; }
        T *get() const { return ptr; }

        T *borrow() const
        {
            return ptr;
        }

        Region &getRegion() const { return *region; }

        void release()
        {
            if (ptr && ownsResource) {
                ptr->~T();
                region->deallocate(ptr);
                ptr = nullptr;
                ownsResource = false;
            }
        }
    };

    template<typename T>
    class Ref
    {
    private:
        T *ptr;
        Region *region;
        size_t expectedGeneration;
        std::atomic<int> *refCount;
        MemoryManager &manager;

        void incrementRefCount()
        {
            if (refCount) {
                refCount->fetch_add(1, std::memory_order_relaxed);
            }
        }

        void decrementRefCount()
        {
            if (refCount && refCount->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                delete refCount;
                if (ptr && isValid()) {
                    ptr->~T();
                    region->deallocate(ptr);
                }
                ptr = nullptr;
                region = nullptr;
                refCount = nullptr;
            }
        }

    public:
        Ref()
            : ptr(nullptr)
            , region(nullptr)
            , expectedGeneration(0)
            , refCount(nullptr)
            , manager(MemoryManager::getInstance())
        {}

        Ref(Region &r, T *p)
            : ptr(p)
            , region(&r)
            , expectedGeneration(r.getGeneration(p))
            , refCount(new std::atomic<int>(1))
            , manager(MemoryManager::getInstance())
        {
        }

        Ref(const Ref &other)
            : ptr(other.ptr)
            , region(other.region)
            , expectedGeneration(other.expectedGeneration)
            , refCount(other.refCount)
            , manager(other.manager)
        {
            incrementRefCount();
        }

        Ref &operator=(const Ref &other)
        {
            if (this != &other) {
                decrementRefCount();
                ptr = other.ptr;
                region = other.region;
                expectedGeneration = other.expectedGeneration;
                refCount = other.refCount;
                incrementRefCount();
            }
            return *this;
        }

        Ref(Ref &&other) noexcept
            : ptr(other.ptr)
            , region(other.region)
            , expectedGeneration(other.expectedGeneration)
            , refCount(other.refCount)
            , manager(other.manager)
        {
            other.ptr = nullptr;
            other.region = nullptr;
            other.refCount = nullptr;
        }

        Ref &operator=(Ref &&other) noexcept
        {
            if (this != &other) {
                decrementRefCount();
                ptr = other.ptr;
                region = other.region;
                expectedGeneration = other.expectedGeneration;
                refCount = other.refCount;
                other.ptr = nullptr;
                other.region = nullptr;
                other.refCount = nullptr;
            }
            return *this;
        }

        ~Ref() { decrementRefCount(); }

        T *operator->() const
        {
            if (!isValid()) {
                throw std::runtime_error("Accessing invalid generational reference");
            }
            return ptr;
        }

        T &operator*() const
        {
            if (!isValid()) {
                throw std::runtime_error("Accessing invalid generational reference");
            }
            return *ptr;
        }

        T *get() const { return ptr; }
        bool isValid() const
        {
            return ptr != nullptr && region->getGeneration(ptr) == expectedGeneration;
        }
        Region &getRegion() const { return *region; }
    };

    template<typename T, typename... Args>
    Linear<T> makeLinear(Region &region, Args &&...args)
    {
        T *obj = region.template create<T>(std::forward<Args>(args)...);
        return Linear<T>(region, obj, *this);
    }

    template<typename T, typename... Args>
    Ref<T> makeRef(Region &region, Args &&...args)
    {
        T *obj = region.template create<T>(std::forward<Args>(args)...);
        return Ref<T>(region, obj);
    }

    class Unsafe
    {
    public:
        static void *allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t))
        {
            std::cout << "Unsafe allocate: " << size << " bytes (alignment: " << alignment << ")\n";
            return DefaultAllocator().allocate(size, alignment);
        }

        static void deallocate(void *ptr) noexcept
        {
            std::cout << "Unsafe deallocate\n";
            DefaultAllocator().deallocate(ptr);
        }

        static void *resize(void *ptr,
                            std::size_t new_size,
                            std::size_t alignment = alignof(std::max_align_t))
        {
            std::cout << "Unsafe resize to " << new_size << " bytes (alignment: " << alignment
                      << ")\n";
            void *new_ptr = DefaultAllocator().allocate(new_size, alignment);
            if (ptr) {
                std::memcpy(new_ptr, ptr, new_size);
                DefaultAllocator().deallocate(ptr);
            }
            return new_ptr;
        }

        static void *allocateZeroed(std::size_t num, std::size_t size)
        {
            std::size_t total = num * size;
            void *ptr = allocate(total);
            if (ptr) {
                std::memset(ptr, 0, total);
            }
            return ptr;
        }

        static void copy(void *dest, const void *src, std::size_t num)
        {
            std::memcpy(dest, src, num);
        }

        static void set(void *ptr, int value, std::size_t num) { std::memset(ptr, value, num); }

        static int compare(const void *ptr1, const void *ptr2, std::size_t num)
        {
            return std::memcmp(ptr1, ptr2, num);
        }

        static void move(void *dest, const void *src, std::size_t num)
        {
            std::memmove(dest, src, num);
        }
    };

    // Singleton instance
    static MemoryManager &getInstance()
    {
        static MemoryManager instance;
        return instance;
    }
};
