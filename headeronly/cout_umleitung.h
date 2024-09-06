#pragma once

//solange ein Cout2Output objekt im code aktiv ist
//wird z.B. cout << "text" << 5 << std::endl; 
//ins Debug Tests fenster umgeleitet
//mit freundlicher unterstützung
//https://gist.github.com/takashyx/937f3a794ad36cd98ec3

#include <string>
#include <sstream>
#include <iostream>

template<typename char_t>
class LoggerStreambuf : public std::basic_streambuf<char_t, std::char_traits<char_t>>
{
public:
	using base_t = std::basic_streambuf<char_t, std::char_traits<char_t>>;
	using int_type = typename base_t::int_type;
	virtual int_type overflow( int_type c = EOF ) 
	{
		static std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t>> buf;
		if( c != EOF )
		{
			if( c != '\n' )
				buf += static_cast<decltype(buf)::traits_type::char_type>( c );
			else
			{
				Logger::WriteMessage( buf.c_str() );
				buf.clear();
			}
		}
		return c;
	}
};
template<typename streambuf_t=LoggerStreambuf<char>>
class Cout2Output
{
	streambuf_t dbgstream;
	std::streambuf *default_stream;

public:
	Cout2Output() {
		default_stream = std::cout.rdbuf( &dbgstream );
	}

	~Cout2Output() {
		std::cout.rdbuf( default_stream );
	}
};
template<typename streambuf_t=LoggerStreambuf<wchar_t>>
class WCout2Output
{
	streambuf_t dbgstream;
	std::wstreambuf *default_stream;

public:
	WCout2Output() {
		default_stream = std::wcout.rdbuf( &dbgstream );
	}

	~WCout2Output() {
		std::wcout.rdbuf( default_stream );
	}
};
