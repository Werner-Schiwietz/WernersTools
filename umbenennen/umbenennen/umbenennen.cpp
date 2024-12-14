// rename.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <algorithm>
#include <tuple>
#include <assert.h>
#include <tchar.h>
#include <filesystem>
#include <deque>

#include <Windows.h>//MultiByteToWideChar
#include <codecvt>

#include "traits.h"
#include "streamPipe.h"
#include "..\..\headeronly\parse_helper.h"
#include "ConsolenCursorOnOff.h"


//#define _T(x) L ## x


int help()
{
	outstream << _T("rename.exe [-test] [-r] [-v LW:\\Startdir] [+b] sourceFilenameWithWildcard destFilenameWithWildcard") << std::endl;
	outstream << _T("	 by Werner Schiwietz") << std::endl;
	outstream << _T("	 -h|? diese hilfe") << std::endl;
	outstream << _T("	 --start [\"|']LW:\\Pfad[\"|'] das Startverzeichnis, ansonst wird das currentdirectory genommen") << std::endl;
	outstream << _T("	 +|-r unterordner werden auch durchsucht") << std::endl;
	outstream << _T("	 +|-test es wird keine veränderung vorgenommen, nur angezeigt, was gemacht werden würde") << std::endl;
	outstream << _T("	 -|+v Verzeichnisse (nicht) umbenennen") << std::endl;
	outstream << _T("	 -|+d Dateien (nicht) umbenennen") << std::endl;
	outstream << _T("	 +|-b die Suchparameter werden mit ausgegeben") << std::endl;

	return 0;
}

auto streamPipe = WS::make_pipe( std::function<void(pipedata &&)>(pipeworker) );

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

	template<typename to_t,typename from_t> to_t Convert( from_t const & v ) = delete;
	template<typename type> type Convert( type const & v )
	{
		return v;
	}


	template<> std::string Convert<std::string,char const *>( char const * const & var )
	{
		return {var};
	}
	template<> std::string Convert<std::string,wchar_t const *>( wchar_t const * const & var )
	{
		static std::locale loc("");
		auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
		#pragma warning(suppress:4996)
		return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
	}
	template<> std::string Convert<std::string,std::wstring>( std::wstring const & var )
	{
		return Convert<std::string>( var.c_str() );
	}

	template<> std::wstring Convert<std::wstring,wchar_t const *>( wchar_t const * const & var )
	{
		return {var};
	}
	template<> std::wstring Convert<std::wstring,char const *>( char const * const & var )
	{
		static std::locale loc("");
		auto &facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
		#pragma warning(suppress:4996)
		return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
	}
	template<> std::wstring Convert<std::wstring,std::string>( std::string const & var )
	{
		return Convert<std::wstring>( var.c_str() );
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
	auto closefix = [&]()->void
	{
		if(fix)
		{
			fix = false;
			s += _T(')');
		}
	};
	auto openfix = [&]()->bool
	{
		if(not fix)
		{
			fix = true;
			s += _T('(');
			return true;
		}
		return false;
	};
	auto parse = WS::iterator_access(parameter);
	for(;parse.len(); )
	{
		if( WS::eat(parse, _T('*')) )
		{
			closefix();
			group.push_back(false);
			s += _T("(.*)");
		}
		else if( WS::eat(parse, _T('?')) )
		{
			closefix();
			group.push_back(false);
			s += _T("(.)");
		}
		else if( WS::eat(parse, WS::iterator_access(_T(".*"))) )
		{
			closefix();
			group.push_back(false);
			s += _T("(\\..*)?");
		}
		else if( WS::eat(parse, _T('.')) )
		{
			if(openfix())
				group.push_back(true);
			s += _T("\\.");
		}
		else 
		{
			if(openfix())
				group.push_back(true);

			s += *parse.begin();
			WS::eat(parse);
		}
	}
	closefix();

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
	try
	{
		if( not std::regex_search( fn, match, searchfor ) )
		{
			return {};
		}
	}
	catch(std::exception &e)
	{
		std::basic_stringstream<TCHAR> stringstream;
		stringstream << _T("std::regex_search '") << fn << _T("' exception  : ") << WS::Convert<string_t>( std::string(e.what()) ) << std::endl;
		streamPipe.AddData( pipedata{stringstream.str(),errstream} );
	}
	catch(...)
	{
		std::basic_stringstream<TCHAR> stringstream;
		stringstream << _T("std::regex_search exception: ") << std::endl;
		streamPipe.AddData( pipedata{stringstream.str(),errstream} );
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

	while( *newfilename.rbegin() == _T('.') )
		newfilename.erase(newfilename.length()-1);
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
	static std::basic_string<TCHAR>::size_type last_line_string_len = 0;
	if( auto newname_string = renamedfilename( name, finddata, replacedata ); not newname_string.empty() )
	{
		auto newname = name;
		newname.replace_filename( newname_string );

		std::error_code ec{};
		if( not param.test )
		{
			std::filesystem::rename( name, newname, ec );
		}
		std::basic_stringstream<TCHAR> stringstream;
		if( param.verbose )
			stringstream << _T("  ") << param.find_str << _T( " " ) << param.replace_str << _T( "   " );
		stringstream << name.c_str() << _T(" -> ") << newname_string;
		if( static_cast<bool>(ec) )
			stringstream << _T(" ec:") << WS::Convert<string_t>(ec.message());

		decltype(auto) str = stringstream.str();
		if( str.length() < last_line_string_len )
			stringstream << std::basic_string<TCHAR>( (last_line_string_len - str.length()), _T(' ') );//warum geht ctor hier nicht mit {} ??

		stringstream << std::endl;
		last_line_string_len = 0;
		if( static_cast<bool>(ec) )
		{
			//errstream << stringstream.str();
			streamPipe.AddData( pipedata{stringstream.str(),errstream} );
		}
		else
		{
			//outstream << stringstream.str();
			streamPipe.AddData( pipedata{stringstream.str(),outstream} );
		}

		if( not param.test )
			return {not static_cast<bool>(ec), name.replace_filename(newname) };
		else 
			return {not static_cast<bool>(ec), name};
	}

	string_t str = name;
	auto len = str.length();
	if( len < last_line_string_len )
		str += string_t( last_line_string_len - len, _T(' ') );

	//outstream << str << _T('\r');
	streamPipe.AddData( pipedata{str + _T('\r'),outstream,true} );
	last_line_string_len = len;
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

	ConsolenCursor::OnOff(false);


	int counter = 0;

	std::deque<std::filesystem::path> dirsumbenannt;
	std::deque<std::filesystem::path> filesumbenannt;

	std::function<void(size_t)> worker;// nötig wg rekursion
	auto fnworker = [&](size_t index)->void
	{
		std::deque<std::filesystem::path> dirs;
		std::deque<std::filesystem::path> dirslokal;
		std::deque<std::filesystem::path> fileslokal;

		auto &		param		= paramcontainer[index];
		regexdata	finddata	= getRegex(param.find_str);
		auto		replacedata = getRegexReplace( param.replace_str);


		try
		{
			for( auto const & direntry : std::filesystem::directory_iterator{param.startdir} )
			{
				if(direntry.is_directory())
				{
					if( param.rekursiv || param.verzeichnisse )
					{
						dirs.push_back(direntry.path());
						dirslokal.push_back(direntry.path());
					}
				}
				else if( param.dateien )
				{
					fileslokal.push_back(direntry.path());
				}
			}
		}
		catch(std::exception &e)
		{
			std::basic_stringstream<TCHAR> stringstream;
			stringstream  << _T("exception: ") << WS::Convert<string_t>( e.what() ) << std::endl;
			streamPipe.AddData( pipedata{stringstream.str(),errstream} );
		}
		catch(...)
		{
			std::basic_stringstream<TCHAR> stringstream;
			stringstream << _T("std::filesystem::directory_iterator exception: ") << std::endl;
			streamPipe.AddData( pipedata{stringstream.str(),errstream} );
		}

		if( param.verzeichnisse )
		{
			for( auto const & dir : dirslokal )
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
			for( auto const & file : fileslokal )
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
				worker(index+1);
			}
		}

		if( not dirs.empty() && param.rekursiv )
		{
			for( auto dir : dirs )
			{
				auto startdir = param.startdir;
				param.startdir = dir;
				worker(index);
				param.startdir = startdir;
			}
		}
	};
	worker = fnworker;
	worker(0);

	ConsolenCursor::OnOff(true);
	return counter;
}

