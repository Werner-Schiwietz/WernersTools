#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <string>
#include <tuple>

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
		TEST_METHOD(UT_assign_compare_reelase_debug_lol)
		{
			int i = 1;
			bool e1 = ((i=2)==i);
			bool e2 = (i==(i=3));
			bool e3 = e1 == e2;
			Assert::IsTrue( e1 == e2 );
			Assert::IsTrue( ((i=2)==i) == (i==(i=3)) );
			Assert::IsTrue(e3);

			//parameter werden von rechts nach links auf den stack gelegt. aber nur im debug(ohne optimierung) im release wird immer der selbe wert auf den stack gelegt, lol
			auto foo = [](int i1,int i2,int i3) 
			{
				return std::tuple<int,int,int>{ i1,i2,i3};
			};

			i=1;
			auto x1 = foo(++i,++i,++i);
			auto x2 = foo(i=12,i,i=14);
		#ifdef _DEBUG
			Assert::IsTrue( x1 == std::tuple<int,int,int>(4,3,2) );
			Assert::IsTrue( x2 == std::tuple<int,int,int>(12,14,14) );
		#else
			Assert::IsTrue( x1 == std::tuple<int,int,int>(4,4,4) );
			Assert::IsTrue( x2 == std::tuple<int,int,int>(12,12,12) );
			//Assert::IsTrue( x2 == std::tuple<int,int,int>(14,14,14) );
		#endif

		}
	};

}
