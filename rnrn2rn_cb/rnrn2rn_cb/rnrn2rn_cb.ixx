#pragma warning(disable:5106 5105 5201 4005)
#include "pch.h" //import <vector>; mit precompiled header ging nicht????

export module rnrn2rn_cb;

import <vector>;
import <algorithm>;


struct raai_OpenClipboard
{
    bool is_open=false;
    raai_OpenClipboard(HWND hwnd)
    {
        this->is_open = OpenClipboard(hwnd);
    }
    ~raai_OpenClipboard()
    {
        try
        {
            if(this->is_open)
            {
                this->is_open = false;
                CloseClipboard();
            }
        }
        catch(...)
        {}
    }
    operator bool() const & {return this->is_open;}
};

using ClipboardFormat = unsigned int; 

template<ClipboardFormat cb_format> requires(cb_format==CF_UNICODETEXT || cb_format==CF_TEXT)
struct traits;
template<>
struct traits<CF_UNICODETEXT>
{
    using char_t = wchar_t;
    using string_t = CStringW;

    char_t const * replace = L"\r\n\r\n";
    char_t const * with = L"\r\n";
};
template<>
struct traits<CF_TEXT>
{
    using char_t = char;
    using string_t = CStringA;

    char_t const * replace = "\r\n\r\n";
    char_t const * with = "\r\n";
};

#pragma region 
template<typename char_t> requires( std::is_same_v<char_t,char> || std::is_same_v<char_t,wchar_t> )
constexpr ClipboardFormat _GetClipboardFormat();

template<> 
constexpr ClipboardFormat _GetClipboardFormat<wchar_t>()
{
    return CF_UNICODETEXT;
}
template<> 
constexpr ClipboardFormat _GetClipboardFormat<char>()
{
    return CF_TEXT;
}
#pragma endregion convert chartype to CF

template<ClipboardFormat cb_format> requires(cb_format==CF_UNICODETEXT || cb_format==CF_TEXT)
void _rnrn2rn()
{
    using char_t = typename traits<cb_format>::char_t;
    using string_t = typename traits<cb_format>::string_t;

    if( auto cb = raai_OpenClipboard(NULL) )
    {

        auto hglb = GetClipboardData( cb_format );
        if( hglb != NULL )
        {
            string_t str = (char_t const *)GlobalLock( hglb );
            if( str )
            {
                GlobalUnlock( hglb );

                auto len = str.GetLength();
                str.Replace( traits<cb_format>{}.replace, traits<cb_format>{}.with );
                len = str.GetLength();

                //EmptyClipboard();

                auto hglbCopy = GlobalAlloc( GMEM_MOVEABLE,
                                             (str.GetLength() + 1) * sizeof( wchar_t ) );
                if( hglbCopy )
                {

                    auto lptstrCopy = (char_t *)GlobalLock( hglbCopy );
                    memcpy( lptstrCopy, str.GetString(),
                            (str.GetLength() + 1) * sizeof( wchar_t ) );
                    GlobalUnlock( hglbCopy );

                    // Place the handle on the clipboard. 

                    SetClipboardData( cb_format, hglbCopy );
                }
            }
        }
    }
}

template<ClipboardFormat cb_format> requires(cb_format==CF_UNICODETEXT || cb_format==CF_TEXT)
auto _GetClipboardText()
{
    using char_t = typename traits<cb_format>::char_t;
    using string_t = typename traits<cb_format>::string_t;
    string_t retvalue; 

    if( auto cb = raai_OpenClipboard(NULL) )
    {
        auto hglb = GetClipboardData( cb_format );
        if( hglb != NULL )
        {
            if( auto pstr = (char_t const *)GlobalLock( hglb ) )
            {
                retvalue = pstr;
                GlobalUnlock( hglb );
            }
        }
    }

    return retvalue;
}

export std::vector<ClipboardFormat> EnumClipboardFormats()
{
    std::vector<ClipboardFormat> retvalue;
    if( auto cb = raai_OpenClipboard(NULL) )
    {
        ClipboardFormat format=0;
        while( format=::EnumClipboardFormats(format) )
            retvalue.push_back(format);
    }

    return retvalue;
}


template<typename cf_container>
void _RemoveBut( cf_container cfs)
{
    auto avaiable_cf = EnumClipboardFormats();
    if( auto cb = raai_OpenClipboard(NULL) )
    {
        for( auto acf : avaiable_cf )
        {
            auto iter = std::find_if(cfs.begin(),cfs.end(),[&](ClipboardFormat cf){return cf==acf;});
            if(iter==cfs.end())
                SetClipboardData(acf,NULL);
        }
    }
}

export void rnrn2rn()
{
    _rnrn2rn<CF_UNICODETEXT>();
    _rnrn2rn<CF_TEXT>();

    _RemoveBut(std::vector<ClipboardFormat>{CF_TEXT,CF_UNICODETEXT});

    auto cfs = EnumClipboardFormats();
}

export CString GetClipboardText()
{
    auto retvalue = _GetClipboardText<_GetClipboardFormat<CString::XCHAR>()>();
    if(retvalue.IsEmpty())
        retvalue = _GetClipboardText<_GetClipboardFormat<CString::YCHAR>()>();
    return retvalue;
}