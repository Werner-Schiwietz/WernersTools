#pragma once
//Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

#include "tribool.h"
#include "char_helper.h"
#include "is_.h"

#include <functional>

#pragma warning(push,4)

//LTH geht die parameterpaare von links nach rechts durch und liefert true wenn l<r ist oder false wenn r<l ist und false, wenn alle paramter ohne ergebnis abgearbeitet sind
//LTH_Member bekommt zwei objekte deren member verglichen werden sollen und eine liste von memberpointern. diese liste wird von links nach rechts abgearbeitet
//	in beiden funktionen koennen (muess aber nicht) hinter den zu vergleichenden werten eine vergleichsfunktion/functor angegeben werden 
//	es sind drei versionen m�glich. 
//		bool(T,T) liefert true, wenn l<r
//		int(T,T) liefert kleiner 0 wenn l<r gr��er 0 wenn r<l und 0 wenn l==r
//		WS::tribool(T,T) liefert kleiner true wenn l<r false wenn r<l und invalid wenn l==r

namespace WS
{
	// FUNCTION TEMPLATE declval
	template <class _Ty> std::add_lvalue_reference_t<_Ty> decllval() noexcept;
	template <class _Ty> std::add_rvalue_reference_t<_Ty> declrval() noexcept;
}

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

	template<typename T> auto Has_less_operator(unsigned long) -> std::false_type;
	//template<typename T> auto Has_less_operator(int) -> decltype(operator<( WS::decllval<T>(), WS::decllval<T>()) , std::true_type{});
	//template<typename T> auto Has_less_operator(int) -> decltype((WS::decllval<T>().operator<(WS::decllval<T>())), std::true_type{});
	template<typename T> auto Has_less_operator(int) -> decltype((WS::decllval<T>() < WS::decllval<T>()), std::true_type{});
	template<typename T> static bool constexpr Has_less_operator_v = decltype(Has_less_operator<T>(0))::value;

	struct less_test{bool operator<(less_test const &)const;};
	struct like_HKEY{int unused;}; 
	static_assert( Has_less_operator_v<like_HKEY> == false );
	static_assert( Has_less_operator_v<less_test> == true );
}

namespace WS	//std-compare-funktion f�r LTH. kann spezialisiert werden
{	
	template<typename value_t> tribool LTHComparePointer( value_t const * l, value_t const * r ) 
	{
		if( l==r )
			return {};
		if( l==nullptr )
			return true;
		if( r==nullptr )
			return false;
		if constexpr ( Has_less_operator_v<value_t const> )
		{
			if( *l < *r )
				return true;
			if( *r < *l )
				return false;
		}
		else//z.b. void* HEKY usw
		{
			if( l < r )
				return true;
			if( r < l )
				return false;
		}
		return {};
	}
	template<typename value_t> tribool LTHCompareValue( value_t const & l, value_t const & r )
	{
		if( l < r )
			return true;
		if( r < l )
			return false;
		return WS::tribool{};
	}
	template<typename value_t> tribool LTHCompare( value_t const & l, value_t const & r )
	{
		if constexpr( WS::is_dereferenceable_v<value_t> )
		{
			if constexpr( std::is_same<decltype(&*l),value_t>::value==false )
				return LTHCompare( &*l, &*r );//z.B. smarte pointer wie std::unique_ptr std::shared_ptr WS::auto_ptr
		}
		if constexpr( std::is_pointer_v<value_t> )
			return LTHComparePointer( l, r );
		else 
			return LTHCompareValue( l, r );
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

	template<> inline tribool LTHCompare<void const *>( void const * const & l, void const * const & r )
	{
		if( l==r )
			return {};
		if( l==nullptr )
			return true;
		if( r==nullptr )
			return false;
		if(l<r)
			return true;
		if(r<l)
			return false;
		return {};
	}

	//compiler macht das, aber nicht nur f�r smart_ptr, sondern auch f�r std::string, also 
	//template<template<typename...> typename smart_ptr, typename value_t, typename ... others> inline tribool LTHCompare( smart_ptr<value_t,others...> const & l, smart_ptr<value_t,others...> const & r )
	//{
	//	return LTHCompare(l.get(),r.get());
	//}

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
		, typename less_t, typename std::enable_if<LTH_Helper::is_callable<less_t,bool(value_t const &,value_t const &)>::value,int>::type = 5//SFINEA nur mit type kann das �bersetztwerden und type gibt es nur, wenn 3.parameter callable ist
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
