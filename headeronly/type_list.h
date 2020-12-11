#pragma once
#ifndef _WS_TYPE_LIST_H_
#define _WS_TYPE_LIST_H_
//Copyright (c) 2020 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

//Copyright (c) 2019-07 by Werner Schiwietz werner.githubpublic@gisbw.de. 
//All rights reserved.
//beispiel der nutzung
//WS::signatur<void(int,short const &)>::parameter_typelist::get_t<0> liefert den type int
//WS::signatur<void(int,short const &)>::parameter_typelist::get_t<1> liefert den type short const &
//WS::signatur<void(int,short)>::tuple_t tuple(5,short(3)) legt objekt std::tuple<int,short> tuple  mit int {5,3} an #include <tuple> notwendig
//
//wozu braucht man das? 
//keine ahnung. Ich suche noch das problem für die lösung.

namespace std
{	//forward
	template <class...> class tuple;
}
namespace WS_old
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
		static size_t constexpr size(){return sizeof...(types);}
		//include <tuple>. liefert std::tuple<types...>
		using tuple_t = std::tuple<types...>;
	};
}

namespace WS
{
	template<size_t N,typename first_t,typename...types> struct get_type : get_type<N-1,types...>
	{
	};
	template<typename first_t,typename...types> struct get_type<0,first_t,types...>
	{
		using type=first_t;
	};
	template<size_t N,typename...types> using get_type_t = typename get_type<N,types...>::type;

	template<typename...types> struct typelist
	{
		static size_t constexpr count=sizeof ... (types);
		//[[deprecated("deprecated use count")]] static size_t constexpr size(){return sizeof...(types);}
		template<size_t N> using type = typename get_type<N,types...>::type;
		template<size_t N> using get = get_type<N,types...>;
		template<size_t N> using get_t = typename get<N>::type;
		using tuple_t = std::tuple<types...>;
	};
	template<> struct typelist<>//spezialisierung leere typelist
	{
		static size_t constexpr count=0;
		//[[deprecated("deprecated use count")]] static size_t constexpr size(){return 0;}
		template<size_t N> using type = typename get_type<N,void>::type;
		template<size_t N> using get = get_type<N,void>;
		template<size_t N> using get_t = typename get<N>::type;
		using tuple_t = std::tuple<>;
	};
	template<> struct typelist<void> : typelist<>{};
}

namespace WS
{
	template<typename signature_t> struct signatur;
	template<typename return_t, typename ... arg_ts> struct signatur<return_t( arg_ts... )>
	{
		using parameter_typelist = typelist<arg_ts...>;
		using return_type = return_t;
	};
}

#endif//#ifndef _WS_TYPE_LIST_H_