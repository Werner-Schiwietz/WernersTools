#pragma once

#include <iostream>
#include <regex>

#ifdef _UNICODE
	using TCHAR = wchar_t;
	using string_t = std::wstring;
	using ostream_t = std::wostream;
	#define outstream std::wcout
	#define errstream std::wcerr
	using regex_t = std::wregex;
	using match_t = std::wsmatch;
#else
	using TCHAR = char;
	using string_t = std::string;
	using ostream_t = std::ostream;
	#define outstream std::cout
	#define errstream std::cerr
	using regex_t = std::regex;
	using match_t = std::smatch;
#endif
