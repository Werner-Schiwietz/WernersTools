#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../../headeronly/tribool.h"

namespace tribool
{
	TEST_CLASS(tribool)
	{
	public:
		
		TEST_METHOD(tribool_compare)
		{
			WP::tribool v1;
			WP::tribool vtrue(true);
			WP::tribool vfalse(!true);

			Assert::IsFalse(v1 == vtrue);
			Assert::IsFalse(v1 == vfalse);
			Assert::IsTrue(v1 == v1);

			Assert::IsFalse(vtrue == v1);
			Assert::IsFalse(vtrue == vfalse);
			Assert::IsTrue (vtrue == vtrue);

			Assert::IsFalse(vfalse == vtrue);
			Assert::IsFalse(vfalse == v1);
			Assert::IsTrue (vfalse == vfalse);

			Assert::IsFalse(vfalse);
			Assert::IsTrue (vtrue);

			try
			{
				Assert::IsTrue(v1);
				Assert::Fail(L"exception erwartet");
			}
			catch(...)
			{
			}
		}
		TEST_METHOD(tribool_assign)
		{
			WP::tribool v1;

			v1 = true;
			Assert::IsTrue(v1 == true);
			v1 = false;
			Assert::IsTrue(v1 == false);

			v1 = WP::tribool{};
			Assert::IsTrue(v1 == WP::tribool{});
			Assert::IsFalse(v1 == WP::tribool{ false });
			Assert::IsFalse(v1 == WP::tribool{ true });

			v1 = WP::tribool{ false };
			Assert::IsTrue(v1 == WP::tribool{ false });
			Assert::IsFalse(v1 == WP::tribool{ true });
			Assert::IsFalse(v1 == WP::tribool{});

			v1 = WP::tribool{ true };
			Assert::IsTrue(v1 == WP::tribool{ true });
			Assert::IsFalse(v1 == WP::tribool{ false });
			Assert::IsFalse(v1 == WP::tribool{});
		}
	};
}
