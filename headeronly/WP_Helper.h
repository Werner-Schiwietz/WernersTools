#pragma once 

#include <type_traits>

namespace WS
{
	template <typename T> struct puretype
	{
		typedef std::remove_volatile_t<std::remove_const_t<std::remove_pointer_t<std::remove_cv_t<std::remove_all_extents_t<std::remove_reference_t<T>>>>>> type;
	};
	template<class _Ty> using puretype_t = typename puretype<_Ty>::type;

	template <typename T> struct is_const_pointer
	{
		static const bool value = std::is_const<std::remove_pointer_t<T>>::value;
	};

	template<typename T> struct chartraits
	{	
		static_assert( std::is_same<char,T>::value || std::is_same<wchar_t,T>::value , "diese struct darf nicht angezogen werden. es lauft etwas schief" );
	};
	template<> struct chartraits<char>
	{	
		typedef char XTYPE;
		typedef wchar_t YTYPE;
	};
	template<> struct chartraits<wchar_t>
	{	
		typedef char YTYPE;
		typedef wchar_t XTYPE;
	};
	template <typename T> using XCHAR = typename chartraits<typename puretype<T>::type>::XTYPE;
	template <typename T> using YCHAR = typename chartraits<typename puretype<T>::type>::YTYPE;

	//quelle https://stackoverflow.com/questions/30407754/how-to-test-if-a-method-is-const
	template<class T> 
	struct is_pointer_to_const_member_function : std::false_type {};
	template<class R, class T, class... Args> 
	struct is_pointer_to_const_member_function<R (T::*)(Args...) const> : std::true_type {};
	template<class R, class T, class... Args> 
	struct is_pointer_to_const_member_function<R (T::*)(Args...) const &> : std::true_type {};
	template<class R, class T, class... Args> 
	struct is_pointer_to_const_member_function<R (T::*)(Args...) const &&> : std::true_type {};
	template<class R, class T, class... Args> 
	struct is_pointer_to_const_member_function<R (T::*)(Args..., ...) const> : std::true_type {};
	template<class R, class T, class... Args> 
	struct is_pointer_to_const_member_function<R (T::*)(Args..., ...) const &> : std::true_type {};
	template<class R, class T, class... Args> 
	struct is_pointer_to_const_member_function<R (T::*)(Args..., ...) const &&> : std::true_type {};

	template< typename T,typename ret_t,typename ... params> T* objecttype_of_memberfunction( ret_t (T::*)(params...) ){return nullptr;}
	template< typename T,typename ret_t,typename ... params> T* objecttype_of_memberfunction( ret_t (T::*)(params...) const ){return nullptr;}
}