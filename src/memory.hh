#pragma once

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

#ifdef _WIN32
#include <dbghelp.h>
#include <windows.h>
#pragma comment(lib, "dbghelp.lib")
#else
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#endif

// Default allocator
class DefaultAllocator
{
public:
    void *allocate(size_t size, size_t alignment)
    {
        void *ptr = nullptr;
        if (posix_memalign(&ptr, alignment, size) != 0) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    void deallocate(void *ptr) noexcept { free(ptr); }
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

    std::string captureStackTrace()
    {
#ifdef _WIN32
        const int max_frames = 32;
        void *callstack[max_frames];
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        WORD frames = CaptureStackBackTrace(0, max_frames, callstack, NULL);

        SYMBOL_INFO *symbol = (SYMBOL_INFO *) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        std::ostringstream trace_stream;
        for (int i = 0; i < frames; i++) {
            SymFromAddr(process, (DWORD64) (callstack[i]), 0, symbol);
            trace_stream << i << ": " << symbol->Name << " - 0x" << symbol->Address << "\n";
        }
        free(symbol);
        return trace_stream.str();
#else
        const int max_frames = 32;
        void *callstack[max_frames];
        int frames = backtrace(callstack, max_frames);
        char **strs = backtrace_symbols(callstack, frames);

        std::ostringstream trace_stream;
        for (int i = 0; i < frames; ++i) {
            Dl_info info;
            if (dladdr(callstack[i], &info)) {
                char *demangled = nullptr;
                int status;
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
                trace_stream << i << ": " << (demangled ? demangled : info.dli_sname) << " + "
                             << (char *) callstack[i] - (char *) info.dli_saddr << "\n";
                free(demangled);
            } else {
                trace_stream << i << ": " << strs[i] << "\n";
            }
        }
        free(strs);
        return trace_stream.str();
#endif
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

        auto info = std::make_unique<AllocationInfo>(size, auditMode ? captureStackTrace() : "");

        if (auditMode) {
            std::cout << "[AUDIT] Allocation: " << size << " bytes at " << ptr
                      << " (alignment: " << alignment << ") "
                      << " (" << getTimestamp() << ")\n";
        }

        allocations[ptr] = std::move(info);
        return ptr;
    }

    void deallocate(void *ptr)
    {
        auto it = allocations.find(ptr);
        if (it != allocations.end()) {
            if (auditMode) {
                auto duration = std::chrono::steady_clock::now() - it->second->timestamp;
                std::cout << "[AUDIT] Deallocation: " << it->second->size << " bytes at " << ptr
                          << " (" << getTimestamp() << "), lived for "
                          << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
                          << "ms\n";
            }
            allocator.deallocate(ptr);
            allocations.erase(it);
        }
    }

public:
    MemoryManager(bool enableAuditMode = false, const Allocator &alloc = Allocator())
        : auditMode(enableAuditMode)
        , allocator(alloc)
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

    size_t getTotalAllocatedMemory() const
    {
        size_t total = 0;
        for (const auto &[ptr, info] : allocations) {
            total += info->size;
        }
        return total;
    }

    ~MemoryManager() { reportLeaks(); }

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
    };

    template<typename T>
    class Ref
    {
    private:
        T *ref;
        Region *region;

    public:
        Ref(Region &r, T *p)
            : ref(p)
            , region(&r)
        {
            std::cout << "Reference created.\n";
        }

        T *operator->() const { return ref; }
        T &operator*() const { return *ref; }
        T *get() const { return ref; }
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
        return Linear<T>(region, region.create<T>(std::forward<Args>(args)...));
    }

    template<typename T, typename... Args>
    Ref<T> makeRef(Region &region, Args &&...args)
    {
        return Ref<T>(region, region.create<T>(std::forward<Args>(args)...));
    }
};

// Example usage
int main()
{
    MemoryManager<> memoryManager(true); // Enable audit mode with default allocator
    MemoryManager<>::Region globalRegion(memoryManager);

    auto linearInt = memoryManager.makeLinear<int>(globalRegion, 42);
    std::cout << "Linear int value: " << *linearInt << "\n";

    struct ComplexObject
    {
        int x;
        double y;
        ComplexObject(int x, double y)
            : x(x)
            , y(y)
        {}
    };

    auto linearComplex = memoryManager.makeLinear<ComplexObject>(globalRegion, 10, 3.14);
    std::cout << "Linear complex object: x = " << linearComplex->x << ", y = " << linearComplex->y
              << "\n";

    // Using Unsafe operations
    void *rawMemory = MemoryManager<>::Unsafe::allocate(1024,
                                                        64); // 1024 bytes aligned to 64-byte boundary
    MemoryManager<>::Unsafe::deallocate(rawMemory);

    std::cout << "\nLeaving scope, all memory should be deallocated.\n";

    return 0;
}
