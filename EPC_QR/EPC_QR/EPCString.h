#pragma once

class CEuroAmount;

struct CountryIbanSpec
{
    wchar_t code[3];
    int     length;
    bool    digitsOnlyBban; // true = ab Index 4 nur Ziffern erlaubt
    bool    bicRequired;    // true = Nicht-EWR-SEPA-Land (BIC in Zeile 5 Pflicht)

	bool valid() const noexcept { return this->length != 0; }
};
CountryIbanSpec const & FindSepaSpecIndex( CString const & iban );


enum class EPCString_Error
{
	success,
	NAME,
	IBAN,
	BIC,
	AMOUNT,
	Ref_Purpose,
	notinitialized
};
inline bool Success( EPCString_Error value ) { return value==EPCString_Error::success; }

struct EPCString
{
	CString			value;
	EPCString_Error	code{EPCString_Error::notinitialized};

	EPCString() = default;
	EPCString( CString value ) : value{value}, code{EPCString_Error::success} {}
	EPCString( EPCString_Error code ) : code{code} {}

	explicit operator bool() const noexcept { return Success(this->code); }
	operator TCHAR const *() const noexcept { return Success(this->code) ? value : nullptr; }
};
EPCString GenerateEPCString(
    CString				name,
    CString				iban,
    CString				bic,
    CEuroAmount const & euroamount,
    CString				reference,
    CString				purposeText );