#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\type_list.h"

#include <tuple>  

namespace SignatureTemplate
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		TEST_METHOD( UT_typelist_to_tuple )
		{
			short s3=3;
			short s=4;
			//WS::typelist<bool, int, short&>::tuple_t tuple(true,5, short(4));//cannot convert argument 3 from 'short' to 'short &'
			WS::typelist<bool, int, short&>::tuple_t tuple( false, 6, s3 );
			WS::typelist<bool, int, short&>::tuple_t tuple2( true, 5, s );
			Assert::IsTrue( &std::get<2>( tuple )==&s3, L"sollte referenz auf s3 sein" );
			tuple = tuple2;//!!Achtung:  die referenz auf tuple.s3 aendert sich nicht aber der inhalt von s3

			Assert::IsTrue( std::get<0>( tuple )==true, L"sollte true sein" );
			Assert::IsTrue( std::get<1>( tuple )==5, L"sollte 5 sein" );
			Assert::IsTrue( &std::get<2>( tuple )==&s3, L"sollte referenz auf s3 sein" );
			Assert::IsTrue( std::get<2>( tuple )==4, L"sollte 4 sein" );
			std::get<2>( tuple ) = short(44);
			Assert::IsTrue( std::get<2>( tuple )==44, L"sollte 44 sein" );
			Assert::IsTrue( std::get<2>( tuple )==s3, L"sollte 44 sein" );
			Assert::IsTrue( &std::get<2>( tuple2 )==&s, L"sollte 44 sein" );
		}
		TEST_METHOD( UT_typelist_types )
		{
			Assert::IsTrue( WS::typelist<bool, int, short&>::size()!=2, L"sollte 3 sein" );
			Assert::IsTrue( WS::typelist<bool, int, short&>::size()==3, L"sollte 3 sein" );
			Assert::IsTrue(std::is_same<bool,WS::typelist<bool,int,short&>::get<0>::type>::value, L"sollte bool sein" );
			Assert::IsTrue( std::is_same<bool, WS::typelist<bool, int, short&>::get_t<1>>::value==false, L"sollte int sein" );
			Assert::IsTrue( std::is_same<int, WS::typelist<bool, int, short&>::get<1>::type>::value, L"sollte int sein" );
			Assert::IsTrue( std::is_same<bool, WS::typelist<bool, int, short&>::get<2>::type>::value==false, L"sollte short& sein" );
			Assert::IsTrue( std::is_same<short&, WS::typelist<bool, int, short&>::get<2>::type>::value, L"sollte short& sein" );
		}
		TEST_METHOD(UT_signatur_types)
		{
			WS::signatur<void(int)> s1;s1;
			WS::signatur<double( double const &, double const &)> d1; 

			Assert::IsTrue(std::is_same<decltype(d1)::return_type,double>::value, L"sollte double sein" );
			Assert::IsFalse( std::is_same<decltype(d1)::parameter_typelist::get_t<0>, double>::value, L"sollte double const & sein" );
			Assert::IsTrue( std::is_same<decltype(d1)::parameter_typelist::get_t<0>, double const &>::value, L"sollte double const & sein" );
			Assert::IsFalse( std::is_same<decltype(d1)::parameter_typelist::get_t<1>, double>::value, L"sollte double const & sein" );
			Assert::IsTrue( std::is_same<decltype(d1)::parameter_typelist::get_t<1>, double const &>::value, L"sollte double const & sein" );
			//decltype(d1)::parameter_typelist::get_t<2> xxx;//error C2338: index out of bounds
		}
	};
}