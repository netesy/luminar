#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

class MemoryManager
{
public:
    // Linear-First Approach: Automatic Region Management
    class Region
    {
    public:
        Region() { std::cout << "Region created.\n"; }

        ~Region()
        {
            for (auto &ptr : allocations) {
                delete[] ptr;
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
            T *obj = new T(std::forward<Args>(args)...);
            allocations.push_back(static_cast<void *>(obj));
            return obj;
        }

    private:
        std::vector<void *> allocations;
    };

    // Linear Type with Automatic Region Management
    template<typename T>
    class Linear
    {
    public:
        explicit Linear(Region &region, T *ptr)
            : ptr(ptr)
            , region(&region)
        {
            std::cout << "Linear object created.\n";
        }

        // Deleting copy constructor and copy assignment to enforce single ownership
        Linear(const Linear &) = delete;
        Linear &operator=(const Linear &) = delete;

        // Allowing move semantics
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

        T *operator->() { return ptr; }
        T &operator*() { return *ptr; }
        T *get() const { return ptr; }

        // Convert to Ref type
        std::unique_ptr<MemoryManager::Ref<T>> toRef()
        {
            std::cout << "Converting Linear to Reference.\n";
            return std::make_unique<MemoryManager::Ref<T>>(*region, ptr);
        }

    private:
        T *ptr;
        Region *region; // Keep track of the region managing this linear type
    };

    // Reference-First Approach with Automatic Region Management
    template<typename T>
    class Ref
    {
    public:
        Ref(Region &region, T *ptr)
            : ref(ptr)
            , region(&region)
        {
            std::cout << "Reference created.\n";
        }

        T *operator->() { return ref; }
        T &operator*() { return *ref; }
        T *get() const { return ref; }

        // Convert to Linear type
        std::unique_ptr<MemoryManager::Linear<T>> toLinear()
        {
            std::cout << "Converting Reference to Linear.\n";
            T *tmp = ref;
            ref = nullptr;
            return std::make_unique<MemoryManager::Linear<T>>(*region, tmp);
        }

    private:
        T *ref;
        Region *region; // Keep track of the region managing this reference
    };

    // Unsafe Mode: Manual Memory Management
    class Unsafe
    {
    public:
        static void *allocate(std::size_t size)
        {
            std::cout << "Unsafe allocate: " << size << " bytes\n";
            return new char[size];
        }

        static void deallocate(void *ptr)
        {
            std::cout << "Unsafe deallocate\n";
            delete[] static_cast<char *>(ptr);
        }
    };
};

//// Example usage
//int main()
//{
//    // Linear-First Approach: Using Regions with Linear Types
//    {
//        std::cout << "\n--- Linear-First Approach ---\n";
//        MemoryManager::Region region;

//        // Create a Linear object
//        auto lin = std::make_unique<MemoryManager::Linear<int>>(region,
//                                                                region.createLinear<int>(42));
//        std::cout << "Linear value: " << *lin << "\n";

//        // Convert Linear to Ref
//        auto ref = lin->toRef();
//        std::cout << "Converted Reference value: " << *ref << "\n";

//        // Convert Ref back to Linear
//        auto linAgain = ref->toLinear();
//        std::cout << "Linear value after conversion: " << *linAgain << "\n";
//    }

//    // Reference-First Approach: Using Regions with References
//    {
//        std::cout << "\n--- Reference-First Approach ---\n";
//        MemoryManager::Region region;

//        // Create a Ref object
//        auto ref = std::make_unique<MemoryManager::Ref<int>>(region, region.createRef<int>(100));
//        std::cout << "Reference value: " << *ref << "\n";

//        // Convert Ref to Linear
//        auto lin = ref->toLinear();
//        std::cout << "Converted Linear value: " << *lin << "\n";

//        // Convert Linear back to Ref
//        auto refAgain = lin->toRef();
//        std::cout << "Reference value after conversion: " << *refAgain << "\n";
//    }

//    // Manual Memory Management: Unsafe Mode
//    {
//        std::cout << "\n--- Manual Memory Management (Unsafe Mode) ---\n";
//        void *buffer = MemoryManager::Unsafe::allocate(1024); // Allocate 1024 bytes
//        // Perform manual memory operations
//        // (In practice, you'd write data to this buffer and use it as needed)
//        MemoryManager::Unsafe::deallocate(buffer); // Deallocate memory when done
//    }

//    return 0;
//}
