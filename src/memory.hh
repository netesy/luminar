#pragma once
// memory.hh

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
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

        auto info = std::make_unique<AllocationInfo>(size, auditMode ? TRACE_INFO() : "");

        if (auditMode) {
            std::cout << "\n[AUDIT] Allocation: " << size << " bytes at " << ptr
                      << " (alignment: " << alignment << ") "
                      << " (" << getTimestamp() << ")\n";
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
        auto it = allocations.find(ptr);
        if (it != allocations.end()) {
            if (auditMode) {
                auto duration = std::chrono::steady_clock::now() - it->second->timestamp;
                std::cout << "\n\n[AUDIT] Deallocation: " << it->second->size << " bytes at " << ptr
                          << " (" << getTimestamp() << "), lived for "
                          << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
                          << "ms\n\n";
            }

            // Update statistics
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
    {}

    void setAuditMode(bool enable) { auditMode = enable; }

    static void logMemoryUsage(const std::string &msg)
    {
        std::cout << "[MemoryManager] " << msg << "\n";
    }

    void reportLeaks()
    {
        if (allocations.empty()) {
            std::cout << "No memory leaks detected.\n";
            return;
        }

        std::cout << "Memory leaks detected:\n";
        for (const auto &[ptr, info] : allocations) {
            auto duration = std::chrono::steady_clock::now() - info->timestamp;
            std::cout << "- Leak: " << info->size << " bytes at " << ptr << ", allocated "
                      << std::chrono::duration_cast<std::chrono::seconds>(duration).count()
                      << " seconds ago\n";
            if (auditMode && !info->stackTrace.empty()) {
                std::cout << "  Stack trace:\n" << info->stackTrace << "\n";
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

    void printStatistics() const
    {
        std::cout << "=======================================\n\n"
                  << "Memory Manager Statistics:\n"
                  << "  Current Total Allocated: " << totalAllocated << " bytes\n"
                  << "  Peak Memory Usage: " << peakMemoryUsage << " bytes\n"
                  << "  Number of Allocations: " << allocationCount << "\n"
                  << "  Number of Deallocations: " << deallocationCount << "\n"
                  << "  Largest Allocation: " << largestAllocation << " bytes\n"
                  << "  Average Allocation Size: " << getAverageAllocationSize() << " bytes\n"
                  << "=======================================\n\n";
    }

    ~MemoryManager()
    {
        reportLeaks();
        printStatistics();
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
            std::cout << "Region created.\n";
        }

        ~Region()
        {
            for (void *ptr : regionAllocations) {
                manager.deallocate(ptr);
            }
            std::cout << "Region destroyed. Memory deallocated.\n";
        }

        template<typename T, typename... Args>
        T *create(Args &&...args)
        {
            void *memory = manager.allocate(sizeof(T), alignof(T));
            T *obj = new (memory) T(std::forward<Args>(args)...);
            regionAllocations.push_back(memory);
            return obj;
        }
    };

    template<typename T>
    class Linear
    {
    private:
        T *ptr;
        Region *region;

    public:
        explicit Linear(Region &r, T *p)
            : ptr(p)
            , region(&r)
        {
            std::cout << "Linear object created.\n";
        }

        Linear(const Linear &) = delete;
        Linear &operator=(const Linear &) = delete;

        Linear(Linear &&other) noexcept
            : ptr(other.ptr)
            , region(other.region)
        {
            other.ptr = nullptr;
            other.region = nullptr;
        }

        Linear &operator=(Linear &&other) noexcept
        {
            if (this != &other) {
                ptr = other.ptr;
                region = other.region;
                other.ptr = nullptr;
                other.region = nullptr;
            }
            return *this;
        }

        ~Linear()
        {
            if (ptr) {
                std::cout << "Linear object destroyed.\n";
            }
        }

        T *operator->() const { return ptr; }
        T &operator*() const { return *ptr; }
        T *get() const { return ptr; }

        T *borrow() const
        {
            std::cout << "Borrowing Linear resource.\n";
            return ptr;
        }
        Region &getRegion() const { return *region; }
    };

    template<typename T>
    class Ref
    {
    private:
        T *ref;
        Region *region;

    public:
        Ref()
            : region(nullptr)
            , ref(nullptr)
        {
            std::cout << "Reference created.\n";
        }
        Ref(Region &r, T *p)
            : ref(p)
            , region(&r)
        {
            std::cout << "Reference created.\n";
        }

        T *operator->() const { return ref; }
        T &operator*() const { return *ref; }
        T *get() const { return ref; }
        Region &getRegion() const { return *region; }
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
                // Handle case where a shared_ptr<T> is passed
                const auto &sharedPtr = std::get<0>(std::forward_as_tuple(args...));
                T *obj = region.template create<T>(*sharedPtr); // Copy from shared_ptr
                return Linear<T>(region, obj);
            }
        }

        // Handle regular case
        return Linear<T>(region, region.template create<T>(std::forward<Args>(args)...));
    }

    template<typename T, typename... Args>
    Ref<T> makeRef(Region &region, Args &&...args)
    {
        if constexpr (sizeof...(Args) == 1) {
            using FirstArg = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
            if constexpr (std::is_same_v<FirstArg, std::shared_ptr<T>>) {
                // Handle case where a shared_ptr<T> is passed
                const auto &sharedPtr = std::get<0>(std::forward_as_tuple(args...));
                T *obj = region.template create<T>(*sharedPtr); // Copy from shared_ptr
                return Ref<T>(region, obj);
            }
        }

        // Handle regular case
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
