#pragma once
//Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

#include "tribool.h"
#include "char_helper.h"

#include <functional>

#pragma warning(push,4)

//LTH geht die parameterpaare von links nach rechts durch und liefert true wenn l<r ist oder false wenn r<l ist und false, wenn alle paramter ohne ergebnis abgearbeitet sind
//LTH_Member bekommt zwei objekte deren member verglichen werden sollen und eine liste von memberpointern. diese liste wird von links nach rechts abgearbeitet
//	in beiden funktionen koennen (muess aber nicht) hinter den zu vergleichenden werten eine vergleichsfunktion/functor angegeben werden 
//	es sind drei versionen möglich. 
//		bool(T,T) liefert true, wenn l<r
//		int(T,T) liefert kleiner 0 wenn l<r größer 0 wenn r<l und 0 wenn l==r
//		WS::tribool(T,T) liefert kleiner true wenn l<r false wenn r<l und invalid wenn l==r

namespace WS	
{
	namespace LTH_Helper//auch in signatur_test.h
	{
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
	}
}

namespace WS	//std-compare-funktion für LTH. kann spezialisiert werden
{	
	template<typename value_t> tribool LTHCompare( value_t const & l, value_t const & r )
	{
		if( l < r )
			return true;
		if( r < l )
			return false;
		return WS::tribool{};
	}
	template<typename value_t, size_t size > tribool LTHCompare( value_t const (& l)[size], value_t const (& r)[size] )
	{
		auto iter_l = &l[0];
		auto iter_r = &r[0];
		for( auto counter=size; counter --> 0; )
		{
		if( *iter_l < *iter_r )
			return true;
		if( *iter_r++ < *iter_l++ )
			return false;
		}
		return WS::tribool{};
	}
	template<typename CharPtrType> tribool LTHCharPtr( CharPtrType const & l, CharPtrType const & r )
	{
		if( l && r )
		{
			auto erg = stringcmp( l, r );
			if( erg<0 )
				return true;
			if( erg>0 )
				return false;
			return WS::tribool{};//gleich
		}
		return false;
	}

	template<> inline tribool LTHCompare<char const *>( char const * const & l, char const * const & r )//null-terminiertes char-arrays
	{
		return LTHCharPtr( l, r );
	}
	template<> inline tribool LTHCompare<wchar_t const *>( wchar_t const * const & l, wchar_t const * const & r )//null-terminiertes char-arrays
	{
		return LTHCharPtr( l, r );
	}
}

namespace WS
{
	struct LTH_ret_t
	{
		LTH_ret_t( bool value) : erg(value){}
		LTH_ret_t( tribool value) : erg(value){}

		bool lowerthan() const { return this->erg==true; }
		bool equal() const { return this->erg.valid()==false; }

		operator bool() const { return lowerthan(); }
	private:
		WS::tribool erg ;
	};
	inline LTH_ret_t LTH(  )
	{
		return WS::tribool{};//equal
	}

	template<typename value_t
		, typename less_t, typename std::enable_if<LTH_Helper::is_callable<less_t,bool(value_t const &,value_t const &)>::value,int>::type = 5//SFINEA nur mit type kann das übersetztwerden und type gibt es nur, wenn 3.parameter callable ist
		, typename ... args_t> 
	LTH_ret_t LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
	template<typename value_t
		, typename less_t, typename std::enable_if<LTH_Helper::is_callable<less_t,tribool(value_t const &,value_t const &)>::value,int>::type = 4//SFINEA 
		, typename ... args_t> 
	LTH_ret_t LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		auto erg = less( l, r );
		if( erg.valid() )
			return erg;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
	template<typename value_t
		, typename less_t, typename std::enable_if<LTH_Helper::is_callable<less_t,int(value_t const &,value_t const &)>::value,int>::type = 3//SFINEA 
		, typename ... args_t> 
	LTH_ret_t LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		auto erg = less( l, r );
		if(erg < 0 )
			return true;
		if(erg > 0 )
			return false;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
	template<typename value_t
		, typename ... args_t> 
	LTH_ret_t LTH( value_t const & l, value_t const & r, args_t && ... args )
	{
		auto erg = WS::LTHCompare( l, r );
		if( erg.valid() )
			return erg;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
}

namespace WS //LTH auf objekte mit pointer auf member using WS::LTH_Member(objekt1,objekt2, &objekt_type::member1[, comparer], &objekt_type::member2[, comparer][, ...])
{
	template <typename objecttype> LTH_ret_t LTH_Member( objecttype const & , objecttype const &  )
	{
		return WS::tribool{};//equal
	}
	template<typename objecttype, typename membertype, typename less_t
		, typename std::enable_if<LTH_Helper::is_callable<less_t,bool(membertype const &,membertype const &)>::value,int>::type = 5//SFINEA 
		, typename ... args_t> 
	LTH_ret_t LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, less_t less, args_t && ... args  )
	{
		if(less( l.*member, r.*member ) )
			return true;
		if(less( r.*member, l.*member ) )
			return false;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}
	template<typename objecttype, typename membertype
		, typename less_t, typename std::enable_if<LTH_Helper::is_callable<less_t,tribool(membertype const &,membertype const &)>::value,int>::type = 4//SFINEA 
		, typename ... args_t> 
	LTH_ret_t LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, less_t less, args_t && ... args  )
	{
		auto erg = less( l.*member, r.*member );
		if( erg.valid() )
			return erg;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}
	template<typename objecttype, typename membertype
		, typename less_t, typename std::enable_if<LTH_Helper::is_callable<less_t,int(membertype const &,membertype const &)>::value,int>::type = 3//SFINEA 
		, typename ... args_t> 
	LTH_ret_t LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, less_t less, args_t && ... args  )
	{
		auto erg = less( l.*member, r.*member );
		if( erg < 0 )
			return true;
		if( erg > 0 )
			return false;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}

	template <typename objecttype, typename membertype
		, typename ... args_t> 
	LTH_ret_t LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, args_t && ... args )
	{
		auto erg = LTHCompare( l.*member, r.*member );
		if( erg.valid() )
			return erg;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}
}
#pragma warning(pop)
