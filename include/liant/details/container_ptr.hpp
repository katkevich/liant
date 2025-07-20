#pragma once
#include "liant/container.hpp"
#include "liant/details/container_settings.hpp"

#ifndef LIANT_MODULE
#include <memory>
#endif

namespace liant::details {

template <OwnershipKind Kind>
struct ContainerPtr;

template <>
struct ContainerPtr<OwnershipKind::RawRef> {
    ContainerBase* inner{};

    ContainerPtr() = default;
    ContainerPtr(const ContainerPtr&) = default;
    ContainerPtr(ContainerPtr&&) = default;
    ContainerPtr& operator=(const ContainerPtr&) = default;
    ContainerPtr& operator=(ContainerPtr&&) = default;

    template <OwnershipKind OwnershipOther>
    ContainerPtr(const ContainerPtr<OwnershipOther>& containerPtr)
        : inner(containerPtr.asRaw()) {}

    ContainerPtr(std::shared_ptr<ContainerBase> container)
        : inner(container.get()) {}

    explicit operator bool() const {
        return inner != nullptr;
    }
    ContainerBase* asRaw() const {
        return inner;
    }
    std::shared_ptr<ContainerBase> asShared() const {
        return inner->shared_from_this();
    }
    std::weak_ptr<ContainerBase> asWeak() const {
        return inner->shared_from_this();
    }
};

template <>
struct ContainerPtr<OwnershipKind::Shared> {
    std::shared_ptr<ContainerBase> inner{};

    ContainerPtr() = default;
    ContainerPtr(const ContainerPtr&) = default;
    ContainerPtr(ContainerPtr&&) = default;
    ContainerPtr& operator=(const ContainerPtr&) = default;
    ContainerPtr& operator=(ContainerPtr&&) = default;

    template <OwnershipKind OwnershipOther>
    ContainerPtr(const ContainerPtr<OwnershipOther>& containerPtr)
        : inner(containerPtr.asShared()) {}

    ContainerPtr(std::shared_ptr<ContainerBase> container)
        : inner(std::move(container)) {}

    explicit operator bool() const {
        return inner.operator bool();
    }
    ContainerBase* asRaw() const {
        return inner.get();
    }
    const std::shared_ptr<ContainerBase>& asShared() const {
        return inner;
    }
    std::weak_ptr<ContainerBase> asWeak() const {
        return inner;
    }
};

template <>
struct ContainerPtr<OwnershipKind::Weak> {
    std::weak_ptr<ContainerBase> inner{};

    ContainerPtr() = default;
    ContainerPtr(const ContainerPtr&) = default;
    ContainerPtr(ContainerPtr&&) = default;
    ContainerPtr& operator=(const ContainerPtr&) = default;
    ContainerPtr& operator=(ContainerPtr&&) = default;

    template <OwnershipKind OwnershipOther>
    ContainerPtr(const ContainerPtr<OwnershipOther>& containerPtr)
        : inner(containerPtr.asWeak()) {}

    ContainerPtr(std::shared_ptr<ContainerBase> container)
        : inner(std::move(container)) {}

    explicit operator bool() const {
        return !inner.expired();
    }
    ContainerBase* asRaw() const {
        return inner.lock().get();
    }
    std::shared_ptr<ContainerBase> asShared() const {
        return inner.lock();
    }
    const std::weak_ptr<ContainerBase>& asWeak() const {
        return inner;
    }
};

} // namespace liant::details