#pragma once

#include <type_traits>

namespace WS
{
	//idee von https://stackoverflow.com/questions/49904809/template-function-for-detecting-pointer-like-dereferencable-types-fails-for-ac Answer 4
	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type can_dereference(unsigned long);
	template <typename T> auto can_dereference(int) -> decltype( * std::declval<T>(), std::true_type{} );		//kann dereferenziert werden
																												
	template <typename T> using is_dereferenceable = decltype(can_dereference<T>(0));
	template <typename T> static auto const is_dereferenceable_v = is_dereferenceable<T>::value;

	template <typename> std::false_type has_pointer_op(unsigned long);//helper, ohne definition
	template <typename T> auto has_pointer_op(int) -> decltype( std::declval<T>().operator->(), std::true_type{} );//hat operator-> //helper, ohne definition
																												
	//template <typename T> using is_pointerable = std::integral_constant<bool,decltype(has_pointer_op<T>(0))::value>;
	template <typename T> using is_pointerable = std::integral_constant<bool,decltype(has_pointer_op<T>(0))::value | std::is_pointer<T>::value>;
	//template <typename T> using is_pointerable = decltype(has_pointer_op<T>(0));
	template <typename T> static auto const is_pointerable_v = is_pointerable<T>::value;
}

namespace WS
{
	template <typename T,typename =void> struct is_std_vector : std::false_type { };
	template <typename T> struct is_std_vector< T, typename std::enable_if< std::is_same<std::decay_t<T>,std::vector< typename std::decay_t<T>::value_type, typename std::decay_t<T>::allocator_type > >::value >::type > : std::true_type {};
	template <typename T> static auto const is_std_vector_v = is_std_vector<T>::value;
	
	template <typename T,typename =void> struct is_std_set : std::false_type { };
	template <typename T> struct is_std_set< T, typename std::enable_if< std::is_same<std::decay_t<T>,std::set< typename std::decay_t<T>::value_type, typename std::decay_t<T>::key_compare, typename std::decay_t<T>::allocator_type > >::value >::type > : std::true_type {};
	template <typename T> static auto const is_std_set_v = is_std_set<T>::value;
}