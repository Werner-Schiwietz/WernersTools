#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <string>

#include "..\..\headeronly\modified.h"

namespace UT_modified
{
	TEST_CLASS(UT_modified)
	{
	public:
		
	#if _HAS_CXX17
		TEST_METHOD(UT_modfied_short)
		{
			using T=short;
			T v1=0;
			T v2=2;

			Assert::IsTrue(		WS::modified{v1}	= v2 );
			Assert::IsTrue(		WS::notmodified{v1}	= v2 );
			Assert::IsFalse(	WS::modified{v1}	= v2 );
			Assert::IsTrue(		WS::modified{v1}	= 5 );
			Assert::IsTrue(		WS::notmodified{v1}	= 5 );
			Assert::IsFalse(	WS::modified{v1}	= 5 );

			//Assert::IsTrue( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
			//Assert::IsFalse( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
		}
		TEST_METHOD(UT_modfied_user_t)
		{
			struct my_type
			{
				std::string text;
			#if _HAS_CXX20 
				bool operator==(my_type const & r) const = default;//c++20
			#else
				bool operator==(my_type const & r) const{ return this->text==r.text; }
			#endif
			};
			using T=my_type;
			T v1;
			T v2{"hallo"};

			Assert::IsTrue(		WS::modified{v1}	= v2 );
			Assert::IsTrue(		WS::notmodified{v1}	= v2 );
			Assert::IsFalse(	WS::modified{v1}	= v2 );
			Assert::IsTrue(		WS::modified{v1}	= T{"welt"} );
			Assert::IsTrue(		WS::notmodified{v1}	= T{"welt"} );
			Assert::IsFalse(	WS::modified{v1}	= T{"welt"} );

			//Assert::IsTrue( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
			//Assert::IsFalse( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
		}
	#else
		TEST_METHOD(UT_modfied_short)
		{
			using T=short;
			T v1=0;
			T v2=2;

			Assert::IsTrue(		WS::modified<T>{v1}		= v2 );
			Assert::IsTrue(		WS::notmodified<T>{v1}	= v2 );
			Assert::IsFalse(	WS::modified<T>{v1}		= v2 );
			Assert::IsTrue(		WS::modified<T>{v1}		= 5 );
			Assert::IsTrue(		WS::notmodified<T>{v1}	= 5 );
			Assert::IsFalse(	WS::modified<T>{v1}		= 5 );

			//Assert::IsTrue( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
			//Assert::IsFalse( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
		}
		TEST_METHOD(UT_modfied_user_t)
		{
			struct my_type
			{
				std::string text;
			#if _HAS_CXX20 
				bool operator==(my_type const & r) const = default;//c++20
			#else
				bool operator==(my_type const & r) const{ return this->text==r.text; }
			#endif
			};
			using T=my_type;
			T v1;
			T v2{"hallo"};

			Assert::IsTrue(		WS::modified<T>{v1}	= v2 );
			Assert::IsTrue(		WS::notmodified<T>{v1}	= v2 );
			Assert::IsFalse(	WS::modified<T>{v1}	= v2 );
			Assert::IsTrue(		WS::modified<T>{v1}	= T{"welt"} );
			Assert::IsTrue(		WS::notmodified<T>{v1}	= T{"welt"} );
			Assert::IsFalse(	WS::modified<T>{v1}	= T{"welt"} );

			//Assert::IsTrue( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
			//Assert::IsFalse( WS::modified{v1}	= 50000 );//warning C4309: 'argument': truncation of constant value
		}
	#endif
	};
}
