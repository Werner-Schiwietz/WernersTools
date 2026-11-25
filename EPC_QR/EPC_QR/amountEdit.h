#pragma once
#include "EPCAmount.h"

class AmountEdit : public CEdit
{
public:
    AmountEdit();

    // Liest den aktuellen Wert direkt als typsicheren CEuroAmount aus
    CEuroAmount GetAmount() const;

    // Setzt einen CEuroAmount formatiert ins Feld (z.B. "1.234,54")
    void SetAmount( const CEuroAmount& amount );

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnChar( UINT ch, UINT repCnt, UINT flags );
    afx_msg void OnKeyDown( UINT ch, UINT repCnt, UINT flags );
    afx_msg LRESULT OnPaste( WPARAM wParam, LPARAM lParam );

private:
    wchar_t decSep{ L',' };
    wchar_t thouSep{ L'.' };

    CString FormatLive( const CString& raw ) const;
    void ReformatKeepCaret();
};
