#pragma once
#include "liant/liant.hpp"
#include "liant/ptr.hpp"

namespace liant {
inline namespace snake_case {

using container_base = ContainerBase;

template <typename TBaseContainer, typename... TTypeMappings>
using container = Container<TBaseContainer, TTypeMappings...>;

using empty_container = EmptyContainer;

template <typename TTypeMapping>
using registered_item = RegisteredItem<TTypeMapping>;

template <typename... TTypes>
using dependencies = Dependencies<TTypes...>;

template <typename TType>
using dependency = Dependency<TType>;

template <typename TType>
using pretty_dependency = PrettyDependency<TType>;


template <typename T>
using shared_ref = SharedRef<T>;

template <typename T>
using shared_ptr = SharedPtr<T>;

template <typename T>
using weak_ptr = WeakPtr<T>;


template <typename T>
using type_identity = TypeIdentity<T>;

template <typename... Ts>
using type_list = TypeList<Ts...>;

template <typename TTypeList, typename... Us>
using type_list_append = TypeListAppend<TTypeList, Us...>;

template <typename TTypeList, typename... Us>
using type_list_append_t = TypeListAppend<TTypeList, Us...>::type;


template <typename... TTypeLists>
using type_list_merge = TypeListMerge<TTypeLists...>;

template <typename... TTypeLists>
using type_list_merge_t = TypeListMerge<TTypeLists...>::type;
} // namespace snake_case
} // namespace liant