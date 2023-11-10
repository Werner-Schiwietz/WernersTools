#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\Ptr_Array.h"

namespace WS
{
	class RAII_malloca : public ptr_array<void*>
	{
	public:
		using base_t = ptr_array<void*>;
		RAII_malloca( size_t bytes );
		~RAII_malloca();

		RAII_malloca( RAII_malloca const & ) = delete;
		RAII_malloca( RAII_malloca && r) : base_t( std::move(r) ){}
		RAII_malloca& operator=( RAII_malloca const & ) = delete;
		RAII_malloca& operator=( RAII_malloca && r){ base_t::operator=( std::move(r) ); return *this; }
	};
}

namespace WS
{
	RAII_malloca::RAII_malloca( size_t bytes )
	{
		*(base_t*)this = ptr_array<void*>{ _malloca( bytes ), bytes };
	}
	RAII_malloca::~RAII_malloca()
	{
		_freea( static_cast<void*>(this->memorystartpos()));
	}
}

namespace UT_RAIImalloc
{
	TEST_CLASS(UTRAIImalloc)
	{
	public:
		
		TEST_METHOD(UT_ctor_memset_dtor)
		{
			auto data = WS::RAII_malloca(100);
			memset(data,0,data.ByteLen());
		}
		TEST_METHOD(UT_fill)
		{
			auto data = WS::RAII_malloca(10);
			*data.useas<int>()++ = 5;
			*data.useas<int>()++ = 6;
			*data.useas<bool>()++ = true;

			auto reader = data.reset();
			Assert::IsTrue( *reader.useas<int>()++ == 5 );
			Assert::IsTrue( *reader.useas<int>()++ == 6 );
			Assert::IsTrue( *reader.useas<bool>()++ == true );

		}
		TEST_METHOD(UT_reuse)
		{
			auto data = WS::RAII_malloca(10);
			data = WS::RAII_malloca(50);
		}
	};
}
