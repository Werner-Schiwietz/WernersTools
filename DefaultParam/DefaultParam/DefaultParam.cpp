#include "pch.h"
#include "CppUnitTest.h"

#include "..\..\headeronly\DefaultParameter.h"

#include <vector>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;



auto foo_defaultParam()
{
	enum ParamNamen {x,y,width,height};
	return WS::defaultParam<ParamNamen>(0, 0, 0, 0);
	//return _defaultParam<ParamNamen>(0, 0, 0, 0);//ohne vollstaendige template-paramerter geht es auch in C++17 nicht
	//return defaultParam(0, 0, 0, 0);//ohne enum-type geht es so in C++17
}
void foo(decltype(foo_defaultParam()) param = foo_defaultParam())
{
	using index_t = decltype(param)::index_t;
	std::stringstream out;
	out << param.get<index_t::x>() << ' ' << param.get<index_t::y>() << ' ' << param.get<index_t::width>() << ' ' << param.get<index_t::height>() << ' ' << std::endl;
}

template<typename enumtype> struct enumtest
{
	using enum_t = enumtype;
	enumtest() {}
	enumtest(enumtype e){}
};

namespace DefaultParam
{
	TEST_CLASS(DefaultParam)
	{
	public:
		TEST_METHOD(TestMethod2)
		{
			foo();
			using index_t=decltype(foo_defaultParam())::index_t;
			foo(foo_defaultParam().set<index_t::x>(1).set<index_t::y>(2));

			auto I=index_t::x;
			//I=0;//error C2440:  '=': cannot convert from 'int' to 'foo_defaultParam::ParamNamen'
			I=index_t(3);
		}
		TEST_METHOD(ctor1)
		{
			auto x = WS::defaultParam(short{ 5 }, std::unique_ptr<char[]>{nullptr});
			using x_t = decltype(x);
			Assert::IsTrue(std::is_same<x_t::types::get_t<0>, short>::value);//optional
			Assert::IsTrue(std::is_same<std::decay_t<decltype(x.get<0>())>,short>::value);
			Assert::IsTrue( x.get<0>()==short{5} );
			Assert::IsTrue(std::is_same<x_t::types::get_t<1>, std::unique_ptr<char[]>>::value);//optional
			Assert::IsTrue(std::is_same<std::decay_t<decltype(x.get<1>())>, std::unique_ptr<char[]>>::value);
			Assert::IsTrue(x.get<1>() == nullptr);
		}
		TEST_METHOD(ctor1_rvalue_set)
		{
			enum  varnames { var1, var2, var3, var4 };
			auto values = WS::defaultParam<varnames>(true, false, false, false).set<varnames::var3>(true).set<varnames::var1>(false);

			using index_t = decltype(values)::index_t;
			Assert::IsTrue( std::is_same<index_t,varnames>::value );
			//index_t index = 0;//error C2440:  'initializing': cannot convert from 'int' to 'index_t'
			//Assert::IsTrue(values.get<2>());//error C2672 : 'WS::_defaultParam<index_type,bool,bool,bool,bool>::get' : no matching overloaded function found
			//Assert::IsFalse(values.get<3>());//error C2672:  'WS::_defaultParam<index_type,bool,bool,bool,bool>::get': no matching overloaded function found

			Assert::IsFalse(values.get<varnames::var1>());
			Assert::IsFalse(values.get<varnames::var2>());
			Assert::IsTrue(values.get<varnames::var3>());
			Assert::IsFalse(values.get<varnames::var4>());

			//index_t index = 0;//error C2440:  'initializing': cannot convert from 'int' to 'index_t'
			Assert::IsTrue(values.get<2>());//compiler-fehler? dürfte hier genauso wenig gehen wie 9 zeilen drüber
			Assert::IsFalse(values.get<3>());
		}
		TEST_METHOD(ctor1_lvalue_set)
		{
			enum varnames { var1, var2, var3, var4 };
			auto values = WS::defaultParam<varnames>(true, false, false, false);
			values.set<varnames::var3>(true).set<varnames::var1>(false);
			Assert::IsFalse(values.get<varnames::var1>());
			Assert::IsFalse(values.get<varnames::var2>());
			Assert::IsTrue(values.get<varnames::var3>());
			Assert::IsFalse(values.get<varnames::var4>());
		}
		TEST_METHOD(mit_vector)
		{
			auto strvalues= std::initializer_list{ L"hallo",L" ",L"welt" };
			auto values = WS::defaultParam(3,std::vector<std::wstring>{strvalues.begin(), strvalues.end()});
			auto i1 = strvalues.begin();
			for( auto & value : values.get<1>() )
				Assert::IsTrue( value == *i1++ );
		}
		TEST_METHOD(TestMethod1)
		{
			auto x = WS::defaultParam(int{5}, (char const *)nullptr);
			using index_t = decltype(x)::index_t;
			auto & v1 = std::get<1>(x.values);v1;
			//std::unique_ptr<char> p = nullptr;
			std::tuple<std::unique_ptr<char[]>> t1(std::unique_ptr<char[]>{});
			auto t2 = std::move(t1);
			{
				auto x2 = WS::defaultParam(std::unique_ptr<char[]>{});
				std::get<0>(x2.values) = std::unique_ptr<char[]>(new char[10]{"hallo"});

				auto x3 = WS::defaultParam(std::unique_ptr<char[]>{}).set<0>(std::unique_ptr<char[]>(new char[10]{ "welt" }));
				//auto x4 = x3;//error C2280:  'WS::_defaultParam<index_type,std::unique_ptr<char [],std::default_delete<_Ty>>>::_defaultParam(const WS::_defaultParam<index_type,std::unique_ptr<_Ty,std::default_delete<_Ty>>> &)': attempting to reference a deleted function
				Assert::IsTrue( x3.get<0>()!=nullptr );
				auto x5 = std::move(x3);
				Assert::IsTrue(x3.get<0>() == nullptr);
				Assert::IsTrue(x5.get<0>() != nullptr);
				Assert::IsTrue(strcmp(x5.get<0>().get(),"welt")==0 );
				x5.set<0>(std::unique_ptr<char[]>(new char[10]{ "hallo" })).set<0>(std::unique_ptr<char[]>(new char[10]{ "HALLO" }));
				Assert::IsTrue(strcmp(x5.get<0>().get(), "HALLO") == 0);
			}

			using index_t = decltype(x)::index_t;
			index_t e = 3;
			e = index_t(3);
			enum E : size_t;
			E ee{1};
			//ee = size_t{2};//error C2440:  '=': cannot convert from 'size_t' to 'DefaultParam::DefaultParam::TestMethod1::E'
			ee=E{2};

			x.set<0>(3).set< 1>("Hallo");
			auto x3 = WS::defaultParam(int{ 5}, (char const*)nullptr).set<1>("Welt").set<0>(21);
			Assert::IsTrue(x3.get<0>()==21);
			Assert::IsTrue(strcmp(x3.get<1>(),"Welt") == 0);
			auto const & x4 = x3;
			Assert::IsTrue(x4.get<0>() == 21);
			Assert::IsTrue(strcmp(x4.get<1>(), "Welt") == 0);
		}
		TEST_METHOD(UT_enumtest)
		{
			enum xxx { a, b, c } XXX = xxx::a;
			auto x = enumtest(xxx::a);
			decltype(x) y;
			auto xx = decltype(x)::enum_t::a;
			//auto x1 = enumtest(enum class xx{x,y,z}{} );
			//auto x2 = enumtest<enum aa{a,b}>{};
			//auto x3 = enumtest < xxx > {};
		}
	};
}

