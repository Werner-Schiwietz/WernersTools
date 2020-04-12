#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../../headeronly/tribool.h"

namespace tribool
{
	TEST_CLASS(tribool)
	{
	public:
		
		TEST_METHOD(tribool_compare_invalid)
		{
			WS::tribool v1;
			WS::tribool vtrue(true);
			WS::tribool vfalse(!true);

			Assert::IsFalse(v1 == vtrue);
			Assert::IsFalse(v1 == true);
			Assert::IsFalse(v1 == vfalse);
			Assert::IsFalse(v1 == false);
			Assert::IsTrue(v1 == v1);

			Assert::IsFalse(vtrue == v1);
			Assert::IsFalse(true == v1);
			Assert::IsFalse(vtrue == vfalse);
			Assert::IsTrue (vtrue == vtrue);

			Assert::IsFalse(vfalse == vtrue);
			Assert::IsFalse(vfalse == v1);
			Assert::IsFalse(false == v1);
			Assert::IsTrue (vfalse == vfalse);

			Assert::IsFalse(vfalse);
			Assert::IsTrue (vtrue);

			try
			{
				Assert::IsTrue(v1);
				Assert::Fail(L"exception erwartet. operator bool auf invalid value");
			}
			catch(...)
			{
			}
			try
			{
				Assert::IsTrue(!v1);
				Assert::Fail(L"exception erwartet. operator ! auf invalid value");
			}
			catch(...)
			{
			}
		}
		TEST_METHOD(tribool_compare)
		{
			WS::tribool v1 = true;

			Assert::IsTrue(v1 == true);
			Assert::IsFalse(v1 == false);
			Assert::IsTrue(v1 == v1);

			Assert::IsTrue(true == v1);
			
			Assert::IsFalse(false == v1);
		}
		TEST_METHOD(tribool_assign)
		{
			WS::tribool v1;

			v1 = true;
			Assert::IsTrue(v1 == true);
			v1 = false;
			Assert::IsTrue(v1 == false);

			v1 = WS::tribool{};
			Assert::IsTrue(v1 == WS::tribool{});
			Assert::IsFalse(v1 == WS::tribool{ false });
			Assert::IsFalse(v1 == WS::tribool{ true });

			v1 = WS::tribool{ false };
			Assert::IsTrue(v1 == WS::tribool{ false });
			Assert::IsFalse(v1 == WS::tribool{ true });
			Assert::IsFalse(v1 == WS::tribool{});

			v1 = WS::tribool{ true };
			Assert::IsTrue(v1 == WS::tribool{ true });
			Assert::IsFalse(v1 == WS::tribool{ false });
			Assert::IsFalse(v1 == WS::tribool{});
		}
	};
}
