#pragma once

#include "iterator_access.h"
#include "char_helper.h"

//ein paar hilfs-funktion zum parsen 
//eat						//genau die items in der reihenfolge, oder das eine item
//eat_if					//ein zeichen wenn bedingung erfüllt ist
//eat_while					//solange wie bedingung erfüllt ist
//eat_oneof					//ein zeichen wenn es in der liste ist
//eat_flanked				//alles eingebettet ist z.B. \"hallo\" in "\"hallo\"" oder "hallo" in ["hallo"]
//eat_integer				//konvertiert digits zu integer

namespace WS
{
	enum class parse_error
	{
		none,
		incomplete,
		tillitem_not_found,
		invalid_escape_sequence
	};
	template<typename T> struct rettype_eat
	{
		_iterator_access<T> eaten;
		_iterator_access<T> eaten_till_error;
		parse_error error = parse_error::none;

		operator bool() { return eaten; }
		operator _iterator_access<T>() { return eaten; }
		operator parse_error() { return error; }

		rettype_eat( _iterator_access<T> const & alive, typename _iterator_access<T>::iterator_t & begin, typename _iterator_access<T>::iterator_t & end )
		{
			eaten = alive;
			eaten.begin() = begin;
			eaten.end() = end;
		}
		rettype_eat( _iterator_access<T> const & alive, typename _iterator_access<T>::iterator_t & begin, typename _iterator_access<T>::iterator_t & end, parse_error error ) : error(error)
		{
			if( error==parse_error::none )
			{
				eaten = alive;
				eaten.begin() = begin;
				eaten.end() = end;
			}
			else
			{
				eaten_till_error = alive;
				eaten_till_error.begin() = begin;
				eaten_till_error.end() = end;
			}
		}
	};

	template<typename T> bool _eat_unchecked( _iterator_access<T> & container, typename _iterator_access<T>::value_t const & item )
	{
		if( *container.begin() == item )
		{
			++container.begin();
			return true;
		}
		return false;
	}	
	template<typename T> bool eat( _iterator_access<T> & container, typename _iterator_access<T>::value_t const & item )
	{
		if( container.begin() != container.end() )
			return _eat_unchecked( container, item );
		return false;
	}

	//return empty or 
	template<typename T> rettype_eat<T> eat( _iterator_access<T> & container_in, _iterator_access<T> const & items )
	{
		auto container = container_in;
		auto error = parse_error::none;
		
		for( auto const & item : items )
			if( eat( container, item ) == false )
				return rettype_eat<T>{ container_in, container_in.begin(), container.begin(), error };
			else
				error = parse_error::incomplete;


		rettype_eat<T> retvalue{ container_in, container_in.begin(), container.begin() };
		container_in = container;
		return retvalue;
	}

	template<typename T> bool _eat_oneof_unchecked( _iterator_access<T> & ) { return false; }
	template<typename T, typename ... items_t> bool _eat_oneof_unchecked( _iterator_access<T> & container, typename _iterator_access<T>::value_t const & item, items_t ... items )
	{
		if( _eat_unchecked( container, item ) )
			return true;
		return _eat_oneof_unchecked( container, items ... );
	}	
	template<typename T, typename ... items_t> _iterator_access<T> eat_oneof( _iterator_access<T> & container, items_t ... items ) 
	{
		_iterator_access<T> retvalue{container.begin(),container.begin(),container.rvalue_lifetime_extender};

		if( container.begin()!=container.end() )
			if( _eat_oneof_unchecked( container, items ... ) )
				retvalue.end() = container.begin();

		return retvalue;
	}
	template<typename T, typename ... items_t> _iterator_access<T> eat_oneof( _iterator_access<T> & container, _iterator_access<T> items ) 
	{
		auto begin = container.begin();

		if( container.begin()!=container.end() )
			for( auto & item : items)
				if( _eat_unchecked( container, item ) )
					return _iterator_access<T> {begin,container.begin(),container.rvalue_lifetime_extender};

		return _iterator_access<T> {begin,begin,container.rvalue_lifetime_extender};
	}

	template<typename T> rettype_eat<T> eat_till( _iterator_access<T> & container_in, typename _iterator_access<T>::value_t const & till_item, typename _iterator_access<T>::value_t const & escape_item )
	{
		auto container = container_in;

		while( container )
		{
			auto cont_esc = container;
			if( eat( cont_esc, escape_item ) )//escape-sequence
			{
				if( eat_oneof( cont_esc, till_item, escape_item ) )
					container.begin()=cont_esc.begin();
				else
					return rettype_eat<T>{container_in, container_in.begin(), container.begin(), parse_error::invalid_escape_sequence};
			}
			else if( eat( cont_esc, till_item ) )//
			{
				rettype_eat<T> retvalue{container_in, container_in.begin(), container.begin()};
				container_in = container;
				return retvalue;
			}
			else
				++container.begin();
		}

		return rettype_eat<T> { container_in, container_in.begin(), container.begin(), parse_error::tillitem_not_found };
	}


	template<typename T> using flanked_t = typename _iterator_access<T>::value_t;
	template<typename T> using left_t = typename _iterator_access<T>::value_t;
	template<typename T> using right_t = typename _iterator_access<T>::value_t;
	template<typename T> using escape_t = typename _iterator_access<T>::value_t;
	template<typename T> auto const & flanked_type	( T const & item )	{ return item; }
	template<typename T> auto const & left_type		( T const & item )	{ return item; }
	template<typename T> auto const & right_type	( T const & item )	{ return item; }
	template<typename T> auto const & escape_type	( T const & item )	{ return item; }

	template<typename T> struct rettype_eat_flanked : rettype_eat<T>
	{
		left_t<T> left{};
		right_t<T> right{};

		operator bool(){return this->error==parse_error::none;}

		auto && setLeft ( left_t<T>  value ) && { this->left =value; return std::move(*this); }
		auto && setRight( right_t<T> value ) && { this->right=value; return std::move(*this); }

		using rettype_eat::rettype_eat;

		rettype_eat_flanked( rettype_eat && r) : rettype_eat(r){}
	};
	template<typename T> rettype_eat_flanked<T> _eat_flanked( _iterator_access<T> & container, right_t<T> const & right_item, escape_t<T> const & escape_item )
	{
		rettype_eat_flanked<T> retvalue = eat_till( container, right_item, escape_item );
		eat( container, right_item );
		return std::move(retvalue).setRight(right_item);
	}
	template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> & container_in, left_t<T> const & left_item, right_t<T> const & right_item, escape_t<T> const & escape_item )
	{
		auto container = container_in;
		if( eat( container, left_item ) )
		{
			auto erg = _eat_flanked( container, right_item, escape_item ).setLeft(left_item);
			if( erg )
				container_in = container;
			return erg;
		}
		return {container,container.begin(),container.begin()};
	}
	template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> && container_in, left_t<T> const & left_item, right_t<T> const & right_item, escape_t<T> const & escape_item )
	{
		return eat_flanked( container_in, left_item, right_item, escape_item );
	}
	template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> & container, flanked_t<T> const & flank_item, escape_t<T> const & escape_item )
	{
		return eat_flanked( container, left_t<T>(flank_item), right_t<T>(flank_item), escape_item );
	}
	template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> && container, flanked_t<T> const & flank_item, escape_t<T> const & escape_item )
	{
		return eat_flanked( container, left_t<T>(flank_item), right_t<T>(flank_item), escape_item );
	}
	template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> & container_in, typename _iterator_access<T> const & till_items, escape_t<T> const & escape_item )
	{
		auto container = container_in;
		if( auto erg_first_last_item = eat_oneof( container, till_items ) )
		{
			auto retvalue = _eat_flanked( container, *erg_first_last_item.begin(), escape_item );
			if( retvalue )
				container_in = container;
			return std::move(retvalue).setLeft(*erg_first_last_item.begin());
		}
		return {container_in, container_in.begin(),container_in.begin()};
	}
	template <typename container_t, typename value_t> container_t& append( container_t & container, value_t value )
	{
		return container += value;
	}
	template <typename container_t, typename iterator_t> container_t& append( container_t & container, iterator_t first, iterator_t last )
	{
		return container += container_t{first, last};
	}
	template<typename container_t, typename T> container_t make_flanked( _iterator_access<T> parse, left_t<T> left, right_t<T> right, escape_t<T> escape)
	{
		container_t retvalue;
		using value_t = typename _iterator_access<T>::value_t;

		append( retvalue, left );

		for(;parse;)
		{
			auto part = eat_while( parse, [right, escape]( value_t const & value ) { return value!=right && value!=escape; } );
			if( part )
				append( retvalue, part.begin(), part.end() );
			if( parse )
			{
				append( retvalue, escape );
				append( retvalue, *parse.begin()++ );
			}
		}
		return append( retvalue, right );
	}

	template<typename container_t, typename T> container_t make_flanked( _iterator_access<T> parse, flanked_t<T> const & flank_item, escape_t<T> escape)
	{
		return make_flanked<container_t>( parse, flank_item, flank_item, escape);
	}

	template<typename iterator_t>  _iterator_access<decltype(&*std::declval<iterator_t>())> remove_flank( _iterator_access<iterator_t> parse, left_t<iterator_t> const & left_item, right_t<iterator_t> const & right_item, escape_t<iterator_t> const & escape_item )
	{
		return remove_escape( eat_flanked( parse, left_item, right_item, escape_item ).eaten, escape_item );
	}

	template<typename iterator_t>  _iterator_access<decltype(&*std::declval<iterator_t>())> remove_flank( _iterator_access<iterator_t> parse, flanked_t<iterator_t> const & flank_item, escape_t<iterator_t> const & escape_item )
	{
		return remove_escape( eat_flanked( parse, flank_item, flank_item, escape_item ), escape_item );
	}


	//entfernt escape-char. macht kopie nur, wenn sie nötig ist
	template<typename iterator_t>  _iterator_access<decltype(&*std::declval<iterator_t>())> remove_escape( _iterator_access<iterator_t> parse, escape_t<iterator_t> escape)
	{
		using retvalue_t = _iterator_access<decltype(&*std::declval<iterator_t>())>;
		using value_t = typename _iterator_access<iterator_t>::value_t;
		using buffer_t = std::basic_string<value_t>;//funktioniert nur, wenn 

		_iterator_access<iterator_t>	retvalue{parse.begin(),parse.begin(),parse.rvalue_lifetime_extender};//copy rvalue_lifetime_extender
		buffer_t						char_buffer;//nur wenn nötig umkopieren

		//bei nötiger veränderung char_buffer benutzen
		auto append =[&]( _iterator_access<iterator_t> parse )
		{
			if( parse.empty() )
				return;
			if( retvalue.empty() )
				retvalue = parse;
			else if( retvalue.end() == parse.begin() )
				retvalue.end() = parse.end();
			else 
			{	//veränderung -> umkopieren
				if( char_buffer.empty() )
					if( retvalue )
						char_buffer = buffer_t{retvalue.begin(),retvalue.end()};
				char_buffer.append( parse.begin(), parse.end() );
			}
		};

		while(parse)
		{
			auto part = eat_while( parse, [escape]( value_t const & value ) { return value!=escape; } );
			append( part );

			if( parse )
			{
				auto b = ++parse.begin();//wg parameter reihenfolge ++ nicht als fn-parameter
				auto e = ++parse.begin();//wg parameter reihenfolge ++ nicht als fn-parameter
				append( iterator_access( b, e ) );
			}
		}

		if( char_buffer.empty() )
		{
			auto b = &*retvalue.begin();
			auto e = b+retvalue.len();
			return retvalue_t{b,e,retvalue.rvalue_lifetime_extender};
		}

		{
			auto temp = iterator_access( std::move( char_buffer ) );//rvalue_lifetime_extender anlegen
			auto b = &*temp.begin();
			auto e = b+temp.len();//*temp.end() geht nicht
			return retvalue_t{b,e,temp.rvalue_lifetime_extender};
		}
	}

	template<typename T, typename function_t> _iterator_access<T> _eat_if_unckecked( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const &)
	{
		_iterator_access<T> retvalue{container.begin(),container.begin(), container.rvalue_lifetime_extender};

		if( function( *container.begin() ) )
			retvalue.end() = ++container.begin();

		return retvalue;
	}	
	template<typename T, typename function_t> _iterator_access<T> eat_if( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const &)
	{
		if( container.begin()!=container.end() )
			return _eat_if_unckecked( container, function );

		return {container.begin(),container.begin(),container.rvalue_lifetime_extender};
	}
	template<typename T, typename function_t> _iterator_access<T> eat_while( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const&)
	{
		_iterator_access<T> retvalue( container.begin(), container.begin(), container.rvalue_lifetime_extender );
		while( eat_if( container, function ) )
			++retvalue.end();
		return retvalue;
	}

	template<typename integer_t,typename T> struct rettype_eat_integer
	{
		integer_t			value = integer_t{0};
		_iterator_access<T> parsed;
		rettype_eat_integer() {};
		rettype_eat_integer(_iterator_access<T> & container) : parsed{container.begin(),container.begin(),container.rvalue_lifetime_extender} {};
		operator bool() { return parsed; }
		bool operator !() { return !parsed; }
		operator integer_t(){ return value; }
	};
	template<typename integer_t,typename T> rettype_eat_integer<integer_t,T> eat_integer( _iterator_access<T> & container ) noexcept(false)//wirft bei ueberlauf exception
	{
		rettype_eat_integer<integer_t,T> ret_value{container};

		using char_t = _iterator_access<T>::value_t;
		using function_t = bool(*)(char_t);

		while( auto erg=eat_if( container, (function_t)&ist_digit ) )
		{
			auto old = ret_value.value;
			ret_value.value = static_cast<integer_t>(ret_value.value*10 + *erg.begin() - char_t( '0' ));
			if( ret_value.value<old )
				throw std::out_of_range( __FUNCTION__ " overflow" );
			++ret_value.parsed.end();
		}
		return ret_value;
	}
	template<typename integer_t,typename T> rettype_eat_integer<integer_t,T> eat_integer( _iterator_access<T> && container ) noexcept(false)//wirft bei ueberlauf exception
	{
		return eat_integer<integer_t>( container );
	}

	template<typename T> auto eat_space( _iterator_access<T> & container )
	{
		using is_t = bool(*)(_iterator_access<T>::value_t);
		return eat_while( container, (is_t)&WS::isspace );
	}
	template<typename T> struct rettype_skip
	{
		_iterator_access<T> eaten;
		operator bool() { return true; }
		operator _iterator_access<T>() { return eaten; }
		operator parse_error() { parse_error::none; }

		rettype_skip() {};
		rettype_skip(_iterator_access<T> eaten) : eaten(eaten) {};
	};
	template<typename T> rettype_skip<T> skip_space( _iterator_access<T> & container )
	{
		using is_t = bool(*)(_iterator_access<T>::value_t);
		return eat_space( container );
	}
}
