#pragma once
namespace liant::details {
enum class OwnershipKind { Shared, Weak, RawRef };
enum class ResolveMode { Ctor, Lazy };

struct SharedOwnership {
    static constexpr OwnershipKind Ownership = OwnershipKind::Shared;
    static constexpr ResolveMode Resolve = ResolveMode::Ctor;
};

struct NonOwningRef {
    static constexpr OwnershipKind Ownership = OwnershipKind::RawRef;
    static constexpr ResolveMode Resolve = ResolveMode::Ctor;
};

struct SharedOwnershipLazy {
    static constexpr OwnershipKind Ownership = OwnershipKind::Shared;
    static constexpr ResolveMode Resolve = ResolveMode::Lazy;
};

struct NonOwningRefLazy {
    static constexpr OwnershipKind Ownership = OwnershipKind::RawRef;
    static constexpr ResolveMode Resolve = ResolveMode::Lazy;
};

struct WeakOwnership {
    static constexpr OwnershipKind Ownership = OwnershipKind::Weak;
    static constexpr ResolveMode Resolve = ResolveMode::Ctor;
};

struct WeakOwnershipLazy {
    static constexpr OwnershipKind Ownership = OwnershipKind::Weak;
    static constexpr ResolveMode Resolve = ResolveMode::Lazy;
};

} // namespace liant::details