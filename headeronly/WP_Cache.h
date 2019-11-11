#pragma once



#pragma warning(push,4)


#include "noimplicitcast.h"

#include <atltime.h>

#include <map>
#include <memory>

namespace WS
{
	CreateNICType(CacheDurationValidInSeconds, __time64_t);
	//CreateNICType(CacheDurationValidInMilliSeconds, __time64_t);

#	undef _INTERFACE_FUNCTION_
#	define _INTERFACE_FUNCTION_ = 0
	class IValid
	{
	public:
		bool operator() ()
		{
			return operator bool ();
		}
		virtual operator bool () _INTERFACE_FUNCTION_;
	};
#	undef _INTERFACE_FUNCTION_
#	define _INTERFACE_FUNCTION_ override
	class AllwaysValid : public IValid
	{
	public:
		virtual operator bool () _INTERFACE_FUNCTION_
		{
			return true;
		}
	};
	class ValidTill : public IValid
	{
		CTime	validtill = 0;
	public:
		ValidTill() {};
		ValidTill( CTime validtill )
			: validtill(validtill){}
		ValidTill( CacheDurationValidInSeconds validduration )
			: validtill(CTime::GetCurrentTime()+CTimeSpan(validduration.toValueType())){}

		virtual operator bool () _INTERFACE_FUNCTION_
		{
			return CTime::GetCurrentTime()<=this->validtill;
		}
	};
	class ValidDurationRefresh : public IValid
	{
		CTime	validtill = 0;
		CacheDurationValidInSeconds	validduration;
	public:
		ValidDurationRefresh() {}
		ValidDurationRefresh( CacheDurationValidInSeconds validduration )
			: validduration(validduration)
			, validtill(CTime::GetCurrentTime()+CTimeSpan(validduration.toValueType())){}

		virtual operator bool () _INTERFACE_FUNCTION_
		{
			bool retvalue = CTime::GetCurrentTime()<=this->validtill;
			if( retvalue )
				validtill = CTime::GetCurrentTime()+CTimeSpan(validduration.toValueType());
			return retvalue;
		}
	};

	template<typename value_t,typename IValid_t> class CacheItem
	{
	public:
		using value_type = value_t;
		using IValid_type = IValid_t;

	private:
		bool						valueset = false;
		value_type					value;
		IValid_type					validcheck;

	public:
		CacheItem() = default;
		CacheItem(CacheItem const &) = default;
		CacheItem(CacheItem &&) = default;
		CacheItem& operator=(CacheItem const &) = default;
		CacheItem& operator=(CacheItem &&) = default;

		CacheItem( value_t value, IValid_t validcheck ) 
			: value(std::move(value))
			, valueset(true)
			, validcheck(std::move(validcheck))
		{
		}
		bool valid()
		{
			return valueset && validcheck();
		}

		struct getValuerettype
		{
			bool			valid;
			value_t const &	value;

			operator bool()
			{
				return valid;
			}
			template<typename value_t> operator value_t const &()
			{
				return value; 
			}
		}
		getValue(  )
		{
			return getValuerettype{ valid(), value };
		}
	};


	template<typename key_t, typename value_t,typename IValid_t> class CacheItems
	{
	public:
		using key_type		= key_t;
		using value_type	= value_t;
		using IValid_type	= IValid_t;
	private:
		std::map<key_t,CacheItem<value_t,IValid_t>> values;
	public:
		void clear()
		{
			values.clear();
		}
		void invalidate( key_type key )
		{
			values[key] = CacheItem<value_type,IValid_type>( );
		}
		void insert( key_type key, value_type value, IValid_type valid )
		{
			values[key] = CacheItem<value_type,IValid_type>( value, valid );
		}
		auto getValue( key_type key )
		{
			return values[key].getValue( );
		}
		bool getValue( key_type key, value_t & value )
		{
			auto cached = getValue( key );
			value = cached.value;
			return cached;
		}
	};


}
#pragma warning(pop)
