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
	};
}
#pragma pop_macro("_CONTAINER_DEBUG_LEVEL")
#pragma pop_macro("_ITERATOR_DEBUG_LEVEL")
