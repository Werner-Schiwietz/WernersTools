#include "pch.h"
#include "CppUnitTest.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <vector>
#include <set>

#include "..\..\headeronly\stdex_make_shared.h"
#include "..\..\headeronly\is_.h"

#include <atlstr.h>

//template< template<typename ... > typename T,typename ... TTs  > auto foo( T<TTs...> && v )
//{
//	return v;
//}
//template< template<typename ... TTs> std::shared_ptr,typename ... TTs  > auto foo( T<TTs...> && v )
//{
//	return v;
//}

namespace stdex_//nicht schlau, der anwender soll dereferenzieren, wenn er von einem shared_ptr eine kopie will
{
	template<class T> auto make_shared( std::shared_ptr<T> & ptr)
	{
		if( ptr == nullptr )
			return std::shared_ptr<T>{}
		return stdex::make_shared( *ptr );
	}
	template<class T> auto make_shared( std::shared_ptr<T> const & ptr)
	{
		if( ptr == nullptr )
			return std::shared_ptr<T>{}
		return stdex::make_shared( *ptr );
	}
	template<class T> auto make_shared( std::shared_ptr<T> && ptr)
	{
		return std::move(ptr);
	}
}
namespace stdex
{
	template<class T> auto make_shared_fromshared( std::shared_ptr<T> const & ptr)
	{
		return stdex::make_shared( *ptr );
	}
}

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
		TEST_METHOD(UT_CreateCopy_des_shared_ptr)
		{
			auto p1 = stdex::make_shared( A{6} );
			auto constp1 = std::make_shared<A const>( A{5} );

			{
				auto p2 = stdex::make_shared( p1 );
				auto p3 = stdex::make_shared_fromshared( p1 );
				
				//Assert::IsFalse( p1 == p2 );
				//Assert::IsTrue( *p1 == *p2 );
				//weil nicht der stdex::make_shared für shared_ptr (siehe oben) gerufen wird, wird shared_ptr<shared_ptr<T>> angelegt
				Assert::IsTrue( p1 == *p2 );
				Assert::IsTrue( *p1 == **p2 );
			}
			{
				//gewünschtes verhalten
				auto p2 = stdex::make_shared( *p1 );
				Assert::IsFalse( p1 == p2 );
				Assert::IsTrue( *p1 == *p2 );
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
		TEST_METHOD(UT_test_shard_ptr_array)
		{
			struct A
			{
				std::string str;
				~A(){
				}
			};
			auto p1 = std::shared_ptr<A[2]>{ new A[2]{"Hallo","Welt"} };
			auto p2 = std::make_shared<int[3]>( size_t{4} );//array [3]{4,4,4}//liefert in c++20 was anderes, oder?
			auto p3 = std::make_shared<int[]>( size_t{4} );//array [2]//liefert in c++20 was anderes, oder?
			auto px1 = p1.get();
			auto px2 = px1+1;
			Assert::IsTrue( (*px1).str=="Hallo" );
			Assert::IsTrue( px2->str=="Welt" );
			//p1.operator*();//error C2672: 'std::shared_ptr<UTsharedextended::UT_sharedextended::UT_test_shard_ptr_array::A [2]>::operator *': no matching overloaded function found
			//auto p11 = stdex::make_shared(*p1);//error C2100: illegal indirection

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
	TEST_CLASS(UT_IsVector)
	{
		TEST_METHOD(UT_is_vector)
		{
			struct less{
				bool operator()( int const &, int const &) const;
			};
			Assert::IsTrue( WS::is_std_vector<std::vector<int>>::value );
			Assert::IsFalse( WS::is_std_vector<int>::value );
			Assert::IsFalse( WS::is_std_vector<std::set<int>>::value );
			Assert::IsFalse( WS::is_std_vector<std::set<int,less>>::value );

			Assert::IsTrue( WS::is_std_set<std::set<int>>::value );
			Assert::IsTrue( WS::is_std_set<std::set<int,less>>::value );
			Assert::IsFalse( WS::is_std_set<int>::value );
		}
		TEST_METHOD(UT_is_vector2)
		{
			auto vec = std::vector<std::vector<int>>{};
			auto const constvec = vec;
			auto & refvec = vec;
			auto const & constrefvec = vec;
			auto volatile volatilevec = vec;
			auto volatile & volatilerefvec = vec;
			auto const volatile & constvolatilerefvec = vec;

			Assert::IsTrue( WS::is_std_vector<decltype(vec)>::value );
			Assert::IsTrue( WS::is_std_vector_v<decltype(vec)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(constvec)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(refvec)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(constvec)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(constrefvec)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(volatilevec)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(volatilerefvec)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(constvolatilerefvec)> );
		}
		TEST_METHOD(UT_is_set)
		{
			auto set = std::set<std::vector<int>>{};
			auto const constset = set;
			auto & refset = set;
			auto const & constrefset = set;

			Assert::IsTrue( WS::is_std_set<decltype(set)>::value );
			Assert::IsTrue( WS::is_std_set_v<decltype(set)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(refset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constrefset)> );
		}
		TEST_METHOD(UT_is_set_less)
		{
			using container_value_t = std::vector<int>;
			struct less{
				bool operator()( container_value_t const &, container_value_t const &) const;
			};
			auto set = std::set<container_value_t,less>{};
			auto const constset = set;
			auto & refset = set;
			auto const & constrefset = set;

			Assert::IsTrue( WS::is_std_set<decltype(set)>::value );
			Assert::IsTrue( WS::is_std_set_v<decltype(set)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(refset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constrefset)> );
		}
		TEST_METHOD(UT_is_set_volatile)
		{
			using container_value_t = int volatile;

			auto set = std::set<container_value_t>{};
			auto const constset = set;
			auto & refset = set;
			auto const & constrefset = set;

			Assert::IsTrue( WS::is_std_set<decltype(set)>::value );
			Assert::IsTrue( WS::is_std_set_v<decltype(set)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(refset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constrefset)> );
		}
		TEST_METHOD(UT_is_set_volatile_less)
		{
			using container_value_t = int volatile;
			struct less{
				bool operator()( container_value_t const &, container_value_t const &) const;
			};

			auto set = std::set<container_value_t,less>{};
			auto const constset = set;
			auto & refset = set;
			auto const & constrefset = set;
			auto volatile volatileset = set;
			auto volatile & volatilerefset = set;
			auto const volatile constvolatileset = set;
			auto const volatile & constvolatilerefset = set;

			Assert::IsFalse( WS::is_std_set<std::string>::value );
			Assert::IsTrue( WS::is_std_set<decltype(set)>::value );
			Assert::IsTrue( WS::is_std_set_v<decltype(set)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(refset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constrefset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(volatileset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(volatilerefset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constvolatileset)> );
			Assert::IsTrue( WS::is_std_set_v<decltype(constvolatilerefset)> );
		}
	};
}
