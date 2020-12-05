#pragma once

#include "combiner_last.h"

#include <optional>//WS::ICombiner
#include <deque>

namespace WS
{
	//Combiner für Signal
	template<typename T> struct combiner_and
	{	//combiner_and liefert alle rückgabewerte mit operator& verknüpft in std::optional<T> value
		using id_t = size_t;//typename Signal_trait<T()>::id_t;
		std::optional<T> value;
		auto operator()() { return this->value; }
		auto & operator()( T const & v_in, id_t  ={} ) &
		{
			if( this->value.has_value() )
				this->value.value() &= v_in;
			else
				this->value = v_in;
			return *this;
		}
		auto operator()( T const & v_in, id_t id={} ) && { return operator()( v_in, id ); }
	};
	template<typename T> struct combiner_or
	{	//combiner_and liefert alle rückgabewerte mit operator| verknüpft in std::optional<T> value
		using id_t = size_t;//typename Signal_trait<T()>::id_t;
		std::optional<T> value;
		auto operator()() { return this->value; }
		auto & operator()( T const & v_in, id_t ={} ) &
		{
			if( this->value.has_value() )
				this->value.value() |= v_in;
			else
				this->value = v_in;
			return *this;
		}
		auto  operator()( T const & v_in, id_t id={} ) && { return operator()( v_in, id ); }
	};
	template<typename T> struct combiner_all
	{	//combiner_and liefert alle rückgabewerte in std::deque<std::pair<T,id_t>> value. der id_t ist die id, die beim Signal.connect im Connection_Guard geliefert wurde
		using id_t = size_t;//typename Signal_trait<T()>::id_t;
		std::deque<std::pair<T, id_t>> value;
		auto operator()() { return this->value; }
		auto & operator()( T const & v_in, id_t id={} ) &
		{
			this->value.push_back( {v_in,id} );
			return *this;
		}
		auto  operator()( T const & v_in, id_t id={} ) && { return operator()( v_in, id ); }
	};
}


