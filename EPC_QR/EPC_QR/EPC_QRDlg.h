#pragma once
#include "GiroCodeCtrl.h"
#include "AccountInfoStore.h"

#include "IbanEdit.h"
#include "AmountEdit.h"

#include "UppercaseEdit.h"

class EPC_QRDlg : public CDialogEx
{
public:
    EPC_QRDlg( CWnd* parent = nullptr );

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_EPC_QR_DIALOG };
#endif

protected:
    virtual void DoDataExchange( CDataExchange* pDX ) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL PreTranslateMessage( MSG* msg ) override;
    virtual void OnOK() override {}
    virtual void OnCancel() override;

    afx_msg void OnClose();

    afx_msg void OnSysCommand( UINT id, LPARAM lParam );
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnToClipboard();
	afx_msg void OnUpdateBicDatabase();
	afx_msg void OnIbanChanged();
	afx_msg void OnRecentAccountChanged();
	afx_msg void OnInputChanged();
	afx_msg void OnContextMenu( CWnd* wnd, CPoint point );
    DECLARE_MESSAGE_MAP()

private:
	void								UpdateQrCode();
	void                                SaveAccountInfoIfChanged();
	void								GenerateQr(EPCString const & EPCString);
	EPCString							GenerateEPCString();

private:
	HICON			icon;
	GiroCodeCtrl	qrCodeCtrl;
	IbanEdit		ibanEdit;
	UppercaseEdit	bicEdit;
	AmountEdit		amountEdit;
	CString			baseBicLabel;
private:
	CString			lastEpcPayload;
	CToolTipCtrl	toolTip;
	CComboBox       recentAccountsCtrl;
	std::vector<AccountInfo> recentAccounts;
	AccountInfo     initialAccountInfo;
	CString         autoFilledBic;

};
