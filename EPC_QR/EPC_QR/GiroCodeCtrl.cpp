#include "pch.h"
#include "GiroCodeCtrl.h"

#include "pch.h"
#include "GiroCodeCtrl.h"
#include "qrcodegen.hpp"

//using qrcodegen::QrCode;

BEGIN_MESSAGE_MAP( GiroCodeCtrl, CStatic )
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL GiroCodeCtrl::OnEraseBkgnd( CDC* dc )
{
    return TRUE;
}

void GiroCodeCtrl::OnPaint()
{
    CPaintDC dc( this );
    CRect rect;
    this->GetClientRect( &rect );

    dc.FillSolidRect( rect, RGB( 255, 255, 255 ) );

    int qrSize = static_cast<int>( this->modules.size() );
    if( qrSize == 0 )
        return;

    int quietZone    = 4;
    int totalModules = qrSize + quietZone * 2;
    int minSide      = std::min( rect.Width(), rect.Height() );
    int moduleSize   = std::max( 1, minSide / totalModules );
    int totalPx      = totalModules * moduleSize;

    int offsetX = rect.left + ( rect.Width() - totalPx ) / 2;
    int offsetY = rect.top + ( rect.Height() - totalPx ) / 2;

    for( int y = 0; y < qrSize; ++y )
    {
        for( int x = 0; x < qrSize; ++x )
        {
            if( this->modules[y][x] )
            {
                int px = offsetX + ( x + quietZone ) * moduleSize;
                int py = offsetY + ( y + quietZone ) * moduleSize;
                dc.FillSolidRect( px, py, moduleSize, moduleSize, RGB( 0, 0, 0 ) );
            }
        }
    }
}


std::string GiroCodeCtrl::ToUTF8( const CString& text )
{
    if( text.IsEmpty() )
        return {};

    CStringW wideText( text );
    int length = WideCharToMultiByte( CP_UTF8, 0, wideText, wideText.GetLength(), nullptr, 0, nullptr, nullptr );
    std::string utf8( length, '\0' );
    WideCharToMultiByte( CP_UTF8, 0, wideText, wideText.GetLength(), utf8.data(), length, nullptr, nullptr );
    return utf8;
}

bool GiroCodeCtrl::SetEPCData( const EPCString& epcData )
{
    this->modules.clear();
	this->epcData = epcData;

    if( not this->epcData)
    {
        if( this->GetSafeHwnd() )
            this->Invalidate();
        return false;
    }

    try
    {
        std::string utf8Payload = ToUTF8( this->epcData.value );
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText( utf8Payload.c_str(), qrcodegen::QrCode::Ecc::MEDIUM );

        int size = qr.getSize();
        this->modules.assign( size, std::vector<bool>( size, false ) );
        for( int y = 0; y < size; ++y )
        {
            for( int x = 0; x < size; ++x )
                this->modules[y][x] = qr.getModule( x, y );
        }
    }
    catch( ... )
    {
        this->modules.clear();
    }

    if( this->GetSafeHwnd() )
        this->Invalidate();

    return not this->modules.empty();
}

void GiroCodeCtrl::Clear()
{
    this->modules.clear();
    if( this->GetSafeHwnd() )
        this->Invalidate();
}

CopyToClipboardError GiroCodeCtrl::CopyToClipboard( GiroCodeCtrl::toCB was, int pixelsPerModule ) const
{
	if( not this->epcData )
		return CopyToClipboardError::nodata;

    int qrSize = static_cast<int>( this->modules.size() );
    if( qrSize == 0 || pixelsPerModule < 1 )
        return CopyToClipboardError::nodata;

	if( (was & (toCB::qr|toCB::text))==0 )
		return CopyToClipboardError::nothing_copied;

    if( not ::OpenClipboard( this->GetSafeHwnd() ) )
        return CopyToClipboardError::openclipboard;
    struct CloseClipboardOnExit{ ~CloseClipboardOnExit() { ::CloseClipboard(); } } closeClipboardGuard;
    ::EmptyClipboard();
	
	if( was & toCB::qr )
	{
		int quietZone    = 4;
		int totalModules = qrSize + quietZone * 2;
		int imageSize    = totalModules * pixelsPerModule;

		// 1. Speicherkontext und Bitmap in hoher Auflösung erstellen (z.B. 10 px pro Modul -> ca. 330x330 px)
		CClientDC screenDC( nullptr );
		CDC memDC;
		if( not memDC.CreateCompatibleDC( &screenDC ) )
			return CopyToClipboardError::devicecontext;

		CBitmap bitmap;
		if( not bitmap.CreateCompatibleBitmap( &screenDC, imageSize, imageSize ) )
			return CopyToClipboardError::bitmap;

		CBitmap* oldBitmap = memDC.SelectObject( &bitmap );

		// Weißer Hintergrund inkl. Ruhezone
		memDC.FillSolidRect( 0, 0, imageSize, imageSize, RGB( 255, 255, 255 ) );

		// Schwarze Module pixelgenau zeichnen (null Unschärfe!)
		for( int y = 0; y < qrSize; ++y )
		{
			for( int x = 0; x < qrSize; ++x )
			{
				if( this->modules[y][x] )
				{
					int px = ( x + quietZone ) * pixelsPerModule;
					int py = ( y + quietZone ) * pixelsPerModule;
					memDC.FillSolidRect( px, py, pixelsPerModule, pixelsPerModule, RGB( 0, 0, 0 ) );
				}
			}
		}

		memDC.SelectObject( oldBitmap );


		// Format A: Das Bild (CF_BITMAP) – Windows übernimmt ab hier den Besitz am HBITMAP!
		::SetClipboardData( CF_BITMAP, bitmap.Detach() );
	}

	if( was & toCB::text )
    // Format B: Der EPC-Payload als Unicode-Text (CF_UNICODETEXT)
    {
        // Für Windows-Editoren \n in \r\n wandeln
        CString winText = this->epcData.value;
        winText.Replace( _T("\n"), _T("\r\n") );

        size_t bytes = ( winText.GetLength() + 1 ) * sizeof( wchar_t );
        HGLOBAL globalMem = ::GlobalAlloc( GMEM_MOVEABLE, bytes );
        if( globalMem != nullptr )
        {
            void* locked = ::GlobalLock( globalMem );
            if( locked != nullptr )
            {
                memcpy( locked, winText.GetString(), bytes );
                ::GlobalUnlock( globalMem );
                ::SetClipboardData( CF_UNICODETEXT, globalMem );
            }
            else
            {
                ::GlobalFree( globalMem );
            }
        }
    }

    return CopyToClipboardError::none;//alles ohne fehler ausgegeben
}
