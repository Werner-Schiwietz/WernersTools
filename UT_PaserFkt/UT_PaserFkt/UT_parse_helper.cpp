#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <algorithm>

#include "..\..\headeronly\parse_helper.h"
#include "..\..\headeronly\iterator_access.h"

//TODO idee compilierbar machen
//#include "..\..\headeronly\SignatureTest.h"
//namespace WS
//{
//	template<typename function_t, typename signatur_t> struct canCall;
//	template<typename function_t,typename ret_t,typename ... args_t> struct canCall<function_t, ret_t(args_t...)> : has_signatur<ret_t(args_t...)>
//	{
//		static bool const value = callable<function_t>();
//	};
//
//	template<typename T, typename function_t, typename signatur_t, bool> struct call;
//	template<typename T,typename function_t,typename ,typename ret_t,typename ... args_t,bool=canCall<function_t,ret_t(args_t...)>::value> struct call<function_t,ret_t(args_t...),true>
//	{
//	};
//	template<typename T,typename function_t,typename ,typename ret_t,typename ... args_t,bool =canCall<function_t,ret_t(args_t...)>::value> struct call<function_t,ret_t(args_t...),canCall<function_t,ret_t(args_t...)>::value,false>
//	{
//	};
//}


namespace UTPaserFkt
{
	TEST_CLASS(UT_OneChar)
	{
	public:
		
		TEST_METHOD(eat_negativ)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			Assert::IsFalse(WS::eat( toparse, 'H' ));
			Assert::IsTrue( len1 == toparse.len() );
		}
		TEST_METHOD(eat_positiv)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			Assert::IsTrue(WS::eat( toparse, 'h' ));
			Assert::IsTrue( len1 == toparse.len() + 1 );
		}
		TEST_METHOD(eat_positiv_all)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			auto len = len1-len1;//0
			Assert::IsTrue(WS::eat( toparse, 'h' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'a' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'l' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'l' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'o' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue( len1 == len );
		}
	};
	TEST_CLASS(UT_Chars)
	{
	public:

		TEST_METHOD(eat_negativ)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			auto erg = WS::eat( toparse, WS::iterator_access( "haL" ));
			Assert::IsFalse(erg);
			Assert::IsTrue( len1 == toparse.len() );
			Assert::IsTrue( erg.eaten_till_error==toparse.left( 2 ) );
		}
		TEST_METHOD(eat_positiv)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			Assert::IsTrue(WS::eat( toparse, WS::iterator_access("hal") ));
			Assert::IsTrue( len1 == toparse.len() + 3 );
		}
		TEST_METHOD(eat_positiv_all)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			auto len = len1-len1;//0
			Assert::IsTrue(WS::eat( toparse, WS::iterator_access("hal") ));
			Assert::IsTrue(WS::eat( toparse, WS::iterator_access("lo") ));
			Assert::IsTrue( toparse.len()==0 );
		}
	};
	TEST_CLASS( UT_eat_while )
	{
	public:

		TEST_METHOD( eat_negativ )
		{
			auto fnIsdigit = []( auto Char ) { return isdigit( Char ); };
			auto toparse = WS::iterator_access( "Hallo" );
			Assert::IsTrue( 0==WS::eat_while( toparse, fnIsdigit ).len() );
			Assert::IsTrue( toparse == WS::iterator_access("Hallo") );
		}
		TEST_METHOD( eat_positiv )
		{
			auto fnIsdigit = []( auto Char ) { return isdigit( Char ); };
			{
				auto toparse = WS::iterator_access( "123Hallo" );
				Assert::IsTrue( 3==WS::eat_while( toparse, fnIsdigit ).len() );
				Assert::IsTrue( toparse == WS::iterator_access("Hallo") );
			}
			{
				auto toparse = WS::iterator_access( L"123Hallo" );
				Assert::IsTrue( 3==WS::eat_while( toparse, fnIsdigit ).len() );
				Assert::IsTrue( toparse == WS::iterator_access(L"Hallo") );
			}
		}
	};
	TEST_CLASS( UT_eat_komplex )
	{
	public:

		TEST_METHOD( UT_eat_integer_negativ )
		{
			{
				auto toparse = WS::iterator_access( L"Hallo" );
				auto erg = WS::eat_integer<int>( toparse );
				Assert::IsFalse( erg );
			}
			{
				auto toparse = WS::iterator_access( L"" );
				auto erg = WS::eat_integer<int>( toparse );
				Assert::IsFalse( erg );
			}
			{
				WS::_iterator_access<char *> toparse;
				auto erg = WS::eat_integer<unsigned short>( toparse );
				Assert::IsFalse( erg );
			}
		}
		TEST_METHOD( UT_eat_integer_positiv )
		{
			{
				auto toparse = WS::iterator_access( L"123Hallo" );
				auto erg = WS::eat_integer<int>( toparse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.value==123 );
			}
			{
				auto toparse = WS::iterator_access( "123" );
				auto erg = WS::eat_integer<unsigned short>( toparse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.value==123 );
			}
		}
		TEST_METHOD( UT_eat_integer_positiv_overflow )
		{
			{
				auto toparse = WS::iterator_access( L"1234" );
				try
				{
					auto erg = WS::eat_integer<__int8>( toparse );
					Assert::Fail( L"should overflow" );
				}
				catch(...)
				{}
			}
		}
	};
}