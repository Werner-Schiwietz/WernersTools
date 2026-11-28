#include "pch.h"
#include "amountEdit.h"

#include <stdlib.h> 
#include <iterator> // für std::size

BEGIN_MESSAGE_MAP( AmountEdit, CEdit )
    ON_WM_CHAR()
    ON_WM_KEYDOWN()
    ON_MESSAGE( WM_PASTE, &AmountEdit::OnPaste )
END_MESSAGE_MAP()

AmountEdit::AmountEdit()
{
    wchar_t decBuf[4]  = {};
    wchar_t thouBuf[4] = {};
    if( GetLocaleInfoEx( LOCALE_NAME_USER_DEFAULT, LOCALE_SDECIMAL, decBuf, 4 ) > 0 && decBuf[0] != L'\0' )
        this->decSep = decBuf[0];
    if( GetLocaleInfoEx( LOCALE_NAME_USER_DEFAULT, LOCALE_STHOUSAND, thouBuf, 4 ) > 0 && thouBuf[0] != L'\0' )
        this->thouSep = thouBuf[0];
}


CEuroAmount AmountEdit::GetAmount() const
{
    CString text;
    this->GetWindowText( text );
    return CEuroAmount( text );
}

void AmountEdit::SetAmount( const CEuroAmount& amount )
{
    this->SetWindowText( amount.FormatForDisplay() );
}

CString AmountEdit::FormatLive( const CString& raw ) const
{
    // 1. Nur Ziffern und das erste Dezimaltrennzeichen extrahieren
    CString intPart;
    CString fracPart;
    bool hasDec = false;

    for( int i = 0; i < raw.GetLength(); ++i )
    {
        wchar_t ch = raw[i];
        if( ch >= L'0' && ch <= L'9' )
        {
            if( not hasDec )
            {
                if( intPart.GetLength() < 9 ) // EPC-Limit: max. 999.999.999 EUR
                    intPart += ch;
            }
            else if( fracPart.GetLength() < 2 ) // Max. 2 Nachkommastellen
            {
                fracPart += ch;
            }
        }
        else if( ch == this->decSep && not hasDec )
        {
            hasDec = true;
        }
    }

    // Führende Nullen im Vorkomma-Teil bereinigen (aber "0" behalten)
    while( intPart.GetLength() > 1 && intPart[0] == L'0' )
        intPart.Delete( 0 );

    if( intPart.IsEmpty() && hasDec )
        intPart = L"0";

    // 2. Tausendertrennzeichen von rechts in 3er-Blöcken einfügen
    CString groupedInt;
    int len = intPart.GetLength();
    for( int i = 0; i < len; ++i )
    {
        if( i > 0 && ( ( len - i ) % 3 ) == 0 && this->thouSep != L'\0' )
            groupedInt += this->thouSep;
        groupedInt += intPart[i];
    }

    if( hasDec )
        return groupedInt + this->decSep + fracPart;

    return groupedInt;
}

void AmountEdit::ReformatKeepCaret()
{
    static bool bInReformat = false;
    if( bInReformat )
        return;

    struct Guard
    {
        bool& flag;
        Guard( bool& f ) : flag( f ) { flag = true; }
        ~Guard() { flag = false; }
    } guard( bInReformat );

    CString oldText;
    this->GetWindowText( oldText );

    int selStart = 0, selEnd = 0;
    this->GetSel( selStart, selEnd );

    // Relevante Zeichen (Ziffern + Dezimaltrenner) vor dem Cursor zählen
    int significantBeforeCaret = 0;
    bool seenDecBeforeCaret = false;
    bool seenNonZeroBeforeCaret = false;

    // Sonderfall: Text beginnt mit Komma (",5" -> wird in FormatLive zu "0,5"):
    // Die automatisch ergänzte '0' virtuell mitzählen, damit der Cursor hinter dem Komma bleibt.
    if( not oldText.IsEmpty() && oldText.GetAt( 0 ) == this->decSep && selStart > 0 )
        significantBeforeCaret = 1;

    for( int i = 0; i < selStart && i < oldText.GetLength(); ++i )
    {
        wchar_t ch = oldText[i];
        if( ch >= L'0' && ch <= L'9' )
        {
            // Führende Nullen ignorieren, die FormatLive gleich wieder entfernt
            // (wenn noch keine Ziffer 1..9 und kein Komma kam und danach noch eine Ziffer folgt)
            bool isStrippedLeadingZero = ( ch == L'0' && not seenNonZeroBeforeCaret && not seenDecBeforeCaret &&
                                           ( i + 1 < oldText.GetLength() ) &&
                                           ( oldText[i + 1] >= L'0' && oldText[i + 1] <= L'9' ) );
            if( not isStrippedLeadingZero )
            {
                ++significantBeforeCaret;
                if( ch != L'0' )
                    seenNonZeroBeforeCaret = true;
            }
        }
        else if( ch == this->decSep && not seenDecBeforeCaret )
        {
            ++significantBeforeCaret;
            seenDecBeforeCaret = true;
        }
    }

    CString newText = this->FormatLive( oldText );
    if( newText == oldText )
        return;

    // Neue Cursor-Position anhand der gezählten relevanten Zeichen finden
    int newCaretPos = 0;
    int counted = 0;
    bool seenDecInNew = false;

    while( newCaretPos < newText.GetLength() && counted < significantBeforeCaret )
    {
        wchar_t ch = newText[newCaretPos];
        if( ch >= L'0' && ch <= L'9' )
        {
            ++counted;
        }
        else if( ch == this->decSep && not seenDecInNew )
        {
            ++counted;
            seenDecInNew = true;
        }
        ++newCaretPos;
    }

    this->SetWindowText( newText );
    this->SetSel( newCaretPos, newCaretPos );
}

void AmountEdit::OnChar( UINT ch, UINT repCnt, UINT flags )
{
    if( ch < 32 )
    {
        if( ch == VK_BACK )
        {
            int selStart = 0, selEnd = 0;
            this->GetSel( selStart, selEnd );

            // Wenn Backspace direkt hinter einem Tausenderpunkt gedrückt wird -> Ziffer davor mitlöschen
            if( selStart == selEnd && selStart >= 2 )
            {
                CString text;
                this->GetWindowText( text );
                if( text[selStart - 1] == this->thouSep )
                    this->SetSel( selStart - 2, selStart );
            }
        }

        CEdit::OnChar( ch, repCnt, flags );
        if( ch == VK_BACK )
            this->ReformatKeepCaret();
        return;
    }

    // Punkt oder Komma automatisch auf das eingestellte Windows-Dezimaltrennzeichen mappen
    wchar_t inputCh = static_cast<wchar_t>( ch );
    if( inputCh == L'.' || inputCh == L',' )
        inputCh = this->decSep;

    bool isDigit = ( inputCh >= L'0' && inputCh <= L'9' );
    bool isDec   = ( inputCh == this->decSep );
    if( not isDigit && not isDec )
        return;

    // Kein zweites Dezimaltrennzeichen zulassen (falls nicht gerade markiert)
    if( isDec )
    {
        CString text;
        this->GetWindowText( text );
        int selStart = 0, selEnd = 0;
        this->GetSel( selStart, selEnd );

        int existingDec = text.Find( this->decSep );
        if( existingDec >= 0 && ( existingDec < selStart || existingDec >= selEnd ) )
            return;
    }

    CEdit::OnChar( inputCh, repCnt, flags );
    this->ReformatKeepCaret();
}

void AmountEdit::OnKeyDown( UINT ch, UINT repCnt, UINT flags )
{
    if( ch == VK_DELETE )
    {
        int selStart = 0, selEnd = 0;
        this->GetSel( selStart, selEnd );

        CString text;
        this->GetWindowText( text );
        if( selStart == selEnd && selStart + 1 < text.GetLength() && text[selStart] == this->thouSep )
            this->SetSel( selStart, selStart + 2 );

        CEdit::OnKeyDown( ch, repCnt, flags );
        this->ReformatKeepCaret();
        return;
    }

    CEdit::OnKeyDown( ch, repCnt, flags );
}

LRESULT AmountEdit::OnPaste( WPARAM wParam, LPARAM lParam )
{
    LRESULT result = this->DefWindowProc( WM_PASTE, wParam, lParam );
    this->ReformatKeepCaret();
    return result;
}
