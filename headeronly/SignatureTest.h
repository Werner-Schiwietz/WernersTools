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
