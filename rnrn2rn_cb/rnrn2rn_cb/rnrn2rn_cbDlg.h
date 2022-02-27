
// rnrn2rn_cbDlg.h : header file
//

#pragma once


// Crnrn2rncbDlg dialog
class Crnrn2rncbDlg : public CDialogEx
{
// Construction
public:
	Crnrn2rncbDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RNRN2RN_CB_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit cb_text;
	afx_msg void OnClipboardUpdate();
};
