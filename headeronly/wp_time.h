#pragma once

#pragma warning(push,4)
//#pragma warning(disable:4996)

#include "noimplicitcast.h"

#include <atltime.h>
#include <ATLComTime.h>
#include <ctime>

#include <sqltypes.h>

#pragma push_macro("ASSERT")
#ifndef ASSERT
#	define ASSERT(x) ((void)0)
#endif // !ASSERT

namespace WP
{
	inline namespace CTIME
	{
		class LocalTime;

		CreateNICTypeBegin( UTC,CTime )
			explicit UTC( LocalTime const & );
		CreateNICTypeEnd;

		CreateNICTypeBegin( LocalTime,CTime )
			explicit LocalTime ( UTC const & );
			static LocalTime CurrentTime();
		CreateNICTypeEnd;


		inline UTC::UTC( LocalTime const & localtime)
		{
			CTime t =  localtime.toValueType();
			if (t.GetTime())
			{
				struct tm TM;
				t.GetGmtTm(&TM);

				this->_value = CTime(1900 + TM.tm_year, 1 + TM.tm_mon, TM.tm_mday, TM.tm_hour, TM.tm_min, TM.tm_sec);
			}
			else
				this->_value = 0;

		}

		inline LocalTime::LocalTime( UTC const & utc )
		{
			CTime t =  utc.toValueType();
			if (t.GetTime())
			{
				struct tm TM;
				t.GetLocalTm(&TM);
				__time64_t timev = _mkgmtime64(&TM);
				this->_value = CTime(timev);
			}
			else
				this->_value = 0;
		}

		inline LocalTime LocalTime::CurrentTime()
		{
			__time64_t  timev;_time64( &timev );
			//struct tm tmstruct;_gmtime64_s( &tmstruct, &timev ); 

			return LocalTime( CTime(timev) );
		}
	}

	namespace STD
	{
		class LocalTime;

		CreateNICTypeBegin( UTC,std::time_t )
			explicit UTC( LocalTime const & );
		CreateNICTypeEnd;

		CreateNICTypeBegin( LocalTime,std::time_t )
			explicit LocalTime ( UTC const & );
			static LocalTime CurrentTime();
		CreateNICTypeEnd;


		inline UTC::UTC( LocalTime const & localtime)
		{
			auto t =  localtime.toValueType();
#pragma warning(suppress:4996)
			std::tm * pTM = std::gmtime(&t);

			this->_value = mktime(  pTM );
		}

		inline LocalTime::LocalTime( UTC const & utc )
		{
			auto t =  utc.toValueType();
#pragma warning(suppress:4996)
			auto pTM = std::localtime(&t);

			this->_value  = _mkgmtime( pTM );
		}

		inline LocalTime LocalTime::CurrentTime()
		{

			return LocalTime( std::time(nullptr) );
		}
	}

	template <typename typedest, typename typesource> typedest Convert( typesource const & );

	template <> inline CTime Convert<CTime,std::time_t>( std::time_t const & r)
	{
		return CTime(r);
	}
	template <> inline std::time_t Convert<std::time_t,CTime>( CTime const & r)
	{
		return std::time_t(r.GetTime());
	}

	template <> inline WP::CTIME::UTC Convert<WP::CTIME::UTC,WP::STD::UTC>( WP::STD::UTC const & r)
	{
		return WP::CTIME::UTC( Convert<CTime>( r.toValueType() ) );
	}
	template <> inline WP::CTIME::LocalTime Convert<WP::CTIME::LocalTime,WP::STD::LocalTime>( WP::STD::LocalTime const & r)
	{
		return WP::CTIME::LocalTime( Convert<CTime>( r.toValueType() ) );
	}

	template <> inline WP::STD::UTC Convert<WP::STD::UTC,WP::CTIME::UTC>( WP::CTIME::UTC const & r)
	{
		return WP::STD::UTC( Convert<std::time_t>( r.toValueType() ) );
	}
	template <> inline WP::STD::LocalTime Convert<WP::STD::LocalTime,WP::CTIME::LocalTime>( WP::CTIME::LocalTime const & r)
	{
		return WP::STD::LocalTime( Convert<std::time_t>( r.toValueType() ) );
	}

	template <> inline COleDateTime Convert<COleDateTime> ( CTime const & r )
	{
		return COleDateTime( r.GetTime() );
	}
	template <> inline CTime Convert<CTime> ( COleDateTime const & r )
	{
		return CTime( r.GetYear(),r.GetMonth(),r.GetDay(),r.GetHour(),r.GetMinute(),r.GetSecond() );
	}

	template <> inline COleDateTime Convert<COleDateTime> ( WP::CTIME::UTC const & r )
	{
		return Convert<COleDateTime>( r.toValueType() );
	}
	template <> inline COleDateTime Convert<COleDateTime> ( WP::CTIME::LocalTime const & r )
	{
		return Convert<COleDateTime>( r.toValueType() );
	}
	template <> inline COleDateTime Convert<COleDateTime> ( WP::STD::UTC const & r )
	{
		return Convert<COleDateTime>( Convert<WP::CTIME::UTC>(r) );
	}
	template <> inline COleDateTime Convert<COleDateTime> ( WP::STD::LocalTime const & r )
	{
		return Convert<COleDateTime>( Convert<WP::CTIME::LocalTime>(r) );
	}
	template <> inline WP::CTIME::UTC Convert<WP::CTIME::UTC> ( COleDateTime const & r )
	{
		return WP::CTIME::UTC( Convert<CTime>( r ) );
	}
	template <> inline WP::CTIME::LocalTime Convert<WP::CTIME::LocalTime> ( COleDateTime const & r )
	{
		return WP::CTIME::LocalTime( Convert<CTime>( r ) );
	}
	template <> inline WP::STD::UTC Convert<WP::STD::UTC> ( COleDateTime const & r )
	{
		return WP::STD::UTC( Convert<std::time_t>(Convert<CTime>( r )) );
	}
	template <> inline WP::STD::LocalTime Convert<WP::STD::LocalTime> ( COleDateTime const & r )
	{
		return WP::STD::LocalTime( Convert<std::time_t>(Convert<CTime>( r )));
	}
	template <> inline TIMESTAMP_STRUCT Convert<TIMESTAMP_STRUCT> ( CTime const & r )
	{
		TIMESTAMP_STRUCT	retvalue{0};
		if( r.GetTime() )
		{
			retvalue.year		= decltype(retvalue.year)( r.GetYear() );
			retvalue.month		= decltype(retvalue.month)( r.GetMonth() );
			retvalue.day		= decltype(retvalue.day)( r.GetDay() );
			retvalue.hour		= decltype(retvalue.hour)( r.GetHour() );
			retvalue.minute		= decltype(retvalue.minute)( r.GetMinute()	);
			retvalue.second		= decltype(retvalue.second)( r.GetSecond()	);
			retvalue.fraction	= 0;//static_cast<SQLUINTEGER>(systemtime.wMilliseconds)*1000*1000;

		}
		return retvalue;
	}
	template <> inline TIMESTAMP_STRUCT Convert<TIMESTAMP_STRUCT> ( COleDateTime  const & r )
	{
		TIMESTAMP_STRUCT	retvalue{0};
		if( r.GetStatus() == COleDateTime::valid && r.GetYear() >= 1900 )
		{
			retvalue.year		= decltype(retvalue.year)( r.GetYear() );
			retvalue.month		= decltype(retvalue.month)( r.GetMonth() );
			retvalue.day		= decltype(retvalue.day)( r.GetDay() );
			retvalue.hour		= decltype(retvalue.hour)( r.GetHour() );
			retvalue.minute		= decltype(retvalue.minute)( r.GetMinute()	);
			retvalue.second		= decltype(retvalue.second)( r.GetSecond()	);
			retvalue.fraction	= 0;//static_cast<SQLUINTEGER>(systemtime.wMilliseconds)*1000*1000;

		}
		return retvalue;
	}
	
	template <> inline CTime Convert<CTime> ( TIMESTAMP_STRUCT const & r )
	{
		if( r.year < 1900 )
			return 0;//CTime();

		ASSERT( r.year >= 1970 && r.year <= 3000 );//kleiner kann CTime nicht
		return CTime ( r.year, r.month, r.day, r.hour, r.minute, r.second );
	}


	inline TIMESTAMP_STRUCT CurrentUTC_TIMESTAMP_STRUCT( )
	{
		TIMESTAMP_STRUCT	r;
		SYSTEMTIME			systemtime;

		GetSystemTime ( &systemtime );//UTC
		r.day		= systemtime.wDay;
		r.hour		= systemtime.wHour;
		r.minute	= systemtime.wMinute;
		r.second	= systemtime.wSecond;
		r.year		= systemtime.wYear;
		r.month		= systemtime.wMonth;
		r.fraction	= 0;//static_cast<SQLUINTEGER>(systemtime.wMilliseconds)*1000*1000;

		return r;
	}
	inline CTime ConvertUTCtoLocalTime( TIMESTAMP_STRUCT const & r )
	{
		CTime retvalue = Convert<CTime>(r);
		if( retvalue.GetTime()==0  )
			return retvalue;//CTime();

		UTC utc( retvalue );
		LocalTime lt( utc );
		retvalue = lt.toValueType();

		return retvalue;
	}
}
#pragma pop_macro("ASSERT")
#pragma warning(pop)
