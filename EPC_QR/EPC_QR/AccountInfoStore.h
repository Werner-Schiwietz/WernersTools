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

struct AccountInfoLoadResult
{
	bool success{};
	std::vector<AccountInfo> accounts;
	CString error;

	explicit operator bool() const noexcept { return this->success; }
};

struct AccountInfoSaveResult
{
	bool success{};
	CString error;

	explicit operator bool() const noexcept { return this->success; }
};

class AccountInfoStore
{
public:
	static constexpr std::size_t MaxRecentAccounts = 5;

	static AccountInfoLoadResult Load();
	static AccountInfoSaveResult Save( std::vector<AccountInfo> const& accounts );
};
