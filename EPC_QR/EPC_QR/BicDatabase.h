#pragma once

#include <cstddef>

struct BicDatabaseLoadResult
{
	bool success =  true;
	CString error;

	BicDatabaseLoadResult() = default;
	BicDatabaseLoadResult( CString error ) : error{error}, success{false} {}

	explicit operator bool() const noexcept { return this->success; }
	operator CString const &() const noexcept { return this->error; }
};

struct BicDatabaseUpdateResult
{
	bool success{};
	std::size_t updatedCount{};
	CString error;

	explicit operator bool() const noexcept { return this->success; }
};

struct BicLookupResult
{
	bool success{};
	CString bic;

	explicit operator bool() const noexcept { return this->success; }
};

class BicDatabase
{
public:
	static BicDatabaseLoadResult Load();
	static BicDatabaseUpdateResult UpdateFromInternet();
	static BicLookupResult FindGermanIbanBic( CString const& iban );
};
