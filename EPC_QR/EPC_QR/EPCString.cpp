#include "pch.h"

#include "EPCString.h"
#include "EPCAmount.h"

namespace
{
    // Alle 42 SEPA-IBAN-Präfixe (alphabetisch sortiert)
    constexpr CountryIbanSpec SepaIbanSpecs[] = {
        { L"AD", 24, false, true  }, // Andorra
        { L"AL", 28, false, true  }, // Albanien
        { L"AT", 20, true,  false }, // Österreich
        { L"BE", 16, true,  false }, // Belgien
        { L"BG", 22, false, false }, // Bulgarien
        { L"CH", 21, true,  true  }, // Schweiz
        { L"CY", 28, false, false }, // Zypern
        { L"CZ", 24, true,  false }, // Tschechien
        { L"DE", 22, true,  false }, // Deutschland
        { L"DK", 18, true,  false }, // Dänemark
        { L"EE", 20, true,  false }, // Estland
        { L"ES", 24, true,  false }, // Spanien
        { L"FI", 18, true,  false }, // Finnland
        { L"FR", 27, false, false }, // Frankreich
        { L"GB", 22, false, true  }, // Vereinigtes Königreich
        { L"GI", 23, false, true  }, // Gibraltar
        { L"GR", 27, false, false }, // Griechenland
        { L"HR", 21, true,  false }, // Kroatien
        { L"HU", 28, true,  false }, // Ungarn
        { L"IE", 22, false, false }, // Irland
        { L"IS", 26, true,  false }, // Island
        { L"IT", 27, false, false }, // Italien
        { L"LI", 21, true,  false }, // Liechtenstein
        { L"LT", 20, true,  false }, // Litauen
        { L"LU", 20, false, false }, // Luxemburg
        { L"LV", 21, false, false }, // Lettland
        { L"MC", 27, false, true  }, // Monaco
        { L"MD", 24, false, true  }, // Moldawien
        { L"ME", 22, true,  true  }, // Montenegro
        { L"MK", 19, false, true  }, // Nordmazedonien
        { L"MT", 31, false, false }, // Malta
        { L"NL", 18, false, false }, // Niederlande
        { L"NO", 15, true,  false }, // Norwegen
        { L"PL", 28, true,  false }, // Polen
        { L"PT", 25, true,  false }, // Portugal
        { L"RO", 24, false, false }, // Rumänien
        { L"RS", 22, true,  true  }, // Serbien
        { L"SE", 24, true,  false }, // Schweden
        { L"SI", 19, true,  false }, // Slowenien
        { L"SK", 24, true,  false }, // Slowakei
        { L"SM", 27, false, true  }, // San Marino
        { L"VA", 22, true,  true  }, // Vatikanstadt
        { {}   , 34 }				 // ungültiger, leerer eintrag mit der max.länge
    };
}
CountryIbanSpec const & GetSepaSpec( )//liefert den ungültigen
{
    return SepaIbanSpecs[_countof(SepaIbanSpecs)-1];
}
CountryIbanSpec const & GetSepaSpec( std::size_t index )
{
	if( index >= _countof(SepaIbanSpecs) )
		index =_countof(SepaIbanSpecs)-1;//der letzte eintrag markiert ungültige

	return SepaIbanSpecs[index];
}
CountryIbanSpec const & FindSepaSpecIndex( CString const & iban )
{
    if( iban.GetLength() < 2 )
        return GetSepaSpec();

    for( auto& SepaIbanSpec : SepaIbanSpecs )
    {
        if( iban[0] == SepaIbanSpec.code[0] && iban[1] == SepaIbanSpec.code[1] )
            return SepaIbanSpec;
    }
    return GetSepaSpec();
}

namespace
{
    constexpr bool IsAsciiDigit( wchar_t ch ) noexcept
    {
        return ch >= L'0' && ch <= L'9';
    }

    constexpr bool IsAsciiUpper( wchar_t ch ) noexcept
    {
        return ch >= L'A' && ch <= L'Z';
    }

    constexpr bool IsAsciiUpperOrDigit( wchar_t ch ) noexcept
    {
        return IsAsciiDigit( ch ) || IsAsciiUpper( ch );
    }

    EPCString_Error IsValidSepaIban( const CString& iban, CountryIbanSpec const & spec )
    {
        if( not spec.valid() ) return EPCString_Error::IBAN;

        int len = iban.GetLength();
        if( len != spec.length )
            return EPCString_Error::IBAN;

        // Prüfziffern (Index 2 & 3)
        if( not IsAsciiDigit( iban[2] ) || not IsAsciiDigit( iban[3] ) )
            return EPCString_Error::IBAN;

        // BBAN-Zeichen (ab Index 4)
        for( int i = 4; i < len; ++i )
        {
            if( spec.digitsOnlyBban )
            {
                if( not IsAsciiDigit( iban[i] ) )
                    return EPCString_Error::IBAN;
            }
            else
            {
                if( not IsAsciiUpperOrDigit( iban[i] ) )
                    return EPCString_Error::IBAN;
            }
        }

        // Universelle Modulo-97-Prüfung (erste 4 Zeichen ans Ende schieben)
        int remainder = 0;
        for( int i = 0; i < len; ++i )
        {
            wchar_t ch = iban[(i + 4) % len];
            if( IsAsciiDigit( ch ) )
            {
                remainder = ( remainder * 10 + (ch - L'0') ) % 97;
            }
            else
            {
                int val = ( ch - L'A' ) + 10;
                remainder = ( remainder * 100 + val ) % 97;
            }
        }

        return ( remainder == 1 ) ? EPCString_Error::success : EPCString_Error::IBAN;
    }

    EPCString_Error IsValidBicForSpec( const CString& bic, CountryIbanSpec const & spec )
    {
        if( not spec.valid() ) return EPCString_Error::BIC;

        if( bic.IsEmpty() )
            return spec.bicRequired ? EPCString_Error::BIC : EPCString_Error::success ;

        int len = bic.GetLength();
        if( len != 8 && len != 11 )
            return EPCString_Error::BIC;

        for( int i = 0; i < len; ++i )
        {
            if( not IsAsciiUpperOrDigit( bic[i] ) )
                return EPCString_Error::BIC;
        }

        // Ländercode im BIC (Stelle 4 & 5) muss zur IBAN passen
        if( bic[4] != spec.code[0] || bic[5] != spec.code[1] )
            return EPCString_Error::BIC;

        return EPCString_Error::success;
    }

    EPCString_Error IsValidPurposeCombination( const CString& reference, const CString& purposeText )
    {
        if( not reference.IsEmpty() && not purposeText.IsEmpty() )
            return EPCString_Error::Ref_Purpose;

        return EPCString_Error::success;
    }

    CString SanitizeTextField( CString text, int maxLen )
    {
        text.Replace( _T("\r\n"), _T(" ") );
        text.Replace( _T('\r'), _T(' ') );
        text.Replace( _T('\n'), _T(' ') );
        text.Trim();
        return text.Left( maxLen );
    }
} // namespace


EPCString GenerateEPCString(
    CString				name,
    CString				iban,
    CString				bic,
    CEuroAmount const & euroamount,
    CString 			reference,
    CString				purposeText )
{
    // 1. Eingaben normalisieren
    iban.Remove( _T(' ') );
    iban.MakeUpper();

    bic.Remove( _T(' ') );
    bic.MakeUpper();

    name        = SanitizeTextField( name, 70 );
    reference   = SanitizeTextField( reference, 35 );
    purposeText = SanitizeTextField( purposeText, 140 );

    // 2. Prüfungen ausführen
    if( name.IsEmpty() )                                                                    return {EPCString_Error::NAME};

    decltype(auto)specIndex = FindSepaSpecIndex( iban );
    if( auto erg = IsValidSepaIban( iban, specIndex ); not Success(erg) )                   return {erg};
    if( auto erg = IsValidBicForSpec( bic, specIndex ); not Success(erg) )                 return {erg};
    if( auto erg = IsValidPurposeCombination( reference, purposeText ); not Success(erg) )  return {erg};

    // 3. EPC-String zusammenbauen
    CString payload;
    payload += _T("BCD\n");                     // 1. Service Tag
    payload += _T("002\n");                     // 2. Version
    payload += _T("1\n");                       // 3. Encoding (UTF-8)
    payload += _T("SCT\n");                     // 4. SEPA Credit Transfer
    payload += bic + _T("\n");                  // 5. BIC
    payload += name + _T("\n");                 // 6. Empfängername
    payload += iban + _T("\n");                 // 7. IBAN
    payload += euroamount.get() + _T("\n");     // 8. Betrag
    payload += _T("\n");                        // 9. Purpose Code (leer)
    payload += reference + _T("\n");            // 10. Referenz
    payload += purposeText;                     // 11. Verwendungszweck

    return payload;
}

