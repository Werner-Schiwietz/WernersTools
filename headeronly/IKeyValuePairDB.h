#pragma once



#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ = 0
struct IKeyValuePairDB
{
	virtual ~IKeyValuePairDB() {}

    virtual bool		Get(const CString& key, bool DefaultValue,	CString const & cstrPath = _T(""), CString const & cstrKompID = _T(""), bool ErzeugeKey=true) const _INTERFACE_FUNCTION_;
	virtual bool		Set(const CString& key, bool Value,			CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_;

	virtual long		Get(const CString& key, long DefaultValue,	CString const & cstrPath = _T(""), CString const & cstrKompID = _T(""), bool ErzeugeKey=true) const _INTERFACE_FUNCTION_;
    virtual bool		Set(const CString& key, long Value,			CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_;

    virtual CString		Get(const CString& key, LPCTSTR DefaultValue,  CString const & cstrPath = _T(""), CString const & cstrKompID = _T(""), bool ErzeugeKey=true) const _INTERFACE_FUNCTION_;
    virtual bool		Set(const CString& key, const CString & Value, CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_;

	virtual bool		DeleteKey (const CString& key, CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_;
	virtual bool		DeletePath (CString const & cstrPath, CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_;
};
#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ override

struct KeyValuePairDBDefault : IKeyValuePairDB
{
	virtual ~KeyValuePairDBDefault() {}

#	pragma warning(suppress:4100)
    virtual bool		Get(const CString& key, bool DefaultValue,	CString const & cstrPath = _T(""), CString const & cstrKompID = _T(""), bool ErzeugeKey=true) const _INTERFACE_FUNCTION_
	{
		return DefaultValue;
	}
#	pragma warning(suppress:4100)
	virtual bool		Set(const CString& key, bool Value,			CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_
	{
		return false;
	}

#	pragma warning(suppress:4100)
	virtual long		Get(const CString& key, long DefaultValue, CString const & cstrPath = _T(""), CString const & cstrKompID = _T(""), bool ErzeugeKey=true) const _INTERFACE_FUNCTION_
	{
		return DefaultValue;
	}
#	pragma warning(suppress:4100)
    virtual bool		Set(const CString& key, long Value,		 CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_
	{
		return false;
	}

#	pragma warning(suppress:4100)
    virtual CString		Get(const CString& key, LPCTSTR DefaultValue, CString const & cstrPath = _T(""), CString const & cstrKompID = _T(""), bool ErzeugeKey=true) const _INTERFACE_FUNCTION_
	{
		return DefaultValue;
	}
#	pragma warning(suppress:4100)
    virtual bool		Set(const CString& key, const CString & Value, CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_
	{
		return false;
	}

#	pragma warning(suppress:4100)
	virtual bool		DeleteKey (const CString& key, CString const & cstrPath = _T(""), CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_
	{
		return false;
	}
#	pragma warning(suppress:4100)
	virtual bool		DeletePath (CString const & cstrPath, CString const & cstrKompID = _T("")) _INTERFACE_FUNCTION_
	{
		return false;
	}
};

