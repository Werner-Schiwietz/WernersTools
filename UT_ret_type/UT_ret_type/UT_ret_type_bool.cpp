#include "pch.h"
#include "CppUnitTest.h"

#include <string>
#include <tuple>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\return_type.h"

namespace UT_compare_bool
{
	TEST_CLASS(UT_ret_type)
	{
	public:
		TEST_METHOD(UT_test_ret_type_noexcept)
		{
			struct X
			{
				X() noexcept {}
				X( X const & ) noexcept(false){} 
				X( X && ) noexcept{} 
			};
			struct Z
			{
				Z() {}
				Z( Z const & ) {} 
				Z( Z && ) noexcept{} 
			};

			{
				auto noexc1 = noexcept( X() );
				auto noexc2 = noexcept( X(std::declval<X>()) );
				auto noexc3 = noexcept( X(std::declval<X const &>() ) );
				Assert::IsTrue( noexc1 );
				Assert::IsTrue( noexc2 );
				Assert::IsFalse( noexc3 );

				auto noexc1_ = noexcept( WS::return_type<X>() );
				auto noexc2_ = noexcept( WS::return_type<X>(std::declval<X&&>()) );
				auto noexc3_ = noexcept( WS::return_type<X>( std::declval<X const &>() ) );
				Assert::AreEqual(noexc1,noexc1_);
				Assert::AreEqual(noexc2,noexc2_);
				Assert::AreEqual(noexc3,noexc3_);
			}
			{
				auto noexc1 = noexcept( Z() );								//Z()
				auto noexc2 = noexcept( Z(std::move(std::declval<Z>())) );	//Z(Z&&)
				auto noexc3 = noexcept( Z( std::declval<Z const>() ) );		//Z(Z const &)
				Assert::IsFalse( noexc1 );
				Assert::IsTrue( noexc2 );
				Assert::IsFalse( noexc3 );

				auto noexc1_ = noexcept( WS::return_type<Z>() );							//Z()
				auto noexc2_ = noexcept( WS::return_type<Z>(std::declval<Z>()) );			//Z(Z&&)
				auto noexc3_ = noexcept( WS::return_type<Z>(std::declval<Z const &>() ) );	//Z(Z const &)
				Assert::AreEqual(noexc1,noexc1_);
				Assert::AreEqual(noexc2,noexc2_);
				Assert::AreEqual(noexc3,noexc3_);
			}
		}
		TEST_METHOD(UT_test_compare_bool)
		{
			struct A : WS::compare_bool
			{
				A(){}
				A(bool valid):valid(valid){}
				int  value {};
				bool valid = false;
				bool Valid() const override { return this->valid;}
			};

			Assert::IsFalse(A{});
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			Assert::IsTrue(A{true});
			Assert::IsTrue(A{true}==true);
			Assert::IsTrue(true==A{true});
			Assert::IsFalse(!A{true});
			Assert::IsFalse(A{true}!=true);
			Assert::IsFalse(true!=A{true});
		}
		TEST_METHOD(UT_test_ret_type_int)
		{
			struct A : WS::return_type<int>
			{
				using base_t = WS::return_type<int>;
				using base_t::base_t;
			};

			Assert::IsFalse(A{});
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			Assert::IsTrue(A{1});
			Assert::IsTrue(A{2}==true);
			Assert::IsTrue(true==A{3});
			Assert::IsFalse(!A{4});
			Assert::IsFalse(A{5}!=true);
			Assert::IsFalse(true!=A{6});
			if( auto a = A{6} )
			{
				Assert::IsTrue( (A::value_t)a==6 );
			}
			Assert::IsFalse(true!=A{6});
		}
		TEST_METHOD(UT_test_ret_type_string)
		{
			struct A : WS::return_type<std::string>
			{
				using base_t = WS::return_type<std::string>;
				using base_t::base_t;
			};

			Assert::IsFalse(A{});
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			Assert::IsTrue(A{"hallo"});
			Assert::IsTrue(A{"welt"}==true);
			Assert::IsTrue(true==A{"Hallo"});
			Assert::IsFalse(!A{"Welt"});
			Assert::IsFalse(A{""}!=true);
			Assert::IsFalse(true!=A{std::string{' '}});
			if( auto a = A{"s"} )
			{
				Assert::IsTrue( a=="s" );
			}
			Assert::IsFalse(true!=A{"6"});
		}
		TEST_METHOD(UT_test_ret_type_string_return)
		{
			auto fn =[](char const * text )->WS::return_type<std::string>{if(text)return {text};return {}; };

			if( auto value = fn(nullptr) )
			{
				Assert::Fail();
			}
			else
			{
				Assert::IsTrue(decltype(value)::value_t{} == value.toValueType() );
				Assert::AreEqual(decltype(value)::value_t{}, value.toValueType() );

			}
			if( auto value = fn("Hallo") )
			{
				Assert::IsTrue( value == "Hallo");//vorsicht boolvergleich
				Assert::IsTrue( value == "Welt");//vorsicht boolvergleich
				//Assert::IsTrue( value == std::string{"Hallo"});//error C2678: binary '==': no operator found which takes a left-hand operand of type 'std::string' (or there is no acceptable conversion)
				//Assert::IsTrue( value == std::string{"Welt"});//error C2678: binary '==': no operator found which takes a left-hand operand of type 'std::string' (or there is no acceptable conversion)
				Assert::IsTrue( value.toValueType() == std::string{"Hallo"});
				Assert::IsTrue( value.toValueType() != std::string{"Welt"});
				Assert::IsTrue( value.toValueType() == "Hallo");
				Assert::IsFalse( value.toValueType() == "Welt");

				auto s = value.toValueType();
				auto const & constvalue = value;
				auto cs = constvalue.toValueType();
				Assert::IsTrue( s == cs );

				Assert::IsTrue( value.toValueType() == s);

				std::move(value).toValueType();//passiert noch nichts
				Assert::IsTrue( value );
				Assert::IsTrue( value.toValueType() == "Hallo");
				s = std::move(value).toValueType();				//value.value ist danach leer aber immer noch valid, weil 
				Assert::IsTrue( value );
				Assert::IsTrue( value.toValueType().empty() );

				Assert::IsTrue( value.toValueType() != s);
				Assert::IsTrue( s == "Hallo" );
			}
			else
			{
				Assert::Fail();
			}
		}
		TEST_METHOD(UT_test_ret_type_bool)
		{
			struct A : WS::return_type<bool>//mit bool funktioniert das nicht wirklich gut
			{
				using base_t = WS::return_type<bool>;
				using base_t::base_t;
			};

			//Assert::IsFalse(A{});//error C2664: 'void Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsFalse(bool,const wchar_t *,const Microsoft::VisualStudio::CppUnitTestFramework::__LineInfo *)': cannot convert argument 1 from 'UT_compare_bool::UT_ret_type::UT_test_ret_type3::A' to 'bool'
			Assert::IsFalse(A{}==true);
			Assert::IsFalse(true==A{});
			Assert::IsTrue(!A{});
			Assert::IsTrue(A{}!=true);
			Assert::IsTrue(true!=A{});

			//Assert::IsTrue(A{0});//error C2664: 'void Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsTrue(bool,const wchar_t *,const Microsoft::VisualStudio::CppUnitTestFramework::__LineInfo *)': cannot convert argument 1 from 'UT_compare_bool::UT_ret_type::UT_test_ret_type3::A' to 'bool'
			Assert::IsTrue(A{0}==true);
			Assert::IsTrue(true==A{0});
			Assert::IsFalse(!A{0});
			Assert::IsFalse(A{0}!=true);
			Assert::IsFalse(true!=A{0});
			auto a1 = A{false};
			if( a1.Valid() )
			{
				Assert::IsTrue( a1.toValueType()==false );
			}
			//if( auto a = A{false} )//error C2451: conditional expression of type 'UT_compare_bool::UT_ret_type::UT_test_ret_type3::A' is illegal
			//{
			//	Assert::IsTrue( a1.toValueType()==false );
			//}
		}
		TEST_METHOD(UT_test_ret_type_return_struct)
		{
			struct A
			{
				struct get_ret_type{std::string str{};int i{};};
				WS::return_type<get_ret_type> get(bool error)
				{
					if(error)
						return {};
					return {get_ret_type{{"hallo"},{5}}};
				}
			};
			A a;
			if( auto v=a.get(false))
			{
				Assert::IsTrue(v.toValueType().str=="hallo");
				Assert::IsTrue(v.toValueType().i==5);
			}
			else
				Assert::Fail();
			if( auto v=a.get(true))
				Assert::Fail();
		}
		TEST_METHOD(UT_test_ret_type_return_tuple)
		{
			struct A
			{
				WS::return_type<std::tuple<std::string,int,bool>> get(bool error)
				{
					if(error)
						return {};
					return {std::tuple<std::string,int,bool>{{"hallo"},{5},{false}}};
				}
			};
			A a;
			if( auto v=a.get(false))
			{
				auto const &[str,i,f] = v.toValueType();//Structured binding (since C++17)
				Assert::IsTrue(str=="hallo");
				Assert::IsTrue(i==5);
				Assert::IsTrue(f==false);
			}
			else
				Assert::Fail();
			if( auto v=a.get(true))
				Assert::Fail();
		}
		TEST_METHOD(UT_test_ret_type_return)
		{
			enum my_errors{invalid=-1,none,err1,err2,err3};
			struct A
			{
				std::string str="hallo";
				WS::return_type<std::string const &,my_errors> getok()
				{
					return {str};
				};
				WS::return_type<std::string const &,my_errors> get1()
				{
					return {my_errors::err1};//ich bin mir nicht sicher, warum das funktioniert, wegen der std::string const & ohne objekt auf das referenziert werden kann !!?
				}
				WS::return_type<std::string const &,my_errors> get2(){return {my_errors::err2};}
				WS::return_type<std::string const &,my_errors> get3(){return {my_errors::err3};}
			};
			A a;
			if( auto v=a.getok())
			{
				Assert::IsTrue(v.toValueType()=="hallo");
				Assert::IsTrue(&v.toValueType()==&a.str);//als ref auf a.str
			}
			else
				Assert::Fail();
			if( auto v=a.get1())
				Assert::Fail();
			else
			{
				auto x = v.toValueType();x;
				Assert::IsTrue( v.error_code == my_errors::err1 );
			}
			if( auto v=a.get2())
				Assert::Fail();
			else
				Assert::IsTrue( v.error_code == my_errors::err2 );
			if( auto v=a.get3())
				Assert::Fail();
			else
				Assert::IsTrue( v.error_code == my_errors::err3 );
		}
		TEST_METHOD(UT_test_ret_type_return2)
		{
			enum my_errors{invalid=-1,none,err1,err2,err3};
			struct A
			{
				std::string str="hallo";
				WS::return_type<std::string,my_errors> getok1()
				{
					return {str};//als kopie 
				};
				WS::return_type<std::string,my_errors> getok2()
				{
					{
						std::string ret{str};
						return {ret};//als kopie 
					}
				};
				WS::return_type<std::string,my_errors> getok3()
				{
					{
						std::string ret{str};
						return {std::move( ret )};//als rvalue-kopie 
					}
				};
				WS::return_type<std::string ,my_errors> get1()
				{
					return {my_errors::err1};//ich bin mir nicht sicher, warum das funktioniert, wegen der std::string const & ohne objekt auf das referenziert werden kann !!?
				}
			};
			A a;
			if( auto v=a.getok1())
			{
				Assert::IsTrue( v.toValueType()=="hallo" );
				Assert::IsTrue(&v.toValueType()!=&a.str);//als kopie
			}
			else
				Assert::Fail();
			if( auto v=a.getok2())
			{
				Assert::IsTrue( v.toValueType()=="hallo" );
				Assert::IsTrue(&v.toValueType()!=&a.str);//als kopie
			}
			else
				Assert::Fail();
			if( auto v=a.getok3())
			{
				Assert::IsTrue( v.toValueType()=="hallo" );
				Assert::IsTrue(&v.toValueType()!=&a.str);//als kopie
			}
			else
				Assert::Fail();

			if( auto v=a.get1())
				Assert::Fail();
			else
			{
				auto x = v.toValueType();
				x=x;
			}
		}

		TEST_METHOD(UT_test_using_ret_value_implicit_cast)
		{
			struct A
			{
				std::string str="hallo";
				WS::return_type<std::string> get() &&
				{
					return {str};
				}
				WS::return_type<std::string const &> get() const &
				{
					return {str};
				}
			};
			auto equ = [](std::string const & l, std::string const & r){return l==r;};

			if( auto x = A{}.get() )
				Assert::IsTrue( equ(x,"hallo") );
			{
				auto a = A{};
				if( auto x = a.get() )
					Assert::IsTrue( equ( x, "hallo" ) );
			}
			{
				auto const a = A{};//const sinnlos, aber...
				if( auto const x = a.get() )//const sinnlos, aber...
					Assert::IsTrue( equ( x, "hallo" ) );
			}
		}
		TEST_METHOD(UT_referenz_ohne_objekt)//hä
		{
			//std::string & str {}; //error C2440: 'initializing': cannot convert from 'initializer list' to 'std::string &'

			std::string const & cstr {};//das ist bestimmt wieder so ein microsoft-mist, ohne warning
			Assert::IsTrue( &cstr!=nullptr );
			//cstr = "hallo";//error C2678: binary '=': no operator found which takes a left-hand operand of type 'const std::string' (or there is no acceptable conversion)
		}
	};
}
