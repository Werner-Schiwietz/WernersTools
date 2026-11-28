#include "pch.h"
#include "AccountInfoStore.h"

#include <wincrypt.h>
#include <shlobj.h>
#include <algorithm>
#include <cstring>
#include <vector>

#pragma comment( lib, "Crypt32.lib" )
#pragma comment( lib, "Shell32.lib" )

namespace
{
	constexpr DWORD previousFileMagic = 0x32514345;
	constexpr DWORD fileMagic = 0x33514345;
	constexpr DWORD maxFileSize = 256 * 1024;
	constexpr wchar_t dataFolderName[] = L"Data";
	constexpr wchar_t dataFileName[] = L"account.dat";
	constexpr wchar_t appDataFolderName[] = L"EPC_QR";

	struct PathResult
	{
		bool success{};
		CString path;
		CString error;

		explicit operator bool() const noexcept { return this->success; }
	};

	CString SystemError( wchar_t const* action, DWORD code = GetLastError() )
	{
		wchar_t* message = nullptr;
		FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						nullptr, code, 0, reinterpret_cast<LPWSTR>( &message ), 0, nullptr );
		CString error;
		error.Format( L"%s (Fehler %lu)%s%s", action, code,
					  message == nullptr ? L"" : L": ", message == nullptr ? L"" : message );
		if( message != nullptr )
			LocalFree( message );
		return error;
	}

	PathResult GetExecutableDirectory()
	{
		PathResult result;
		std::vector<wchar_t> path( MAX_PATH );
		for( ;; )
		{
			DWORD length = GetModuleFileNameW( nullptr, path.data(), static_cast<DWORD>( path.size() ) );
			if( length == 0 )
			{
				result.error = SystemError( L"Pfad der Programmdatei kann nicht ermittelt werden" );
				return result;
			}
			if( length < path.size() - 1 )
			{
				result.path.SetString( path.data(), static_cast<int>( length ) );
				int separator = result.path.ReverseFind( L'\\' );
				if( separator < 0 )
				{
					result.error = L"Der Programmordner kann nicht ermittelt werden.";
					return result;
				}
				result.path.Truncate( separator );
				result.success = true;
				return result;
			}
			if( path.size() >= 32768 )
			{
				result.error = L"Der Pfad der Programmdatei ist zu lang.";
				return result;
			}
			path.resize( std::min<std::size_t>( path.size() * 2, 32768 ) );
		}
	}

	bool EnsureWritableDirectory( CString const& directory )
	{
		if( not CreateDirectoryW( directory, nullptr ) && GetLastError() != ERROR_ALREADY_EXISTS )
			return false;

		wchar_t temporary[MAX_PATH]{};
		if( GetTempFileNameW( directory, L"epc", 0, temporary ) == 0 )
			return false;
		return DeleteFileW( temporary ) != FALSE;
	}

	PathResult GetStoragePath()
	{
		PathResult result;
		auto executableDirectory = GetExecutableDirectory();
		if( not executableDirectory )
			return executableDirectory;

		CString portableDirectory = executableDirectory.path + L"\\" + dataFolderName;
		if( EnsureWritableDirectory( portableDirectory ) )
		{
			result.path = portableDirectory + L"\\" + dataFileName;
			result.success = true;
			return result;
		}

		wchar_t localAppData[MAX_PATH]{};
		HRESULT folderResult = SHGetFolderPathW( nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
											 nullptr, SHGFP_TYPE_CURRENT, localAppData );
		if( FAILED( folderResult ) )
		{
			result.error.Format( L"Der lokale Anwendungsdatenordner kann nicht ermittelt werden (HRESULT 0x%08lX).",
							 static_cast<unsigned long>( folderResult ) );
			return result;
		}

		CString fallbackDirectory = CString( localAppData ) + L"\\" + appDataFolderName;
		if( not EnsureWritableDirectory( fallbackDirectory ) )
		{
			result.error = SystemError( L"Kein beschreibbarer Speicherort für Kontodaten verfügbar" );
			return result;
		}

		result.path = fallbackDirectory + L"\\" + dataFileName;
		result.success = true;
		return result;
	}

	template<typename T>
	std::vector<BYTE> AppendValue( std::vector<BYTE> data, T value )
	{
		auto const* bytes = reinterpret_cast<BYTE const*>( &value );
		data.insert( data.end(), bytes, bytes + sizeof( value ) );
		return data;
	}

	std::vector<BYTE> AppendString( std::vector<BYTE> data, CString const& value )
	{
		DWORD length = static_cast<DWORD>( value.GetLength() );
		data = AppendValue( std::move( data ), length );
		auto const* bytes = reinterpret_cast<BYTE const*>( static_cast<LPCWSTR>( value ) );
		data.insert( data.end(), bytes, bytes + length * sizeof( wchar_t ) );
		return data;
	}

	template<typename T>
	struct ValueReadResult
	{
		bool success{};
		T value{};
		BYTE const* next{};

		explicit operator bool() const noexcept { return this->success; }
	};

	struct StringReadResult
	{
		bool success{};
		CString value;
		BYTE const* next{};

		explicit operator bool() const noexcept { return this->success; }
	};

	struct AccountReadResult
	{
		bool success{};
		AccountInfo account;
		BYTE const* next{};

		explicit operator bool() const noexcept { return this->success; }
	};

	struct AccountListReadResult
	{
		bool success{};
		std::vector<AccountInfo> accounts;

		explicit operator bool() const noexcept { return this->success; }
	};

	template<typename T>
	ValueReadResult<T> ReadValue( BYTE const* current, BYTE const* end )
	{
		ValueReadResult<T> result;
		if( static_cast<std::size_t>( end - current ) < sizeof( result.value ) )
			return result;
		memcpy( &result.value, current, sizeof( result.value ) );
		result.next = current + sizeof( result.value );
		result.success = true;
		return result;
	}

	StringReadResult ReadString( BYTE const* current, BYTE const* end )
	{
		StringReadResult result;
		auto lengthResult = ReadValue<DWORD>( current, end );
		if( not lengthResult || lengthResult.value > 4096 )
			return result;
		current = lengthResult.next;
		std::size_t byteLength = static_cast<std::size_t>( lengthResult.value ) * sizeof( wchar_t );
		if( static_cast<std::size_t>( end - current ) < byteLength )
			return result;
		result.value.SetString( reinterpret_cast<wchar_t const*>( current ), static_cast<int>( lengthResult.value ) );
		result.next = current + byteLength;
		result.success = true;
		return result;
	}

	AccountReadResult ReadAccountInfo( BYTE const* current, BYTE const* end )
	{
		AccountReadResult result;
		auto name = ReadString( current, end );
		if( not name )
			return result;
		current = name.next;
		result.account.name = std::move( name.value );

		auto iban = ReadString( current, end );
		if( not iban )
			return result;
		current = iban.next;
		result.account.iban = std::move( iban.value );

		auto bic = ReadString( current, end );
		if( not bic )
			return result;
		current = bic.next;
		result.account.bic = std::move( bic.value );

		auto amount = ReadString( current, end );
		if( not amount )
			return result;
		current = amount.next;
		result.account.amount = std::move( amount.value );

		auto purpose = ReadString( current, end );
		if( not purpose )
			return result;
		result.account.purpose = std::move( purpose.value );
		result.next = purpose.next;
		result.success = true;
		return result;
	}

	AccountListReadResult ReadAccountInfoList( BYTE const* data, DWORD size )
	{
		AccountListReadResult result;
		std::vector<AccountInfo> accounts;
		BYTE const* end = data + size;
		auto magicResult = ReadValue<DWORD>( data, end );
		if( not magicResult )
			return result;
		BYTE const* current = magicResult.next;

		if( magicResult.value == previousFileMagic )
		{
			auto account = ReadAccountInfo( current, end );
			if( not account || account.next != end )
				return result;
			accounts.push_back( std::move( account.account ) );
			result.accounts = std::move( accounts );
			result.success = true;
			return result;
		}
		if( magicResult.value != fileMagic )
			return result;

		auto countResult = ReadValue<DWORD>( current, end );
		if( not countResult || countResult.value > AccountInfoStore::MaxRecentAccounts )
			return result;
		current = countResult.next;
		for( DWORD i = 0; i < countResult.value; ++i )
		{
			auto account = ReadAccountInfo( current, end );
			if( not account )
				return result;
			current = account.next;
			accounts.push_back( std::move( account.account ) );
		}
		result.success = current == end;
		if( result.success )
			result.accounts = std::move( accounts );
		return result;
	}
}

	AccountInfoLoadResult AccountInfoStore::Load()
{
	AccountInfoLoadResult result;
	CString& error = result.error;
	auto storagePath = GetStoragePath();
	if( not storagePath )
	{
		error = storagePath.error;
		return result;
	}
	CString const& path = storagePath.path;

	HANDLE file = CreateFileW( path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL, nullptr );
	if( file == INVALID_HANDLE_VALUE )
	{
		DWORD code = GetLastError();
		if( code == ERROR_FILE_NOT_FOUND || code == ERROR_PATH_NOT_FOUND )
		{
			result.success = true;
			return result;
		}
		error = SystemError( L"Gespeicherte Kontodaten können nicht geöffnet werden", code );
		return result;
	}

	LARGE_INTEGER fileSize{};
	if( not GetFileSizeEx( file, &fileSize ) || fileSize.QuadPart <= 0 || fileSize.QuadPart > maxFileSize )
	{
		CloseHandle( file );
		error = L"Die Datei mit den gespeicherten Kontodaten hat eine ungültige Größe.";
		return result;
	}

	std::vector<BYTE> encrypted( static_cast<std::size_t>( fileSize.QuadPart ) );
	DWORD bytesRead = 0;
	BOOL readResult = ReadFile( file, encrypted.data(), static_cast<DWORD>( encrypted.size() ), &bytesRead, nullptr );
	DWORD readError = readResult ? ERROR_SUCCESS : GetLastError();
	CloseHandle( file );
	if( not readResult || bytesRead != encrypted.size() )
	{
		error = SystemError( L"Gespeicherte Kontodaten können nicht vollständig gelesen werden",
							 readError == ERROR_SUCCESS ? ERROR_HANDLE_EOF : readError );
		return result;
	}

	DATA_BLOB input{ bytesRead, encrypted.data() };
	DATA_BLOB output{};
	if( not CryptUnprotectData( &input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output ) )
	{
		error = SystemError( L"Kontodaten können für diesen Windows-Benutzer nicht entschlüsselt werden" );
		return result;
	}

	auto parsed = ReadAccountInfoList( output.pbData, output.cbData );
	SecureZeroMemory( output.pbData, output.cbData );
	LocalFree( output.pbData );
	if( not parsed )
		error = L"Die entschlüsselten Kontodaten haben ein ungültiges Format.";
	result.accounts = std::move( parsed.accounts );
	result.success = parsed.success;
	return result;
}

AccountInfoSaveResult AccountInfoStore::Save( std::vector<AccountInfo> const& accounts )
{
	AccountInfoSaveResult result;
	CString& error = result.error;
	if( accounts.size() > MaxRecentAccounts )
	{
		error = L"Es können höchstens fünf Konten gespeichert werden.";
		return result;
	}
	for( auto const& account : accounts )
	{
		if( account.name.GetLength() > 4096 || account.iban.GetLength() > 4096 ||
			account.bic.GetLength() > 4096 || account.amount.GetLength() > 4096 ||
			account.purpose.GetLength() > 4096 )
		{
			error = L"Die eingegebenen Daten sind zu lang und können nicht gespeichert werden.";
			return result;
		}
	}

	auto storagePath = GetStoragePath();
	if( not storagePath )
	{
		error = storagePath.error;
		return result;
	}
	CString const& path = storagePath.path;

	std::vector<BYTE> plain;
	plain = AppendValue( std::move( plain ), fileMagic );
	plain = AppendValue( std::move( plain ), static_cast<DWORD>( accounts.size() ) );
	for( auto const& account : accounts )
	{
		plain = AppendString( std::move( plain ), account.name );
		plain = AppendString( std::move( plain ), account.iban );
		plain = AppendString( std::move( plain ), account.bic );
		plain = AppendString( std::move( plain ), account.amount );
		plain = AppendString( std::move( plain ), account.purpose );
	}

	DATA_BLOB input{ static_cast<DWORD>( plain.size() ), plain.data() };
	DATA_BLOB output{};
	if( not CryptProtectData( &input, L"EPC QR Kontodaten", nullptr, nullptr, nullptr,
							  CRYPTPROTECT_UI_FORBIDDEN, &output ) )
	{
		SecureZeroMemory( plain.data(), plain.size() );
		error = SystemError( L"Kontodaten können nicht verschlüsselt werden" );
		return result;
	}
	SecureZeroMemory( plain.data(), plain.size() );

	wchar_t temporary[MAX_PATH]{};
	CString directory = path.Left( path.ReverseFind( L'\\' ) );
	if( GetTempFileNameW( directory, L"epc", 0, temporary ) == 0 )
	{
		SecureZeroMemory( output.pbData, output.cbData );
		LocalFree( output.pbData );
		error = SystemError( L"Temporäre Datei für Kontodaten kann nicht angelegt werden" );
		return result;
	}

	HANDLE file = CreateFileW( temporary, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL, nullptr );
	if( file == INVALID_HANDLE_VALUE )
	{
		DeleteFileW( temporary );
		SecureZeroMemory( output.pbData, output.cbData );
		LocalFree( output.pbData );
		error = SystemError( L"Temporäre Datei für Kontodaten kann nicht geöffnet werden" );
		return result;
	}

	DWORD bytesWritten = 0;
	BOOL writeResult = WriteFile( file, output.pbData, output.cbData, &bytesWritten, nullptr );
	DWORD writeError = writeResult ? ERROR_SUCCESS : GetLastError();
	BOOL flushResult = writeResult ? FlushFileBuffers( file ) : FALSE;
	if( writeResult && not flushResult )
		writeError = GetLastError();
	CloseHandle( file );
	SecureZeroMemory( output.pbData, output.cbData );
	LocalFree( output.pbData );
	if( not writeResult || bytesWritten != output.cbData || not flushResult )
	{
		DeleteFileW( temporary );
		error = SystemError( L"Kontodaten können nicht vollständig gespeichert werden",
						 writeError == ERROR_SUCCESS ? ERROR_WRITE_FAULT : writeError );
		return result;
	}

	if( not MoveFileExW( temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) )
	{
		DWORD code = GetLastError();
		DeleteFileW( temporary );
		error = SystemError( L"Die gespeicherten Kontodaten können nicht aktualisiert werden", code );
		return result;
	}
	result.success = true;
	return result;
}
