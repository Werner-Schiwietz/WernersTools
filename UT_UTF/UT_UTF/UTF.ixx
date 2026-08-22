module;

export module UTF;

export import <type_traits>;
export import <string>;
export import <array>;
export import <stdexcept>;

export template<typename T>
concept UTFChar = std::is_same_v<T,char8_t> || std::is_same_v<T,char16_t> || std::is_same_v<T,char32_t> || std::is_same_v<T,wchar_t>;

export enum class UTFError 
{
	None,
	InvalidSequence,
	IncompleteSequence,
	OverlongEncoding
};

export struct UTF32Result 
{
	char32_t codepoint = 0;
	UTFError error = UTFError::None;
	size_t units_consumed = 0;
};

export UTF32Result UTF8toUTF32( char8_t const *&str ) 
{

	char8_t const * instr = str;
	auto make_error = [&]( UTFError err ) -> UTF32Result 
	{
		UTF32Result result{.error=err};
		str = instr;
		return result;
	};

	if( not str or *str == u8'\0' ) 
	{
		return make_error( UTFError::IncompleteSequence );
	}

	UTF32Result result{};
	// 1-Byte Sequenz (ASCII: 0xxxxxxx)
	if( (*str & 0x80) == 0x00 )
	{
		result.codepoint = *str++;
		result.units_consumed = 1;
	}
	// 2-Byte Sequenz (110xxxxx)
	else if( (*str & 0xE0) == 0xC0 ) 
	{
		result.units_consumed = 2;
		result.codepoint = *str++ & 0x1F;
	}
	// 3-Byte Sequenz (1110xxxx)
	else if( (*str & 0xF0) == 0xE0 ) 
	{
		result.units_consumed = 3;
		result.codepoint = *str++ & 0x0F;
	}
	// 4-Byte Sequenz (11110xxx)
	else if( (*str & 0xF8) == 0xF0 ) 
	{
		result.units_consumed = 4;
		result.codepoint = *str++ & 0x07;
	}
	else 
	{
		return make_error( UTFError::InvalidSequence );
	}

	// Folgebytes einlesen und validieren
	for( auto i = result.units_consumed; i --> 1; )
	{
		// Prüfen, ob der String unerwartet endet
		if( *str == u8'\0' ) 
		{
			return make_error( UTFError::InvalidSequence );
		}

		// Jedes Folgebyte muss das Format 10xxxxxx haben (0x80 bis 0xBF)
		if((*str & 0xC0) != 0x80)
		{
			return make_error( UTFError::InvalidSequence );
		}

		// Bits an den Codepoint anhängen (jeweils 6 Bits herüberschieben)
		result.codepoint = (result.codepoint << 6) | (*str++ & 0x3F);
	}

	// Overlong-Encoding Schutz (Sicherheitsprüfung nach Unicode-Standard)
	if( (result.units_consumed == 2 and result.codepoint < 0x80) or
		(result.units_consumed == 3 and result.codepoint < 0x0800) or
		(result.units_consumed == 4 and result.codepoint < 0x010000) or
		(result.codepoint > 0x10FFFF) ) // Ungültiger Unicode-Bereich
	{
		return make_error( UTFError::OverlongEncoding );
	}

	// Erfolg: Pointer um gelesene Bytes weiterrücken
	return result;
}

export UTF32Result UTF16toUTF32( char16_t const *&str )
{
	char16_t const * instr = str;
	auto make_error = [&]( UTFError err ) -> UTF32Result
	{
		UTF32Result result{.error = err};
		str = instr;
		return result;
	};

	if( not str || *str == u'\0' )
	{
		return make_error( UTFError::IncompleteSequence );
	}

	UTF32Result result{};
	char16_t first = *str++;

	// Unpaired low surrogate -> invalid
	if( first >= 0xDC00 && first <= 0xDFFF )
	{
		return make_error( UTFError::InvalidSequence );
	}

	// High surrogate -> must be followed by low surrogate
	if( first >= 0xD800 && first <= 0xDBFF )
	{
		if( *str == u'\0' )
		{
			return make_error( UTFError::IncompleteSequence );
		}

		char16_t second = *str;
		if(not(second >= 0xDC00 && second <= 0xDFFF))
		{
			return make_error( UTFError::InvalidSequence );
		}

		++str; // consume low surrogate
		char32_t hi = first - 0xD800u;
		char32_t lo = second - 0xDC00u;
		result.codepoint = ((hi << 10) | lo) + 0x10000u;
		result.units_consumed = 2; // 2 UTF-16 code units consumed (surrogate pair)
		return result;
		}

	// BMP character (not surrogate)
	result.codepoint = static_cast<char32_t>(first);
	result.units_consumed = 1; // 1 UTF-16 code unit consumed (BMP)
	return result;
}

template<typename> constexpr bool dependent_false = false;

export struct UTF_convert_error : std::runtime_error
{
	using base_t = std::runtime_error;
	using base_t::base_t;
};


// Small, stack-allocated UTF-8 result for a single codepoint (max 4 bytes + NUL)
export struct UTF8Small
{
	std::array<char8_t, 5> buf{}; // last element is NUL
	size_t len = 0;
};
export struct UTF16Small
{
	std::array<char16_t, 3> buf{}; // last element is NUL
	size_t len = 0;
};

export UTF8Small UTF32ToUTF8Small(char32_t cp)
{
	UTF8Small out{};

	if(cp > 0x10FFFFu) 
		throw UTF_convert_error("invalid utf-char");

	if(cp <= 0x7Fu)
	{
		out.buf[0] = static_cast<char8_t>(cp);
		out.len = 1;
	}
	else if(cp <= 0x7FFu)
	{
		out.buf[0] = static_cast<char8_t>(0xC0 | ((cp >> 6) & 0x1Fu));
		out.buf[1] = static_cast<char8_t>(0x80 | (cp & 0x3Fu));
		out.len = 2;
	}
	else if(cp <= 0xFFFFu)
	{
		out.buf[0] = static_cast<char8_t>(0xE0 | ((cp >> 12) & 0x0Fu));
		out.buf[1] = static_cast<char8_t>(0x80 | ((cp >> 6) & 0x3Fu));
		out.buf[2] = static_cast<char8_t>(0x80 | (cp & 0x3Fu));
		out.len = 3;
	}
	else
	{
		out.buf[0] = static_cast<char8_t>(0xF0 | ((cp >> 18) & 0x07u));
		out.buf[1] = static_cast<char8_t>(0x80 | ((cp >> 12) & 0x3Fu));
		out.buf[2] = static_cast<char8_t>(0x80 | ((cp >> 6) & 0x3Fu));
		out.buf[3] = static_cast<char8_t>(0x80 | (cp & 0x3Fu));
		out.len = 4;
	}

	return out;
}

export UTF16Small UTF32ToUTF16Small( char32_t cp )
{
	UTF16Small out{};
	if( cp > 0x10FFFFu ) 
		throw UTF_convert_error( "invalid utf-char" );

	if( cp <= 0xFFFFu )
	{
		if( cp >= 0xD800u && cp <= 0xDFFFu ) 
			throw UTF_convert_error( "reserved surrogate range" );

		out.buf[0] = static_cast<char16_t>(cp);
		out.len = 1;
	}
	else
	{
		char32_t v = cp - 0x10000u;
		char16_t hi = static_cast<char16_t>((v >> 10) + 0xD800u);
		char16_t lo = static_cast<char16_t>((v & 0x3FFu) + 0xDC00u);
		out.buf[0] = hi;
		out.buf[1] = lo;
		out.len = 2;
	}
	return out;
}


export template<UTFChar char_t>
std::basic_string<char_t> Convert( char_t const * str )
{
	return {str};
}

export template<UTFChar RetT, UTFChar InT>
	requires (!std::is_same_v<RetT, InT>)
std::basic_string<RetT> Convert(InT const * str) = delete;

template<> std::basic_string<char8_t> Convert<char8_t, char16_t>( char16_t const * str )
{
	if( not str )
		return {};

	std::basic_string<char8_t> out;
	
	for( auto p = str; *p; ++p )
	{
		UTF32Result r = UTF16toUTF32( p );
		if( r.error != UTFError::None )
		{
			throw UTF_convert_error( "invalid utf16-input" );
		}
		auto utf8 = UTF32ToUTF8Small( r.codepoint );
		for( size_t i = 0; i < utf8.len; ++i )
		{
			out.push_back( utf8.buf[i] );
		}
	}
	return out;
}
template<> std::basic_string<char32_t> Convert<char32_t, char16_t>( char16_t const * str )
{
	if( not str ) 
		return {};

	std::basic_string<char32_t> out;
	
	for( auto p = str; *p; ++p )
	{
		UTF32Result r = UTF16toUTF32( p );
		if( r.error != UTFError::None )
		{
			throw UTF_convert_error( "invalid utf16-input" );
		}
		
		out.push_back( r.codepoint );
	}
	return out;
}
template<> std::basic_string<char8_t> Convert<char8_t, wchar_t>(wchar_t const * str)
{
	auto p = reinterpret_cast<char16_t const *>(str);
	return Convert<char8_t>( p );
}
template<> std::basic_string<char32_t> Convert<char32_t, wchar_t>(wchar_t const * str)
{
	auto p = reinterpret_cast<char16_t const *>(str);
	return Convert<char32_t>(p);
}

template<> std::basic_string<char16_t> Convert<char16_t,char8_t>(char8_t const * str)
{
	if( not str ) 
		return {};

	std::basic_string<char16_t> out;
	
	for( auto p = str; *p; ++p )
	{
		UTF32Result r = UTF8toUTF32(p);
		if(r.error != UTFError::None)
		{
			throw UTF_convert_error( "invalid utf8-input" );
		}

		auto utf16 = UTF32ToUTF16Small(r.codepoint);
		for( size_t i = 0; i < utf16.len; ++i )
		{
			out.push_back( utf16.buf[i] );
		}
	}
	return out;
}
template<> std::basic_string<wchar_t>  Convert<wchar_t,char8_t>(char8_t const * str)
{
	if( not str )
		return {};

	std::basic_string<wchar_t> out;

	for( char8_t const *p = str; *p; ++p )
	{
		UTF32Result r = UTF8toUTF32( p );
		if( r.error != UTFError::None )
		{
			throw UTF_convert_error( "invalid utf8-input" );
		}

		if constexpr( sizeof( wchar_t ) == 2 )
		{
			auto utf16 = UTF32ToUTF16Small( r.codepoint );
			for( size_t i = 0; i < utf16.len; ++i )
			{
				out.push_back( utf16.buf[i] );
			}
		}
		else //if constexpr( sizeof( wchar_t ) == 4 )
		{
			out.push_back( static_cast<wchar_t>( r.codepoint ) );
		}
	}
	return out;
}
template<> std::basic_string<char32_t> Convert<char32_t,char8_t>(char8_t const * str)
{
	if( not str ) 
		return {};

	std::basic_string<char32_t> out;
	
	for( auto p = str; *p; ++p )
	{
		UTF32Result r = UTF8toUTF32(p);
		if(r.error != UTFError::None)
		{
			throw UTF_convert_error( "invalid utf8-input" );
		}

		out.push_back( r.codepoint );
	}
	return out;
}

template<> std::basic_string<char8_t> Convert<char8_t, char32_t>( char32_t const * str )
{
	if( not str )
		return {};

	std::basic_string<char8_t> out;
	
	for( auto p = str; *p; ++p )
	{
		auto utf8 = UTF32ToUTF8Small( *p );
		for( size_t i = 0; i < utf8.len; ++i )
		{
			out.push_back( utf8.buf[i] );
		}
	}
	return out;
}
template<> std::basic_string<char16_t> Convert<char16_t, char32_t>( char32_t const * str )
{
	if( not str )
		return {};

	std::basic_string<char16_t> out;
	
	for( auto p = str; *p; ++p )
	{
		auto utf16 = UTF32ToUTF16Small( *p );
		for( size_t i = 0; i < utf16.len; ++i )
		{
			out.push_back( utf16.buf[i] );
		}
	}
	return out;
}
template<> std::basic_string<wchar_t>  Convert<wchar_t,char32_t>(char32_t const * str)
{
	if( not str ) 
		return {};

	std::basic_string<wchar_t> out;
	
	for( auto p = str; *p; ++p )
	{
		if constexpr ( sizeof(wchar_t) == 2 )
		{
			auto utf16 = UTF32ToUTF16Small(*p);
			for( size_t i = 0; i < utf16.len; ++i )
			{
				out.push_back( utf16.buf[i] );
			}
		}
		else //if constexpr( sizeof( wchar_t ) == 4 )
		{
			out.push_back( static_cast<wchar_t>(*p) );
		}
	}
	return out;
}
