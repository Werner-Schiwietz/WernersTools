#pragma once

#include <algorithm>

#pragma warning( push,4)

#ifndef ASSERT
#	if !defined(_ASSERT) 
#		define ASSERT(x) assert(x)
#	else
#		define ASSERT(x) _ASSERT(x)
#	endif
#endif

namespace WS
{
	using std::begin;
	using std::end;

	template <typename T> struct _exist_std_begin
	{
	private:
		typedef char (& yes)[1];
		typedef char (& no)[2];

		template<typename U>static yes check(decltype(begin(std::declval<U>()))*);
		template<typename>  static no  check(...);
	public:
		static bool const value = sizeof(check<std::remove_reference_t<T>>(nullptr)) != sizeof(no);

		template<typename T> static auto _begin( T && container )
		{
			return begin( container );
		}
		template<typename T> static auto _end( T && container )
		{
			return end( container );
		}
	};	
}
namespace WS
{
	template <typename T> struct exist_std_begin
	{
	private:
		typedef char (& yes)[1];
		typedef char (& no)[2];

		template<typename U>static yes check(decltype(std::begin(std::declval<U>()))*);
		template<typename>  static no  check(...);
	public:
		static bool const value = sizeof(check<std::remove_reference_t<T>>(nullptr)) != sizeof(no);

		template<typename T> static auto _begin( T && container )
		{
			return std::begin( container );
		}
		template<typename T> static auto _end( T && container )
		{
			return std::end( container );
		}
	};	
	template <typename T> struct exist_global_begin
	{
	private:
		typedef char (& yes)[1];
		typedef char (& no)[2];

		template<typename U>static yes check(decltype(::begin(std::declval<U>()))*);
		template<typename>  static no  check(...);
	public:
		static bool const value = sizeof(check<std::remove_reference_t<T>>(nullptr)) != sizeof(no);
		template<typename T> static auto _begin( T && container )
		{
			return ::begin( container );
		}
		template<typename T> static auto _end( T && container )
		{
			return ::end( container );
		}
	};	
	template <typename T> struct exist__begin
	{
	private:
		typedef char (& yes)[1];
		typedef char (& no)[2];

		template<typename U>static yes check(decltype(begin(std::declval<U>()))*);
		template<typename>  static no  check(...);
	public:
		static bool const value = sizeof(check<std::remove_reference_t<T>>(nullptr)) != sizeof(no);
		template<typename T> static auto _begin( T && container )
		{
			return begin( container );
		}
		template<typename T> static auto _end( T && container )
		{
			return end( container );
		}
	};	
	template <typename T> struct exist_begin
	{
		//using type = exist_std_begin<T>;
		using type = _exist_std_begin<T>;
		static bool const value = type::value;// | exist_global_begin<T>::value | exist__begin<T>::value;
	};
}

namespace WS
{

	template<typename T> struct bereich_t
	{
		T lower;
		T upper;
		bereich_t( T lower, T upper ):upper(upper),lower(lower){ASSERT(lower<upper||!(upper<lower));}
		bool is_in(T value) const { return !(value<this->lower || this->upper<value); }
	};
	template<typename T>bereich_t<T> bereich( T const & lower, T const & upper ){return bereich_t<T>(lower,upper);}//weil bereich(1,2) schoener als bereich_t<int>(1,2) aussieht
	template<typename T>bool operator<( WS::bereich_t<T> const & bereich, T const & value )
	{
		return bereich.upper < value;
	}
	template<typename T>bool operator<( T const & value, WS::bereich_t<T> const & bereich )
	{
		return value < bereich.lower;
	}

	template <typename value_t, typename container_t, typename begin_end_t> bool is_in_container( value_t const & gesucht, container_t const & liste )
	{
		auto b = begin_end_t::_begin(liste);
		auto e = begin_end_t::_end(liste);
		return std::find( b, e, gesucht ) != e;
	}

	template <typename value_t, typename U,bool istContainer=exist_begin<U>::value> struct _is_in
	{
		bool operator()( value_t const & v, U const & constainer ) const
		{
			return is_in_container<value_t,U,exist_begin<U>::type>(v,constainer);
		}
	};
	template <typename value_t, typename U> struct _is_in<value_t, U, false>
	{
		bool operator()( value_t const & v, U const & vergleichoperand ) const
		{
			return v==vergleichoperand;
		}
	};

	template <typename value_t, typename U> bool is_in( value_t const & gesucht, U const & other )
	{
		return _is_in<value_t,U>{}( gesucht, other );
	}

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