#pragma once

#include "tribool.h"
#include "char_helper.h"
#include "SignatureTest.h"

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
	template<typename value_t> WS::tribool LTHCompare( value_t const & l, value_t const & r )
	{
		if( l < r )
			return true;
		if( r < l )
			return false;
		return WS::tribool();
	}
	template<typename CharPtrType> WS::tribool LTHCharPtr( CharPtrType const & l, CharPtrType const & r )
	{
		if( l && r )
		{
			auto erg = stringcmp( l, r );
			if( erg<0 )
				return true;
			if( erg>0 )
				return false;
			return WS::tribool();//gleich
		}
		return false;
	}

	template<> inline WS::tribool LTHCompare<char const *>( char const * const & l, char const * const & r )
	{
		return LTHCharPtr( l, r );
	}
	template<> inline WS::tribool LTHCompare<wchar_t const *>( wchar_t const * const & l, wchar_t const * const & r )
	{
		return LTHCharPtr( l, r );
	}
	template<> inline WS::tribool LTHCompare<char *>( char * const & l, char * const & r )
	{
		return LTHCharPtr( l, r );
	}
	template<> inline WS::tribool LTHCompare<wchar_t *>( wchar_t * const & l, wchar_t * const & r )
	{
		return LTHCharPtr( l, r );
	}
}

namespace WS
{
	inline bool LTH(  )
	{
		return false;
	}

	template<typename value_t, typename less_t, typename std::enable_if_t<WS::canCall<less_t,bool(value_t,value_t)>::value,int>			= 5,typename ... args_t> bool LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
	template<typename value_t, typename less_t, typename std::enable_if_t<WS::canCall<less_t,WS::tribool(value_t,value_t)>::value,int>	= 4,typename ... args_t> bool LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		auto erg = less( l, r );
		if( erg.valid() )
			return erg;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
	template<typename value_t, typename less_t, typename std::enable_if_t<WS::canCall<less_t,int(value_t,value_t)>::value,int>			= 3,typename ... args_t> bool LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		auto erg = less( l, r );
		if(erg < 0 )
			return true;
		if(erg > 0 )
			return false;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
	template<typename value_t, typename ... args_t> bool LTH( value_t const & l, value_t const & r, args_t && ... args )
	{
		auto erg = WS::LTHCompare( l, r );
		if( erg.valid() )
			return erg;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
}

namespace WS //LTH auf objekte mit pointer auf member
{
	template <typename objecttype> bool LTH_Member( objecttype const & , objecttype const &  )
	{
		return false;
	}
	template<typename objecttype, typename membertype, typename less_t, typename std::enable_if_t<WS::canCall<less_t,bool(membertype,membertype)>::value,int>			= 5,typename ... args_t> bool LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, less_t less, args_t && ... args  )
	{
		if(less( l.*member, r.*member ) )
			return true;
		if(less( r.*member, l.*member ) )
			return false;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}
	template<typename objecttype, typename membertype, typename less_t, typename std::enable_if_t<WS::canCall<less_t,WS::tribool(membertype,membertype)>::value,int>	= 4,typename ... args_t> bool LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, less_t less, args_t && ... args  )
	{
		auto erg = less( l.*member, r.*member );
		if( erg.valid() )
			return erg;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}
	template<typename objecttype, typename membertype, typename less_t, typename std::enable_if_t<WS::canCall<less_t,int(membertype,membertype)>::value,int>			= 3,typename ... args_t> bool LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, less_t less, args_t && ... args  )
	{
		auto erg = less( l.*member, r.*member );
		if( erg < 0 )
			return true;
		if( erg > 0 )
			return false;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}

	template <typename objecttype, typename membertype, typename ... args_t> bool LTH_Member( objecttype const & l, objecttype const & r, membertype objecttype::* member, args_t && ... args )
	{
		auto erg = LTHCompare( l.*member, r.*member );
		if( erg.valid() )
			return erg;

		return LTH_Member( l, r, std::forward<args_t>(args) ... );
	}
}
#pragma warning(pop)
