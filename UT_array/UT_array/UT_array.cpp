#include "pch.h"
#include "CppUnitTest.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#pragma push_macro("_ITERATOR_DEBUG_LEVEL")
#undef _ITERATOR_DEBUG_LEVEL
#define _ITERATOR_DEBUG_LEVEL 0
#pragma push_macro("_CONTAINER_DEBUG_LEVEL")
#undef _CONTAINER_DEBUG_LEVEL
#define _CONTAINER_DEBUG_LEVEL 0

#include "..\..\headeronly\array.h"
#include <algorithm>

namespace UTarray
{
	TEST_CLASS(UTarray)
	{
	public:
		TEST_METHOD(bounds_check)
		{
			WS::array<int,4> wsarray{};
			decltype(wsarray)::base_t & stdarray = wsarray;

			try
			{
				[[maybe_unused]] auto v_unchecked = stdarray[4];
			}
			catch(...)
			{
				Assert::Fail(L"keine &exception erwartet");
			}
			[[maybe_unused]] auto 
			v_checked = wsarray[0];
			v_checked = wsarray[1];
			v_checked = wsarray[2];
			v_checked = wsarray[3];
			try
			{
				v_checked = wsarray[4];
				Assert::Fail(L"exception erwartet");
			}
			catch(...)
			{}
		}
		TEST_METHOD(sort)
		{
			WS::array<int,5> wsarray{2,3,1,4,0};
			std::sort(wsarray.begin(),wsarray.end());

			int last=0;
			for( auto v : wsarray )
			{
				Assert::IsTrue(last <= v);
				last=v;
			}
			std::sort(wsarray._Unchecked_begin(),wsarray._Unchecked_end(),[](int l,int r){return r<l;});
			for( auto v : wsarray )
			{
				Assert::IsTrue(v <= last);
				last=v;
			}
		}
		TEST_METHOD(get)
		{
			//WS::array<int,5> wsarray{0,1,2,3,4,5};//error C2078: too many initializers
			WS::array<int,5> wsarray{0,1,2,3};
			Assert::IsTrue( 0 == std::get<0>(wsarray) );
			Assert::IsTrue( 3 == std::get<3>(wsarray) );
			Assert::IsTrue( 0 == std::get<4>(wsarray) );
			//[[maybe_unused]]auto v5 = std::get<5>(wsarray);//error C2338: array index out of bounds
		}
	};
}
#pragma pop_macro("_CONTAINER_DEBUG_LEVEL")
#pragma pop_macro("_ITERATOR_DEBUG_LEVEL")
