#pragma once

#include "char_helper.h"
#include "headeronly\noimplicitcast.h"

namespace WS
{
	template<typename string_t=CString> class CStringTGetBufferPtr
	{
	public:
		typedef string_t string_t;
		using char_t = typename string_t::XCHAR;
	private:
		string_t *		pstr	= nullptr;
		char_t *		ptr		= nullptr;

		void swap( CStringTGetBufferPtr & r )
		{
			std::swap( this->pstr, r.pstr );
			std::swap( this->ptr, r.ptr );
		}
	public:
		~CStringTGetBufferPtr()
		{
			release();
		}
		CStringTGetBufferPtr() = default;
		CStringTGetBufferPtr( CStringTGetBufferPtr && r )
		{
			swap(r);
		}
		CStringTGetBufferPtr( string_t &str ) : pstr( &str ) , ptr( str.GetBuffer()){}
		CStringTGetBufferPtr( string_t &str, int charsMinBufferLength ) : pstr( &str ) , ptr(str.GetBuffer(charsMinBufferLength)){}
		TYPEDEF(SetLength,int);
		CStringTGetBufferPtr( string_t &str, SetLength charsLength ) : pstr( &str ) , ptr(str.GetBufferSetLength(charsLength.toValueType())){}

		CStringTGetBufferPtr& operator=( CStringTGetBufferPtr const & ) = delete;
		CStringTGetBufferPtr& operator=( CStringTGetBufferPtr && r ) 
		{
			if( this->ptr && this->pstr == r.pstr )
				throw std::logic_error( __FUNCTION__ " GetBuffer ohne ReleaseBuffer auf selben string");
			CStringTGetBufferPtr (std::move(r)).swap(*this);
			return *this;
		}

		operator typename string_t::XCHAR * ()
		{
			return ptr;
		}

		void release()
		{
			if( this->pstr && this->ptr )
			{
				this->pstr->ReleaseBuffer();
				this->ptr = nullptr;
			}
		}
		auto GetBuffer()
		{
			if( this->pstr )
			{
				release();
				ptr = this->pstr->GetBuffer();
			}
			return this->ptr;
		}
		auto GetBuffer( int charsMinBufferLength )
		{
			if( this->pstr )
			{
				release();
				ptr = this->pstr->GetBuffer(charsMinBufferLength);
			}
			return this->ptr;
		}
		auto GetBufferSetLength( int charsLength )
		{
			if( this->pstr )
			{
				release();
				ptr = this->pstr->GetBufferSetLength(charsLength);
			}
			return this->ptr;
		}
	};
	


	template<typename string_t> 
#	if _MSVC_LANG >= 201703
	[[nodiscard]]//reuckgabewert darf nicht ignoriert werden, der tut die arbeit
#	endif
	auto GetBuffer( string_t & str, int charsMinBufferLength )
	{
		return 	CStringTGetBufferPtr<string_t>( str, charsMinBufferLength );
	}

	template<typename string_t> 
#	if _MSVC_LANG >= 201703
	[[nodiscard]]//reuckgabewert darf nicht ignoriert werden, der tut die arbeit
#	endif
	auto GetBuffer( string_t & str )
	{
		return 	CStringTGetBufferPtr<string_t>( str );
	}

	template<typename string_t> 
#	if _MSVC_LANG >= 201703
	[[nodiscard]]//reuckgabewert darf nicht ignoriert werden, der tut die arbeit
#	endif
	auto GetBufferSetLength( string_t & str, int charsLength )//WS::GetBufferSetLength ruft CString::GetBufferSetLength und automatisch CString::ReleaseBuffer auf
	{
		return CStringTGetBufferPtr<string_t>( str, CStringTGetBufferPtr<string_t>::SetLength(charsLength) );
	}


}