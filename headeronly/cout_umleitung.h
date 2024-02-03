#pragma once

//solange ein Cout2Output objekt im code aktiv ist
//wird z.B. cout << "text" << 5 << std::endl; 
//ins Debug Tests fenster umgeleitet
//mit freundlicher unterstützung
//https://gist.github.com/takashyx/937f3a794ad36cd98ec3

#include <string>
#include <sstream>
#include <iostream>

class LoggerStreambuf : public std::streambuf
{

public:
	virtual int_type overflow( int_type c = EOF ) {
		static std::string buf;
		if( c != EOF )
		{
			if( c != '\n' )
				buf += static_cast<char>(c);
			else
			{
				Logger::WriteMessage( buf.c_str() );
				buf.clear();
			}
		}
		return c;
	}
};
template<typename streambuf_t=LoggerStreambuf>
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
