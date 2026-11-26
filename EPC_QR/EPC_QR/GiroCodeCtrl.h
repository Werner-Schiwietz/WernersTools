#pragma once

#include <vector>
#include <string>
#include "EPCString.h"

#pragma region CopyToClipboardError
enum class CopyToClipboardError
{
	none,
	nodata,
	devicecontext,
	bitmap,
	openclipboard,
	nothing_copied,
	globalalloc,
	globallock,
	clipboarddata
};
inline bool Success(CopyToClipboardError value) { return value==CopyToClipboardError::none; }
#pragma endregion

class GiroCodeCtrl : public CStatic
{
public:
    GiroCodeCtrl() = default;
    virtual ~GiroCodeCtrl() = default;

    bool					SetEPCData( const EPCString& epcData );
    void					Clear();
	enum toCB
	{
		qr		= 1,
		text	= 2,
		all		= qr | text
	};
	CopyToClipboardError	CopyToClipboard( toCB, int pixelsPerModule = 10 ) const;

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd( CDC* dc );

private:
    std::vector<std::vector<bool>>	modules;
	EPCString						epcData;

    static std::string ToUTF8( const CString& text );
};

