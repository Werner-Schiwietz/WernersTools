#pragma once
//Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

#include <string>

#include "Ptr_Array.h"


//gibt mittels #PRAGMA COMPILEINFO(Text ohne "") eine Meldung während des compilierens ins OUTPUT-Fenster mit Dateinamen und Zeilennummer aus
#define LINE_STRING2(x) #x				//macht aus der zahl einen sting
#define LINE_STRING1(x) LINE_STRING2(x) //nötig, damit __LINE__ zur Zahl wird
#define _LINE_ LINE_STRING1(__LINE__)	//
#define COMPILEINFO(textWithoutQuotes)      message(__FILE__"("_LINE_") : " #textWithoutQuotes)

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

#define _CHAR8(x) x
#define _CHAR16(x) WIDEN(x)


typedef char	CHAR8;
typedef wchar_t CHAR16;

inline bool ist_digit( char ch )
{
	return !!isdigit( (int)(unsigned char)ch );
}
inline bool ist_digit( wchar_t ch )
{
	return !!iswdigit( static_cast<wint_t>(ch) );
}
inline size_t stringlen( char const * psz )
{
	return strlen(psz);
}
inline size_t stringlen( wchar_t const * psz )
{
	return wcslen(psz);
}
inline errno_t stringcpy_s( char *strDestination, size_t numberOfElements, const char *strSource )
{
	return strcpy_s( strDestination, numberOfElements, strSource );
}
inline errno_t stringcpy_s( wchar_t *strDestination, size_t numberOfElements, const wchar_t *strSource )
{
	return wcscpy_s( strDestination, numberOfElements, strSource );
}

inline char* stringncpy_s( WS::ptr_array<char*> &pdest, char const * psource ) 
{
	strncpy_s( pdest, pdest.ElementCount(), psource, _TRUNCATE );
	return pdest;
}
inline wchar_t* stringncpy_s( WS::ptr_array<wchar_t*> &pdest, wchar_t const * psource ) 
{
	wcsncpy_s( pdest, pdest.ElementCount(), psource, _TRUNCATE );
	return pdest;
}
inline char* stringncpy_s( WS::ptr_array<char*> &pdest, char const * psource, size_t charcount ) 
{
	strncpy_s( pdest, pdest.ElementCount(), psource, charcount );
	return pdest;
}
inline wchar_t* stringncpy_s( WS::ptr_array<wchar_t*> &pdest, wchar_t const * psource, size_t charcount ) 
{
	wcsncpy_s( pdest, pdest.ElementCount(), psource, charcount );
	return pdest;
}
inline errno_t stringncpy_s( char * pdest, size_t numberOfElements, char const * psource, size_t charcount ) 
{
	return strncpy_s( pdest, numberOfElements, psource, charcount );//die sind so doof, geht nur wenn nullterminiert ist, anders als mit strncpy
}
inline errno_t stringncpy_s( wchar_t * pdest, size_t numberOfElements, wchar_t const * psource, size_t charcount ) 
{
	return wcsncpy_s( pdest, numberOfElements, psource, charcount );//die sind so doof, geht nur wenn nullterminiert ist, anders als mit strncpy
}

[[deprecated("deprecated: stringncpy_s soll sicherer sein")]]
inline char* stringncpy( char * pdest, char const * psource, size_t charcount ) 
{
#	pragma warning(suppress:4996)
	return strncpy( pdest, psource, charcount );//die sind so doof, geht nur wenn nullterminiert ist, anders als mit strncpy
}
[[deprecated("deprecated: stringncpy_s soll sicherer sein")]]
inline wchar_t* stringncpy( wchar_t * pdest, wchar_t const * psource, size_t charcount ) 
{
#	pragma warning(suppress:4996)
	return wcsncpy( pdest, psource, charcount );//die sind so doof, geht nur wenn nullterminiert ist, anders als mit strncpy
}

[[deprecated("deprecated: stringncpy_s soll sicherer sein")]]
inline char* stringcpy( char * pdest, char const * psource ) 
{
#	pragma warning(suppress:4996)
	return strcpy( pdest, psource );
}
[[deprecated("deprecated: stringncpy_s soll sicherer sein")]]
inline wchar_t* stringcpy( wchar_t * pdest, wchar_t const * psource ) 
{
#	pragma warning(suppress:4996)
	return wcscpy( pdest, psource );
}

inline int stringicmp( char const * sz1, char const * sz2 )
{
	return _stricmp( sz1, sz2 );
}
inline int stringicmp( wchar_t const * sz1, wchar_t const * sz2 )
{
	return _wcsicmp( sz1, sz2 );
}

inline int stringcmp( char const * sz1, char const * sz2 )
{
	return strcmp( sz1, sz2 );
}
inline int stringcmp( wchar_t const * sz1, wchar_t const * sz2 )
{
	return wcscmp( sz1, sz2 );
}
inline int stringncmp( char const * sz1, char const * sz2, size_t count )
{
	return strncmp( sz1, sz2, count );
}
inline int stringncmp( wchar_t const * sz1, wchar_t const * sz2, size_t count )
{
	return wcsncmp( sz1, sz2, count );
}

inline char* stringstr( char* str, char const * search )
{
	return strstr( str, search );
}
inline char const * stringstr( char const * str, char const * search )
{
	return strstr( str, search );
}
inline wchar_t* stringstr( wchar_t* str, wchar_t const * search )
{
	return wcsstr( str, search );
}
inline wchar_t const * stringstr( wchar_t const * str, wchar_t const * search )
{
	return wcsstr( str, search );
}

inline int stringtoi( char const * psz )
{
	//#pragma COMPILEINFO(TODO Warning 4996) 
	return atoi( psz );
}
inline int stringtoi( wchar_t const * psz )
{
	//#pragma COMPILEINFO(TODO Warning 4996) 
	return _wtoi( psz );
}

inline char* itostring_s(    int value, char *psz, size_t charcount, int radix )
{
	_itoa_s( value, psz, charcount, radix );
	return psz;
}
inline wchar_t* itostring_s(    int value, wchar_t *psz, size_t charcount, int radix )
{
	_itow_s( value, psz, charcount, radix );
	return psz;
}
template<size_t size> inline char* itostring_s(    int value, char (&sz)[size], int radix )
{
	_itoa_s( value, sz, size, radix );
	return sz;
}
template<size_t size> inline wchar_t* itostring_s(    int value, wchar_t (&sz)[size], int radix )
{
	_itow_s( value, sz, size, radix );
	return sz;
}

namespace WS
{
	inline bool isspace( char ch )
	{
		return !!::isspace( ch );
	}
	inline bool isspace( char ch, _locale_t _Locale )
	{
		return !!::_isspace_l( ch, _Locale );
	}
	inline bool isspace( wchar_t ch )
	{
		return !!::iswspace( ch );
	}
	inline bool isspace( wchar_t ch, _locale_t _Locale )
	{
		return !!::_iswspace_l( ch, _Locale );
	}
	//wer will schon boost
	template<typename char_t> std::basic_string<char_t> replace_all(std::basic_string<char_t> str, char_t const *from, char_t const * to)
	{
		size_t start_pos;
		if( (start_pos = str.find(from, 0)) != std::basic_string<char_t>::npos )
		{
			size_t repalcelen	= stringlen(from);
			size_t tolen		= stringlen(to);
			do
			{
				str.replace(start_pos, repalcelen, to);
				start_pos += tolen; // Handles case where 'to' is a substring of 'from'
			}while( (start_pos = str.find(from, start_pos)) != std::string::npos );
		}
		return str;
	}

	inline char toupper( char ch )
	{
		return static_cast<char>( ::toupper( static_cast<int>(ch) ) );
	}
	inline wchar_t toupper( wchar_t ch )
	{
		return static_cast<wchar_t>( ::towupper( static_cast<wint_t>(ch) ) );
	}
	template<typename iterator> iterator toupper( iterator first, iterator last )
	{
		for( auto iter=first; iter!=last; ++iter )
			(*iter) = static_cast<puretype_t<decltype(*iter)>>( WS::toupper( *iter ) );
		return first;
	}
	template<typename chartype> chartype * toupper( chartype * psz )
	{
		return WS::toupper( psz, psz + stringlen( psz ) );
	}

	inline char tolower( char ch )
	{
		return static_cast<char>( ::tolower( static_cast<int>(ch) ) );
	}
	inline wchar_t tolower( wchar_t ch )
	{
		return static_cast<wchar_t>( ::towlower( static_cast<wint_t>(ch) ) );
	}
	template<typename iterator> iterator tolower( iterator first, iterator last )
	{
		for( auto iter=first; iter!=last; ++iter )
			(*iter) = static_cast<puretype_t<decltype(*iter)>>( WS::tolower( *iter ) );
		return first;
	}
	template<typename chartype> chartype * tolower( chartype * psz )
	{
		return WS::tolower( psz, psz + stringlen( psz ) );
	}
}

namespace WS
{
#	if defined(__CSTRINGT_H__)//#include <atlstr.h>
		//template< typename BaseType, class StringTraits> auto begin( CStringT<BaseType, StringTraits> & r )
		//{
		//	return r.GetString();
		//}
		template< typename BaseType, class StringTraits> auto begin( CStringT<BaseType, StringTraits> const & r )
		{
			return r.GetString();
		}
		template< typename BaseType, class StringTraits>
		[[deprecated( "return value Char-pointer bei rvalue koennte wegen der lifetime des CStringT problematisch werden" )]]
		auto begin( CStringT<BaseType, StringTraits> && r )
		{
			return r.GetString();
		}
		//template< typename BaseType, class StringTraits> auto end( CStringT<BaseType, StringTraits> & r )
		//{
		//	return r.GetString()+r.GetLength();
		//}
		template< typename BaseType, class StringTraits> auto end( CStringT<BaseType, StringTraits> const & r )
		{
			return r.GetString()+r.GetLength();
		}
		template< typename BaseType, class StringTraits>
		[[deprecated( "return value Char-pointer bei rvalue koennte wegen der lifetime des CStringT problematisch werden" )]]
		auto end( CStringT<BaseType, StringTraits> && r )
		{
			return r.GetString()+r.GetLength();
		}
#	endif
}
