#pragma once
#include "liant/dependency.hpp"

namespace liant::test {

template <auto IdV>
struct Trivial {
    decltype(IdV) Id = IdV;
};

template <auto IdV>
struct NonTrivial {
    NonTrivial(decltype(IdV) Id)
        : Id(Id) {}
    decltype(IdV) Id;
};

template <auto IdV>
struct Interface {
    virtual ~Interface() = default;
    virtual const decltype(IdV)& getId() const = 0;
};

template <auto IdV>
struct TrivialDerived : Interface<IdV> {
    decltype(IdV) Id = IdV;

    virtual const decltype(IdV)& getId() const override {
        return Id;
    }
};

template <auto IdV>
struct NonTrivialDerived : Interface<IdV> {
    decltype(IdV) Id;

    NonTrivialDerived(decltype(IdV) Id)
        : Id(Id) {}

    virtual const decltype(IdV)& getId() const override {
        return Id;
    }
};

template <auto IdV, typename... TInterfaces>
struct TrivialDerivedMultipleInterfaces : TInterfaces... {
    decltype(IdV) Id = IdV;

    virtual const decltype(IdV)& getId() const override {
        return Id;
    }
};

template <auto IdV, typename... TInterfaces>
struct NonTrivialDerivedMultipleInterfaces : TInterfaces... {
    decltype(IdV) Id;

    NonTrivialDerivedMultipleInterfaces(decltype(IdV) Id)
        : Id(Id) {}

    virtual const decltype(IdV)& getId() const override {
        return Id;
    }
};

struct Stats {
    std::string creationOrder;
    std::string destroyingOrder;
};

template <auto IdV>
struct Trackable {
    void postCreate() {
        stats.creationOrder += "Trackable" + std::to_string(IdV) + " ";
    }
    void preDestroy() {
        stats.destroyingOrder += "Trackable" + std::to_string(IdV) + " ";
    }
    Stats& stats;
};


namespace linked {
struct TrivialType1 {};
struct Interface1 {};
struct DerivedType1 : Interface1 {
    DerivedType1(liant::Dependencies<TrivialType1> di)
        : di(di) {}
    liant::Dependencies<TrivialType1> di;
};

struct TrivialType2 {};
struct Interface2 {};
struct DerivedType2 : Interface2 {
    DerivedType2(liant::Dependencies<Interface1, TrivialType2> di)
        : di(di) {}
    liant::Dependencies<Interface1, TrivialType2> di;
};

struct TrivialType34 {};
struct Interface3 {};
struct Interface4 {};
struct DerivedType34 : Interface3, Interface4 {
    DerivedType34(liant::Dependencies<Interface1, Interface2, TrivialType34> di)
        : di(di) {}
    liant::Dependencies<Interface1, Interface2, TrivialType34> di;
};
} // namespace linked

} // namespace liant::test