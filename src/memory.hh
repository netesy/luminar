#pragma once
// memory.hh

#include <algorithm>
#include <atomic> // For atomic reference counting
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

// Compile-time stack trace macros
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CURRENT_FUNCTION __FUNCTION__
#define CURRENT_LINE __LINE__

#define TRACE_INFO() (std::string(CURRENT_FUNCTION) + " at line " + TOSTRING(CURRENT_LINE))

// Default allocator (unchanged)
class DefaultAllocator
{
public:
    void *allocate(size_t size, size_t alignment)
    {
        void *ptr = nullptr;

#ifdef _WIN32
        // Windows-specific alignment allocation
        ptr = _aligned_malloc(size, alignment);
        if (!ptr) {
            throw std::bad_alloc();
        }
#else
        // POSIX-specific alignment allocation
        if (posix_memalign(&ptr, alignment, size) != 0) {
            throw std::bad_alloc();
        }
#endif

        return ptr;
    }

    void deallocate(void *ptr) noexcept
    {
#ifdef _WIN32
        // Windows-specific deallocation
        _aligned_free(ptr);
#else
        // POSIX-specific deallocation
        free(ptr);
#endif
    }
};

template<typename Allocator = DefaultAllocator>
class MemoryManager
{
private:
    //std::ofstream logFile;
    mutable std::ofstream logFile;
    mutable std::mutex logMutex;

    void log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << "[" << getTimestamp() << "] " << message << std::endl;
            logFile.flush();
        }
    }
    struct AllocationInfo
    {
        size_t size;
        std::chrono::steady_clock::time_point timestamp;
        std::string stackTrace;

        AllocationInfo(size_t s, const std::string &st)
            : size(s)
            , timestamp(std::chrono::steady_clock::now())
            , stackTrace(st)
        {}
    };

    std::unordered_map<void *, std::unique_ptr<AllocationInfo>> allocations;
    bool auditMode;
    Allocator allocator;

    // Memory statistics
    size_t totalAllocated;
    size_t peakMemoryUsage;
    size_t allocationCount;
    size_t deallocationCount;
    size_t largestAllocation;

    std::atomic<size_t> activeRegionsCount{0};
    std::atomic<size_t> activeReferencesCount{0};
    std::atomic<size_t> activeLinearsCount{0};

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
        // std::cout << "Allocated " << size << " bytes at " << ptr << std::endl;
        log("Allocated " + std::to_string(size) + " bytes at "
            + std::to_string(reinterpret_cast<uintptr_t>(ptr)));

        auto info = std::make_unique<AllocationInfo>(size, auditMode ? TRACE_INFO() : "");

        if (auditMode) {
            log("[AUDIT] Allocation: " + std::to_string(size) + " bytes at "
                + std::to_string(reinterpret_cast<uintptr_t>(ptr))
                + " (alignment: " + std::to_string(alignment) + ") " + " (" + getTimestamp() + ")");
        }

        allocations[ptr] = std::move(info);

        // Update statistics
        totalAllocated += size;
        peakMemoryUsage = std::max(peakMemoryUsage, totalAllocated);
        allocationCount++;
        largestAllocation = std::max(largestAllocation, size);

        return ptr;
    }

    void deallocate(void *ptr)
    {
        std::cout << "Deallocating memory at " << ptr << std::endl;
        // log("Deallocating memory at " + std::to_string(ptr));
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

            // Update statistics
            totalAllocated -= it->second->size;
            deallocationCount++;

            allocator.deallocate(ptr);
            allocations.erase(it);
        }
    }

    // Non-const log function
    void logToFile()
    {
        std::ofstream logFile("memory_manager.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << "=======================================\n"
                    << "Memory Manager Statistics:\n"
                    << "---------------------------------------\n"
                    << "  Current Total Allocated: " << totalAllocated << " bytes\n"
                    << "  Peak Memory Usage: " << peakMemoryUsage << " bytes\n"
                    << "  Number of Allocations: " << allocationCount << "\n"
                    << "  Number of Deallocations: " << deallocationCount << "\n"
                    << "  Largest Allocation: " << largestAllocation << " bytes\n"
                    << "  Active Regions: " << getActiveRegionsCount() << "\n"
                    << "  Active References: " << getActiveReferencesCount() << "\n"
                    << "  Active Linears: " << getActiveLinearsCount() << "\n";

            if (allocationCount > 0) {
                logFile << "  Average Allocation Size: " << std::fixed << std::setprecision(2)
                        << static_cast<double>(totalAllocated) / allocationCount << " bytes\n";
            } else {
                logFile << "  Average Allocation Size: N/A (no allocations)\n";
            }

            logFile << "=======================================\n";
        }
    }

public:
    // New methods to get active counts
    size_t getActiveRegionsCount() const { return activeRegionsCount.load(); }
    size_t getActiveReferencesCount() const { return activeReferencesCount.load(); }
    size_t getActiveLinearsCount() const { return activeLinearsCount.load(); }

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

    void setAuditMode(bool enable)
    {
        auditMode = enable;
        log("Audit mode " + std::string(enable ? "enabled" : "disabled"));
    }

    static void logMemoryUsage(const std::string &msg)
    {
        std::ofstream logFile("memory.log", std::ios::app);
        logFile << "[MemoryManager] " << msg << std::endl;
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

    size_t getTotalAllocatedMemory() const { return totalAllocated; }

    size_t getPeakMemoryUsage() const { return peakMemoryUsage; }

    size_t getAllocationCount() const { return allocationCount; }

    size_t getDeallocationCount() const { return deallocationCount; }

    size_t getLargestAllocation() const { return largestAllocation; }

    double getAverageAllocationSize() const
    {
        return allocationCount > 0 ? static_cast<double>(totalAllocated) / allocationCount : 0.0;
    }

    void printStatistics()
    {
        std::stringstream ss;
        ss << "=======================================\n"
           << "Memory Manager Statistics:\n"
           << "---------------------------------------\n"
           << "  Current Total Allocated: " << totalAllocated << " bytes\n"
           << "  Peak Memory Usage: " << peakMemoryUsage << " bytes\n"
           << "  Number of Allocations: " << allocationCount << "\n"
           << "  Number of Deallocations: " << deallocationCount << "\n"
           << "  Largest Allocation: " << largestAllocation << " bytes\n"
           << "  Active Regions: " << getActiveRegionsCount() << "\n"
           << "  Active References: " << getActiveReferencesCount() << "\n"
           << "  Active Linears: " << getActiveLinearsCount() << "\n";

        if (allocationCount > 0) {
            ss << "  Average Allocation Size: " << std::fixed << std::setprecision(2)
               << static_cast<double>(totalAllocated) / allocationCount << " bytes\n";
        } else {
            ss << "  Average Allocation Size: N/A (no allocations)\n";
        }

        ss << "=======================================\n";

        std::string result = ss.str();
        log(result);
        std::cout << result;
        // Log the statistics to a file
        logToFile();
    }

    ~MemoryManager()
    {
        reportLeaks();
        log("MemoryManager destroyed");
        logFile.close();
        // printStatistics();
    }

    class Region
    {
    private:
        MemoryManager &manager;
        std::vector<void *> regionAllocations;

    public:
        explicit Region(MemoryManager &mgr)
            : manager(mgr)
        {
            manager.activeRegionsCount++;
            manager.log("Region created. Active Regions: "
                        + std::to_string(manager.getActiveRegionsCount()));
        }

        ~Region()
        {
            for (void *ptr : regionAllocations) {
                manager.deallocate(ptr);
            }
            manager.activeRegionsCount--;
            manager.log("Region destroyed. Active Regions: "
                        + std::to_string(manager.getActiveRegionsCount()));
        }

        template<typename T, typename... Args>
        T *create(Args &&...args)
        {
            void *memory = manager.allocate(sizeof(T), alignof(T));
            T *obj = new (memory) T(std::forward<Args>(args)...);
            regionAllocations.push_back(memory);
            return obj;
        }

        void deallocate(void *ptr)
        {
            auto it = std::find(regionAllocations.begin(), regionAllocations.end(), ptr);
            if (it != regionAllocations.end()) {
                manager.deallocate(ptr);
                regionAllocations.erase(it);
            }
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
                        + std::to_string(manager.getActiveLinearsCount()));
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
                ptr->~T(); // Call destructor
                region->deallocate(ptr);
                ptr = nullptr;
                ownsResource = false;
                manager.activeLinearsCount--;
                manager.log("Linear object destroyed. Active Linears: "
                            + std::to_string(manager.getActiveLinearsCount()));
            }
        }
    };

    template<typename T>
    class Ref
    {
    private:
        T *ref;
        Region *region;
        std::atomic<int> *refCount; // Atomic reference counter
        MemoryManager manager;

        void incrementRefCount()
        {
            if (refCount) {
                refCount->fetch_add(1, std::memory_order_relaxed);
                manager.activeReferencesCount++;
                manager.log("Reference count incremented. Active References: "
                            + std::to_string(manager.getActiveReferencesCount()));
            }
        }

        void decrementRefCount()
        {
            if (refCount && refCount->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                // std::cout << "Destroying Ref object at " << static_cast<void *>(ref) << std::endl;
                manager.log("Destroying Ref object");
                // manager.log("Destroying Ref object at " + std::to_string(*ref->data));
                delete refCount;
                if (ref) {
                    ref->~T(); // Call destructor
                    region->deallocate(ref);
                }
                ref = nullptr;
                region = nullptr;
                refCount = nullptr;
                manager.activeReferencesCount--;
                manager.log("Reference object destroyed. Active References: "
                            + std::to_string(manager.getActiveReferencesCount()));
            }
        }

    public:
        T *operator->() const { return ref; }
        T &operator*() const { return *ref; }
        T *get() const { return ref; }
        Region &getRegion() const { return *region; }

        Ref()
            : ref(nullptr)
            , region(nullptr)
            , refCount(nullptr)
        {}

        Ref(Region &r, T *p)
            : ref(p)
            , region(&r)
            , refCount(new std::atomic<int>(1))
        {
            manager.activeReferencesCount++;
            manager.log("Reference created. Active References: "
                        + std::to_string(manager.getActiveReferencesCount()));
        }

        Ref(const Ref &other)
            : ref(other.ref)
            , region(other.region)
            , refCount(other.refCount)
        {
            incrementRefCount();
        }

        Ref &operator=(const Ref &other)
        {
            if (this != &other) {
                decrementRefCount();
                ref = other.ref;
                region = other.region;
                refCount = other.refCount;
                incrementRefCount();
            }
            return *this;
        }

        Ref(Ref &&other) noexcept
            : ref(other.ref)
            , region(other.region)
            , refCount(other.refCount)
        {
            other.ref = nullptr;
            other.region = nullptr;
            other.refCount = nullptr;
        }

        Ref &operator=(Ref &&other) noexcept
        {
            if (this != &other) {
                decrementRefCount();
                ref = other.ref;
                region = other.region;
                refCount = other.refCount;
                other.ref = nullptr;
                other.region = nullptr;
                other.refCount = nullptr;
            }
            return *this;
        }

        ~Ref() { decrementRefCount(); }
    };

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

    template<typename T, typename... Args>
    Linear<T> makeLinear(Region &region, Args &&...args)
    {
        if constexpr (sizeof...(Args) == 1) {
            using FirstArg = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
            if constexpr (std::is_same_v<FirstArg, std::shared_ptr<T>>) {
                const auto &sharedPtr = std::get<0>(std::forward_as_tuple(args...));
                T *obj = region.template create<T>(*sharedPtr);
                return Linear<T>(region, obj, *this);
            }
        }
        return Linear<T>(region, region.template create<T>(std::forward<Args>(args)...), *this);
    }

    template<typename T, typename... Args>
    Ref<T> makeRef(Region &region, Args &&...args)
    {
        // Handle regular case
        if constexpr (sizeof...(Args) == 1) {
            using FirstArg = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
            if constexpr (std::is_same_v<FirstArg, std::shared_ptr<T>>) {
                auto &sharedPtr = std::get<0>(std::forward_as_tuple(args...));
                T *obj = region.template create<T>(*sharedPtr);
                return Ref<T>(region, obj);
            }
        }
        return Ref<T>(region, region.template create<T>(std::forward<Args>(args)...));
    }
};

// Example usage
//int main()
//{
//    // Enable audit mode to show memory allocations and deallocations
//    MemoryManager<> memoryManager(true);
//    MemoryManager<>::Region globalRegion(memoryManager); // Global region to handle memory

//    // Safe code demonstration with linear types and regions
//    {
//        std::cout << "\n--- Safe Code: Linear Types and Regions Demonstration ---\n";

//        // Create a linear object in the global region
//        auto linearInt = memoryManager.makeLinear<int>(globalRegion, 42);
//        std::cout << "Linear int value: " << *linearInt << "\n";

//        // Create a more complex linear object
//        struct ComplexObject
//        {
//            int x;
//            double y;
//            ComplexObject(int x, double y)
//                : x(x)
//                , y(y)
//            {}
//        };

//        auto linearComplex = memoryManager.makeLinear<ComplexObject>(globalRegion, 10, 3.14);
//        std::cout << "Linear complex object: x = " << linearComplex->x
//                  << ", y = " << linearComplex->y << "\n";

//        // Create a reference to an int in the global region
//        auto refInt = memoryManager.makeRef<int>(globalRegion, 100);
//        std::cout << "Reference int value: " << *refInt << "\n";

//        // Borrow the value from a linear object (safe borrowing)
//        int *borrowedInt = linearInt.borrow();
//        std::cout << "Borrowed int value: " << *borrowedInt << "\n";

//        // Demonstrate move semantics with linear types (safe move)
//        auto movedLinearInt = std::move(linearInt);
//        std::cout << "Moved linear int value: " << *movedLinearInt << "\n";

//        // Report total allocated memory in safe code
//        std::cout << "Total allocated memory: " << memoryManager.getTotalAllocatedMemory()
//                  << " bytes\n";

//        // Objects will be automatically deallocated when the region is destroyed
//    }

//    // Unsafe code demonstration
//    std::cout << "\n--- Unsafe Memory Operations Demonstration ---\n";

//    // 1. Allocate memory (unsafe)
//    std::cout << "Allocating 100 bytes using unsafe allocate...\n";
//    void *unsafeMemory = MemoryManager<>::Unsafe::allocate(100);

//    // 2. Set memory to a value (unsafe)
//    std::cout << "Setting all bytes to 'A'...\n";
//    MemoryManager<>::Unsafe::set(unsafeMemory, 'A', 100);

//    // 3. Copy memory (unsafe)
//    std::cout << "Allocating another 100 bytes...\n";
//    void *unsafeCopyMemory = MemoryManager<>::Unsafe::allocate(100);
//    std::cout << "Copying memory content to new block...\n";
//    MemoryManager<>::Unsafe::copy(unsafeCopyMemory, unsafeMemory, 100);

//    // 4. Compare memory (unsafe)
//    std::cout << "Comparing the two blocks...\n";
//    if (MemoryManager<>::Unsafe::compare(unsafeMemory, unsafeCopyMemory, 100) == 0) {
//        std::cout << "Memory blocks are identical.\n";
//    } else {
//        std::cout << "Memory blocks are different.\n";
//    }

//    // 5. Resize memory (unsafe, can lead to leaks)
//    std::cout << "Resizing the first block to 200 bytes (possible leak if not handled)...\n";
//    void *resizedMemory = MemoryManager<>::Unsafe::resize(unsafeMemory, 200);

//    // 6. Move memory (unsafe)
//    std::cout << "Moving content from the second block to a third block...\n";
//    void *unsafeMoveMemory = MemoryManager<>::Unsafe::allocate(100);
//    MemoryManager<>::Unsafe::move(unsafeMoveMemory, unsafeCopyMemory, 100);

//    // Incorrect usage leading to memory leak:
//    std::cout << "\n--- Incorrect Usage: Memory Leak ---\n";
//    std::cout << "Forgetting to deallocate the original second block...\n";
//    // Memory leak here, since the second block is not deallocated before overwriting.

//    // Correct usage: Deallocate all the blocks
//    std::cout << "\n--- Correct Usage: Cleaning Up ---\n";
//    MemoryManager<>::Unsafe::deallocate(resizedMemory);    // Properly deallocate resized memory
//    MemoryManager<>::Unsafe::deallocate(unsafeCopyMemory); // Deallocate copy memory
//    MemoryManager<>::Unsafe::deallocate(unsafeMoveMemory); // Deallocate moved memory

//    std::cout << "All memory blocks have been properly deallocated.\n";

//    // Unsafe memory management with linear types and references
//    std::cout << "\n--- Unsafe Memory Management with Linear Types and References ---\n";

//    // 1. Allocate memory using linear types
//    {
//        // Use a region to handle memory
//        MemoryManager<>::Region region(memoryManager);

//        // Create objects with potential circular references
//        struct Node
//        {
//            int value;
//            Node *next;
//            Node(int v)
//                : value(v)
//                , next(nullptr)
//            {}
//        };

//        // Create two nodes with a circular reference
//        auto node1 = memoryManager.makeLinear<Node>(region, 1);
//        auto node2 = memoryManager.makeLinear<Node>(region, 2);

//        // Circular reference assignment
//        node1->next = node2.get();
//        node2->next = node1.get();

//        std::cout << "Node1 value: " << node1->value << "\n";
//        std::cout << "Node2 value: " << node2->value << "\n";

//        // Explicitly handle circular references:
//        node1->next = nullptr; // Break the cycle
//        node2->next = nullptr;

//        // Clean up is handled automatically by the region
//    }

//    return 0;
//}
