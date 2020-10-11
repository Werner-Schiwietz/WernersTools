#pragma once

#include <type_traits>

namespace WS
{
	//idee von https://stackoverflow.com/questions/49904809/template-function-for-detecting-pointer-like-dereferencable-types-fails-for-ac Answer 4
	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type can_dereference(unsigned long);
	template <typename T> auto can_dereference(int) -> decltype( * std::declval<T>(), std::true_type{} );		//kann dereferenziert werden
																												//template <typename T> auto can_dereference(long) -> decltype( std::declval<T>()->operator->(), std::true_type{} );	//gibt es den operator -> //hier nicht hilfreich
	template <typename T> using is_dereferenceable = decltype(can_dereference<T>(0));
	template <typename T> static auto const is_dereferenceable_v = is_dereferenceable<T>::value;

	template <typename> std::false_type has_pointer_op(unsigned long);
	template <typename T> auto has_pointer_op(int) -> decltype( std::declval<T>().operator->(), std::true_type{} );//hat operator->
																												   //template <typename T> auto has_pointer_op(long) -> decltype( * std::declval<T>(), std::true_type{} );		//kann dereferenziert werden? hier nicht hilfreich
	template <typename T> using is_pointer = decltype(has_pointer_op<T>(0));
	template <typename T> static auto const is_pointer_v = is_pointer<T>::value;
}
