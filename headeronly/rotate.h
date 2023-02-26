#pragma once

#include <type_traits>	//std::make_unsigned
#include <climits>		//std::numeric_limits

namespace WS
{
	template< typename integral_type>
	constexpr auto bits()
	{
		static_assert(std::is_integral_v<integral_type>);
		if constexpr (std::is_signed_v<integral_type>)
			return std::numeric_limits<integral_type>::digits+1;
		return std::numeric_limits<integral_type>::digits;
	}

	template<typename integral_type> integral_type rotr( integral_type value, int bits );
	template<typename integral_type> integral_type rotl( integral_type value, int bits )
	{
		if( bits < 0 ) return rotr(value,-bits);
		if( !value ) return value;

		while( bits >= WS::bits<integral_type>() )
			bits -= WS::bits<integral_type>();

		auto v1 = static_cast<integral_type>(value << bits);
		auto v2 = static_cast<integral_type>(static_cast<std::make_unsigned_t<integral_type>>(value) >> (WS::bits<integral_type>() - bits));
		auto v3 = v1 | v2;
		return v3;
	}
	template<typename integral_type> integral_type rotr( integral_type value, int bits )
	{
		if( bits < 0 ) return rotl(value,-bits);
		if( !value ) return value;

		while( bits >= WS::bits<integral_type>() )
			bits -= WS::bits<integral_type>();

		auto v1 = static_cast<integral_type>(static_cast<std::make_unsigned_t<integral_type>>(value) >> bits);
		auto v2 = static_cast<integral_type>(value << (WS::bits<integral_type>() - bits));
		auto v3 = v1 | v2;
		return v3;
	}
}
