#pragma once
//by Werner Schiwietz

#include <type_traits>	//std::make_unsigned
#include <climits>		//std::numeric_limits

#include "to_underlying.h"

namespace WS
{
	/// <summary>
	/// WS::bits liefert die Anzahl bits eines integral/enum-datentyp
	///		siehe auch std::numeric_limits<integral_type>::digits allerdings liefert WS::bits<T>() und WS::bits<unsigned T> den selben Wert
	/// </summary>
	/// <typeparam name="integral_type"></typeparam>
	/// <returns>integral_type alle bits des datentyps sind auf 1 gesetzt</returns>
	/// 
	template< typename integral_type> constexpr auto bits()
	{
		if constexpr (std::is_enum_v<integral_type> )
		{
			return bits<std::underlying_type_t<integral_type>>();
		}
		else
		{
			static_assert(std::is_integral_v<integral_type>);
			return std::numeric_limits<std::make_unsigned_t<integral_type>>::digits;
		}
	}

	/// <summary>
	/// WS::all_bits_mask liefert den wert des datentyp mit allen bits auf 1 (wie -1)
	/// </summary>
	/// <typeparam name="integral_type">integral/enum-datentyp</typeparam>
	/// <returns></returns>
	template<typename integral_type> constexpr integral_type all_bits_mask()
	{
		static_assert( std::is_integral_v<integral_type> || std::is_enum_v<integral_type> );
		return static_cast<integral_type>( ~ WS::integral_t<integral_type>{0} );
	}
}

namespace WS
{
	template<typename T> T vereinigung( T const & l, T const & r)//liefert vereinigungsmenge von l und r
	{
		return T{to_integral(l) | to_integral(r)};
	}
	template<typename T, typename ... more> T vereinigung( T const & l, T const & r, more const & ... rest)
	{
		return vereinigung( vereinigung(l,r), rest...); 
	}
	template<typename T> T schnittmenge( T const & l, T const & r)//liefert schnittmenge von l und r
	{
		return T{to_integral(l) & to_integral(r)}; 
	}
	template<typename T> T schnitt( T const & l, T const & r)//liefert schnittmenge von l und r
	{
		return schnittmenge(l,r);
	}
	template<typename T> T ohne( T const & l, T const & r) //entfernt r aus l
	{
		return T{to_integral(l) & ~to_integral(r)}; 
	}
	template<typename T> bool enthaelt( T const & l, T const & r) // liefert true, wenn schnittmenge von l und r nicht leer ist
	{
		return !!( to_integral(l) & to_integral(r) );
	}
	template<typename T> bool enthaelt_nicht( T const & l, T const & r) // liefert true, wenn schnittmenge von l und r leer ist
	{
		return !( to_integral(l) & to_integral(r) );
	}
	template<typename T> bool leer( T const & v )//liefert true, wenn der integralwert von v 0 ist
	{
		return !to_integral(v);
	}
	template<typename T> bool nichtleer( T const & v )//liefert false, wenn der integralwert von v 0 ist
	{
		return !!to_integral(v);
	}
}
