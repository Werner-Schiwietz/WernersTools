#include "pch.h"

#include "EPCAmount.h"

#include <array>
#include <string_view>

namespace
{
    constexpr bool IsAsciiDigit( wchar_t ch ) noexcept
    {
        return ch >= L'0' && ch <= L'9';
    }
    constexpr bool IsAsciiDigit( char ch ) noexcept
    {
        return ch >= '0' && ch <= '9';
    }
}

static auto begin( CString const & s )
{
	return s.GetString();
}
static auto end( CString const & s )
{
	return s.GetString() + s.GetLength();
}

namespace
{
	bool skipSpaces( std::wstring_view & text )
	{
        bool ret_v = false;
		while( text.size() && iswspace( text[0] ) )
        {
            text.remove_prefix( 1 );
            ret_v = true;
        }
		return ret_v;
	}
    wchar_t pop_front( std::wstring_view& text )
    {
        if( text.empty() ) 
            return {};//sollte nie passieren

        const wchar_t ch = text.front();
        text.remove_prefix( 1 );
        return ch;
    }
    bool eat_eur( std::wstring_view & textIn )
    {
        auto text = textIn;
		if( auto ch = pop_front(text); ch != L'E' && ch != L'e' ) return false;
		if( auto ch = pop_front(text); ch != L'U' && ch != L'u' ) return false;
		if( auto ch = pop_front(text); ch != L'R' && ch != L'r' ) return false;

        textIn = text;
        return true;
    }

    bool eat( std::wstring_view & textIn , wchar_t const * gesucht)
    {
        auto text = textIn;

        for( ; *gesucht; ++gesucht )
        {
            if( pop_front(text) != *gesucht )
                return false;
        }
        textIn = text;
        return true;
    }

    std::int64_t getEuroInCent( CString rawText )
    {
        std::wstring_view text{ rawText.GetString() };

        skipSpaces(text);
        //if( not eat_eur(text) ) return CEuroAmount::InvalidCents;

        #pragma region "lokale Trennzeichen abfragen und auf Gültigkeit prüfen"
        std::array<wchar_t, 4> decSep{L',', L'\0'};
        std::array<wchar_t, 4> thouSep{L'.', L'\0'};
        GetLocaleInfoEx( LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, decSep.data(), static_cast<int>(decSep.size()) );
        GetLocaleInfoEx( LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, thouSep.data(), static_cast<int>(thouSep.size()) );

        auto isValidSeparator = []( const wchar_t* sep ) noexcept
        {
            if( sep == nullptr || sep[0] == L'\0' )
                return false;

            for( int i = 0; sep[i] != L'\0'; ++i )
            {
                if( IsAsciiDigit( sep[i] ) )
                    return false;
            }
            return true;
        };
        if( not isValidSeparator( decSep.data() ) )
            decSep = {L',', L'\0'};
        if( not isValidSeparator( thouSep.data() ) || wcscmp( decSep.data(), thouSep.data() ) == 0 )
            thouSep = {L'.', L'\0'};
        #pragma endregion

		std::int64_t euros = 0;
		std::int64_t cents = 0;
		bool inCents = false;
		int centDigits = 0;
		while( text.size() )
		{
			if( auto ch = text.front(); IsAsciiDigit( ch ) )
			{
                text.remove_prefix( 1 );
				if( inCents )
				{
					if( centDigits >= 2 ) return CEuroAmount::InvalidCents;//maximal 2 Nachkommastellen erlaubt
					cents = cents * 10 + (ch - L'0');
					++centDigits;
				}
				else
				{
                    if( euros > 99'999'999 )
                        return CEuroAmount::InvalidCents;//die zahl würde größer als 999'999'999. das ist nicht erlaubt
					euros = euros * 10 + (ch - L'0');
				}
			}
			else if( eat( text, decSep.data() ) )
			{
				if( inCents ) return CEuroAmount::InvalidCents;//zweiter Dezimaltrenner ist ungültig
				inCents = true;
			}
			else if( eat( text, thouSep.data() ) )
			{
				// Tausendertrenner im Vorkommateil entfernen
				if( inCents ) return CEuroAmount::InvalidCents;//Tausendertrenner im Nachkommateil ist ungültig
			}
			else if( iswspace( ch ) )
			{
				skipSpaces( text );
				break;
			}
			else
			{
				return CEuroAmount::InvalidCents;//ungültiges Zeichen
			}
		}
		skipSpaces( text );
		if( not text.empty() ) return CEuroAmount::InvalidCents;//ungültige Zeichen am Ende

		while( centDigits++ < 2 )
			cents *= 10;

		const __int64 parsed = euros * 100LL + cents;
		return parsed;
    }
}

CEuroAmount::CEuroAmount( CString rawText )
{
    this->cents = getEuroInCent( rawText );
}

// Laut EPC069-12 ist "EUR0.00" verboten (Minimum ist EUR0.01).
// Kein Betrag (0 Cent) muss als leere Zeile uebergeben werden – manche Banking-Apps
// erwarten in der Praxis allerdings zwingend einen Betrag >= 1 Cent.
CString CEuroAmount::get() const//amount für qrgencode
{
    if( not IsValid() || IsEmpty() )
        return _T("");

    CString epc;
    epc.Format( _T("EUR%lld.%02lld"), this->cents / 100, this->cents % 100 );
    return epc;
}


CString CEuroAmount::FormatForDisplay() const
{
    if( this->cents <= 0 )
        return _T("");

    CString raw;
    raw.Format( _T("%lld.%02lld"), this->cents / 100, this->cents % 100 );

    std::array<wchar_t, 64> formatted{};
    int res = GetNumberFormatEx(
        LOCALE_NAME_USER_DEFAULT,
        0,
        raw,
        nullptr,
        formatted.data(),
        static_cast<int>( formatted.size() )
    );

    return ( res > 0 ) ? CString( formatted.data() ) : raw;
}


