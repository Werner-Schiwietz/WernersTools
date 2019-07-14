#pragma once
#ifndef _WS_TYPE_LIST_H_
#define _WS_TYPE_LIST_H_

//Copyright (c) 2019-07 by Werner Schiwietz werner.githubpublic@gisbw.de. 
//All rights reserved.
//beispiel der nutzung
//WS::signatur<void(int,short const &)>::parameter_typelist::get_t<0> liefert den type int
//WS::signatur<void(int,short const &)>::parameter_typelist::get_t<1> liefert den type short const &
//WS::signatur<void(int,short)>::tuple_t tuple(5,short(3)) legt objekt std::tuple<int,short> tuple  mit int {5,3} an #include <tuple> notwendig
//
//wozu braucht man das? 
//keine ahnung. Ich suche noch das problem für die lösung.

namespace WS
{
	//von https://codereview.stackexchange.com/questions/129058/type-list-with-utilities abgeleitet
	template<typename ... types> struct typelist
	{
	private:
		template<size_t index, size_t toget, typename ... types>			struct getter;
		template<size_t index, size_t toget, typename T, typename ... Rest> struct getter<index, toget, T, Rest...> { using type=typename getter<index+1, toget, Rest...>::type; };
		template<size_t toget, typename T, typename ... Rest>				struct getter<toget, toget, T, Rest...> { using type=typename T; };
	public:
		//get<i>:type bzw get_t<i> liefert den typ an "index" der template-parameter-liste. index=0 ist der erste typ von links
		template<size_t index> struct get
		{
			static_assert(index < sizeof...(types), "index out of bounds");
			using type=typename getter<0, index, types ...>::type;
		};
		template<size_t index> using get_t = typename get<index>::type;
		//size() liefert die anzahl der typen
		constexpr static size_t size(){return sizeof...(types);}
		//include <tuple>. liefert std::tuple<types...>
		struct tuple{using type=std::tuple<types...>;};
		using tuple_t = typename tuple::type;
	};
}

namespace WS
{
	template<typename signature_t> struct signatur;
	template<typename return_t, typename ... arg_ts> struct signatur<return_t( arg_ts... )>
	{
		using parameter_typelist = WS::typelist<arg_ts...>;
		using return_type = return_t;
	};
}

#endif//#ifndef _WS_TYPE_LIST_H_