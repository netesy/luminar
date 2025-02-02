#pragma once

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
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "memory_analyzer.hh"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define TRACE_INFO() (std::string(__FUNCTION__) + " at line " + TOSTRING(__LINE__))

// Size classes for small allocations (in bytes)
constexpr std::array<size_t, 8> SMALL_SIZES = {16, 32, 64, 128, 256, 512, 1024, 2048};
constexpr size_t MAX_SMALL_SIZE = SMALL_SIZES.back();
constexpr size_t BLOCK_SIZE = 64 * 1024; // 64KB blocks
constexpr size_t MAX_BLOCKS_PER_CHUNK = 64;

class DefaultAllocator
{
private:
    struct Block
    {
        uint8_t *memory;
        size_t size_class;
        size_t objects_per_block;
        std::bitset<MAX_BLOCKS_PER_CHUNK> free_list;
        size_t free_count;

        Block(size_t block_size, size_t obj_size)
            : memory(new uint8_t[block_size])
            , size_class(obj_size)
            , objects_per_block(block_size / obj_size)
            , free_count(objects_per_block)
        {
            free_list.set(); // Mark all slots as free
        }

        ~Block() { delete[] memory; }

        bool has_free() const { return free_count > 0; }

        void *allocate()
        {
            if (!has_free())
                return nullptr;

            // Find first free bit
            size_t index = 0;
            unsigned long long bits = free_list.to_ullong();
            if (bits != 0) {
                while ((bits & 1ULL) == 0) {
                    bits >>= 1;
                    ++index;
                }
            }

            // Mark slot as used
            free_list.reset(index);
            free_count--;

            // Calculate pointer to the allocated memory
            return memory + (index * size_class);
        }

        bool owns(void *ptr) const
        {
            return ptr >= memory && ptr < memory + (objects_per_block * size_class);
        }

        bool deallocate(void *ptr)
        {
            if (!owns(ptr))
                return false;

            size_t index = (static_cast<uint8_t *>(ptr) - memory) / size_class;
            free_list.set(index);
            free_count++;
            return true;
        }
    };

    struct ThreadCache
    {
        std::vector<Block *> small_blocks[SMALL_SIZES.size()];
        std::vector<std::pair<void *, size_t>> large_allocations;

        ~ThreadCache()
        {
            for (auto &blocks : small_blocks) {
                for (auto *block : blocks) {
                    delete block;
                }
            }
            for (auto &[ptr, size] : large_allocations) {
                delete[] static_cast<uint8_t *>(ptr);
            }
        }
    };

    // Thread-local storage for per-thread caches
    static thread_local ThreadCache thread_cache;

    // Find the appropriate size class for small allocations
    static size_t get_size_class(size_t size)
    {
        for (size_t i = 0; i < SMALL_SIZES.size(); i++) {
            if (size <= SMALL_SIZES[i])
                return i;
        }
        return static_cast<size_t>(-1);
    }

    // Get or create a block for the given size class
    static Block *get_block(size_t size_class_index)
    {
        auto &blocks = thread_cache.small_blocks[size_class_index];

        // Try existing blocks first
        for (auto *block : blocks) {
            if (block->has_free()) {
                return block;
            }
        }

        // Create new block if needed
        if (blocks.size() < MAX_BLOCKS_PER_CHUNK) {
            auto *new_block = new Block(BLOCK_SIZE, SMALL_SIZES[size_class_index]);
            blocks.push_back(new_block);
            return new_block;
        }

        return nullptr; // No space available
    }

public:
    void *allocate(size_t size, size_t alignment)
    {
        // Handle alignment requirements
        size = std::max(size, alignment);

        // Small allocation path
        if (size <= MAX_SMALL_SIZE) {
            size_t size_class = get_size_class(size);
            Block *block = get_block(size_class);

            if (block) {
                void *ptr = block->allocate();
                if (ptr)
                    return ptr;
            }
        }

        // Large allocation path
        size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
        void *ptr = new (std::nothrow) uint8_t[aligned_size];
        if (!ptr)
            throw std::bad_alloc();

        thread_cache.large_allocations.emplace_back(ptr, aligned_size);
        return ptr;
    }

    void deallocate(void *ptr) noexcept
    {
        if (!ptr)
            return;

        // Try small allocation blocks first
        for (size_t i = 0; i < SMALL_SIZES.size(); i++) {
            for (Block *block : thread_cache.small_blocks[i]) {
                if (block->deallocate(ptr)) {
                    return;
                }
            }
        }

        // Check large allocations
        auto &large_allocs = thread_cache.large_allocations;
        for (auto it = large_allocs.begin(); it != large_allocs.end(); ++it) {
            if (it->first == ptr) {
                delete[] static_cast<uint8_t *>(ptr);
                large_allocs.erase(it);
                return;
            }
        }
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

    std::atomic<size_t> activeRegionsCount{0}, activeReferencesCount{0}, activeLinearsCount{0};
    size_t totalAllocated = 0, peakMemoryUsage = 0, allocationCount = 0;
    size_t deallocationCount = 0, largestAllocation = 0;

    void log(const std::string &message) const
    {
        std::lock_guard lock(logMutex);
        if (logFile.is_open()) {
            logFile << "[" << getTimestamp() << "] " << message << std::endl;
            logFile.flush();
        }
    }

    std::string getTimestamp() const
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

public:
    MemoryManager(bool enableAudit = false)
        : auditMode(enableAudit)
    {
        logFile.open("memory.log", std::ios::app);
        if (!logFile)
            throw std::runtime_error("Failed to open memory.log");
        log("MemoryManager initialized");
    }

    ~MemoryManager()
    {
        analyzeMemoryUsage();
        log("MemoryManager destroyed");
        logFile.close();
    }

    void setAuditMode(bool enable)
    {
        auditMode = enable;
        log("Audit mode " + std::string(enable ? "enabled" : "disabled"));
    }

    void *allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        void *ptr = allocator.allocate(size, alignment);
        analyzer.recordAllocation(ptr, size, auditMode ? TRACE_INFO() : "");
        tracker.add(ptr, size, auditMode ? TRACE_INFO() : "", 1);
        totalAllocated += size;
        peakMemoryUsage = std::max(peakMemoryUsage, totalAllocated);
        allocationCount++;
        largestAllocation = std::max(largestAllocation, size);
        log("Allocated " + std::to_string(size) + " bytes at "
            + std::to_string(reinterpret_cast<uintptr_t>(ptr)));
        return ptr;
    }

    void deallocate(void *ptr)
    {
        analyzer.recordDeallocation(ptr);
        if (auto *info = tracker.get(ptr)) {
            totalAllocated -= info->size;
            deallocationCount++;
            allocator.deallocate(ptr);
            tracker.remove(ptr);
        }
    }

    void reportLeaks() const
    {
        log("Memory leaks detected:");
        for (const auto &[ptr, info] : this->tracker) {
            auto duration = std::chrono::steady_clock::now() - info->timestamp;
            log("- Leak: " + std::to_string(info->size) + " bytes at "
                + std::to_string(reinterpret_cast<uintptr_t>(ptr)) + ", allocated "
                + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(duration).count())
                + " seconds ago");
        }
    }

    void analyzeMemoryUsage() const {
        auto report = analyzer.generateReport();
        std::cout << "\nMemory Analysis Report:\n";
        std::cout << std::string(50, '=') << "\n";
        std::cout << "Overall Health Score: " << std::fixed << std::setprecision(1)
                  << report.overallHealth << "/100\n\n";

        if (!report.memoryLeaks.empty()) {
            std::cout << "Memory Leaks:\n";
            std::cout << std::string(20, '-') << "\n";
            for (const auto& leak : report.memoryLeaks) {
                std::cout << "- " << leak << "\n";
            }
            std::cout << "\n";
        }

        if (!report.fragmentationIssues.empty()) {
            std::cout << "Fragmentation Issues:\n";
            std::cout << std::string(20, '-') << "\n";
            for (const auto& issue : report.fragmentationIssues) {
                std::cout << "- " << issue << "\n";
            }
            std::cout << "\n";
        }

        if (!report.performanceWarnings.empty()) {
            std::cout << "Performance Warnings:\n";
            std::cout << std::string(20, '-') << "\n";
            for (const auto& warning : report.performanceWarnings) {
                std::cout << "- " << warning << "\n";
            }
            std::cout << "\n";
        }

        std::cout << "Detailed Memory Statistics:\n";
        std::cout << std::string(50, '=') << "\n\n";

        // Size Class Distribution
        std::cout << "Allocation Size Distribution:\n";
        std::cout << std::string(20, '-') << "\n";
        auto sizeDistribution = analyzer.getSizeDistribution();
        if (sizeDistribution.empty()) {
            std::cout << "No allocations recorded yet\n";
        } else {
            for (const auto& [sizeClass, count] : sizeDistribution) {
                std::cout << std::setw(8) << sizeClass << " bytes: "
                          << std::string(count / 10, '|') << " " << count << "\n";
            }
        }
        std::cout << "\n";

        // Temporal Analysis
        std::cout << "Temporal Analysis:\n";
        std::cout << std::string(20, '-') << "\n";
        auto temporalMetrics = analyzer.getTemporalMetrics();
        std::cout << "Peak Allocation Rate: " << std::fixed << std::setprecision(1)
                  << temporalMetrics.peakAllocationRate << " allocs/sec\n";
        std::cout << "Average Allocation Lifetime: " << std::fixed << std::setprecision(1)
                  << temporalMetrics.averageLifetime << " ms\n";

        if (!temporalMetrics.hotspots.empty()) {
            std::cout << "\nAllocation Hotspots:\n";
            for (const auto& hotspot : temporalMetrics.hotspots) {
                std::cout << "- " << hotspot << "\n";
            }
        }
        std::cout << "\n";

        // Thread Analysis
        std::cout << "Thread Analysis:\n";
        std::cout << std::string(20, '-') << "\n";
        auto threadMetrics = analyzer.getThreadMetrics();
        for (const auto& [threadId, metrics] : threadMetrics) {
            std::cout << "Thread " << threadId << ":\n";
            std::cout << "  Total Allocations: " << metrics.totalAllocations << "\n";
            std::cout << "  Active Allocations: " << metrics.activeAllocations << "\n";
            std::cout << "  Peak Memory Usage: " << metrics.peakMemoryUsage << " bytes\n\n";
        }

        // Alignment Analysis
        std::cout << "Alignment Analysis:\n";
        std::cout << std::string(20, '-') << "\n";
        auto alignmentMetrics = analyzer.getAlignmentMetrics();
        std::cout << "Suboptimal Alignments: " << alignmentMetrics.suboptimalCount << "\n";
        std::cout << "Average Padding Waste: " << std::fixed << std::setprecision(1)
                  << alignmentMetrics.averagePaddingWaste << " bytes\n\n";

        // Cache Analysis
        std::cout << "Cache Performance:\n";
        std::cout << std::string(20, '-') << "\n";
        auto cacheMetrics = analyzer.getCacheMetrics();
        double hitRate = 0.0;
        if (cacheMetrics.hits + cacheMetrics.misses > 0) {
            hitRate = (cacheMetrics.hits * 100.0) / (cacheMetrics.hits + cacheMetrics.misses);
        }
        std::cout << "Cache Hit Rate: " << std::fixed << std::setprecision(1) << hitRate << "%\n";
        std::cout << "Average Cache Access Time: " << std::fixed << std::setprecision(1)
                  << cacheMetrics.averageAccessTime << " ns\n\n";

        // Memory Access Patterns
        std::cout << "Memory Access Patterns:\n";
        std::cout << std::string(20, '-') << "\n";
        auto accessPatterns = analyzer.getAccessPatterns();
        if (accessPatterns.empty()) {
            std::cout << "No access patterns recorded yet\n";
        } else {
            for (const auto& pattern : accessPatterns) {
                std::cout << "- " << pattern.description << "\n";
            }
        }
        std::cout << "\n";

        // Recommendations
        std::cout << "Recommendations:\n";
        std::cout << std::string(20, '-') << "\n";
        auto recommendations = analyzer.getRecommendations();
        for (const auto& rec : recommendations) {
            std::stringstream ss;
            ss << "- " << rec;
            std::string recommendation = ss.str();
            // Ensure the line doesn't break in the middle of a word
            if (recommendation.find("\n") != std::string::npos) {
                recommendation.erase(std::remove(recommendation.begin(), recommendation.end(), '\n'),
                                     recommendation.end());
            }
            std::cout << recommendation << "\n";
        }
        std::cout << "\n";

        // Health Score Breakdown
        std::cout << "Health Score Breakdown:\n";
        std::cout << std::string(20, '-') << "\n";
        auto healthMetrics = analyzer.getHealthMetrics();
        std::cout << "Memory Fragmentation: " << std::fixed << std::setprecision(1)
                  << healthMetrics.fragmentationScore << "/100\n";
        std::cout << "Allocation Efficiency: " << std::fixed << std::setprecision(1)
                  << healthMetrics.efficiencyScore << "/100\n";
        std::cout << "Cache Utilization: " << std::fixed << std::setprecision(1)
                  << healthMetrics.cacheScore << "/100\n";
        std::cout << "Memory Safety: " << std::fixed << std::setprecision(1)
                  << healthMetrics.safetyScore << "/100\n";
        std::cout << std::string(20, '-') << "\n";
    }

    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
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
        analyzeMemoryUsage();
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
