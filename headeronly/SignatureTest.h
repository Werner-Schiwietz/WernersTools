#pragma once

template<typename signatur> struct has_signatur;
template<typename ret_t, typename ...args_t> struct has_signatur<ret_t(args_t...)>
{
	template<typename f_t> static constexpr bool function(f_t & v)
	{
		v;
		return WS::is_function_with_sig<decltype(&v),ret_t(args_t...)>::value;
	}
	template<typename f_t> static constexpr bool function(f_t && v)
	{
		v;
		return WS::is_function_with_sig<decltype(&v),ret_t(args_t...)>::value;
	}
	template<typename f_t> static constexpr bool function()
	{
		f_t v; v;
		return WS::is_function_with_sig<f_t,ret_t(args_t...)>::value;
	}
	template<typename f_t> static constexpr bool functor(f_t & )
	{
		return WS::is_functor<f_t,ret_t(args_t...)>::value;
	}
	template<typename f_t> static constexpr bool functor(f_t && )
	{
		return WS::is_functor<f_t,ret_t(args_t...)>::value;
	}
	template<typename f_t> static constexpr bool functor()
	{
		return WS::is_functor<f_t,ret_t(args_t...)>::value;
	}

	//is_callable funktioniert nicht vollständig. der ret_t muss gleich sein, aber die parameter müssen nur grob passen. und mit std::bind kann es zu compilefehlker kommen
	template<typename l_t, typename signature> struct is_callable;
	template<typename T, typename ret_t, typename ... args_t> struct is_callable<T, ret_t(args_t...)> 
	{
		template <typename U> struct type_check : std::false_type{};
		template <> struct type_check<ret_t> : std::true_type{}; 
		template<typename _1> using R = decltype(std::declval<_1>()( std::declval<args_t>()...) );

		template <typename _1,bool ret_t_equal = type_check<R<_1>>::value>
		static constexpr bool chk( type_check<R<_1>> * ) { return ret_t_equal; }
		template <typename> 
		static constexpr bool chk( ... ) { return false; }

		static bool const value = chk<T>(nullptr);
	};

	//callable functioniert nicht vollständig. der ret_t wird geprüft, aber die parameter müssen nur grob passen
	template<typename functional_t> static constexpr bool callable(functional_t  &)
	{
		return is_callable<functional_t,ret_t(args_t...)>::value;
	}
	template<typename functional_t> static constexpr bool callable(functional_t  && v )
	{
		return is_callable<functional_t,ret_t(args_t...)>::value;
	}
	template<typename functional_t> static constexpr bool callable( )
	{
		return is_callable<functional_t,ret_t(args_t...)>::value;
	}

};

namespace WS
{
	template<typename function_t, typename signatur_t> struct canCall;
	template<typename function_t,typename ret_t,typename ... args_t> struct canCall<function_t, ret_t(args_t...)> : has_signatur<ret_t(args_t...)>
	{
		static bool const value = callable<function_t>();
	};

	//template<typename T, typename function_t, typename signatur_t, bool> struct call;
	//template<typename T,typename function_t,typename ,typename ret_t,typename ... args_t,bool=canCall<function_t,ret_t(args_t...)>::value> struct call<function_t,ret_t(args_t...),true>
	//{
	//};
	//template<typename T,typename function_t,typename ,typename ret_t,typename ... args_t,bool =canCall<function_t,ret_t(args_t...)>::value> struct call<function_t,ret_t(args_t...),canCall<function_t,ret_t(args_t...)>::value,false>
	//{
	//};

}
