#pragma once

/// <summary>
/// um CueBanner klein, eingetragen text aber groß darzustellen
/// </summary>
class UppercaseEdit : public CEdit
{
protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnChar( UINT ch, UINT repCnt, UINT flags );
    afx_msg LRESULT OnPaste( WPARAM, LPARAM );
};


