#include "pch.h"

#include "UppercaseEdit.h"


BEGIN_MESSAGE_MAP( UppercaseEdit, CEdit )
    ON_WM_CHAR()
    ON_MESSAGE( WM_PASTE, &UppercaseEdit::OnPaste )
END_MESSAGE_MAP()

void UppercaseEdit::OnChar( UINT ch, UINT repCnt, UINT flags )
{
    TCHAR upperChar = static_cast<TCHAR>(ch);
    CharUpperBuff( &upperChar, 1 );

    DefWindowProc( WM_CHAR, upperChar, MAKELPARAM( repCnt, flags ) );
}

LRESULT UppercaseEdit::OnPaste( WPARAM, LPARAM )
{
    const LRESULT result = Default();
    CString text;
    this->GetWindowText( text );
    const CString original = text;
    text.MakeUpper();

    if( text != original )
    {
        int selStart = 0;
        int selEnd = 0;
        this->GetSel( selStart, selEnd );
        this->SetWindowText( text );
        this->SetSel( selStart, selEnd );
    }

    return result;
}

