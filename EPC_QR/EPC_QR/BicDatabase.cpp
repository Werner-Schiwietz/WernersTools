#include "pch.h"
#include "BicDatabase.h"

#include <winhttp.h>
#include <shlobj.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <vector>

#pragma comment( lib, "Winhttp.lib" )
#pragma comment( lib, "Shell32.lib" )

namespace
{
	template<typename Character, typename Traits, typename Allocator>
	Character const* StringData( std::basic_string<Character, Traits, Allocator> const& value ) noexcept
	{
		return value.c_str();
	}

	template<typename Character, class StringTraits>
	auto StringData( ATL::CStringT<Character, StringTraits> const& value ) noexcept
	{
		return value.GetString();
	}

	constexpr wchar_t dataFolderName[] = L"Data";
	constexpr wchar_t localFolderName[] = L"EPC_QR";
	constexpr wchar_t databaseFileName[] = L"blz-bic-de.csv";
	constexpr wchar_t bundesbankHost[] = L"www.bundesbank.de";
	constexpr wchar_t bundesbankPage[] = L"/de/aufgaben/unbarer-zahlungsverkehr/serviceangebot/bankleitzahlen/download-bankleitzahlen-602592";
	constexpr std::string_view csvFileName = "blz-aktuell-csv-data.csv";
	constexpr std::size_t maxDownloadSize = 20 * 1024 * 1024;

	template<typename T>
	struct OperationResult
	{
		bool success{};
		T value{};
		CString error;

		explicit operator bool() const noexcept { return this->success; }

		template<typename U = T>
		auto c_str() const noexcept -> decltype( StringData( std::declval<U const&>() ) )
			requires std::is_same_v<U, T>
		{
			return StringData( this->value );
		}
	};

	struct StatusResult
	{
		bool success{};
		CString error;

		explicit operator bool() const noexcept { return this->success; }
	};

	std::unordered_map<std::string, std::string> bicByBlz;

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

	bool EnsureWritableDirectory( CString const& directory )
	{
		if( not CreateDirectoryW( directory, nullptr ) && GetLastError() != ERROR_ALREADY_EXISTS )
			return false;

		wchar_t temporary[MAX_PATH]{};
		if( GetTempFileNameW( directory, L"bic", 0, temporary ) == 0 )
			return false;
		return DeleteFileW( temporary ) != FALSE;
	}

	OperationResult<CString> GetDatabasePath()
	{
		std::vector<wchar_t> executablePath( MAX_PATH );
		for( ;; )
		{
			DWORD length = GetModuleFileNameW( nullptr, executablePath.data(),
												static_cast<DWORD>( executablePath.size() ) );
			if( length == 0 )
			{
				return {false,{}, SystemError( L"Pfad der Programmdatei kann nicht ermittelt werden" )};
			}
			if( length < executablePath.size() - 1 )
				break;
			if( executablePath.size() >= 32768 )
			{
				return {false,{}, SystemError( L"Der Pfad der Programmdatei ist zu lang." )};
			}
			executablePath.resize( std::min<std::size_t>( executablePath.size() * 2, 32768 ) );
		}

		CString executableDirectory( executablePath.data(), static_cast<int>( wcslen( executablePath.data() ) ) );
		int separator = executableDirectory.ReverseFind( L'\\' );
		if( separator < 0 )
		{
			return {false,{}, L"Der Programmordner kann nicht ermittelt werden."};
		}

		CString portableDirectory = executableDirectory.Left( separator ) + L"\\" + dataFolderName;
		if( EnsureWritableDirectory( portableDirectory ) )
		{
			return {true, portableDirectory + L"\\" + databaseFileName, {}};
		}

		wchar_t localAppData[MAX_PATH]{};
		HRESULT folderResult = SHGetFolderPathW( nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
											 nullptr, SHGFP_TYPE_CURRENT, localAppData );
		if( FAILED( folderResult ) )
		{
			return {false,{}, SystemError( L"Der lokale Anwendungsdatenordner kann nicht ermittelt werden (HRESULT 0x%08lX).",
										   static_cast<unsigned long>( folderResult ) )};
		}

		CString fallbackDirectory = CString( localAppData ) + L"\\" + localFolderName;
		if( not EnsureWritableDirectory( fallbackDirectory ) )
		{
			return {false,{}, SystemError( L"Kein beschreibbarer Speicherort für die BIC-Datenbank verfügbar" )};
		}
		return {true, fallbackDirectory + L"\\" + databaseFileName, {}};
	}

	bool IsAsciiDigit( char value )
	{
		return value >= '0' && value <= '9';
	}

	bool IsBic( std::string_view bic )
	{
		if( bic.size() != 8 && bic.size() != 11 )
			return false;
		for( char value : bic )
		{
			if( not ( ( value >= 'A' && value <= 'Z' ) || IsAsciiDigit( value ) ) )
				return false;
		}
		return bic.substr( 4, 2 ) == "DE";
	}

	std::vector<std::string_view> SplitCsvRow( std::string_view row )
	{
		std::vector<std::string_view> fields;
		std::size_t position = 0;
		while( position <= row.size() )
		{
			if( position < row.size() && row[position] == '"' )
			{
				std::size_t start = ++position;
				while( position < row.size() )
				{
					if( row[position] == '"' )
					{
						if( position + 1 < row.size() && row[position + 1] == '"' )
						{
							position += 2;
							continue;
						}
						break;
					}
					++position;
				}
				fields.push_back( row.substr( start, position - start ) );
				if( position < row.size() )
					++position;
				if( position < row.size() && row[position] == ';' )
					++position;
				else if( position < row.size() )
					return {};
			}
			else
			{
				std::size_t end = row.find( ';', position );
				if( end == std::string_view::npos )
				{
					fields.push_back( row.substr( position ) );
					break;
				}
				fields.push_back( row.substr( position, end - position ) );
				position = end + 1;
			}
			if( position == row.size() )
			{
				fields.emplace_back();
				break;
			}
		}
		return fields;
	}

	OperationResult<std::unordered_map<std::string, std::string>> ParseBundesbankCsv( std::string const& csv )
	{
		OperationResult<std::unordered_map<std::string, std::string>> result;
		std::unordered_map<std::string, std::string> records;
		std::size_t position = 0;
		bool headerFound = false;
		while( position < csv.size() )
		{
			std::size_t end = csv.find( '\n', position );
			if( end == std::string::npos )
				end = csv.size();
			std::string_view row( csv.data() + position, end - position );
			if( not row.empty() && row.back() == '\r' )
				row.remove_suffix( 1 );
			if( not headerFound && row.size() >= 3 &&
				static_cast<unsigned char>( row[0] ) == 0xEF &&
				static_cast<unsigned char>( row[1] ) == 0xBB &&
				static_cast<unsigned char>( row[2] ) == 0xBF )
				row.remove_prefix( 3 );
			position = end + ( end < csv.size() ? 1 : 0 );
			if( row.empty() )
				continue;

			auto fields = SplitCsvRow( row );
			if( not headerFound )
			{
				if( fields.size() < 13 || fields[0] != "Bankleitzahl" ||
					fields[1] != "Merkmal" || fields[7] != "BIC" )
				{
					result.error = L"Die heruntergeladene Datei hat nicht das erwartete Bundesbank-CSV-Format.";
					return result;
				}
				headerFound = true;
				continue;
			}

			if( fields.size() < 13 || fields[1] != "1" || fields[10] == "D" )
				continue;
			std::string_view blz = fields[0];
			std::string_view bic = fields[7];
			if( blz.size() != 8 || not std::all_of( blz.begin(), blz.end(), IsAsciiDigit ) ||
				not IsBic( bic ) )
				continue;
			records[std::string( blz )] = std::string( bic );
		}

		if( not headerFound || records.size() < 100 )
		{
			result.error = L"Die Bundesbank-Datei enthält keine ausreichend vollständige BLZ/BIC-Zuordnung.";
			return result;
		}
		result.value = std::move( records );
		result.success = true;
		return result;
	}

	OperationResult<std::string> HttpGet( std::wstring const& path )
	{
		OperationResult<std::string> result;
		HINTERNET session = WinHttpOpen( L"EPC QR BIC updater/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
										 WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );
		if( session == nullptr )
		{
			result.error = SystemError( L"HTTPS-Verbindung kann nicht initialisiert werden" );
			return result;
		}
		WinHttpSetTimeouts( session, 10000, 10000, 15000, 15000 );

		HINTERNET connection = WinHttpConnect( session, bundesbankHost, INTERNET_DEFAULT_HTTPS_PORT, 0 );
		if( connection == nullptr )
		{
			result.error = SystemError( L"Verbindung zur Deutschen Bundesbank fehlgeschlagen" );
			WinHttpCloseHandle( session );
			return result;
		}

		HINTERNET request = WinHttpOpenRequest( connection, L"GET", path.c_str(), nullptr,
													WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
													WINHTTP_FLAG_SECURE );
		if( request == nullptr )
		{
			result.error = SystemError( L"HTTPS-Anfrage kann nicht erstellt werden" );
			WinHttpCloseHandle( connection );
			WinHttpCloseHandle( session );
			return result;
		}

		if( not WinHttpSendRequest( request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
								   WINHTTP_NO_REQUEST_DATA, 0, 0, 0 ) ||
			not WinHttpReceiveResponse( request, nullptr ) )
		{
			result.error = SystemError( L"HTTPS-Anfrage an die Deutsche Bundesbank ist fehlgeschlagen" );
		}
		else
		{
			DWORD status = 0;
			DWORD statusSize = sizeof( status );
			if( not WinHttpQueryHeaders( request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
										 WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize,
										 WINHTTP_NO_HEADER_INDEX ) )
			{
				result.error = SystemError( L"HTTP-Status der Bundesbank-Antwort kann nicht gelesen werden" );
			}
			else if( status != HTTP_STATUS_OK )
			{
				result.error.Format( L"Die Bundesbank antwortete mit HTTP-Status %lu.", status );
			}
			else
			{
				for( ;; )
				{
					DWORD available = 0;
					if( not WinHttpQueryDataAvailable( request, &available ) )
					{
						result.error = SystemError( L"Daten der Bundesbank-Antwort können nicht gelesen werden" );
						break;
					}
					if( available == 0 )
					{
						result.success = true;
						break;
					}
					if( result.value.size() + available > maxDownloadSize )
					{
						result.error = L"Die Bundesbank-Antwort überschreitet die erlaubte Dateigröße.";
						break;
					}
					std::size_t offset = result.value.size();
					result.value.resize( offset + available );
					DWORD bytesRead = 0;
					if( not WinHttpReadData( request, result.value.data() + offset, available, &bytesRead ) ||
						bytesRead != available )
					{
						result.error = SystemError( L"Bundesbank-Daten wurden nicht vollständig empfangen" );
						result.value.clear();
						break;
					}
				}
			}
		}

		WinHttpCloseHandle( request );
		WinHttpCloseHandle( connection );
		WinHttpCloseHandle( session );
		if( not result.success )
			result.value.clear();
		return result;
	}

	OperationResult<std::wstring> FindCurrentCsvPath( std::string const& html )
	{
		OperationResult<std::wstring> result;
		std::size_t filePosition = html.find( csvFileName );
		if( filePosition == std::string::npos )
		{
			result.error = L"Auf der Bundesbank-Seite wurde kein aktueller CSV-Download gefunden.";
			return result;
		}
		std::size_t quoteStart = html.find_last_of( "\"'", filePosition );
		std::size_t hrefStart = quoteStart == std::string::npos ? std::string::npos : html.rfind( "href", quoteStart );
		std::size_t quoteEnd = html.find_first_of( "\"'", filePosition );
		if( hrefStart == std::string::npos || quoteEnd == std::string::npos || quoteEnd <= quoteStart )
		{
			result.error = L"Der aktuelle CSV-Downloadlink auf der Bundesbank-Seite ist ungültig.";
			return result;
		}

		std::string url = html.substr( quoteStart + 1, quoteEnd - quoteStart - 1 );
		constexpr std::string_view origin = "https://www.bundesbank.de";
		if( url.compare( 0, origin.size(), origin ) == 0 )
			url.erase( 0, origin.size() );
		if( url.empty() || url.front() != '/' || url.find( ".." ) != std::string::npos ||
			url.find( csvFileName ) == std::string::npos )
		{
			result.error = L"Der gefundene Bundesbank-Downloadlink ist kein gültiger lokaler HTTPS-Pfad.";
			return result;
		}

		int required = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, url.data(),
												 static_cast<int>( url.size() ), nullptr, 0 );
		if( required <= 0 )
		{
			result.error = L"Der Bundesbank-Downloadpfad enthält ungültige Zeichen.";
			return result;
		}
		result.value.resize( required );
		MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, url.data(), static_cast<int>( url.size() ),
							 result.value.data(), required );
		result.success = true;
		return result;
	}

	StatusResult WriteDatabaseFile( CString const& path,
									std::unordered_map<std::string, std::string> const& records )
	{
		StatusResult result;
		CString directory = path.Left( path.ReverseFind( L'\\' ) );
		wchar_t temporary[MAX_PATH]{};
		if( GetTempFileNameW( directory, L"bic", 0, temporary ) == 0 )
		{
			result.error = SystemError( L"Temporäre lokale BIC-Datei kann nicht angelegt werden" );
			return result;
		}

		HANDLE file = CreateFileW( temporary, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
								  FILE_ATTRIBUTE_NORMAL, nullptr );
		if( file == INVALID_HANDLE_VALUE )
		{
			DWORD code = GetLastError();
			DeleteFileW( temporary );
			result.error = SystemError( L"Temporäre lokale BIC-Datei kann nicht geöffnet werden", code );
			return result;
		}

		std::string content = "# Deutsche Bundesbank BLZ/BIC, ohne Gewaehr\r\nBLZ;BIC\r\n";
		for( auto const& [blz, bic] : records )
			content += blz + ";" + bic + "\r\n";

		DWORD bytesWritten = 0;
		BOOL writeResult = WriteFile( file, content.data(), static_cast<DWORD>( content.size() ),
									  &bytesWritten, nullptr );
		DWORD writeError = writeResult ? ERROR_SUCCESS : GetLastError();
		BOOL flushResult = writeResult ? FlushFileBuffers( file ) : FALSE;
		if( writeResult && not flushResult )
			writeError = GetLastError();
		CloseHandle( file );
		if( not writeResult || bytesWritten != content.size() || not flushResult )
		{
			DeleteFileW( temporary );
			result.error = SystemError( L"Lokale BIC-Datenbank kann nicht vollständig geschrieben werden",
									writeError == ERROR_SUCCESS ? ERROR_WRITE_FAULT : writeError );
			return result;
		}
		if( not MoveFileExW( temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) )
		{
			DWORD code = GetLastError();
			DeleteFileW( temporary );
			result.error = SystemError( L"Lokale BIC-Datenbank kann nicht aktualisiert werden", code );
			return result;
		}
		result.success = true;
		return result;
	}

	OperationResult<std::unordered_map<std::string, std::string>> LoadLocalDatabase( CString const& path )
	{
		OperationResult<std::unordered_map<std::string, std::string>> result;
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
			result.error = SystemError( L"Lokale BIC-Datenbank kann nicht geöffnet werden", code );
			return result;
		}

		LARGE_INTEGER size{};
		if( not GetFileSizeEx( file, &size ) || size.QuadPart <= 0 || size.QuadPart > maxDownloadSize )
		{
			CloseHandle( file );
			result.error = L"Die lokale BIC-Datenbank hat eine ungültige Dateigröße.";
			return result;
		}

		std::string content( static_cast<std::size_t>( size.QuadPart ), '\0' );
		DWORD bytesRead = 0;
		BOOL readResult = ReadFile( file, content.data(), static_cast<DWORD>( content.size() ), &bytesRead, nullptr );
		DWORD readError = readResult ? ERROR_SUCCESS : GetLastError();
		CloseHandle( file );
		if( not readResult || bytesRead != content.size() )
		{
			result.error = SystemError( L"Lokale BIC-Datenbank kann nicht vollständig gelesen werden",
									 readError == ERROR_SUCCESS ? ERROR_HANDLE_EOF : readError );
			return result;
		}

		std::unordered_map<std::string, std::string> records;
		std::size_t position = 0;
		while( position < content.size() )
		{
			std::size_t end = content.find( '\n', position );
			if( end == std::string::npos )
				end = content.size();
			std::string_view line( content.data() + position, end - position );
			if( not line.empty() && line.back() == '\r' )
				line.remove_suffix( 1 );
			position = end + ( end < content.size() ? 1 : 0 );
			if( line.empty() || line.front() == '#' || line == "BLZ;BIC" )
				continue;
			std::size_t separator = line.find( ';' );
			if( separator != 8 || line.find( ';', separator + 1 ) != std::string_view::npos )
			{
				result.error = L"Die lokale BIC-Datenbank enthält eine ungültige Zeile.";
				return result;
			}
			std::string_view blz = line.substr( 0, separator );
			std::string_view bic = line.substr( separator + 1 );
			if( not std::all_of( blz.begin(), blz.end(), IsAsciiDigit ) || not IsBic( bic ) )
			{
				result.error = L"Die lokale BIC-Datenbank enthält eine ungültige BLZ/BIC-Zuordnung.";
				return result;
			}
			records[std::string( blz )] = std::string( bic );
		}
		if( records.size() < 100 )
		{
			result.error = L"Die lokale BIC-Datenbank ist leer oder unvollständig.";
			return result;
		}
		result.value = std::move( records );
		result.success = true;
		return result;
	}
}

BicDatabaseLoadResult BicDatabase::Load()
{
	auto databasePath = GetDatabasePath();
	if( not databasePath )
	{
		return {databasePath.error};
	}
	auto loadedResult = LoadLocalDatabase( databasePath.value );
	if( not loadedResult )
	{
		return {loadedResult.error};
	}
	auto loaded = std::move( loadedResult.value );

	if( loaded.empty() )
	{
		std::filesystem::path databaseFilePath( databasePath.c_str() );
		CString legacyPath = ( databaseFilePath.parent_path() / L"blz-bic.csv" ).c_str();
		auto legacyResult = LoadLocalDatabase( legacyPath );
		if( not legacyResult )
		{
			return {legacyResult.error};
		}
		auto legacyRecords = std::move( legacyResult.value );
		if( not legacyRecords.empty() )
		{
			
			if( auto writeResult = WriteDatabaseFile( databasePath.value, legacyRecords ); not writeResult )
			{
				return {writeResult.error};
			}
			loaded = std::move( legacyRecords );
		}
	}

	bicByBlz = std::move( loaded );
	return {};
}

BicLookupResult BicDatabase::FindGermanIbanBic( CString const& iban )
{
	BicLookupResult result;
	CString& bic = result.bic;
	CString normalized = iban;
	normalized.Remove( L' ' );
	normalized.Trim();
	normalized.MakeUpper();
	if( normalized.GetLength() < 12 || normalized.Left( 2 ) != L"DE" )
		return result;

	std::string blz;
	blz.reserve( 8 );
	for( int index = 4; index < 12; ++index )
	{
		wchar_t digit = normalized[index];
		if( digit < L'0' || digit > L'9' )
			return result;
		blz.push_back( static_cast<char>( digit ) );
	}

	auto found = bicByBlz.find( blz );
	if( found == bicByBlz.end() )
		return result;
	for( char character : found->second )
		bic += static_cast<wchar_t>( character );
	result.success = true;
	return result;
}

BicDatabaseUpdateResult BicDatabase::UpdateFromInternet()
{
	BicDatabaseUpdateResult result;
	auto pageResult = HttpGet( bundesbankPage );
	if( not pageResult )
	{
		result.error = pageResult.error;
		return result;
	}

	auto csvPathResult = FindCurrentCsvPath( pageResult.value );
	if( not csvPathResult )
	{
		result.error = csvPathResult.error;
		return result;
	}

	auto csvResult = HttpGet( csvPathResult.value );
	if( not csvResult )
	{
		result.error = csvResult.error;
		return result;
	}

	auto parseResult = ParseBundesbankCsv( csvResult.value );
	if( not parseResult )
	{
		result.error = parseResult.error;
		return result;
	}
	auto downloaded = std::move( parseResult.value );

	auto databasePath = GetDatabasePath();
	if( not databasePath )
	{
		result.error = databasePath.error;
		return result;
	}
	auto writeResult = WriteDatabaseFile( databasePath.value, downloaded );
	if( not writeResult )
	{
		result.error = writeResult.error;
		return result;
	}

	bicByBlz = std::move( downloaded );
	result.updatedCount = bicByBlz.size();
	result.success = true;
	return result;
}
