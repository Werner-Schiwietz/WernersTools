#pragma once

namespace WS
{
	//von https://codereview.stackexchange.com/questions/129058/type-list-with-utilities abgeleitet
	template<typename ... type_ts> struct type_list
	{
	private:
		template<size_t index, size_t toget, typename ... type_ts> struct getter;
		template<size_t index, size_t toget, typename T, typename ... Rest_ts>
		struct getter<index, toget, T, Rest_ts...> { using type=typename getter<index+1, toget, Rest_ts...>::type; };
		template<size_t toget, typename T, typename ... Rest_ts>
		struct getter<toget, toget, T, Rest_ts...> { using type=typename T; };
	public:
		template<int index> struct get
		{
			static_assert(index < sizeof...(type_ts), "index out of bounds");
			using type=typename getter<0, index, type_ts ...>::type;
		};
		template<int index>
		using get_t = typename get<index>::type;
	};
}

namespace WS
{
	template<typename signature_t> struct signatur;
	template<typename return_t, typename ... arg_ts> struct signatur<return_t( arg_ts... )>
	{
		using parameter_ts = WS::type_list<arg_ts...>;
		using return_type = return_t;
	};
}
