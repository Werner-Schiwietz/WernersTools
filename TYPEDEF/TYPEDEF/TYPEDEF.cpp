#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\noimplicitcast.h"


namespace UT_CreateNICType
{
	CreateNICCalcNormalType(IntType,int);
	TEST_CLASS(UTCreateNICType)
	{
	public:
		TEST_METHOD(TestMethod1)
		{
			auto i = IntType{5};
			Assert::IsTrue(i++==IntType{5});
			Assert::IsTrue(++i==IntType{7});
			Assert::IsTrue(--i==IntType{6});
			Assert::IsTrue(i--==IntType{6});
			Assert::IsTrue(i==IntType{5});
		}
	};
}
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
					TYPEDEF(IntType, int, WS::Explicit_Type_Trait_incpre, WS::Explicit_Type_Trait_equ_less);
					TYPEDEF2(ShortType, short, WS::Explicit_Type_Trait_incpre, WS::Explicit_Type_Trait_equ_less);
#else
					TYPEDEF2(IntType, int, WS::Explicit_Type_Trait_incpre, WS::Explicit_Type_Trait_equ_less);
					TYPEDEF2(ShortType, short, WS::Explicit_Type_Trait_incpre, WS::Explicit_Type_Trait_equ_less);
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
