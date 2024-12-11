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

#include "..\..\headeronly\parse_helper.h"

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
	outstream << _T("	 --start [\"|']LW:\\Pfad[\"|'] das Startverzeichnis, ansonst wird das currentdirectory genommen") << std::endl;
	outstream << _T("	 -|+r unterordner werden auch durchsucht") << std::endl;
	outstream << _T("	 -|+test es wird keine veränderung vorgenommen, nur angezeigt, was gemacht werden würde") << std::endl;
	outstream << _T("	 -|+v nur Verzeichnisse umbenennen") << std::endl;
	outstream << _T("	 -|+d nur Dateien umbenennen") << std::endl;
	outstream << _T("	 +|-b die Suchparameter werden mit ausgegeben") << std::endl;

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
struct regexdata
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

string_t renamedfilename( std::filesystem::path const & filename, regexdata const & find, auto replace )
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


struct parameter
{
	bool			rekursiv		= false;
	bool			test			= false;
	bool			verbose			= false;
	bool			verzeichnisse	= true;
	bool			dateien			= true;

	string_t		startdir = std::filesystem::current_path();
	TCHAR const *	find_str{};
	TCHAR const *	replace_str{};
};

bool GetParameter( parameter & param, int & aktargvindex, int argc, TCHAR const *argv[] )
{
	auto & index = aktargvindex;

param:
	if(argc<=index)
		return false;

	auto parse = WS::iterator_access(argv[index]);
	decltype(parse) parse2;
	if( auto optchr = WS::eat_oneof( parse, '-','/', '+' ) )
	{
		if( WS::eat_oneof( parse, 'h','H','?') && parse2.len()==0 )
			return false;
		if( WS::eat( (parse2=parse), WS::iterator_access("-start")) && parse2.len()==0 )
		{
			if(argc<=++index)
				return false;
			parse2 = WS::iterator_access(argv[index]);
			if( auto anfuerung = eat_oneof(parse2,'"','\'') )
			{
				if( auto dir=WS::eat_till(parse2,*anfuerung.begin(),'\\') )
					param.startdir = string_t{dir.eaten.begin(),dir.eaten.len()};
				else
					return false;
			}
			else
			{
				param.startdir = static_cast<TCHAR const *>(parse2);
			}

			++index;
			goto param;
		}
		if( WS::eat_oneof( (parse2=parse), 'r','R') && parse2.len()==0 )
		{

			param.rekursiv = *static_cast<TCHAR const *>(optchr)==_T('+');
			++index;
			goto param;
		}
		if( WS::eat( (parse2=parse), WS::iterator_access("test")) && parse2.len()==0 )
		{
			param.test = *static_cast<TCHAR const *>(optchr)==_T('+');
			++index;
			goto param;
		}
		if( WS::eat_oneof( (parse2=parse), 'b','B') && parse2.len()==0 )
		{
			param.verbose = *static_cast<TCHAR const *>(optchr)==_T('+');
			++index;
			goto param;
		}
		if( WS::eat_oneof( (parse2=parse), 'v','V') && parse2.len()==0 )
		{
			param.verzeichnisse = *static_cast<TCHAR const *>(optchr)==_T('+');
			++index;
			goto param;
		}
		if( WS::eat_oneof( (parse2=parse), 'd','D') && parse2.len()==0 )
		{
			param.dateien = *static_cast<TCHAR const *>(optchr)==_T('+');
			++index;
			goto param;
		}
		return false;
	}

	if( index + 2 > argc )
		return false;

	param.find_str = argv[index];
	param.replace_str = argv[++index];

	return true;
}

struct
{
	bool		match{};
	string_t	newname{};

	explicit operator bool() const noexcept { return this->match;} 
	operator string_t() const noexcept{ return newname; }//wenn match==true
}
Rename( std::filesystem::path name, auto const & finddata, auto const & replacedata, auto const & param)
{
	if( auto newname_string = renamedfilename( name, finddata, replacedata ); not newname_string.empty() )
	{
		auto newname = name;
		newname.replace_filename( newname_string );

		std::error_code ec{};
		if( not param.test )
		{
			std::filesystem::rename( name, newname, ec );
		}
		if( param.verbose )
			outstream << _T("  ") << param.find_str << _T(" ") << param.replace_str << _T("   ");
		outstream << name << _T(" -> ") << newname_string;
		if( static_cast<bool>(ec) )
			outstream << _T(" ec:") << WS::Convert<string_t>(ec.message());
		outstream << std::endl;

		if( not param.test )
			return {not static_cast<bool>(ec), name.replace_filename(newname) };
		else 
			return {not static_cast<bool>(ec), name};
	}
	return {false};
}

int _tmain(int argc, TCHAR const *argv[])
{
	std::deque<parameter> paramcontainer;
	paramcontainer.emplace_back();

	{
		int index = 1;
		if(argc<=index)
			return help();

		if( not GetParameter(paramcontainer[0], index, argc, argv) )
			return help();//fehler oder help

		for(;;)
		{
			auto param = paramcontainer.back();//eine kopie
			auto index2 = ++index;
			if( not GetParameter(param, index, argc, argv) )
			{
				if( index!=index2 )
					return help();//fehler oder help
				else
					break; //for
			}
			else 
				paramcontainer.push_back(param);
		}
	}

	//irgendein testname
	//auto file = std::filesystem::path{_T("(ARCHIV_20211102)_SOLL_TC1194804.pdf")};

	int counter = 0;

	std::deque<std::filesystem::path> dirs;
	std::deque<std::filesystem::path> dirsumbenannt;
	std::deque<std::filesystem::path> files;
	std::deque<std::filesystem::path> filesumbenannt;

	std::function<void(size_t)> worker;// nötig wg rekursion
	auto fnworker = [&](size_t index)->void
	{
		auto &		param		= paramcontainer[index];
		regexdata	finddata	= getRegex(param.find_str);
		auto		replacedata = getRegexReplace( param.replace_str);


		for( auto const & direntry : std::filesystem::directory_iterator{param.startdir} )
		{
			if(direntry.is_directory())
			{
				if( param.rekursiv || param.verzeichnisse )
				{
					dirs.push_back(direntry.path());
				}
			}
			else if( param.dateien )
				files.push_back(direntry.path());
		}

		if( param.verzeichnisse )
		{
			for( auto const & dir : dirs )
			{
				if( auto erg = Rename( dir, finddata, replacedata, param ) )
				{
					++counter;
					dirsumbenannt.push_back( static_cast<string_t>(erg) );
				}
			}
		}

		if( param.dateien )
		{
			for( auto const & file : files )
			{
				if( auto erg = Rename( file, finddata,  replacedata, param ) )
				{
					++counter;
					filesumbenannt.push_back( static_cast<string_t>(erg) );
				}
			}
		}

		if( not dirsumbenannt.empty() && index+1 < paramcontainer.size() )
		{
			auto & paramsub = paramcontainer[index+1];
			for( auto & dirumbenannt : dirsumbenannt )
			{
				paramsub.startdir = dirumbenannt;
				files.clear();
				worker(index+1);
			}
		}

		if( not dirs.empty() && param.rekursiv )
		{
			param.startdir = dirs.front();
			dirs.pop_front();
			files.clear();
			worker(index);
		}
	};
	worker = fnworker;
	worker(0);

	return counter;
}

