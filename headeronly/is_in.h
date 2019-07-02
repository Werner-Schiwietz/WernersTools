#pragma once

#include <algorithm>

#pragma warning( push,4)

namespace WP
{

	template<typename T> struct bereich_t
	{
		T lower;
		T upper;
		bereich_t( T lower, T upper ):upper(upper),lower(lower){ASSERT(lower<upper||!(upper<lower));}
		bool is_in(T value) const { return !(value<this->lower || this->upper<value); }
	};
	template<typename T>bereich_t<T> bereich( T const & lower, T const & upper ){return bereich_t<T>(lower,upper);}//weil bereich(1,2) schoener als bereich_t<int>(1,2) aussieht
	template<typename T>bool operator<( WP::bereich_t<T> const & bereich, T const & value )
	{
		return bereich.upper < value;
	}
	template<typename T>bool operator<( T const & value, WP::bereich_t<T> const & bereich )
	{
		return value < bereich.lower;
	}

	template <typename value_t, typename container_t> bool is_in_container( value_t const & gesucht, container_t const & liste )
	{
		return std::find( liste.begin(), liste.end(), gesucht ) != liste.end();
	}

	template <typename value_t, typename container_t> bool is_in( value_t const & gesucht, container_t const & liste )
	{
		return is_in_container( gesucht, liste );
	}
	////wird nicht gebraucht, wird aufgeloest von  "template <typename value_t, typename container_t> bool is_in( value_t const & gesucht, container_t const & liste )"
	//template <typename value_t> bool is_in( value_t const & gesucht, std::initializer_list<value_t> const & liste )
	//{
	//	return is_in_container( gesucht, liste );
	//}
	template<typename value_t, size_t size> bool is_in( value_t const & value, value_t const (&values)[size] )
	{
		for( auto const & item : values )
			if( value == item )
				return true;
		return false;
	}	
	template<typename value_t> bool is_in( value_t const & value, bereich_t<value_t> const & bereich )
	{
		return bereich.is_in( value );
	}
	template<typename value_t> bool is_in( value_t const & value, value_t const & vergleichoperand )
	{
		return value == vergleichoperand;
	}
	template<typename value_t, typename vergleichoperand_t, typename... others> bool  is_in( value_t const & value, vergleichoperand_t const & vergleichoperand, others const & ... Rest )
	{
		return is_in( value, vergleichoperand ) || is_in( value, Rest ... );
	}

}

#pragma warning( pop )