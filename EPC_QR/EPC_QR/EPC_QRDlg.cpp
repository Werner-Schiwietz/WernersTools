#include "pch.h"
#include "framework.h"
#include "EPC_QR.h"
#include "EPC_QRDlg.h"
#include "afxdialogex.h"

#include "EPCString.h"
#include "EPCAmount.h"
#include "AccountInfoStore.h"
#include "BicDatabase.h"

#include <algorithm>
#include <afxtaskdialog.h> // Oben in der .cpp (oder in pch.h) einbinden

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	CString LoadResourceString(UINT nID)
	{
		CString str;
		str.LoadString(nID);
		return str;
	}

	CString AccountLabel( AccountInfo const& account )
	{
		return account.name + _T(" | ") + account.iban;
	}

	CString NormalizeIban( CString iban )
	{
		iban.Remove( _T(' ') );
		iban.Trim();
		iban.MakeUpper();
		return iban;
	}
}
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() : CDialogEx( IDD_ABOUTBOX ) {}

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMClickSyslinkMit( NMHDR* pNMHDR, LRESULT* pResult );
};

BEGIN_MESSAGE_MAP( CAboutDlg, CDialogEx )
    ON_NOTIFY( NM_CLICK,  IDC_SYSLINK_MIT, &CAboutDlg::OnNMClickSyslinkMit )
    ON_NOTIFY( NM_RETURN, IDC_SYSLINK_MIT, &CAboutDlg::OnNMClickSyslinkMit )
END_MESSAGE_MAP()

void CAboutDlg::OnNMClickSyslinkMit( NMHDR*, LRESULT* pResult )
{
    static constexpr wchar_t mitText[] = //text in fliestext ohne steuerzeichen
LR"(QR Code generator library (C++): Copyright (c) Project Nayuki. (MIT License)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.)";

	CTaskDialog dlg( mitText, _T("MIT License"), _T("Lizenzhinweis"), TDCBF_OK_BUTTON );
	dlg.SetMainIcon( TD_INFORMATION_ICON );
	dlg.SetDialogWidth( 280 ); // Breite in Dialog-Einheiten (DLUs)
	dlg.DoModal();
	*pResult = 0;
}


EPC_QRDlg::EPC_QRDlg( CWnd* parent ) : CDialogEx( IDD_EPC_QR_DIALOG, parent )
{
    this->icon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

void EPC_QRDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_ORCODE, this->qrCodeCtrl );
	DDX_Control( pDX, IDC_EDIT_IBAN, this->ibanEdit );
	DDX_Control( pDX, IDC_EDIT_BIC, this->bicEdit );
	DDX_Control( pDX, IDC_EDIT_AMOUNT, this->amountEdit );
	DDX_Control( pDX, IDC_COMBO_RECENT_ACCOUNTS, this->recentAccountsCtrl );
}

BEGIN_MESSAGE_MAP( EPC_QRDlg, CDialogEx )
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_CONTEXTMENU()
    ON_BN_CLICKED( IDC_BTN_TOCLIPBOARD, &OnToClipboard )
    ON_BN_CLICKED( IDC_BTN_UPDATE_BIC_DATABASE, &OnUpdateBicDatabase )
    ON_CBN_SELCHANGE( IDC_COMBO_RECENT_ACCOUNTS, &OnRecentAccountChanged )
    ON_EN_CHANGE(IDC_EDIT_NAME,      &OnInputChanged)
    ON_EN_CHANGE(IDC_EDIT_IBAN,      &OnIbanChanged)
    ON_EN_CHANGE(IDC_EDIT_BIC,       &OnInputChanged)
    ON_EN_CHANGE(IDC_EDIT_AMOUNT,    &OnInputChanged)
    ON_EN_CHANGE(IDC_EDIT_REF,		 &OnInputChanged)
    ON_EN_CHANGE(IDC_EDIT_PURPOSE,	 &OnInputChanged)
END_MESSAGE_MAP()

BOOL EPC_QRDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CMenu* sysMenu = this->GetSystemMenu( FALSE );
    if( sysMenu != nullptr )
    {
        CString aboutMenu;
        if( aboutMenu.LoadString( IDS_ABOUTBOX ) && not aboutMenu.IsEmpty() )
        {
            sysMenu->AppendMenu( MF_SEPARATOR );
            sysMenu->AppendMenu( MF_STRING, IDM_ABOUTBOX, aboutMenu );
        }
    }

	// In EPC_QRDlg::OnInitDialog():
	static_cast<CEdit*>( this->GetDlgItem( IDC_EDIT_NAME ) )->SetCueBanner( LoadResourceString(IDS_PFLICHTFELD) );
	static_cast<CEdit*>( this->GetDlgItem( IDC_EDIT_IBAN ) )->SetCueBanner( LoadResourceString(IDS_PFLICHTFELD_IBAN) );
	static_cast<CEdit*>( this->GetDlgItem( IDC_EDIT_BIC ) )->SetCueBanner ( LoadResourceString(IDS_PFLICHTFELD_BIC) );

	this->GetDlgItemText( IDC_STATIC_BIC, this->baseBicLabel );

	this->toolTip.Create( this );
	this->toolTip.SetMaxTipWidth( 400 ); // Aktiviert mehrzeilige Tooltips (max. 300 Pixel breit)
	this->toolTip.AddTool( this->GetDlgItem( IDC_BTN_TOCLIPBOARD ), LoadResourceString(IDS_TOOLTIP_COPY_QR) );
	this->toolTip.AddTool( this->GetDlgItem( IDC_BTN_UPDATE_BIC_DATABASE ), LoadResourceString(IDS_TOOLTIP_UPDATE_BIC_DATABASE) );
	this->toolTip.Activate( TRUE );

    this->SetIcon( this->icon, TRUE );
    this->SetIcon( this->icon, FALSE );

	auto accountLoadResult = AccountInfoStore::Load();
	if( accountLoadResult )
	{
		this->recentAccounts = std::move( accountLoadResult.accounts );
		if( this->recentAccounts.empty() )
		{
			this->recentAccountsCtrl.AddString( _T("Keine gespeicherten Konten") );
			this->recentAccountsCtrl.EnableWindow( FALSE );
			this->initialAccountInfo.name = _T( "Max Mustermann" );
			this->initialAccountInfo.iban = _T( "DE89370400440532013000" );
			this->initialAccountInfo.bic = _T( "COBADEFFXXX" );
			this->initialAccountInfo.amount = _T( "1.234,54" );
			this->initialAccountInfo.purpose = _T( "Test-Ueberweisung" );
		}
		else
		{
			for( auto const& account : this->recentAccounts )
				this->recentAccountsCtrl.AddString( AccountLabel( account ) );
			this->recentAccountsCtrl.SetCurSel( 0 );
			this->initialAccountInfo = this->recentAccounts.front();
		}

		this->SetDlgItemText( IDC_EDIT_NAME, this->initialAccountInfo.name );
		this->SetDlgItemText( IDC_EDIT_IBAN, this->initialAccountInfo.iban );
		this->SetDlgItemText( IDC_EDIT_BIC, this->initialAccountInfo.bic );
		this->SetDlgItemText( IDC_EDIT_AMOUNT, this->initialAccountInfo.amount );
		this->SetDlgItemText( IDC_EDIT_PURPOSE, this->initialAccountInfo.purpose );
	}
	else
	{
		this->recentAccountsCtrl.EnableWindow( FALSE );
		this->MessageBox( accountLoadResult.error, _T("Kontodaten"), MB_ICONWARNING );
	}

	auto bicDatabaseLoadResult = BicDatabase::Load();
	if( not bicDatabaseLoadResult )
		this->MessageBox( bicDatabaseLoadResult.error, _T("BIC-Datenbank"), MB_ICONWARNING );
	else
		this->OnIbanChanged();

	this->UpdateQrCode();
	return TRUE;
}

void EPC_QRDlg::OnSysCommand( UINT id, LPARAM lParam )
{
    if( ( id & 0xFFF0 ) == IDM_ABOUTBOX )
    {
        CAboutDlg aboutDlg;
        aboutDlg.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand( id, lParam );
    }
}

BOOL EPC_QRDlg::PreTranslateMessage( MSG* msg )
{
    if( this->toolTip.GetSafeHwnd() != nullptr )
        this->toolTip.RelayEvent( msg );

    if( msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN )
    {
        CWnd* focused = this->GetFocus();
        if( focused != nullptr && focused->GetDlgCtrlID() != IDC_EDIT_PURPOSE )
        {
            UINT dlgCode = static_cast<UINT>( focused->SendMessage( WM_GETDLGCODE ) );
            if( ( dlgCode & DLGC_BUTTON ) == 0 )
            {
                this->NextDlgCtrl();
                return TRUE;
            }
        }
    }

    return CDialogEx::PreTranslateMessage( msg );
}


void EPC_QRDlg::OnPaint()
{
    if( this->IsIconic() )
    {
        CPaintDC dc( this );
        this->SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>( dc.GetSafeHdc() ), 0 );

        int cxIcon = GetSystemMetrics( SM_CXICON );
        int cyIcon = GetSystemMetrics( SM_CYICON );
        CRect rect;
        this->GetClientRect( &rect );
        int x = ( rect.Width() - cxIcon + 1 ) / 2;
        int y = ( rect.Height() - cyIcon + 1 ) / 2;

        dc.DrawIcon( x, y, this->icon );
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

HCURSOR EPC_QRDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>( this->icon );
}
void EPC_QRDlg::OnToClipboard()
{
    //// Erst aktuellen QR-Code erzeugen (falls noch nicht geschehen) und dann kopieren
    //this->UpdateQrCode();

    if( auto erg = this->qrCodeCtrl.CopyToClipboard(GiroCodeCtrl::toCB::qr); Success(erg) )
		AfxMessageBox( IDS_CBCOPY_QR_OK, MB_ICONINFORMATION );
	else
		AfxMessageBox( IDS_CBCOPY_ERROR, MB_ICONERROR );
}
// Rechtsklick auf "Kopieren" -> EPC-Rohdaten als Text (für Editoren / Debug)
void EPC_QRDlg::OnContextMenu( CWnd* wnd, CPoint point )
{
    if( wnd != nullptr && wnd->GetDlgCtrlID() == IDC_BTN_TOCLIPBOARD )
    {
        //this->UpdateQrCode();

        if( auto erg = this->qrCodeCtrl.CopyToClipboard(GiroCodeCtrl::toCB::text); Success(erg) )
            AfxMessageBox( IDS_CBCOPY_TEXT_OK, MB_ICONINFORMATION );
	else
		AfxMessageBox( IDS_CBCOPY_ERROR, MB_ICONERROR );
        return;
    }

    CDialogEx::OnContextMenu( wnd, point );
}

EPCString EPC_QRDlg::GenerateEPCString()
{
    CString name, iban, bic, amountText, reference, purpose;
    this->GetDlgItemText( IDC_EDIT_NAME,    name );
    this->GetDlgItemText( IDC_EDIT_IBAN,    iban );
    this->GetDlgItemText( IDC_EDIT_BIC,     bic );
    this->GetDlgItemText( IDC_EDIT_AMOUNT,  amountText );
    this->GetDlgItemText( IDC_EDIT_REF,     reference );
    this->GetDlgItemText( IDC_EDIT_PURPOSE, purpose );

	return ::GenerateEPCString( name, iban, bic, this->amountEdit.GetAmount(), reference, purpose );
}

void EPC_QRDlg::GenerateQr(EPCString const & epc)
{
    if( epc )
    {
        this->qrCodeCtrl.SetEPCData( epc );
        return;
    }

    this->qrCodeCtrl.Clear();

    switch( epc.code )
    {
		case EPCString_Error::notinitialized:
		case EPCString_Error::success:
			break;

        case EPCString_Error::NAME:        this->MessageBox( _T("Empfaengername fehlt."), _T("Fehler"), MB_ICONWARNING ); break;
        case EPCString_Error::IBAN:        this->MessageBox( _T("IBAN ist ungueltig."), _T("Fehler"), MB_ICONWARNING ); break;
        case EPCString_Error::BIC:         this->MessageBox( _T("BIC ist ungueltig oder fehlt."), _T("Fehler"), MB_ICONWARNING ); break;
        case EPCString_Error::Ref_Purpose: this->MessageBox( _T("Entweder Referenz ODER Verwendungszweck angeben."), _T("Fehler"), MB_ICONWARNING ); break;
        default:                           this->MessageBox( _T("Unbekannter Eingabefehler."), _T("Fehler"), MB_ICONWARNING ); break;
    }
}

void EPC_QRDlg::OnCancel()
{
    this->SaveAccountInfoIfChanged();
    CDialogEx::OnCancel();
}

void EPC_QRDlg::OnClose()
{
    this->SaveAccountInfoIfChanged();
    CDialogEx::OnClose();
}

void EPC_QRDlg::SaveAccountInfoIfChanged()
{
    AccountInfo currentAccountInfo;
    this->GetDlgItemText( IDC_EDIT_NAME, currentAccountInfo.name );
    this->GetDlgItemText( IDC_EDIT_IBAN, currentAccountInfo.iban );
    this->GetDlgItemText( IDC_EDIT_BIC, currentAccountInfo.bic );
    this->GetDlgItemText( IDC_EDIT_AMOUNT, currentAccountInfo.amount );
    this->GetDlgItemText( IDC_EDIT_PURPOSE, currentAccountInfo.purpose );

    if( currentAccountInfo != this->initialAccountInfo )
    {
        CString currentIban = NormalizeIban( currentAccountInfo.iban );
        auto existing = std::find_if( this->recentAccounts.begin(), this->recentAccounts.end(),
            [&currentAccountInfo, &currentIban]( AccountInfo const& account )
            {
                CString savedIban = NormalizeIban( account.iban );
                if( not currentIban.IsEmpty() )
                    return savedIban == currentIban;
                return savedIban.IsEmpty() && account.name == currentAccountInfo.name &&
                       account.bic == currentAccountInfo.bic;
            } );

        if( existing != this->recentAccounts.end() )
            this->recentAccounts.erase( existing );
        this->recentAccounts.insert( this->recentAccounts.begin(), currentAccountInfo );
        if( this->recentAccounts.size() > AccountInfoStore::MaxRecentAccounts )
            this->recentAccounts.resize( AccountInfoStore::MaxRecentAccounts );

        auto saveResult = AccountInfoStore::Save( this->recentAccounts );
        if( not saveResult )
            this->MessageBox( saveResult.error, _T("Kontodaten konnten nicht gespeichert werden"), MB_ICONWARNING );
    }
}

void EPC_QRDlg::OnRecentAccountChanged()
{
    int selection = this->recentAccountsCtrl.GetCurSel();
    if( selection < 0 || static_cast<std::size_t>( selection ) >= this->recentAccounts.size() )
        return;

    AccountInfo const& account = this->recentAccounts[selection];
    this->SetDlgItemText( IDC_EDIT_NAME, account.name );
    this->SetDlgItemText( IDC_EDIT_IBAN, account.iban );
    this->SetDlgItemText( IDC_EDIT_BIC, account.bic );
    this->SetDlgItemText( IDC_EDIT_AMOUNT, account.amount );
    this->SetDlgItemText( IDC_EDIT_PURPOSE, account.purpose );
    this->autoFilledBic.Empty();
    this->UpdateQrCode();
    this->amountEdit.SetFocus();
    this->amountEdit.SetSel( 0, -1 );
}

void EPC_QRDlg::OnUpdateBicDatabase()
{
    CWaitCursor wait;
    auto updateResult = BicDatabase::UpdateFromInternet();
    if( updateResult )
    {
        CString message;
        message.Format( _T("Die lokale Deutschland-BIC-Datenbank wurde mit %zu Zuordnungen aktualisiert."),
                        updateResult.updatedCount );
        this->MessageBox( message, _T("BIC-Datenbank"), MB_ICONINFORMATION );
        this->OnIbanChanged();
    }
    else
    {
        this->MessageBox( updateResult.error, _T("BIC-Datenbank konnte nicht aktualisiert werden"), MB_ICONWARNING );
    }
}

void EPC_QRDlg::OnIbanChanged()
{
    CString iban;
    this->GetDlgItemText( IDC_EDIT_IBAN, iban );
    auto lookupResult = BicDatabase::FindGermanIbanBic( iban );
    if( lookupResult )
    {
        this->SetDlgItemText( IDC_EDIT_BIC, lookupResult.bic );
        this->autoFilledBic = lookupResult.bic;
    }
    else
    {
        CString currentBic;
        this->GetDlgItemText( IDC_EDIT_BIC, currentBic );
        if( not this->autoFilledBic.IsEmpty() && currentBic == this->autoFilledBic )
            this->SetDlgItemText( IDC_EDIT_BIC, _T( "" ) );
        this->autoFilledBic.Empty();
    }
    this->UpdateQrCode();
}

void EPC_QRDlg::OnInputChanged()
{
    UpdateQrCode();
}

void EPC_QRDlg::UpdateQrCode()
{
    CString name, iban, bic, amountText, reference, purpose;
    this->GetDlgItemText( IDC_EDIT_NAME,    name );
    this->GetDlgItemText( IDC_EDIT_IBAN,    iban );
    this->GetDlgItemText( IDC_EDIT_BIC,     bic );
    this->GetDlgItemText( IDC_EDIT_AMOUNT,  amountText );
    this->GetDlgItemText( IDC_EDIT_REF,     reference );
    this->GetDlgItemText( IDC_EDIT_PURPOSE, purpose );

	auto notvalid = [this]()
	{
		lastEpcPayload.Empty();
        qrCodeCtrl.ShowWindow(SW_HIDE); // Oder leeres Bild setzen
		this->GetDlgItem(IDC_BTN_TOCLIPBOARD)->EnableWindow(FALSE);
	};
	auto insertAsterix = [](CString const & labelin )->CString
	{
		auto label = labelin;
		if( label.Right( 1 ) == _T(":") )
			label.Insert( label.GetLength() - 1, _T(" *") ); // Aus "BIC:" wird "BIC *:"
		else
			label += _T(" *");                                // Fallback, falls kein Doppelpunkt am Ende steht
		return label;
	};

	bool hasRef     = not reference.Trim().IsEmpty();
	bool hasPurpose = not purpose.Trim().IsEmpty();
	this->GetDlgItem( IDC_EDIT_PURPOSE )->EnableWindow( not hasRef );
	this->GetDlgItem( IDC_EDIT_REF)->EnableWindow( not hasPurpose );

	auto spec = FindSepaSpecIndex(iban);
	if( not spec.valid() ) return notvalid();
	if( spec.bicRequired )
	{
		this->SetDlgItemText( IDC_STATIC_BIC, insertAsterix(this->baseBicLabel) );
		static_cast<CEdit*>( this->GetDlgItem( IDC_EDIT_BIC) )->SetCueBanner( _T("Pflichtfeld") );
	}
	else
	{
		static_cast<CEdit*>( this->GetDlgItem( IDC_EDIT_BIC ) )->SetCueBanner( _T("Optional (Pflicht außerhalb EWR)") );
		this->SetDlgItemText(IDC_STATIC_BIC,this->baseBicLabel);
	}

	if( decltype(auto) amount = amountEdit.GetAmount(); not amount || amount.IsEmpty() )
		return notvalid();


    // EPC-Payload zusammenbauen
    auto payload = GenerateEPCString();
	if( not payload ) return notvalid();

    // Flackern & doppelte Berechnung (z.B. durch ReformatKeepCaret) vermeiden
    if (payload == lastEpcPayload)
        return;


    // QR-Code neu generieren und anzeigen
    lastEpcPayload = payload;
    GenerateQr(payload);
    qrCodeCtrl.ShowWindow(SW_SHOW);
	this->GetDlgItem(IDC_BTN_TOCLIPBOARD)->EnableWindow(TRUE);
}