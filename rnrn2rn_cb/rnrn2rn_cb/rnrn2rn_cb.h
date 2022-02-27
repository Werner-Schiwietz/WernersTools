
// rnrn2rn_cb.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Crnrn2rncbApp:
// See rnrn2rn_cb.cpp for the implementation of this class
//

class Crnrn2rncbApp : public CWinApp
{
public:
	Crnrn2rncbApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Crnrn2rncbApp theApp;
