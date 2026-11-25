
// EPC_QR.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CEPCQRApp:
// See EPC_QR.cpp for the implementation of this class
//

class CEPCQRApp : public CWinApp
{
public:
	CEPCQRApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CEPCQRApp theApp;
