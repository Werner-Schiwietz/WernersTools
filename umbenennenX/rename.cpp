// rename.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <regex>
#include <algorithm>
#include <tuple>
#include <assert.h>
#include <tchar.h>
#include <filesystem>
#include <deque>

#include <Windows.h>//MultiByteToWideChar
#include <codecvt>

#include "..\headeronly\parse_helper.h"

#ifdef _UNICODE
	using TCHAR = wchar_t;
	using string_t = std::wstring;
	#define outstream std::wcout
	using regex_t = std::wregex;
	using match_t = std::wsmatch;
#else
	using TCHAR = char;
	using string_t = std::string;
	#define outstream std::cout
	using regex_t std::regex;
	using match_t = std::smatch;
#endif

//#define _T(x) L ## x


int help()
{
	outstream << _T("rename.exe [-test] [-r] [-v LW:\\Startdir] [+b] sourceFilenameWithWildcard destFilenameWithWildcard") << std::endl;
	outstream << _T("	 by Werner Schiwietz") << std::endl;
	outstream << _T("	 -h|? diese hilfe") << std::endl;
	outstream << _T("	 -v [\"|']LW:\\Pfad[\"|'] das Startverzeichnis, ansonst wird das currentdirectory genommen") << std::endl;
	outstream << _T("	 -r unterordner werden auch durchsucht") << std::endl;
	outstream << _T("	 -test es wird keine veränderung vorgenommen, nur angezeigt, was gemacht werden würde") << std::endl;
	outstream << _T("	 +b die Suchparameter werden mit ausgegeben") << std::endl;

	return 0;
}

namespace WS
{
	template<char_type char_t> auto replace( std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t>> string, char_t const * _replace, char_t const * _with )
	{
		for( auto pos = string.find(_replace); pos != std::string::npos; pos = string.find(_replace, pos + stringlen(_with) ) )
		{
			string.replace(pos, stringlen(_replace), _with );
		}
		return string;
	}

	template<typename to_t,typename from_t> to_t Convert( from_t v ) = delete;
	template<typename type> type Convert( type v )
	{
		return v;
	}
	template<> std::wstring Convert<std::wstring,std::string>( std::string v )
	{
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &v[0], (int)v.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &v[0], static_cast<int>(v.size()), &wstrTo[0], size_needed);
		return wstrTo;
	}
	template<> std::string Convert<std::string,std::wstring>( std::wstring v )
	{
		#pragma warning(suppress:4996)
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		#pragma warning(suppress:4996)
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		#pragma warning(suppress:4996)
		return converterX.to_bytes(v);
	}
}
struct 
{
	string_t string;
	std::vector<bool> group;
} getRegex( TCHAR const * parameter)
{
	if(parameter==nullptr)
		return {};

	std::vector<bool> group;
	string_t s{};
	bool fix=false;
	for(;*parameter; ++parameter)
	{
		switch(*parameter)
		{
		case _T('*'):
			if(fix)
			{
				fix = false;
				s += _T(')');
			}
			group.push_back(false);
			s += _T("(.*)");
			break;
		case _T('?'):
			if(fix)
			{
				fix = false;
				s += _T(')');
			}
			group.push_back(false);
			s += _T("(.?)");
			break;
		default:
			if(not fix)
			{
				group.push_back(true);
				fix = true;
				s += _T('(');
			}
			s += *parameter;
			break;
		}
	}
	if(fix)
	{
		fix = false;
		s += _T(')');
	}

	return {s,group};
}
std::vector<string_t> getRegexReplace( TCHAR const * parameter)
{
	if(parameter==nullptr)
		return {};

	std::vector<string_t> retvalue;
	string_t s;
	for(;*parameter; ++parameter)
	{
		switch(*parameter)
		{
		case _T('*'):
		case _T('?'):
			if(not s.empty())
			{	//default-zweig abschließen
				retvalue.push_back(s);
				s.clear();
			}
			retvalue.push_back(s);//leeren eintrag für wildcard einfügen
			break;
		default:
			s += *parameter;
			break;
		}
	}
	if( not s.empty() )
		retvalue.push_back(s);

	return retvalue;
}

string_t renamedfilename( std::filesystem::path const & filename, auto find, auto replace )
{
	regex_t searchfor( find.string, std::regex_constants::icase );
	match_t match;
	auto fn = static_cast<string_t>(filename.filename());
	if( not std::regex_search( fn, match, searchfor ) )
	{
		return {};
	}

	string_t newfilename{};
	auto iterReplace = replace.begin();
	auto fnStatischenZieltext = [&]()
	{
		while( iterReplace != replace.end() && (*iterReplace).empty() )
			++iterReplace;

		return iterReplace != replace.end() && not (*iterReplace).empty();
	};

	auto iterGroup = find.group.begin();
	auto iter = ++match.begin();//den ersten eintrag(gesamtstring) überspringen
	for( ; iter!=match.end(); ++iter,++iterGroup )
	{
		assert(iterGroup != find.group.end());
		if( not *iterGroup )
			newfilename += *iter;
		else if( fnStatischenZieltext() )
		{
			newfilename += *iterReplace++;
		}
	}
	while( fnStatischenZieltext() )
	{	//hier sollte wir im normalfall nicht rein kommen, sonst passen quell und ziel dateiname nicht zusammen
		newfilename += *iterReplace++;
	}

	return newfilename;
}


int _tmain(int argc, TCHAR *argv[])
{
	MessageBox(NULL,L"hallo",L"xxx",IDOK);

	int counter = 0;
	#pragma region parameter auswerten
	int index = 1;

	bool rekursiv = false;
	bool test = false;
	bool verbose = false;
	std::filesystem::path startdir {};

param:
	if(argc<=index)
		return help();

	auto parse = WS::iterator_access(argv[index]);
	decltype(parse) parse2;
	if( WS::eat_oneof( parse, '-','/' ) )
	{
		if( WS::eat_oneof( parse, 'h','H','?') && parse2.len()==0 )
			return help();
		if( WS::eat_oneof( (parse2=parse), 'v','V') && parse2.len()==0 )
		{
			if(argc<=++index)
				return help();
			parse2 = WS::iterator_access(argv[index]);
			if( auto anfuerung = eat_oneof(parse2,'"','\'') )
			{
				if( auto dir=WS::eat_till(parse2,*anfuerung.begin(),'\\') )
					startdir = string_t{dir.eaten.begin(),dir.eaten.len()};
				else
					return help();
			}
			else
			{
				startdir = static_cast<TCHAR const *>(parse2);
			}

			++index;
			goto param;
		}
		if( WS::eat_oneof( (parse2=parse), 'r','R') && parse2.len()==0 )
		{
			rekursiv = true;
			++index;
			goto param;
		}
		if( WS::eat( (parse2=parse), WS::iterator_access("test")) && parse2.len()==0 )
		{
			test = true;
			++index;
			goto param;
		}
		return help();
	}
	if( WS::eat( parse, '+' ) )
	{
		if( WS::eat_oneof( (parse2=parse), 'b','B') && parse2.len()==0 )
		{
			verbose = true;
			++index;
			goto param;
		}
	}

	if( index + 2 != argc )
		return help();

	auto const finddata = getRegex(argv[index]);
	auto const replace = argv[++index];
	auto const replacedata = getRegexReplace(replace);
	parse = WS::iterator_access(replace);

	if( WS::eat_oneof( parse, '-','/' ) )
		return help();//replace-string ist eine opotion. das ist eine fasche syntax

	if( startdir.empty() )
		startdir = std::filesystem::current_path();

	#pragma endregion

	//irgendein testname
	//auto file = std::filesystem::path{_T("(ARCHIV_20211102)_SOLL_TC1194804.pdf")};

	std::deque<std::filesystem::path> dirs;
	std::deque<std::filesystem::path> files;

subdir:
	for( auto const & direntry : std::filesystem::directory_iterator{startdir} )
	{
		if(direntry.is_directory())
		{
			if(rekursiv)
			{
				dirs.push_back(direntry.path());
			}
		}
		else
			files.push_back(direntry.path());
		//outstream << direntry << (direntry.is_directory() ? _T(" DIR ") : _T(" FILE")) << std::endl;
	}

	for( auto const & file : files )
	{
		if( auto newfilename = renamedfilename( file, finddata,  replacedata ); not newfilename.empty() )
		{
			std::error_code ec{};
			if( not test )
			{
				std::filesystem::rename( file, newfilename, ec );
			}
			if( verbose )
				outstream << _T("  ") << finddata.string << _T(" ") << replace << _T("   ");
			outstream << file << _T(" -> ") << newfilename;
			if( static_cast<bool>(ec) )
				outstream << _T(" ec:") << WS::Convert<string_t>(ec.message());
			outstream << std::endl;
			++counter;
		}
	}

	if( not dirs.empty() )
	{
		startdir = *dirs.begin();
		dirs.pop_front();
		files.clear();
		goto subdir;
	}

	return counter;
}

