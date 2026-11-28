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

	void SetSystemError( CString& error, wchar_t const* action, DWORD code = GetLastError() )
	{
		wchar_t* message = nullptr;
		FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						nullptr, code, 0, reinterpret_cast<LPWSTR>( &message ), 0, nullptr );
		error.Format( L"%s (Fehler %lu)%s%s", action, code,
					  message == nullptr ? L"" : L": ", message == nullptr ? L"" : message );
		if( message != nullptr )
			LocalFree( message );
	}

	bool GetExecutableDirectory( CString& directory, CString& error )
	{
		std::vector<wchar_t> path( MAX_PATH );
		for( ;; )
		{
			DWORD length = GetModuleFileNameW( nullptr, path.data(), static_cast<DWORD>( path.size() ) );
			if( length == 0 )
			{
				SetSystemError( error, L"Pfad der Programmdatei kann nicht ermittelt werden" );
				return false;
			}
			if( length < path.size() - 1 )
			{
				directory.SetString( path.data(), static_cast<int>( length ) );
				int separator = directory.ReverseFind( L'\\' );
				if( separator < 0 )
				{
					error = L"Der Programmordner kann nicht ermittelt werden.";
					return false;
				}
				directory.Truncate( separator );
				return true;
			}
			if( path.size() >= 32768 )
			{
				error = L"Der Pfad der Programmdatei ist zu lang.";
				return false;
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

	bool GetStoragePath( CString& filePath, CString& error )
	{
		CString executableDirectory;
		if( not GetExecutableDirectory( executableDirectory, error ) )
			return false;

		CString portableDirectory = executableDirectory + L"\\" + dataFolderName;
		if( EnsureWritableDirectory( portableDirectory ) )
		{
			filePath = portableDirectory + L"\\" + dataFileName;
			return true;
		}

		wchar_t localAppData[MAX_PATH]{};
		HRESULT result = SHGetFolderPathW( nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
										   nullptr, SHGFP_TYPE_CURRENT, localAppData );
		if( FAILED( result ) )
		{
			error.Format( L"Der lokale Anwendungsdatenordner kann nicht ermittelt werden (HRESULT 0x%08lX).",
						  static_cast<unsigned long>( result ) );
			return false;
		}

		CString fallbackDirectory = CString( localAppData ) + L"\\" + appDataFolderName;
		if( not EnsureWritableDirectory( fallbackDirectory ) )
		{
			SetSystemError( error, L"Kein beschreibbarer Speicherort für Kontodaten verfügbar" );
			return false;
		}

		filePath = fallbackDirectory + L"\\" + dataFileName;
		return true;
	}

	template<typename T>
	void AppendValue( std::vector<BYTE>& data, T value )
	{
		auto const* bytes = reinterpret_cast<BYTE const*>( &value );
		data.insert( data.end(), bytes, bytes + sizeof( value ) );
	}

	void AppendString( std::vector<BYTE>& data, CString const& value )
	{
		DWORD length = static_cast<DWORD>( value.GetLength() );
		AppendValue( data, length );
		auto const* bytes = reinterpret_cast<BYTE const*>( static_cast<LPCWSTR>( value ) );
		data.insert( data.end(), bytes, bytes + length * sizeof( wchar_t ) );
	}

	template<typename T>
	bool ReadValue( BYTE const*& current, BYTE const* end, T& value )
	{
		if( static_cast<std::size_t>( end - current ) < sizeof( value ) )
			return false;
		memcpy( &value, current, sizeof( value ) );
		current += sizeof( value );
		return true;
	}

	bool ReadString( BYTE const*& current, BYTE const* end, CString& value )
	{
		DWORD length = 0;
		if( not ReadValue( current, end, length ) || length > 4096 )
			return false;
		std::size_t byteLength = static_cast<std::size_t>( length ) * sizeof( wchar_t );
		if( static_cast<std::size_t>( end - current ) < byteLength )
			return false;
		value.SetString( reinterpret_cast<wchar_t const*>( current ), static_cast<int>( length ) );
		current += byteLength;
		return true;
	}

	bool ReadAccountInfo( BYTE const*& current, BYTE const* end, AccountInfo& info )
	{
		return ReadString( current, end, info.name ) &&
			ReadString( current, end, info.iban ) &&
			ReadString( current, end, info.bic ) &&
			ReadString( current, end, info.amount ) &&
			ReadString( current, end, info.purpose );
	}

	bool ReadAccountInfoList( BYTE const* data, DWORD size, std::vector<AccountInfo>& accounts )
	{
		BYTE const* current = data;
		BYTE const* end = data + size;
		DWORD magic = 0;
		if( not ReadValue( current, end, magic ) )
			return false;

		if( magic == previousFileMagic )
		{
			AccountInfo account;
			if( not ReadAccountInfo( current, end, account ) || current != end )
				return false;
			accounts.push_back( std::move( account ) );
			return true;
		}
		if( magic != fileMagic )
			return false;

		DWORD count = 0;
		if( not ReadValue( current, end, count ) || count > AccountInfoStore::MaxRecentAccounts )
			return false;
		for( DWORD i = 0; i < count; ++i )
		{
			AccountInfo account;
			if( not ReadAccountInfo( current, end, account ) )
				return false;
			accounts.push_back( std::move( account ) );
		}
		return current == end;
	}
}

bool AccountInfoStore::Load( std::vector<AccountInfo>& accounts, CString& error )
{
	error.Empty();
	accounts.clear();
	CString path;
	if( not GetStoragePath( path, error ) )
		return false;

	HANDLE file = CreateFileW( path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL, nullptr );
	if( file == INVALID_HANDLE_VALUE )
	{
		DWORD code = GetLastError();
		if( code == ERROR_FILE_NOT_FOUND || code == ERROR_PATH_NOT_FOUND )
			return true;
		SetSystemError( error, L"Gespeicherte Kontodaten können nicht geöffnet werden", code );
		return false;
	}

	LARGE_INTEGER fileSize{};
	if( not GetFileSizeEx( file, &fileSize ) || fileSize.QuadPart <= 0 || fileSize.QuadPart > maxFileSize )
	{
		CloseHandle( file );
		error = L"Die Datei mit den gespeicherten Kontodaten hat eine ungültige Größe.";
		return false;
	}

	std::vector<BYTE> encrypted( static_cast<std::size_t>( fileSize.QuadPart ) );
	DWORD bytesRead = 0;
	BOOL readResult = ReadFile( file, encrypted.data(), static_cast<DWORD>( encrypted.size() ), &bytesRead, nullptr );
	DWORD readError = readResult ? ERROR_SUCCESS : GetLastError();
	CloseHandle( file );
	if( not readResult || bytesRead != encrypted.size() )
	{
		SetSystemError( error, L"Gespeicherte Kontodaten können nicht vollständig gelesen werden",
						readError == ERROR_SUCCESS ? ERROR_HANDLE_EOF : readError );
		return false;
	}

	DATA_BLOB input{ bytesRead, encrypted.data() };
	DATA_BLOB output{};
	if( not CryptUnprotectData( &input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output ) )
	{
		SetSystemError( error, L"Kontodaten können für diesen Windows-Benutzer nicht entschlüsselt werden" );
		return false;
	}

	bool valid = ReadAccountInfoList( output.pbData, output.cbData, accounts );
	SecureZeroMemory( output.pbData, output.cbData );
	LocalFree( output.pbData );
	if( not valid )
		error = L"Die entschlüsselten Kontodaten haben ein ungültiges Format.";
	return valid;
}

bool AccountInfoStore::Save( std::vector<AccountInfo> const& accounts, CString& error )
{
	error.Empty();
	if( accounts.size() > MaxRecentAccounts )
	{
		error = L"Es können höchstens fünf Konten gespeichert werden.";
		return false;
	}
	for( auto const& account : accounts )
	{
		if( account.name.GetLength() > 4096 || account.iban.GetLength() > 4096 ||
			account.bic.GetLength() > 4096 || account.amount.GetLength() > 4096 ||
			account.purpose.GetLength() > 4096 )
		{
			error = L"Die eingegebenen Daten sind zu lang und können nicht gespeichert werden.";
			return false;
		}
	}

	CString path;
	if( not GetStoragePath( path, error ) )
		return false;

	std::vector<BYTE> plain;
	AppendValue( plain, fileMagic );
	AppendValue( plain, static_cast<DWORD>( accounts.size() ) );
	for( auto const& account : accounts )
	{
		AppendString( plain, account.name );
		AppendString( plain, account.iban );
		AppendString( plain, account.bic );
		AppendString( plain, account.amount );
		AppendString( plain, account.purpose );
	}

	DATA_BLOB input{ static_cast<DWORD>( plain.size() ), plain.data() };
	DATA_BLOB output{};
	if( not CryptProtectData( &input, L"EPC QR Kontodaten", nullptr, nullptr, nullptr,
							  CRYPTPROTECT_UI_FORBIDDEN, &output ) )
	{
		SecureZeroMemory( plain.data(), plain.size() );
		SetSystemError( error, L"Kontodaten können nicht verschlüsselt werden" );
		return false;
	}
	SecureZeroMemory( plain.data(), plain.size() );

	wchar_t temporary[MAX_PATH]{};
	CString directory = path.Left( path.ReverseFind( L'\\' ) );
	if( GetTempFileNameW( directory, L"epc", 0, temporary ) == 0 )
	{
		SecureZeroMemory( output.pbData, output.cbData );
		LocalFree( output.pbData );
		SetSystemError( error, L"Temporäre Datei für Kontodaten kann nicht angelegt werden" );
		return false;
	}

	HANDLE file = CreateFileW( temporary, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL, nullptr );
	if( file == INVALID_HANDLE_VALUE )
	{
		DeleteFileW( temporary );
		SecureZeroMemory( output.pbData, output.cbData );
		LocalFree( output.pbData );
		SetSystemError( error, L"Temporäre Datei für Kontodaten kann nicht geöffnet werden" );
		return false;
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
		SetSystemError( error, L"Kontodaten können nicht vollständig gespeichert werden",
						writeError == ERROR_SUCCESS ? ERROR_WRITE_FAULT : writeError );
		return false;
	}

	if( not MoveFileExW( temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) )
	{
		DWORD code = GetLastError();
		DeleteFileW( temporary );
		SetSystemError( error, L"Die gespeicherten Kontodaten können nicht aktualisiert werden", code );
		return false;
	}
	return true;
}
