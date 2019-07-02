#pragma once

#include "tribool.h"
#include "char_helper.h"

#include <functional>

#pragma warning(push,4)

//LTH geht die parameterpaare von links nach rechts durch, bis ein paar kleiner oder nicht kleiner meldet
//LTH_Member bekommt zwei objekte, deren member verglichen werden sollen und eine liste von memberpointern. diese liste wird von links nach rechts abgearbeitet, bis der verwendete vergleich eine ungleichheit feststellt
// in beiden funktionen koennen (muess aber nicht) hinter den vergleichstypen vergleichenfunktionen der form std::function<WP::tribool>(datentyp const &,datentyp const &)> uebergeben werden
namespace WP 
{
	//static std::function<WP::tribool(CString const &l, CString const &r)> LTH_CStringNoCase = [](CString const &l, CString const &r)-> WP::tribool
	//{
	//	auto erg = l.CompareNoCase( r );
	//	if( erg < 0 )
	//		return true;
	//	else if ( erg > 0 )
	//		return false;
	//	return WP::tribool();
	//};

	inline bool LTH(  )
	{
		return false;
	}
	template<typename value_t> bool LTH( value_t const & l, value_t const & r )
	{
		if( l < r )
			return true;
		return false;
	}
	template<typename CharPtrType> WP::tribool LTHCharPtr( CharPtrType const & l, CharPtrType const & r )
	{
		if( l && r )
		{
			auto erg = stringcmp( l, r );
			if( erg<0 )
				return true;
			if( erg>0 )
				return false;
			return WP::tribool();//gleich
		}
		return false;
	}
	template<> inline bool LTH<char const *>( char const * const & l, char const * const & r )
	{
		return LTHCharPtr( l, r )==true;//false und invalide liefern false
	}
	template<> inline bool LTH<wchar_t const *>( wchar_t const * const & l, wchar_t const * const & r )
	{
		return LTHCharPtr( l, r )==true;//false und invalide liefern false
	}

	template<typename value_t, size_t size_l, size_t size_r> inline bool LTH( value_t const (& l)[size_l], value_t const (& r)[size_r] )
	{
		static_assert( std::is_same<value_t,char>::value || std::is_same<value_t,wchar_t>::value, "LTH nur auf 0-terminiertes char-array geprueft" );
		return LTH<value_t const*>(l,r);
	}
	template<typename value_t, size_t size_r> inline bool LTH( value_t const * const & l, value_t const (& r)[size_r] )
	{
		static_assert( std::is_same<value_t,char>::value || std::is_same<value_t,wchar_t>::value, "LTH nur auf 0-terminiertes char-array geprueft" );
		return LTH<value_t const*>(l,r);
	}
	template<typename value_t, size_t size_l> inline bool LTH( value_t const (& l)[size_l], value_t const * const & r )
	{
		static_assert( std::is_same<value_t,char>::value || std::is_same<value_t,wchar_t>::value, "LTH nur auf 0-terminiertes char-array geprueft" );
		return LTH<value_t const*>(l,r);
	}

	template<typename value_t> WP::tribool LTHCompare( value_t const & l, value_t const & r )
	{
		if( LTH(l,r) )
			return true;
		if( LTH(r,l) )
			return false;
		return WP::tribool();
	}
	template<> inline WP::tribool LTHCompare<char const *>( char const * const & l, char const * const & r )
	{
		return LTHCharPtr( l, r );
	}
	template<> inline WP::tribool LTHCompare<wchar_t const *>( wchar_t const * const & l, wchar_t const * const & r )
	{
		return LTHCharPtr( l, r );
	}
	template<> inline WP::tribool LTHCompare<char *>( char * const & l, char * const & r )
	{
		return LTHCharPtr( l, r );
	}
	template<> inline WP::tribool LTHCompare<wchar_t *>( wchar_t * const & l, wchar_t * const & r )
	{
		return LTHCharPtr( l, r );
	}

	template<typename value_t, typename ... Args> bool LTH( value_t const & l, value_t const & r, std::function<WP::tribool( value_t const&,value_t const&)> const & less, Args const & ... args )
	{
		auto erg = less( l, r );
		if( erg.valid() )
			return erg;

		return LTH( args ... );
	}
	template<typename value_t, typename ... Args> bool LTH( value_t const & l, value_t const & r, Args const & ... args )
	{
		auto erg = LTHCompare( l, r );
		if( erg.valid() )
			return erg;

		return LTH( args ... );
	}
}


namespace WP //LTH auf objekte mit pointer auf member
{
	template <typename objecttype> bool LTH_Member( objecttype const & , objecttype const &  )
	{
		return false;
	}
	template <typename objecttype, typename membertype, typename ... Args> bool LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, std::function<WP::tribool( membertype const&,membertype const&)> const & less, Args const & ... args )
	{
		auto erg = less( l.*member, r.*member );
		if( erg.valid() )
			return erg;

		return LTH_Member( l, r, args ... );
	}
	template <typename objecttype, typename membertype, typename ... Args> bool LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, Args const & ... args )
	{
		auto erg = LTHCompare( l.*member, r.*member );
		if( erg.valid() )
			return erg;

		return LTH_Member( l, r, args ... );
	}
}
#pragma warning(pop)
