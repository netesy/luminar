#pragma once

#include <atomic>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

class MemoryManager
{
private:
    std::vector<void *> allocations;

public:
    static void logMemoryUsage(const std::string &msg)
    {
        std::cout << "[MemoryManager] " << msg << "\n";
    }

    template<typename T>
    void trackAllocation(T *ptr)
    {
        logMemoryUsage("Allocated " + std::to_string(sizeof(T)) + " bytes.");
        allocations.push_back(ptr);
    }

    void reportLeaks()
    {
        for (void *ptr : allocations) {
            if (ptr) {
                std::cout << "Leaked memory at " << ptr << "\n";
            }
        }
    }

    ~MemoryManager() { reportLeaks(); }

    class Region
    {
    public:
        Region() { std::cout << "Region created.\n"; }

        ~Region()
        {
            for (auto &ptr : allocations) {
                delete[] static_cast<char *>(ptr);
            }
            std::cout << "Region destroyed. Memory deallocated.\n";
        }

        template<typename T, typename... Args>
        T *createLinear(Args &&...args)
        {
            T *obj = new T(std::forward<Args>(args)...);
            allocations.push_back(static_cast<void *>(obj));
            return obj;
        }

        template<typename T, typename... Args>
        T *createRef(Args &&...args)
        {
            return createLinear<T>(std::forward<Args>(args)...);
        }

    private:
        std::vector<void *> allocations;
    };

    template<typename T>
    class Ref;

    template<typename T>
    class Linear
    {
    public:
        explicit Linear(Region &region, T *ptr) noexcept
            : ptr(ptr)
            , region(&region)
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

        T *operator->() const noexcept { return ptr; }
        T &operator*() const noexcept { return *ptr; }
        T *get() const noexcept { return ptr; }

        Ref<T> toRef() const
        {
            std::cout << "Converting Linear to Reference.\n";
            return Ref<T>(*region, ptr);
        }

        void transferOwnership(Linear<T> &target)
        {
            if (this != &target) {
                target.ptr = this->ptr;
                target.region = this->region;
                this->ptr = nullptr;
                this->region = nullptr;
            }
        }

        T *borrow() const noexcept
        {
            std::cout << "Borrowing Linear resource.\n";
            return ptr;
        }

    private:
        T *ptr;
        Region *region;
    };

    template<typename T>
    class Ref
    {
    public:
        Ref(Region &region, T *ptr) noexcept
            : ref(ptr)
            , region(&region)
        {
            std::cout << "Reference created.\n";
        }

        T *operator->() const noexcept { return ref; }
        T &operator*() const noexcept { return *ref; }
        T *get() const noexcept { return ref; }

        Linear<T> toLinear()
        {
            std::cout << "Converting Reference to Linear.\n";
            T *tmp = ref;
            ref = nullptr;
            return Linear<T>(*region, tmp);
        }

        class WeakRef
        {
        public:
            WeakRef() = default;

            explicit WeakRef(std::atomic<T *> &ref_ptr) noexcept
                : weak_ref(&ref_ptr)
            {}

            T *lock() const noexcept
            {
                T *tmp = weak_ref->load();
                return tmp;
            }

        private:
            std::atomic<T *> *weak_ref{nullptr};
        };

        WeakRef toWeakRef() const noexcept { return WeakRef(weak_ref); }

    private:
        T *ref;
        Region *region;
        mutable std::atomic<T *> weak_ref{nullptr};
        mutable std::mutex ref_mutex;

        void makeThreadSafe() { std::lock_guard<std::mutex> lock(ref_mutex); }
    };

    class Unsafe
    {
    public:
        static void *allocate(std::size_t size)
        {
            std::cout << "Unsafe allocate: " << size << " bytes\n";
            return new char[size];
        }

        static void deallocate(void *ptr) noexcept
        {
            std::cout << "Unsafe deallocate\n";
            delete[] static_cast<char *>(ptr);
        }

        static void *resize(void *ptr, std::size_t new_size)
        {
            std::cout << "Unsafe resize to " << new_size << " bytes\n";
            void *new_ptr = ::operator new(new_size);
            if (ptr) {
                std::memcpy(new_ptr, ptr, new_size); // Note: Need to copy the old data here
                ::operator delete(ptr);
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
};

// Example usage
int main()
{
    // Linear-First Approach: Using Regions with Linear Types
    {
        std::cout << "\n--- Linear-First Approach ---\n";
        MemoryManager::Region region;

        // Create a Linear object
        MemoryManager::Linear<int> lin(region, region.createLinear<int>(42));
        std::cout << "Linear value: " << *lin << "\n";

        // Convert Linear to Ref
        auto ref = lin.toRef();
        std::cout << "Converted Reference value: " << *ref << "\n";

        // Borrow the resource
        int *borrowed = lin.borrow();
        std::cout << "Borrowed Linear value: " << *borrowed << "\n";

        // Convert Ref back to Linear
        auto linAgain = ref.toLinear();
        std::cout << "Linear value after conversion: " << *linAgain << "\n";
    }

    // Reference-First Approach: Using Regions with References
    {
        std::cout << "\n--- Reference-First Approach ---\n";
        MemoryManager::Region region;

        // Create a Ref object
        MemoryManager::Ref<int> ref(region, region.createRef<int>(100));
        std::cout << "Reference value: " << *ref << "\n";

        // Convert Ref to Linear
        auto lin = ref.toLinear();
        std::cout << "Converted Linear value: " << *lin << "\n";

        // Convert Linear back to Ref
        auto refAgain = lin.toRef();
        std::cout << "Reference value after conversion: " << *refAgain << "\n";

        // Create a weak reference
        auto weakRef = refAgain.toWeakRef();
        if (auto lockedRef = weakRef.lock()) {
            std::cout << "Weak reference locked value: " << *lockedRef << "\n";
        } else {
            std::cout << "Weak reference is expired.\n";
        }
    }

    // Manual Memory Management: Unsafe Mode
    {
        std::cout << "\n--- Manual Memory Management (Unsafe Mode) ---\n";
        // Allocate memory
        int *buffer = static_cast<int *>(MemoryManager::Unsafe::allocate(5 * sizeof(int)));

        // Initialize memory
        for (int i = 0; i < 5; ++i) {
            buffer[i] = i * 10;
        }

        // Print initial values
        std::cout << "Initial values: ";
        for (int i = 0; i < 5; ++i) {
            std::cout << buffer[i] << " ";
        }
        std::cout << "\n";

        // Resize to a larger size
        int *new_buffer = static_cast<int *>(
            MemoryManager::Unsafe::resize(buffer, 10 * sizeof(int)));
        MemoryManager::Unsafe::copy(new_buffer, buffer, 5 * sizeof(int));
        MemoryManager::Unsafe::deallocate(buffer);
        buffer = new_buffer;

        // Initialize new elements
        for (int i = 5; i < 10; ++i) {
            buffer[i] = i * 10;
        }

        // Print new values
        std::cout << "After resize: ";
        for (int i = 0; i < 10; ++i) {
            std::cout << buffer[i] << " ";
        }
        std::cout << "\n";

        // Use set to set all values to 0
        MemoryManager::Unsafe::set(buffer, 0, 10 * sizeof(int));

        // Print after set
        std::cout << "After set: ";
        for (int i = 0; i < 10; ++i) {
            std::cout << buffer[i] << " ";
        }
        std::cout << "\n";

        // Allocate and initialize new memory using allocateZeroed
        int *zeroed = static_cast<int *>(MemoryManager::Unsafe::allocateZeroed(5, sizeof(int)));

        // Print zeroed memory
        std::cout << "Zeroed memory: ";
        for (int i = 0; i < 5; ++i) {
            std::cout << zeroed[i] << " ";
        }
        std::cout << "\n";

        // Compare memory
        MemoryManager::Unsafe::copy(zeroed, buffer, 5 * sizeof(int));
        int cmp_result = MemoryManager::Unsafe::compare(buffer, zeroed, 5 * sizeof(int));
        std::cout << "Compare result: " << cmp_result << "\n";

        // Cleanup
        MemoryManager::Unsafe::deallocate(buffer);
        MemoryManager::Unsafe::deallocate(zeroed);
    }

    return 0;
}
