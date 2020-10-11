#include "pch.h"
#include "CppUnitTest.h"

#include <atlstr.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\stdex_make_shared.h"
#include "..\..\headeronly\is_deref_is_pointer.h"

namespace UTsharedextended
{
	struct A
	{
		using value_t = int;
		~A(){value=0xdddd'dddd;}
		A(){}
		A(A const & r):value(r.value){}
		A(value_t value):value(value){}

		operator value_t&		() &		{return value;} 
		operator value_t const &() const &	{return value;}
		operator value_t		() &&		{return value;} 

		value_t value{};
	};

	struct op_value
	{
		int i={};
		char const *p = {};
	};
	struct op_pointer : op_value
	{
		op_value * operator->(){ return this;}
	};
	struct op_deref : op_value
	{
		op_value & operator*(){ return *this;}
	};
	struct op_deref_pointer : op_deref, op_pointer
	{
	};

	TEST_CLASS(UT_sharedextended)
	{
		A getA7()
		{
			return A{7};
		}
		TEST_METHOD(UT_struct_A)
		{
			{
				auto a = A{};
				auto const & ca = a;
				{
				#pragma warning(suppress:4239)//warning C4239: nonstandard extension used: 'initializing': conversion from 'UTAllerei::UT_shared_ptr::A' to 'UTAllerei::UT_shared_ptr::A &'
					A & ra = A{6};ra;//keine gute idee, referenz auf ein rvalue, aber der compiler macht den dtor erst später??
					Assert::IsTrue(ra.value==6);

					getA7();//hier wird der destructor von A sofort durchlaufen
				#pragma warning(suppress:4239)//warning C4239: nonstandard extension used: 'initializing': conversion from 'UTAllerei::UT_shared_ptr::A' to 'UTAllerei::UT_shared_ptr::A &'
					A & ra7 = getA7();ra7;//keine gute idee, referenz auf ein rvalue, aber der compiler macht den dtor erst später??
					Assert::IsTrue(ra7.value==7);
				}
				int v = A{};v;
				//int & v = A{};//error C2440: 'initializing': cannot convert from 'UTAllerei::UT_shared_ptr::A' to 'int &'
				int & v2 = a;v2;
				int const & v3 = a;v3;
				//int & v4 = ca;//error C2440: 'initializing': cannot convert from 'const UTAllerei::UT_shared_ptr::A' to 'int &'
				int const & v4 = ca;v4;
			}
		}
		TEST_METHOD(UT_CreateCopy_des_shared_ptr_neue_referenz)
		{
			//using ptr_t = std::shared_ptr<A>;
			//using constptr_t = std::shared_ptr<A const>;

			auto p1 = stdex::make_shared( A{6} );
			auto constp1 = std::make_shared<A const>( A{5} );

			++p1->value;
			decltype(*p1) copyp1_1 = *p1;
			auto copyp1_1_ptr = stdex::make_shared(copyp1_1);
			++copyp1_1_ptr->value;
			auto & constcopyp1_1 = *constp1;
			Assert::IsTrue( constcopyp1_1.value==constp1->value );
			//constcopyp1_1.value++;//error C3490: 'value' cannot be modified because it is being accessed through a const object
			auto constcopyp1_1_ptr = stdex::make_shared(constcopyp1_1);//kopie eines shared_ptr<T const> soll shared_ptr<T> sein
			Assert::IsTrue( constcopyp1_1.value==constcopyp1_1_ptr->value );
			++constcopyp1_1_ptr->value;
			Assert::IsFalse( constcopyp1_1.value==constcopyp1_1_ptr->value );

			auto p3 = stdex::make_shared(*p1);
			++p3->value;

			//++constp1->value;//error C3892: 'constp1': you cannot assign to a variable that is const
			auto p2 = stdex::make_shared(*constp1);
			++p2->value;
		}
		TEST_METHOD(UT_CreateCopy_des_shared_ptr_neue_referenz2)
		{
			auto ptr = std::make_shared<A>(5);
			//auto ptr2 = std::make_shared<A>(ptr);//compile-fehler
			auto ptr3 = stdex::make_shared(ptr);//macht leider einen shared_ptr<shared_ptr<A>>, und keinen shared_ptr<A>
			//auto ptr4 = stdex::make_shared<decltype(ptr)::element_type>(ptr);//zu kompliziert, fehleranfällig. macht keinen shared_ptr<shared_ptr<A>>, sondern einen shared_ptr<A>

			auto ptr2 = stdex::make_shared(*ptr);//so am einfachsten, macht keinen shared_ptr<shared_ptr<A>>, sondern einen shared_ptr<A>
			Assert::IsTrue(ptr2!=ptr);
			Assert::IsTrue(*ptr2==*ptr);
			(*ptr)++;
			Assert::IsTrue(*ptr2!=*ptr);
		}
		TEST_METHOD(UT_kopie_des_shared_ptr_gleiche_referenz)
		{
			auto ptr = std::make_shared<int>(5);
			auto ptr2 = ptr;
			std::shared_ptr<decltype(ptr)::element_type const> constptr2 = ptr;//über constptr2 sind keine änderung möglich, aber änderungen an z.b. ptr2 ändern auch constptr2
			auto  test=[&]
			{
				Assert::IsTrue( ptr==ptr2);//pointer sind gleich
				Assert::IsTrue( ptr==constptr2);//pointer sind gleich
				Assert::IsTrue( *ptr==*ptr2);//inhalt dann natürlich auch
				Assert::IsTrue( *ptr==*constptr2);//inhalt dann natürlich auch
			};
			test();
			(*ptr)++;
			test();
			(*ptr2)++;
			test();
			//(*constptr2)++;//error C3892: 'constptr2': you cannot assign to a variable that is const
		}
		TEST_METHOD(UT_shared_ptr_is_pointer_test)
		{
			static_assert(std::is_pointer<int>::value==false);
			static_assert(std::is_pointer_v<int*> );
			//static_assert(std::is_pointer_v<std::unique_ptr<int>>==false);//lustiger error error C2947: expecting '>' to terminate template-argument-list, found '>>='
			static_assert(std::is_pointer_v<std::unique_ptr<int>> == false);//so geht es
			static_assert(std::is_pointer<std::unique_ptr<int>>::value==false);//nicht das gewünscht ergebnis


			std::remove_reference_t<decltype(*std::declval<int*>())> i = 5;i;


			static_assert(WS::is_dereferenceable<int>::value == false );
			static_assert(WS::is_dereferenceable<int const>::value == false );
			static_assert(WS::is_dereferenceable<int volatile>::value == false );
			static_assert(WS::is_dereferenceable<int const volatile>::value == false );

			static_assert(WS::is_dereferenceable<int*>::value == true );
			static_assert(WS::is_dereferenceable<int const *>::value == true );
			static_assert(WS::is_dereferenceable<int volatile *>::value == true );
			static_assert(WS::is_dereferenceable<int const volatile *>::value == true );

			static_assert(WS::is_dereferenceable<std::unique_ptr<int>>::value == true );
			static_assert(WS::is_dereferenceable<std::unique_ptr<int const>>::value == true );

			static_assert(WS::is_dereferenceable<std::shared_ptr<int>>::value == true );
			static_assert(WS::is_dereferenceable<std::shared_ptr<int const>>::value == true );

		}
		TEST_METHOD(UT_stdex_make_shared)
		{
			{
				auto str = CString{L"hallo"};
				static_assert( WS::is_dereferenceable_v<CString> == true );
				auto str_data = *str;
				Assert::IsTrue( str_data==L'h');
				static_assert( WS::is_pointerable_v<CString> == false);
				//auto str_dataptr = str.operator->();//error C2039: '->': is not a member of 'ATL::CStringT<wchar_t,ATL::StrTraitATL<wchar_t,ATL::ChTraitsCRT<wchar_t>>>'

				auto x = stdex::make_shared(str);//lv
				auto x1 = stdex::make_shared(CString{L"hallo"});//rv
				static_assert( std::is_same<decltype(x),decltype(x1)>::value);
				Assert::IsTrue( *x==L"hallo" );
				auto xx = stdex::make_shared(x);//shard_ptr<shared_ptr<CString>>
				Assert::IsTrue( **xx==L"hallo" );
				auto xxx = stdex::make_shared(*x);
				Assert::IsTrue( *x==L"hallo" );
			}
			{
				auto x = stdex::make_shared(CString{L"hallo"});
				Assert::IsTrue( *x==L"hallo" );
			}

		}
		TEST_METHOD(UT_test3)
		{			
			struct op_value
			{
				int i={};
				char const *p = {};
			};
			struct op_pointer : op_value
			{
				op_value * operator->(){ return this;}
			};
			struct op_deref : op_value
			{
				op_value & operator*(){ return *this;}
			};
			struct op_deref_pointer : op_deref, op_pointer
			{
			};

			static_assert(WS::is_dereferenceable<op_deref>::value == true );
			static_assert(WS::is_dereferenceable<op_deref_pointer>::value == true );


			static_assert(WS::is_dereferenceable<op_pointer>::value == false );

			op_pointer v1;
			(*v1.operator->()) = {4};
			decltype( std::declval<op_pointer>().operator->() ) pi = v1.operator->();
			++(*pi).i;
			Assert::IsTrue( v1.i == 5 );

			static_assert(WS::is_pointerable<op_pointer>::value == true );
			static_assert(WS::is_pointerable<op_deref>::value == false );
			static_assert(WS::is_pointerable<op_deref_pointer>::value == true );

		}
	};
	TEST_CLASS(UT_IsDeref_IsPointer)
	{
		TEST_METHOD(UT_IsDeref)
		{
			Assert::IsTrue( WS::is_dereferenceable<char*>::value == true );

			Assert::IsTrue( WS::is_dereferenceable<char>::value == false );

			Assert::IsTrue( WS::is_dereferenceable<std::unique_ptr<char>>::value == true );

			Assert::IsTrue( WS::is_dereferenceable<std::shared_ptr<char>>::value == true );

			Assert::IsTrue( WS::is_dereferenceable<CString>::value == true );

			Assert::IsTrue( WS::is_dereferenceable<op_value>::value == false );
			Assert::IsTrue( WS::is_dereferenceable<op_deref>::value == true );
			Assert::IsTrue( WS::is_dereferenceable<op_pointer>::value == false );
			Assert::IsTrue( WS::is_dereferenceable<op_deref_pointer>::value == true );

		}
		TEST_METHOD(UT_IsPointer)
		{
			Assert::IsTrue( WS::is_pointerable<char*>::value == true );
			Assert::IsTrue( std::is_pointer<char*>::value == true );

			Assert::IsTrue( WS::is_pointerable<char>::value == false );
			Assert::IsTrue( std::is_pointer<char>::value == false );

			Assert::IsTrue( WS::is_pointerable<std::unique_ptr<char>>::value == true );
			Assert::IsTrue( std::is_pointer<std::unique_ptr<char>>::value == false );

			Assert::IsTrue( WS::is_pointerable<std::shared_ptr<char>>::value == true );
			Assert::IsTrue( std::is_pointer<std::shared_ptr<char>>::value == false );

			Assert::IsTrue( WS::is_pointerable<CString>::value == false );
			Assert::IsTrue( std::is_pointer<CString>::value == false );

			Assert::IsTrue( WS::is_pointerable<op_value>::value == false );
			Assert::IsTrue( WS::is_pointerable<op_deref>::value == false );
			Assert::IsTrue( WS::is_pointerable<op_pointer>::value == true );
			Assert::IsTrue( WS::is_pointerable<op_deref_pointer>::value == true );

			Assert::IsTrue( std::is_pointer<op_value>::value == false );
			Assert::IsTrue( std::is_pointer<op_deref>::value == false );
			Assert::IsTrue( std::is_pointer<op_pointer>::value == false );
			Assert::IsTrue( std::is_pointer<op_deref_pointer>::value == false );

		}
	};
}

