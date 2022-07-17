#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\dtor_call.h"

namespace WS
{
	template<class T>
	typename std::add_lvalue_reference<T>::type decllval() noexcept;
}

namespace UT_DTOR
{
	TEST_CLASS(UT_Dtor)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			std::unique_ptr<int> ptr{ new int{5} };
			std::unique_ptr<int> ptr2{ new int{6} };
			struct cleanup
			{
				std::unique_ptr<int> doit( std::unique_ptr<int> & ptr ) //&
				{
					return std::move(ptr) ;
					//geht

					//using x_t = decltype(CreateDtorCall( &cleanup::doit, &std::declval<cleanup>(), std::declval<std::unique_ptr<int>>() ));//warning C4238: nonstandard extension used: class rvalue used as lvalue
					using x_t = decltype(CreateDtorCall( &cleanup::doit, &WS::decllval<cleanup>(), std::declval<std::unique_ptr<int>>() ));
				}
				//std::unique_ptr<int> doit( std::unique_ptr<int> & ptr ) &&
				//{
				//	return std::move(ptr) ;
				//	//geht
				//	using x_t = decltype(CreateDtorCall( &cleanup::doit, &std::declval<cleanup>(), std::declval<std::unique_ptr<int>>() ));
				//}
				//geht NICHT
				//using x_t = decltype(CreateDtorCall( &aufraeumen_t::doit, &std::declval<aufraeumen_t>(), std::declval<std::unique_ptr<int>>() ));
				//using x_t = decltype(get_type());
			};
			//geht
			//using x_t = decltype(CreateDtorCall( &cleanup::doit, &std::declval<cleanup>(), std::declval<std::unique_ptr<int>>() ));//warning C4238: nonstandard extension used: class rvalue used as lvalue
			using x_t = decltype(CreateDtorCall( &cleanup::doit, &WS::decllval<cleanup>(), std::declval<std::unique_ptr<int>>() ));

			cleanup cleanup_obj;

			auto aufraeumen = CreateDtorCall( &cleanup::doit, &cleanup_obj, std::move(ptr) );
			aufraeumen = CreateDtorCall( &cleanup::doit, &cleanup_obj, std::move(ptr2) );

			Assert::IsTrue( ptr==nullptr );
			std::unique_ptr<int> ptr3;
			Assert::IsTrue(aufraeumen.CallitNow( ptr3 ) );
			Assert::IsTrue( *ptr3==6 );
			Assert::IsFalse(aufraeumen.CallitNow( ptr3 ) );
			Assert::IsTrue( *ptr3==6 );
			Assert::IsTrue( ptr2==nullptr );
		}
	};
}

namespace UT_AutoResource
{
	TEST_CLASS(UT_AutoResource)
	{
	public:
		TEST_METHOD(LockResource1)
		{
			auto locked = WS::LockResource(
				[](int )->int
				{
					return 1;
				}(5),
				[](int v)->void
				{
					Assert::IsTrue(v==1);
				}  );

			Assert::IsTrue(locked == 1);
			locked.release();
		}
		//TEST_METHOD(LockResource_void)
		//{
		//	auto locked = WS::LockResource<void(void)>(
		//		[](void)->void
		//	{
		//	}  );

		//	locked.release();
		//}
	};
}
