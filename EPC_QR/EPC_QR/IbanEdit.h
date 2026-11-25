#pragma once

class IbanEdit : public CEdit
{
public:
    static CString FormatIban( CString raw );

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnChar( UINT ch, UINT repCnt, UINT flags );
    afx_msg LRESULT OnPaste( WPARAM wParam, LPARAM lParam );

private:
    void ReformatKeepCaret();
};
