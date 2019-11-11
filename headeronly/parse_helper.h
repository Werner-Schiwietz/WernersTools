#pragma once

#include "iterator_access.h"
#include "char_helper.h"
//#include "SignatureTest.h"

namespace WS
{
	template<typename T> bool eat( _iterator_access<T> & container, typename _iterator_access<T>::value_t const & item )
	{
		if( container.begin() != container.end() )
			if( *container.begin() == item )
			{
				++container.begin();
				return true;
			}
		return false;
	}

	template<typename T> struct rettype_eat
	{
		_iterator_access<T> eaten;
		_iterator_access<T> eaten_till_error;

		operator bool() { return eaten; }
		operator _iterator_access<T>() { return eaten; }

		rettype_eat( typename _iterator_access<T>::iterator_t & begin, typename _iterator_access<T>::iterator_t & end, _iterator_access<T> rettype_eat<T>::* member_to_set )
		{
			(this->*member_to_set).begin() = begin;
			(this->*member_to_set).end() = end;
		}
	};
	template<typename T> rettype_eat<T> eat( _iterator_access<T> & container_in, _iterator_access<T> const & items )
	{
		auto container = container_in;
		using container_t = std::remove_reference_t<decltype(container)>;

		for( auto const & item : items )
			if( eat( container, item ) == false )
			{
				return rettype_eat<T>{ container_in.begin(), container.begin(), &rettype_eat<T>::eaten_till_error };
			}

		rettype_eat<T> retvalue{ container_in.begin(), container.begin(), &rettype_eat<T>::eaten };
		container_in = container;
		return retvalue;
	}
	template<typename T, typename function_t> _iterator_access<T> eat_if( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const &)
	{
		auto retvalue = container; retvalue.end()=retvalue.begin();

		if( container.begin()!=container.end() && function( *container.begin() ) )
		{
			++container.begin();
			++retvalue.end();
		}
		return retvalue;
	}
	template<typename T, typename function_t> _iterator_access<T> eat_while( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const&)
	{
		using container_t = std::remove_reference_t<decltype(container)>;
		container_t retvalue( container.begin(), container.begin() );
		while( eat_if( container, function ) )
			++retvalue.end();
		return retvalue;
	}

	template<typename integer_t,typename T> struct rettype_eat_integer
	{
		integer_t			value = integer_t{0};
		_iterator_access<T> parsed;
		rettype_eat_integer() {};
		rettype_eat_integer(_iterator_access<T> & container) : parsed{container.begin(),container.begin()} {};
		operator bool() { return parsed; }
	};
	template<typename integer_t,typename T> rettype_eat_integer<integer_t,T> eat_integer( _iterator_access<T> & container ) noexcept(false)//wirft bei ueberlauf exception
	{
		rettype_eat_integer<integer_t,T> ret_value{container};

		using char_t = _iterator_access<T>::value_t;
		using function_t = bool(*)(char_t);

		while( auto erg=eat_if( container, (function_t)&ist_digit ) )
		{
			auto old = ret_value.value;
			ret_value.value = ret_value.value*10 + *erg.begin() - char_t( '0' );
			if( ret_value.value<old )
				throw std::out_of_range( __FUNCTION__ " overflow" );
			++ret_value.parsed.end();
		}
		return ret_value;
	}
}
