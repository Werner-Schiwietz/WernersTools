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
#include <vector>

#include "Ptr_Array.h"
#include "is_char_t.h"
#include "dependent_false.h"
#include "to_underlying.h"

#include <cwctype>
#include <cctype>

#if _HAS_CXX20
    template<typename T> concept char_type = std::is_same_v<T,char> || std::is_same_v<T,wchar_t> ;
    template<typename T> concept integral_type = std::is_integral_v<T>;
#endif

//gibt mittels #PRAGMA COMPILEINFO(Text ohne "") eine Meldung während des compilierens ins OUTPUT-Fenster mit Dateinamen und Zeilennummer aus
#define LINE_STRING2(x) #x				//macht aus der zahl einen sting
#define LINE_STRING1(x) LINE_STRING2(x) //nötig, damit __LINE__ zur Zahl wird
#define _LINE_ LINE_STRING1(__LINE__)	//
#define COMPILEINFO(textWithoutQuotes)      message(__FILE__"("_LINE_") : " #textWithoutQuotes)

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

//#define _CHAR8(x) x
//#define _CHAR16(x) WIDEN(x)

//using CHAR8 = char;
//using CHAR16 = wchar_t;

struct rettype_digit
{
    bool is=false;
    unsigned int value{};

    rettype_digit(int value):value(value),is(true){}
    rettype_digit(){}

    operator bool() const{ return is;}
    bool operator !() const{ return !is;}
};

template<typename char_t> struct digit_range
{
    unsigned int    lower;
    unsigned int    upper;
    char_t          start_char;

    digit_range( unsigned int lower, unsigned int upper, char_t start_char)
        : lower(lower)
        , upper(upper)
        , start_char(start_char){}
};

template<unsigned int radix, typename char_t>  auto const & digit_def_ranges()
{
    if constexpr ( radix <= 10 )
    {
        static std::vector<digit_range<char_t>> const v { digit_range<char_t>{0,10,char_t{'0'}} };
        return v;
    }
    else
    {
        static std::vector<digit_range<char_t>> const v{ digit_range<char_t>{0,10,char_t{'0'}}, digit_range{10,36,char_t{'A'}}, digit_range{10,36,char_t{'a'}} };
        return v;
    }
}

template<unsigned int radix=10,typename char_t, typename ranges_t> rettype_digit digit( char_t ch, ranges_t const & ranges )
{
    static_assert(radix>0 && radix<=36,"radix out of range");
    //static_assert(std::is_same_v<char_t,wchar_t> || std::is_same_v<char_t,char> || std::is_same_v<char_t,signed char> || std::is_same_v<char_t,unsigned char>);//wegen char - char = int weg gemacht

    for( auto range : ranges )
    {
        if( range.lower < radix )
        {
            if( ch>=char_t{range.start_char} && ch<char_t(range.start_char+range.upper) )
            {
                rettype_digit erg = ch - range.start_char + range.lower;
                if( erg.value < radix )
                    return erg;
            }
        }
    }
    return {};//false
}
template<unsigned int radix=10,typename char_t> rettype_digit digit( char_t ch )
{
    return digit<radix>( ch, digit_def_ranges<radix,char_t>() );
}

inline bool ist_digit( char ch )
{
    return !!isdigit( (int)(unsigned char)ch );
}
inline bool ist_digit( wchar_t ch )
{
    return !!iswdigit( static_cast<wint_t>(ch) );
}

inline bool ist_print( char ch )
{
    return !!std::isprint( (int)(unsigned char)ch );
}
inline bool ist_print( wchar_t ch )
{
    return !!std::iswprint( static_cast<wint_t>(ch) );
}

template<typename string_t> void Reserve( [[maybe_unused]]string_t& string, [[maybe_unused]]std::size_t chars ){}
template<> inline void Reserve<std::string>( std::string& string, std::size_t chars )
{
    string.reserve( chars );
}
template<> inline void Reserve<std::wstring>( std::wstring& string, std::size_t chars )
{
    string.reserve( chars );
}

//länge bis zur 0-terminierung, aber max max_len (für chararrays die ggf nicht nullterminiert sind)
template<typename char_type> auto stringnlen(char_type const *p, size_t max_len) ->std::enable_if_t<WS::is_char_type_v<char_type>,size_t>
{
    size_t len{0};
    if( p && max_len )
    {
        for( ; *p++ && ++len<max_len; ){}
    }
    return len;
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
#          pragma warning(suppress:4996)
    return strcpy( pdest, psource );
}
[[deprecated("deprecated: stringncpy_s soll sicherer sein")]]
inline wchar_t* stringcpy( wchar_t * pdest, wchar_t const * psource )
{
#   pragma warning(suppress:4996)
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

template<typename string_t,typename char_t, typename filter_fn>
string_t FilterChars( char_t const * psz, typename filter_fn filter ) //filter <bool(char_t)>
{
    string_t retvalue;

    if( psz )
    {
        Reserve( retvalue, stringlen(psz) );
        for(;*psz; ++psz)
        {
            if(filter(*psz))
                retvalue += *psz;
        }
    }

    return retvalue;
}

template<typename string_t,typename char_t>
string_t RemoveNotPrintable( char_t const * psz )
{
    //auto filter = []( char_t ch )->bool{return ist_print(ch);};//ä ist z.b. ohne locale kein print-char
    auto filter = []( char_t ch )->bool{return !(ch>char_t(0) && ch<char_t(' '));};
    return FilterChars<string_t>( psz, filter );
}


namespace helper
{           //for internal use
            //partielle spezialisierung geht nicht mit functionen
    template <typename ret_type, bool =sizeof(ret_type)<=4>
    struct _char_to
    {
        static ret_type action( char const * psz )
        {
            return static_cast<ret_type>(atoi( psz ));
        }
        static ret_type action( wchar_t const * psz )
        {
            return static_cast<ret_type>(_wtoi( psz ));
        }
    };
    template <typename ret_type>struct _char_to<ret_type,false>
    {
        static ret_type action( char const * psz )
        {
            return static_cast<ret_type>(atoll( psz ));
        }
        static ret_type action( wchar_t const * psz )
        {
            return static_cast<ret_type>(_wtoll( psz ));
        }
    };
    template< typename integral_type> constexpr auto digit_bits() //liefert die anzahl der bits, int liefert 31, unsigned int liefert 32 usw
    {
        static_assert(std::is_integral_v<integral_type>);
        return std::numeric_limits<integral_type>::digits;
    }
}

template<typename ret_t, typename char_t> ret_t _stringto( char_t const * string )
{
    static_assert( std::is_integral_v<ret_t> );
    static_assert( std::is_same_v<char_t,char> || std::is_same_v<char_t,signed char> || std::is_same_v<char_t,unsigned char> || std::is_same_v<char_t,wchar_t> );

    if( not string || not *string )
        return ret_t{0};

    try
    {
        if constexpr ( helper::digit_bits<ret_t>() < 32 )
        {
            return static_cast<ret_t>( std::stol( string, nullptr, 10 ) );
        }
        else if constexpr ( helper::digit_bits<ret_t>() == 32 )
        {
            return static_cast<ret_t>( std::stoul( string, nullptr, 10 ) );
        }
        else if constexpr ( helper::digit_bits<ret_t>() < 64 )
        {
            return static_cast<ret_t>( std::stoll( string, nullptr, 10 ) );
        }
        else if constexpr ( helper::digit_bits<ret_t>() == 64 )
        {
            return static_cast<ret_t>( std::stoull( string, nullptr, 10 ) );
        }
        else
        {
            static_assert(WS::dependent_false, "ret_t ist zu groß");
        }
    }
    catch(std::invalid_argument)
    {
        return ret_t{0};
    }
    catch(...)
    {
        throw;
    }
}
template<typename ret_t> ret_t stringto( char const * string )
{
    using type = WS::integral_t<ret_t>;
    return static_cast<ret_t>(_stringto<type>(string));
}
template<typename ret_t> ret_t stringto( wchar_t const * string )
{
    using type = WS::integral_t<ret_t>;
    return static_cast<ret_t>(_stringto<type>(string));
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

#pragma region block tostring
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

namespace
{
    //template<typename int_t,typename char_t> char_t* tostring( int_t value, char_t *buf, size_t buf_size, int radix ){ static_assert(false,"die Implementierung von " __FUNCSIG__ " fehlt" );}
    wchar_t* tostring( int const & value, wchar_t *buf, size_t bufsize, int radix ){_itow_s(value,buf,bufsize,radix);return buf;}
    char* tostring( int const & value, char *buf, size_t bufsize, int radix ){_itoa_s(value,buf,bufsize,radix);return buf;}
    wchar_t* tostring( unsigned int const & value, wchar_t *buf, size_t bufsize, int radix ){_ultow_s(value,buf,bufsize,radix);return buf;}
    char* tostring( unsigned int const & value, char *buf, size_t bufsize, int radix ){_ultoa_s(value,buf,bufsize,radix);return buf;}
    wchar_t* tostring( long const & value, wchar_t *buf, size_t bufsize, int radix ){_ltow_s(value,buf,bufsize,radix);return buf;}
    char* tostring( long const & value, char *buf, size_t bufsize, int radix ){_ltoa_s(value,buf,bufsize,radix);return buf;}
    wchar_t* tostring( unsigned long const & value, wchar_t *buf, size_t bufsize, int radix ){_ultow_s(value,buf,bufsize,radix);return buf;}
    char* tostring( unsigned long const & value, char *buf, size_t bufsize, int radix ){_ultoa_s(value,buf,bufsize,radix);return buf;}

    wchar_t* tostring( __int64 const & value, wchar_t *buf, size_t bufsize, int radix ){_i64tow_s(value,buf,bufsize,radix);return buf;}
    char* tostring( __int64 const & value, char *buf, size_t bufsize, int radix ){_i64toa_s(value,buf,bufsize,radix);return buf;}
    wchar_t* tostring( unsigned __int64 const & value, wchar_t *buf, size_t bufsize, int radix ){_ui64tow_s(value,buf,bufsize,radix);return buf;}
    char* tostring( unsigned __int64 const & value, char *buf, size_t bufsize, int radix ){_ui64toa_s(value,buf,bufsize,radix);return buf;}
    template<typename int_t, typename char_t, size_t size> char_t * tostring( int_t v, char_t (&buf)[size], int radix )
    {
        return tostring( v, buf, size, radix );
    }
#if _HAS_CXX17
    /// <summary>
    /// konvertiert einen integral-type in einen verwalteten string-type.
    /// voraussetzung, der stringtyp hat einen ctor string_type{char_type const*}
    ///  also std::is_constructible_v<string_type,char const *> muss für char_type 'char' true liefern
    /// </summary>
    /// <example>
    /// <code>
    ///        std::string string = tpstring<std::string>( 5 );//default-radix 10 -> "5"
    ///        std::wstring string = tpstring<std::wstring,2>( 5 );//radix 2 -> "101"
    ///        CString string = tpstring<CString,8>( 0765 );//radix 8(octal) -> "765"
    ///        CString string = tpstring<CString,16>( 0765 );//radix 16(hex) -> "1f5"
    /// </code>
    /// </example>


    //_mypow is unused
    template<unsigned int basis,unsigned int exp,typename ret_type=unsigned int> constexpr ret_type _mypow()
    {
        if constexpr ( exp==0 )
            return 1;
        else
            return basis * _mypow<basis,exp-1,ret_type>();
    }
    template<typename integral_t,int radix> constexpr size_t tostring_buffersize()
    {
        return sizeof(integral_t) * 8 + 1;//anzahl bits was für radix 2 passt ist für radix 10 evtl nunnötig groß aber kein problem
    }

#if _HAS_CXX20
    template<typename string_type,int radix=10,integral_type value_type> string_type tostring(value_type value)
#else 
    template<typename string_type,int radix=10,typename value_type> string_type tostring(value_type value)
#endif
    {
        static_assert(std::is_pointer_v<string_type> == false, "it must be a string-class like std::wstring CString ..." );
        static_assert(radix>1, "radix invalid" );
        if constexpr ( std::is_constructible_v<string_type,char const *> )
        {
            char buf[tostring_buffersize<value_type,radix>()];
            return string_type{tostring(value,buf,radix)};
        }
        else if constexpr ( std::is_constructible_v<string_type,wchar_t const *> )
        {
            wchar_t buf[tostring_buffersize<value_type,radix>()];
            return string_type{tostring( value, buf, radix )};
        }
        else
            static_assert(WS::dependent_false<string_type>,"what?");
    }
#endif
}
#pragma endregion


inline int _vscTprintf( const char *format, va_list argptr )
{
    return _vscprintf( format, argptr );
}
inline int _vscTprintf( const wchar_t *format, va_list argptr )
{
    return _vscwprintf( format, argptr );
}
inline int vsTprintf_s( wchar_t *buffer, size_t numberOfElements, const wchar_t *format, va_list argptr )
{
    return vswprintf_s( buffer, numberOfElements, format, argptr );
}
inline int vsTprintf_s( char* buffer, size_t numberOfElements, const char* format, va_list argptr )
{
    return vsprintf_s( buffer, numberOfElements, format, argptr );
}

template<typename ... values_t> int scTprintf( char const * format, values_t && ... values )
{
    return _scprintf( format, std::forward<values_t>(values)... );
}
template<typename ... values_t> int scTprintf( const wchar_t *format, values_t && ... values )
{
    return _scwprintf( format, std::forward<values_t>(values)... );
}
template<typename ... values_t> int sTprintf_s( wchar_t *buffer, size_t numberOfElements, const wchar_t *format, values_t && ... values )
{
    return swprintf_s( buffer, numberOfElements, format, std::forward<values_t>(values)... );
}
template<typename ... values_t> int sTprintf_s( char* buffer, size_t numberOfElements, const char* format, values_t && ... values )
{
    return sprintf_s( buffer, numberOfElements, format, std::forward<values_t>(values)... );
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
