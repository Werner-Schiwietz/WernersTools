#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\rotate.h"

namespace UTrotate
{
	TEST_CLASS(UTrotate)
	{
	public:
		
		TEST_METHOD(rotiere_bits)
		{

			Assert::IsTrue((-1>>1)==-1);//unerwartet, aber was soll's. es wird wohl immer ein bit nachgeschoben
			Assert::IsTrue((1>>1)==0);

			Assert::IsTrue( 4 == WS::rotl(1,2));
			Assert::IsTrue( 1 == WS::rotr(4,2));

			auto v=static_cast<__int8>(0b10101010);
			auto v2 = WS::rotl(v,2);
			Assert::IsTrue( v == WS::rotl(v,2));
			Assert::IsTrue( v == WS::rotr(v,2));
			Assert::IsTrue( 0b01010101 == WS::rotl(v,1));

			for( int shift : {1,2,8,-1,-2,-8,12,-144,0} )
				Assert::IsTrue( WS::rotl(WS::rotr(v,shift),shift) == v );

			{
				auto ll = 0xf00f'5cc5'f00f'8001ll;
				for( int shift : {1,2,8,-1,-2,-8,12,-144,0} )
					Assert::IsTrue( WS::rotl(WS::rotr(ll,shift),shift) == ll );

			}
		}
		TEST_METHOD(bits1)
		{
			Assert::IsTrue(8==std::numeric_limits<unsigned char>::digits);
			Assert::IsTrue(7==std::numeric_limits<signed char>::digits);
			Assert::IsTrue(7==std::numeric_limits<char>::digits);
			Assert::IsTrue(7==std::numeric_limits<__int8>::digits);
			Assert::IsTrue(8==std::numeric_limits<unsigned __int8>::digits);

			Assert::IsTrue(15==std::numeric_limits<__int16>::digits);
			Assert::IsTrue(16==std::numeric_limits<unsigned __int16>::digits);

			Assert::IsTrue(32==std::numeric_limits<unsigned int>::digits);
			Assert::IsTrue(31==std::numeric_limits<int>::digits);
			Assert::IsTrue(32==std::numeric_limits<unsigned long>::digits);
			Assert::IsTrue(31==std::numeric_limits<long>::digits);

			Assert::IsTrue(64==std::numeric_limits<unsigned long long>::digits);
			Assert::IsTrue(63==std::numeric_limits<long long>::digits);

		}
		TEST_METHOD(bits2)
		{
			Assert::IsTrue(8==WS::bits<unsigned char>());
			Assert::IsTrue(8==WS::bits<signed char>());
			Assert::IsTrue(8==WS::bits<char>());
			Assert::IsTrue(8==WS::bits<__int8>());
			Assert::IsTrue(8==WS::bits<unsigned __int8>());

			Assert::IsTrue(16==WS::bits<__int16>());
			Assert::IsTrue(16==WS::bits<unsigned __int16>());

			Assert::IsTrue(32==WS::bits<unsigned int>());
			Assert::IsTrue(32==WS::bits<int>());
			Assert::IsTrue(32==WS::bits<unsigned long>());
			Assert::IsTrue(32==WS::bits<long>());

			Assert::IsTrue(64==WS::bits<unsigned long long>());
			Assert::IsTrue(64==WS::bits<long long>());

		}
	};
}
