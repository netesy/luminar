#pragma once
// memory.hh

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CURRENT_FUNCTION __FUNCTION__
#define CURRENT_LINE __LINE__
#define TRACE_INFO() (std::string(CURRENT_FUNCTION) + " at line " + TOSTRING(CURRENT_LINE))

class DefaultAllocator
{
public:
    void *allocate(size_t size, size_t alignment)
    {
        void *ptr = nullptr;
#ifdef _WIN32
        ptr = _aligned_malloc(size, alignment);
        if (!ptr)
            throw std::bad_alloc();
#else
        if (posix_memalign(&ptr, alignment, size) != 0)
            throw std::bad_alloc();
#endif
        return ptr;
    }

    void deallocate(void *ptr) noexcept
    {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
};

template<typename Allocator = DefaultAllocator>
class MemoryManager
{
private:
    mutable std::ofstream logFile;
    mutable std::mutex logMutex;
    bool auditMode;
    Allocator allocator;

    struct AllocationInfo
    {
        size_t size;
        std::chrono::steady_clock::time_point timestamp;
        std::string stackTrace;
        size_t generation;

        AllocationInfo(size_t s, const std::string &st, size_t gen)
            : size(s)
            , timestamp(std::chrono::steady_clock::now())
            , stackTrace(st)
            , generation(gen)
        {}
    };

    std::unordered_map<void *, std::unique_ptr<AllocationInfo>> allocations;
    size_t totalAllocated;
    size_t peakMemoryUsage;
    size_t allocationCount;
    size_t deallocationCount;
    size_t largestAllocation;
    std::atomic<size_t> activeRegionsCount{0};
    std::atomic<size_t> activeReferencesCount{0};
    std::atomic<size_t> activeLinearsCount{0};

    void log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << "[" << getTimestamp() << "] " << message << std::endl;
            logFile.flush();
        }
        //std::cout << "[" << getTimestamp() << "] " << message << std::endl;
    }

    std::string getTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    void *allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        void *ptr = allocator.allocate(size, alignment);
        log("Allocated " + std::to_string(size) + " bytes at "
            + std::to_string(reinterpret_cast<uintptr_t>(ptr)));

        size_t generation = 1; // Initial generation
        auto info = std::make_unique<AllocationInfo>(size,
                                                     auditMode ? TRACE_INFO() : "",
                                                     generation);

        if (auditMode) {
            log("[AUDIT] Allocation: " + std::to_string(size) + " bytes at "
                + std::to_string(reinterpret_cast<uintptr_t>(ptr))
                + " (alignment: " + std::to_string(alignment) + ") " + " (" + getTimestamp() + ")");
        }

        allocations[ptr] = std::move(info);

        totalAllocated += size;
        peakMemoryUsage = std::max(peakMemoryUsage, totalAllocated);
        allocationCount++;
        largestAllocation = std::max(largestAllocation, size);

        return ptr;
    }

    void deallocate(void *ptr)
    {
        auto it = allocations.find(ptr);
        if (it != allocations.end()) {
            if (auditMode) {
                auto duration = std::chrono::steady_clock::now() - it->second->timestamp;
                log("[AUDIT] Deallocation: " + std::to_string(it->second->size) + " bytes at "
                    + std::to_string(reinterpret_cast<uintptr_t>(ptr)) + " (" + getTimestamp()
                    + "), lived for "
                    + std::to_string(
                        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count())
                    + "ms");
            }

            totalAllocated -= it->second->size;
            deallocationCount++;

            allocator.deallocate(ptr);
            allocations.erase(it);
        }
    }

public:
    MemoryManager(bool enableAuditMode = false, const Allocator &alloc = Allocator())
        : auditMode(enableAuditMode)
        , allocator(alloc)
        , totalAllocated(0)
        , peakMemoryUsage(0)
        , allocationCount(0)
        , deallocationCount(0)
        , largestAllocation(0)
    {
        logFile.open("memory.log", std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open memory.log file");
        }
        log("MemoryManager initialized");
    }

    ~MemoryManager()
    {
        reportLeaks();
        log("MemoryManager destroyed");
        logFile.close();
    }

    void setAuditMode(bool enable)
    {
        auditMode = enable;
        log("Audit mode " + std::string(enable ? "enabled" : "disabled"));
    }

    void reportLeaks()
    {
        if (allocations.empty()) {
            log("No memory leaks detected.");
            return;
        }

        log("Memory leaks detected:\n");
        for (const auto &[ptr, info] : allocations) {
            auto duration = std::chrono::steady_clock::now() - info->timestamp;
            log("- Leak: " + std::to_string(info->size) + " bytes at "
                + std::to_string(reinterpret_cast<uintptr_t>(ptr)) + ", allocated "
                + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(duration).count())
                + " seconds ago");
            if (auditMode && !info->stackTrace.empty()) {
                log("  Stack trace:\n" + info->stackTrace);
            }
        }
    }

    void printStatistics() const
    {
        std::stringstream ss;
        ss << "Memory Manager Statistics:\n"
           << "  Current Total Allocated: " << totalAllocated << " bytes\n"
           << "  Peak Memory Usage: " << peakMemoryUsage << " bytes\n"
           << "  Number of Allocations: " << allocationCount << "\n"
           << "  Number of Deallocations: " << deallocationCount << "\n"
           << "  Largest Allocation: " << largestAllocation << " bytes\n"
           << "  Active Regions: " << activeRegionsCount << "\n"
           << "  Active References: " << activeReferencesCount << "\n"
           << "  Active Linears: " << activeLinearsCount << "\n";

        if (allocationCount > 0) {
            ss << "  Average Allocation Size: " << std::fixed << std::setprecision(2)
               << static_cast<double>(totalAllocated) / allocationCount << " bytes\n";
        } else {
            ss << "  Average Allocation Size: N/A (no allocations)\n";
        }

        //log(ss.str());
        std::cout << ss.str();
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
            manager.activeRegionsCount++;
            manager.log("Region created. Active Regions: "
                        + std::to_string(manager.activeRegionsCount));
        }

        ~Region()
        {
            for (const auto &[ptr, gen] : objectGenerations) {
                manager.deallocate(ptr);
            }
            manager.activeRegionsCount--;
            manager.log("Region destroyed. Active Regions: "
                        + std::to_string(manager.activeRegionsCount));
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
            manager.activeLinearsCount++;
            manager.log("Linear object created. Active Linears: "
                        + std::to_string(manager.activeLinearsCount));
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
            manager.log("Borrowing Linear resource.");
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
                manager.activeLinearsCount--;
                manager.log("Linear object destroyed. Active Linears: "
                            + std::to_string(manager.activeLinearsCount));
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
                manager.activeReferencesCount++;
                manager.log("Reference count incremented. Active References: "
                            + std::to_string(manager.activeReferencesCount));
            }
        }

        void decrementRefCount()
        {
            if (refCount && refCount->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                manager.log("Destroying Ref object");
                delete refCount;
                if (ptr && isValid()) {
                    ptr->~T();
                    region->deallocate(ptr);
                }
                ptr = nullptr;
                region = nullptr;
                refCount = nullptr;
                manager.activeReferencesCount--;
                manager.log("Ref object destroyed. Active References: "
                            + std::to_string(manager.activeReferencesCount));
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
            manager.activeReferencesCount++;
            manager.log("Ref created. Active References: "
                        + std::to_string(manager.activeReferencesCount));
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
