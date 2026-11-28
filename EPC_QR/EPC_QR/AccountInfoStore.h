#pragma once

#include <vector>

struct AccountInfo
{
	CString name;
	CString iban;
	CString bic;
	CString amount;
	CString purpose;

	bool operator==( AccountInfo const& ) const = default;
};

class AccountInfoStore
{
public:
	static constexpr std::size_t MaxRecentAccounts = 5;

	static bool Load( std::vector<AccountInfo>& accounts, CString& error );
	static bool Save( std::vector<AccountInfo> const& accounts, CString& error );
};
