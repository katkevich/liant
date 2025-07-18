#pragma once
#include "liant/export_macro.hpp"

#ifndef LIANT_MODULE
#include <memory>
#endif

// clang-format off
LIANT_EXPORT
// clang-format on
namespace liant {
namespace details {
enum class ContainerPtrKind;

template <ContainerPtrKind PtrKind, typename... TInterfaces>
class ContainerSliceImpl;
} // namespace details

class ContainerBase;

// fat 'Dependency' shared non-null pointer
// holds reference to the 'Dependency' from the 'Container' and shared pointer to the 'Container' itself
// the 'Container' won't be destroyed until all 'SharedRef's & 'SharedPtr's go out of scope
// use with caution coz you don't really want to block the deletion of the 'Container'
template <typename T>
class SharedRef {
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class Container;

    template <details::ContainerPtrKind PtrKindOther, typename... UInterfaces>
    friend class details::ContainerSliceImpl;

    template <typename U>
    class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    SharedRef(T& ref, std::shared_ptr<ContainerBase> owner)
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

    T& get() const {
        return *ptr;
    }

    T* operator->() const {
        return get();
    }

    T& operator*() const {
        return *get();
    }

private:
    friend bool operator==(const SharedRef& lhs, const SharedRef& rhs) {
        return lhs.ptr == rhs.ptr;
    }

    friend bool operator!=(const SharedRef& lhs, const SharedRef& rhs) {
        return !(lhs == rhs);
    }

private:
    T* ptr{};                               // never nullptr
    std::shared_ptr<ContainerBase> owner{}; // never nullptr
};


// fat 'Dependency' shared pointer
// holds pointer to the 'Dependency' from the 'Container' and shared pointer to the 'Container' itself
// the 'Container' won't be destroyed until all 'SharedRef's & 'SharedPtr's go out of scope
// use with caution coz you don't really want to block the deletion of the 'Container'
template <typename T>
class SharedPtr {
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class Container;

    template <details::ContainerPtrKind PtrKind, typename... TInterfaces>
    friend class details::ContainerSliceImpl;

    template <typename U>
    friend class WeakPtr;

    SharedPtr(T* ptr, std::shared_ptr<const ContainerBase> owner)
        : ptr(ptr)
        , owner(ptr ? std::move(owner) : nullptr) {}

public:
    SharedPtr(const SharedRef<T>& other)
        : ptr(other.ptr)
        , owner(other.owner) {}

    SharedPtr(SharedRef<T>&& other)
        : ptr(other.ptr)
        , owner(std::move(other.owner)) {}

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

    T* get() const {
        if (owner) {
            return ptr;
        } else {
            return nullptr;
        }
    }

    T* operator->() const {
        return get();
    }

    T& operator*() const {
        return *get();
    }

    explicit operator bool() const {
        return owner && ptr;
    }

    SharedRef<T> toSharedRef() const {
        return SharedRef<T>(*ptr, owner);
    }

private:
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

private:
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

template <typename T>
class NotNull;

template <typename T>
class NotNull<T*> {
public:
    NotNull(T& ref)
        : ptr(std::addressof(ref)) {}

    T* operator->() const {
        return ptr;
    }

    T& operator*() const {
        return *ptr;
    }

    operator T&() {
        return *ptr;
    }

    operator T*&() {
        return ptr;
    }

    operator T* const&() const {
        return ptr;
    }

private:
    T* ptr{};
};

template <typename T>
class NotNull<SharedPtr<T>> {
public:
    NotNull(const SharedRef<T>& ref)
        : ptr(ref) {}
    NotNull(SharedRef<T>&& ref)
        : ptr(std::move(ref)) {}

    T* operator->() const {
        return ptr.get();
    }

    T& operator*() const {
        return *ptr;
    }

    operator SharedRef<T>() {
        return ptr.toSharedRef();
    }

    operator SharedPtr<T>&() {
        return ptr;
    }

    operator SharedPtr<T> const&() const {
        return ptr;
    }


private:
    friend bool operator==(const NotNull& lhs, const NotNull& rhs) {
        return lhs.ptr == rhs.ptr;
    }

    friend bool operator!=(const NotNull& lhs, const NotNull& rhs) {
        return !(lhs == rhs);
    }

private:
    SharedPtr<T> ptr{};
};
} // namespace liant