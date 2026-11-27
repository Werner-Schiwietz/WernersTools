#include "pch.h"

#include "IbanEdit.h"

CString IbanEdit::FormatIban(CString raw)
{
    raw.Remove( _T(' ') );
    raw.MakeUpper();

    // Max. 34 Zeichen (längste SEPA-IBAN)
    if( raw.GetLength() > 34 )
        raw = raw.Left( 34 );

    CString formatted;
    for( int i = 0; i < raw.GetLength(); ++i )
    {
        if( i > 0 && ( i % 4 ) == 0 )
            formatted += _T(' ');
        formatted += raw[i];
    }
    return formatted;
}


BEGIN_MESSAGE_MAP( IbanEdit, CEdit )
    ON_WM_CHAR()
    ON_MESSAGE( WM_PASTE, &IbanEdit::OnPaste )
END_MESSAGE_MAP()

void IbanEdit::ReformatKeepCaret()
{
    int selStart = 0, selEnd = 0;
    this->GetSel( selStart, selEnd );

    CString oldText;
    this->GetWindowText( oldText );

    // Zählen, wie viele echte Zeichen (ohne Leerzeichen) vor dem Cursor standen
    int charsBeforeCaret = 0;
    for( int i = 0; i < selStart && i < oldText.GetLength(); ++i )
    {
        if( oldText[i] != _T(' ') )
            ++charsBeforeCaret;
    }

    CString newText = FormatIban( oldText );
    if( newText == oldText )
        return;

    // Neue Cursorposition passend zu charsBeforeCaret berechnen
    int newCaretPos = 0;
    int countedChars = 0;
    while( newCaretPos < newText.GetLength() && countedChars < charsBeforeCaret )
    {
        if( newText[newCaretPos] != _T(' ') )
            ++countedChars;
        ++newCaretPos;
    }

    // Falls der Cursor genau auf einer 4er-Grenze landet und noch Text folgt, hinter das Leerzeichen springen
    if( newCaretPos < newText.GetLength() && newText[newCaretPos] == _T(' ') )
        ++newCaretPos;

    this->SetWindowText( newText );
    this->SetSel( newCaretPos, newCaretPos );
}

void IbanEdit::OnChar( UINT ch, UINT repCnt, UINT flags )
{
    // Steuerzeichen (Backspace, Strg+C, Strg+V, Strg+X, Tab etc.) durchlassen
    if( ch < 32 )
    {
        if( ch == VK_BACK )
        {
            int selStart = 0, selEnd = 0;
            this->GetSel( selStart, selEnd );

            // Wenn keine Auswahl aktiv ist und direkt links vom Cursor ein Leerzeichen steht:
            // Markiere auch das Zeichen vor dem Leerzeichen, damit Backspace beides löscht
            if( selStart == selEnd && selStart >= 2 )
            {
                CString text;
                this->GetWindowText( text );
                if( text[selStart - 1] == _T(' ') )
                    this->SetSel( selStart - 2, selStart );
            }
        }

        CEdit::OnChar( ch, repCnt, flags );
        if( ch == VK_BACK )
            this->ReformatKeepCaret();
        return;
    }


    // Nur Ziffern und Buchstaben zulassen (Leerzeichen machen wir selbst)
    bool isDigit = ( ch >= L'0' && ch <= L'9' );
    bool isAlpha = ( ch >= L'A' && ch <= L'Z' ) || ( ch >= L'a' && ch <= L'z' );
    if( not isDigit && not isAlpha )
        return;

    // Großbuchstaben erzwingen und Standard-Eingabe ausführen, macht ReformatKeepCaret zwar eh, aber wer weiß
    TCHAR upperChar = static_cast<TCHAR>(ch);
    CharUpperBuff( &upperChar, 1 );
    DefWindowProc( WM_CHAR, upperChar, MAKELPARAM( repCnt, flags ) );

    this->ReformatKeepCaret();
}

LRESULT IbanEdit::OnPaste( WPARAM wParam, LPARAM lParam )
{
    LRESULT res = Default(); // Windows fügt den Text ein
    this->ReformatKeepCaret();
    return res;
}
