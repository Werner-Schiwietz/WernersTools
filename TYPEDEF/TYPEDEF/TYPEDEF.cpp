#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\noimplicitcast.h"


namespace UT_TYPEDEF
{
	TEST_CLASS(TYPEDEF)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			{
				TYPEDEF(IntType, int);

				int i{5};
				IntType I{i};
				Assert::IsTrue(I++==IntType(i));
				Assert::IsFalse(I == IntType(i));
			}
			{
#if				_MSC_VER >= 1921 //Visual Studio 2019 version 16.1 	
					//ab 16.1 reicht TYPEDEF
					TYPEDEF(IntType, int, WP::Explicit_Type_Trait_incpre, WP::Explicit_Type_Trait_equ_less);
					TYPEDEF2(ShortType, short, WP::Explicit_Type_Trait_incpre, WP::Explicit_Type_Trait_equ_less);
#else
					TYPEDEF2(IntType, int, WP::Explicit_Type_Trait_incpre, WP::Explicit_Type_Trait_equ_less);
					TYPEDEF2(ShortType, short, WP::Explicit_Type_Trait_incpre, WP::Explicit_Type_Trait_equ_less);
#endif

				int i{ 5 };
				IntType I{ i };
				//Assert::IsTrue(I++ == IntType(i));//error C2678:  binary '++': no operator found which takes a left-hand operand of type 'UT_TYPEDEF::TYPEDEF::TestMethod1::IntType' (or there is no acceptable conversion)
				Assert::IsFalse(++I == IntType(i));
				Assert::IsTrue(I == IntType(++i));
			}

		}
	};
}