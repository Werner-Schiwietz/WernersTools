#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#include "..\..\headeronly\noimplicitcast.h"
#include "..\..\headeronly\to_underlying.h"
#include "..\..\headeronly\cout_umleitung.h"
#include "..\..\headeronly\noimplicitcast.h"

#include <sstream>
#include <iostream>
#include <string>


template<typename T>
auto do_with_t( T const & value) -> std::enable_if_t<std::is_enum_v<T>,void>
{
	std::cout << __FUNCSIG__ << " " << typeid(T).name() << " -> " << std::endl;
	do_with_t(WS::to_underlying(value));
}
template<typename T>
auto do_with_t( T const & value) -> std::enable_if_t<WS::HasMethod_to_underlying_v<T>,void>
{
	std::cout << __FUNCSIG__ << " " << typeid(T).name() << " -> " << std::endl;
	do_with_t( value.to_underlying() );
}
template<typename T>
auto do_with_t( T const & value) -> std::enable_if_t<!(std::is_enum_v<T> || WS::HasMethod_to_underlying_v<T>),void>
{
	std::cout << __FUNCSIG__ << " " << typeid(T).name() << ":" << value << std::endl;
}

namespace UT_CreateNICType
{
	CreateNICCalcNormalType(IntType,int);
	TEST_CLASS(UTCreateNICType)
	{
	public:
		TEST_METHOD(UT_tracevalue_per_parameteraufloesung)
		{
			Cout2Output<> umleitung;
			enum e1{rot,blau,grün};
			enum class e2{less,equal,greater};
			TYPEDEF(T1,unsigned) t1{42};
			TYPEDEF(T2,std::string) t2{"welt"};

			do_with_t( 5 );
			do_with_t( e1::grün );
			do_with_t( e2::equal );
			do_with_t( "hallo" );
			do_with_t( t2 );
			do_with_t( t1 );
		}
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
