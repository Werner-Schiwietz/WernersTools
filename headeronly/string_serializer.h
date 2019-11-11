#pragma once

//WS::oSerializer dient z.zt. nur dem anhängen von werten als zeichenkette an einen string_typ 
//per operator<< 
//
//using string_type = CString;// oder CStringA std::string std::wstring
//usage 
//string_type str = WS::oSerializer<string_type>() << value1 << value2 << valueN;
//oder
//string_type str;
//WS::oSerializer<string_type>(str) << value1 << value2 << valueN;

#include "char_helper.h"

namespace WS
{
	//template <class T, class U> concept Derived = std::is_base_of<U, T>::value; //c++20

	template<typename T> struct is_string_type : std::false_type {};
	template<>  struct is_string_type<CStringA> : std::true_type {};
	template<>  struct is_string_type<CStringW> : std::true_type {};
	template<>  struct is_string_type<std::string> : std::true_type {};
	template<>  struct is_string_type<std::wstring> : std::true_type {};

	template<typename senke_t> class oSerializer//hilfsklasse weil template<T> T& operator<<(l,t) zu compilefehler mit beliebigen code der a<<b macht, fuehren koennte
	{
		static_assert(WS::is_string_type<senke_t>::value, "senke_t muss z.zt. ein StringType sein." );
		typedef senke_t senke_t;
		mutable senke_t ownsenke;
	public:
		senke_t & senke;

		oSerializer() : senke(ownsenke){}
		oSerializer(senke_t & senke) : senke(senke){}
		oSerializer( oSerializer const & ) = delete;
		oSerializer operator=( oSerializer const & ) = delete;

		operator senke_t & () const
		{
			return senke;
		}
	};
}

template<typename senke_t> WS::oSerializer<senke_t> const & operator<<( WS::oSerializer<senke_t> const & o, senke_t const & r )
{
	o.senke += r;
	return o;
}
template<typename senke_t> WS::oSerializer<senke_t> const & operator<<( WS::oSerializer<senke_t> const & o, wchar_t const * r )
{
	return o << senke_t( r );
}
template<typename senke_t> WS::oSerializer<senke_t> const & operator<<( WS::oSerializer<senke_t> const & o, char const * r )
{
	return o << senke_t( r );
}

template<typename senke_t> WS::oSerializer<senke_t> const & operator<<( WS::oSerializer<senke_t> const & o, int r )
{
	std::remove_reference_t<decltype(o.senke.operator[](0))> buf[20];
	return o << itostring_s(r,buf,10);
}

//die fehlenden operator<< bitte nach obigen muster selbst schreiben
