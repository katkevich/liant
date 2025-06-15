#pragma once
#include <memory>

namespace liant {

class ContainerBase;

// fat 'Dependency' shared non-null pointer
// holds reference to the 'Dependency' from the 'Container' and shared pointer to the 'Container' itself
// the 'Container' won't be destroyed until all 'SharedRef's & 'SharedPtr's go out of scope
// use with caution coz you don't really want to block the deletion of the 'Container'
template <typename T>
class SharedRef {
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class Container;

    template <typename U>
    friend class WeakPtr;

    SharedRef(T& ref, std::shared_ptr<const ContainerBase> owner)
        : ptr(std::addressof(ref))
        , owner(std::move(owner)) {}

public:
    SharedRef(const SharedRef& other)
        : ptr(other.ptr)
        , owner(other.owner) {}

    SharedRef(SharedRef&& other)
        : ptr(other.ptr)
        , owner(std::move(other.owner)) {}

    SharedRef& operator=(SharedRef other) {
        swap(*this, other);
        return *this;
    }

    friend void swap(SharedRef& first, SharedRef& second) {
        using std::swap;
        swap(first.ptr, second.ptr);
        swap(first.owner, second.owner);
    }

    T& get() {
        return *ptr;
    }

    const T& get() const {
        return const_cast<SharedRef*>(this)->get();
    }

    T* operator->() {
        return get();
    }

    const T* operator->() const {
        return const_cast<SharedRef*>(this)->get();
    }

    friend bool operator==(const SharedRef& lhs, const SharedRef& rhs) {
        return lhs.ptr == rhs.ptr;
    }

    friend bool operator!=(const SharedRef& lhs, const SharedRef& rhs) {
        return !(lhs == rhs);
    }

private:
    T* ptr{};                                     // never nullptr
    std::shared_ptr<const ContainerBase> owner{}; // never nullptr
};


// fat 'Dependency' shared pointer
// holds pointer to the 'Dependency' from the 'Container' and shared pointer to the 'Container' itself
// the 'Container' won't be destroyed until all 'SharedRef's & 'SharedPtr's go out of scope
// use with caution coz you don't really want to block the deletion of the 'Container'
template <typename T>
class SharedPtr {
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class Container;

    template <typename U>
    friend class WeakPtr;

    SharedPtr(T* ptr, std::shared_ptr<const ContainerBase> owner)
        : ptr(ptr)
        , owner(ptr ? std::move(owner) : nullptr) {}

public:
    SharedPtr(const SharedPtr& other)
        : ptr(other.ptr)
        , owner(other.owner) {}

    SharedPtr(SharedPtr&& other)
        : ptr(other.ptr)
        , owner(std::move(other.owner)) {}

    SharedPtr& operator=(SharedPtr other) {
        swap(*this, other);
        return *this;
    }

    friend void swap(SharedPtr& first, SharedPtr& second) {
        using std::swap;
        swap(first.ptr, second.ptr);
        swap(first.owner, second.owner);
    }

    void reset() {
        owner.reset();
    }

    T* get() {
        if (owner != nullptr) {
            return ptr;
        } else {
            return nullptr;
        }
    }

    const T* get() const {
        return const_cast<SharedPtr*>(this)->get();
    }

    T* operator->() {
        return get();
    }

    const T* operator->() const {
        return const_cast<SharedPtr*>(this)->get();
    }

    explicit operator bool() const {
        return owner != nullptr && ptr != nullptr;
    }

    friend bool operator==(const SharedPtr& lhs, const SharedPtr& rhs) {
        return lhs.ptr == rhs.ptr;
    }

    friend bool operator!=(const SharedPtr& lhs, const SharedPtr& rhs) {
        return !(lhs == rhs);
    }

private:
    T* ptr{};
    std::shared_ptr<const ContainerBase> owner{};
};

// fat 'Dependency' weak pointer
// holds pointer to the 'Dependency' from the 'Container' and weak pointer to the 'Container' itself
// become empty after the 'Container' goes out of scope
template <typename T>
class WeakPtr {
    WeakPtr(T* ptr, std::shared_ptr<const ContainerBase> owner)
        : ptr(ptr)
        , owner(ptr ? std::move(owner) : nullptr) {}

public:
    WeakPtr(SharedPtr<T> sharedPtr)
        : ptr(sharedPtr.ptr)
        , owner(sharedPtr.owner) {}

    WeakPtr(SharedRef<T> sharedRef)
        : ptr(sharedRef.ptr)
        , owner(sharedRef.owner) {}

    WeakPtr(const WeakPtr& other)
        : ptr(other.ptr)
        , owner(other.owner) {}

    WeakPtr(WeakPtr&& other)
        : ptr(other.ptr)
        , owner(std::move(other.owner)) {}

    WeakPtr& operator=(WeakPtr other) {
        swap(*this, other);
        return *this;
    }

    friend void swap(WeakPtr& first, WeakPtr& second) {
        using std::swap;
        swap(first.ptr, second.ptr);
        swap(first.owner, second.owner);
    }

    SharedPtr<T> lock() {
        return SharedPtr<T>(ptr, owner.lock());
    }

    void reset() {
        owner.reset();
    }

    explicit operator bool() const {
        return owner.lock() != nullptr && ptr != nullptr;
    }

    friend bool operator==(const WeakPtr& lhs, const WeakPtr& rhs) {
        return lhs.ptr == rhs.ptr;
    }

    friend bool operator!=(const WeakPtr& lhs, const WeakPtr& rhs) {
        return !(lhs == rhs);
    }

private:
    T* ptr{};
    std::weak_ptr<const ContainerBase> owner{};
};

} // namespace liant