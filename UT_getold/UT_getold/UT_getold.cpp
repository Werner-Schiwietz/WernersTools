#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <string>
#include <tuple>

#include "..\..\headeronly\getold.h"

namespace UT_getold
{
	TEST_CLASS(UT_getold)
	{
	public:
		TEST_METHOD(UT_short)
		{
			using T=short;
			T v = 5;
			auto vo = WS::getold{v} = 6;
			Assert::IsTrue( vo == 5 );
			vo = WS::getold{v} = 7;
			Assert::IsTrue( vo == 6 );
		}
		TEST_METHOD(UT_unique_ptr_short)
		{
			using v_t = short;
			using T=std::unique_ptr<v_t>;
			auto v = T{ new short(5) };
			auto vo = WS::getold{v} = T{ new short(6) };
			Assert::IsTrue( *vo == 5 );
			vo = WS::getold{v} = T{ new short(7) };
			Assert::IsTrue( *vo == 6 );
		}
	};
}
