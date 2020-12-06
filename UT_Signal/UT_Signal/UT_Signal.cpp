#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\signal.h"
#include "..\..\headeronly\combiner.h"
#include "..\..\headeronly\char_helper.h"


#include <optional>

namespace
{
	bool foo1(int a, int b)
	{
		return a==b;
	}
	struct foo2
	{
		bool operator()(int a,int b)
		{
			return a==b;
		}
	};
	auto foo3 = [](int a, int b)->bool{return a==b;};
	auto foo4 = [v=5](int a, int b)->bool{return a==b;};
	std::function<bool(int,int)> foo;
}

namespace UTSignal
{
	TEST_CLASS(UTSignal)
	{
	public:
		TEST_METHOD(UT_combiner_last)
		{
			using T=int;
			{
				WS::combiner_last<T,T{6}> combiner{};
				Assert::IsTrue( combiner.init_val == T{6} );
				Assert::IsTrue( combiner() == T{6} );
			}
			WS::combiner_last<T> combiner{};
			Assert::IsTrue( WS::combiner_last<T>::init_val == T{} );

			static_assert(std::is_same_v<void,T> == false );
			std::optional<T> value;
			for( auto i : {1,2,4,5} )
			{
				value = combiner(i);
				Assert::IsTrue( value.value()==i );
			}
			Assert::IsTrue( value.value()==5);
		}
		TEST_METHOD(UT_combiner_last_void)
		{
			using T=void;
			WS::combiner_last<T> combiner{};
		}
		TEST_METHOD(UT_combiner_and)
		{
			Assert::IsTrue(  WS::combiner_and<bool>{}(true)(true)(true)(true).value.value() );
			Assert::IsFalse( WS::combiner_and<bool>{}(true)(true)(true)(false).value.value() );
			Assert::IsFalse( WS::combiner_and<bool>{}(false)(true)(true)(true).value.value() );
			Assert::IsFalse( WS::combiner_and<bool>{}.value.has_value() );
		}
		TEST_METHOD(UT_combiner_or)
		{
			Assert::IsTrue(  WS::combiner_or<bool>{}(true)(true)(true)(true).value.value() );
			Assert::IsTrue(  WS::combiner_or<bool>{}(true)(true)(true)(false).value.value() );
			Assert::IsTrue(  WS::combiner_or<bool>{}(false)(true)(true)(true).value.value() );
			Assert::IsFalse( WS::combiner_or<bool>{}(false)(false)(false)(false).value.value() );
			Assert::IsFalse( WS::combiner_or<bool>{}.value.has_value() );
		}

		TEST_METHOD(UT_Signal_combiner_and)
		{
			WS::Signal<bool(int,int),WS::combiner_and<bool>> sig;
			Assert::IsFalse( sig(5,5)().has_value() );
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				Assert::IsTrue(  sig(5,5)().has_value() );
				Assert::IsTrue(  sig(5,5)().value() );
				Assert::IsFalse( sig(5,6)().value() );
			}
			Assert::IsFalse( sig(5,5)().has_value() );
		}
		TEST_METHOD(UT_Signal_combiner_or)
		{
			WS::Signal<bool(int,int),WS::combiner_or<bool>> sig;
			Assert::IsFalse( sig(5,5)().has_value() );
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				Assert::IsTrue(  sig(5,5)().has_value() );
				Assert::IsTrue(  sig(5,5)().value() );
				Assert::IsFalse( sig(5,6)().value() );
			}
			Assert::IsFalse( sig(5,5)().has_value() );
		}
		TEST_METHOD(UT_Signal_standard_combinerlast)
		{
			WS::Signal<bool(int,int)> sig;
			sig(5,5);
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				Assert::IsTrue(  sig(5,5) );
				Assert::IsFalse( sig(5,6) );
			}
			sig(5,5);
		}
		TEST_METHOD(UT_disconnect_wrong_id)
		{
			//WS::Signal<bool(int,int)>::Connection_Guard connection//besser nicht vor signal anlegen. signal w�rde vor dem guard zerst�rt werden!
			WS::Signal<bool(int,int)> signal;
			decltype(signal)::Connection_Guard connection;

			try
			{
				signal.disconnect(10);
				Assert::Fail(L"exception erwartet");
			}
			catch(std::exception & e)
			{
				Logger::WriteMessage((std::string(__FUNCTION__ " exception:") + e.what()).c_str());
			}
		}
		TEST_METHOD(UT_Connection_multi_assign)
		{
			//WS::Signal<bool(int,int)>::Connection_Guard connection//besser nicht vor signal anlegen. signal w�rde vor dem guard zerst�rt werden!
			WS::Signal<bool(int,int)> signal;
			decltype(signal)::Connection_Guard connection;

			Assert::IsTrue( signal.callbacks.size()==0 );
			connection = signal.connect(foo1);
			Assert::IsTrue( signal.callbacks.size()==1 );
			connection = signal.connect(foo1);
			Assert::IsTrue( signal.callbacks.size()==1 );
			connection = signal.connect(foo3);
			Assert::IsTrue( signal.callbacks.size()==1 );
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(UT_Signal_connect_until_exception)
			TEST_IGNORE()
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(UT_Signal_connect_until_exception)//der speicher geht aus, bevor die map des signal voll ist
		{
			WS::Signal<bool(int,int)> signal;
			Logger::WriteMessage(__FUNCTION__ " das kann dauern" );
			try
			{
				for(;;)
					(void)signal.connect(foo1).release();//never do this
			}
			catch( std::exception & e )
			{
				Logger::WriteMessage(__FUNCTION__);
				Logger::WriteMessage(e.what());
				
			}
		}
		BEGIN_TEST_METHOD_ATTRIBUTE(UT_Signal_next_id_over_4Giga_times)
			TEST_IGNORE()
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(UT_Signal_next_id_over_4Giga_times)
		{
			WS::Signal<bool(int,int)> signal;
			decltype(signal)::Connection_Guard connection;

			Logger::WriteMessage(__FUNCTION__ " das kann dauern" );
			try
			{
				for(__int64 i=0;i<5'000'000'000i64;++i)
					connection = signal.connect(foo1);
			}
			catch( std::exception & e )
			{
				Logger::WriteMessage(__FUNCTION__);
				Logger::WriteMessage(e.what());

			}
		}
		TEST_METHOD(UT_Signal_own_disconnect_und_method_calling)
		{
			WS::Signal<bool(int,int)> sig;
			decltype(sig)::id_t id5{};
			decltype(sig)::id_t id6{};
			decltype(sig)::id_t id7{};

			struct Data
			{
				size_t callingcounter = 0;
				size_t dtorcounter = 0;
				size_t ctorcounter = 0;
			}bdata;

			sig(5,5);
			{
				struct A
				{
					size_t callingcounter = 0;
					~A()
					{
						test_invalid_local_var = {"A"};
					}
					std::string test_invalid_local_var{"hallo A"};
					bool operator()(int l, int r)
					{
						++callingcounter;
						return l==r;
					}
				} a;
				struct B
				{
					Data& data;
					~B(){++data.dtorcounter;}
					B()=delete;
					B(B const &)=delete;
					B(Data& data) : data(data)
					{
						++data.ctorcounter;
					}
					std::string test_invalid_local_var{"hallo B"};
					bool operator()(int l, int r)
					{
						++data.callingcounter;
						return l==r;
					}
				} b{bdata};

				{
					auto connection1 = sig.connect(foo1);
					auto connection2 = sig.connect(foo2{});
					auto connection3 = sig.connect(foo3);
					auto connection4 = sig.connect(foo4);
					id5 = sig.connect(a).release();//wenn a zerst�rt wird bleibt der callback in sig erhalten. was passiert? sig legt kopie von a an, nicht das was beabsichtigt war
												   //sig.connect(b);//error C2280: 'UTSignal::UTSignal::UT_Signal_own_disconnect::B::B(const UTSignal::UTSignal::UT_Signal_own_disconnect::B &)': attempting to reference a deleted function
												   //sig.connect<B&>(b);//error C2280: 'UTSignal::UTSignal::UT_Signal_own_disconnect::B::B(const UTSignal::UTSignal::UT_Signal_own_disconnect::B &)': attempting to reference a deleted function
					id6 = sig.connect(std::reference_wrapper<B>{b}).release();
					id7 = sig.connect([&b](int l, int r){return b(l,r);}).release();//wenn b kann aber nicht kopiert werden, aber per lambda
					Assert::IsTrue(  sig(5,5) );
				}
				Assert::IsTrue(a.callingcounter==0);//es wurde eine kopie gerufen
				Assert::IsTrue(b.data.callingcounter==2);
				Assert::IsFalse( sig(5,6) );
				Assert::IsTrue(a.callingcounter==0);
				Assert::IsTrue(b.data.callingcounter==4);
				Assert::IsTrue(bdata.callingcounter==4);
			}
			Assert::IsTrue(bdata.callingcounter==4);
			Assert::IsTrue(bdata.ctorcounter==bdata.dtorcounter);//eigentlich sind alle objekte kaputt
			auto erg = sig(5,5);//was passiert mit deleteten objekten(es gibt kein B mehr)? nichts! warum?
			Assert::IsTrue(bdata.callingcounter==6);

		}
		TEST_METHOD(UT_Signal_standard_void)
		{
			WS::Signal<void(int,int)> sig;
			sig(5,5);
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				sig(5,5);
				sig(5,6);
			}
			sig(5,5);
		}
		TEST_METHOD(UT_Signal_standard_all)
		{
			WS::Signal<bool(int,int),WS::combiner_all<bool>> sig;
			Assert::IsTrue( sig(5,5)().size()==0 );
			{
				auto connection1 = sig.connect(foo1);
				auto connection2 = sig.connect(foo2{});
				auto connection3 = sig.connect(foo3);
				auto connection4 = sig.connect(foo4);
				auto erg = sig(5,5)();
				Assert::IsTrue( erg.size()==4 );
				Assert::IsTrue( erg[0].first );
				Assert::IsTrue( erg[0].first==erg[1].first );
				Assert::IsTrue( erg[0].first==erg[2].first );
				Assert::IsTrue( erg[0].first==erg[3].first );
				erg = sig(5,6)();
				Assert::IsTrue(  erg.size()==4 );
				Assert::IsFalse( erg[0].first );
				Assert::IsTrue(  erg[0].first==erg[1].first );
				Assert::IsTrue(  erg[0].first==erg[2].first );
				Assert::IsTrue(  erg[0].first==erg[3].first );
			}
			Assert::IsTrue( sig(5,5)().size()==0 );
		}
		TEST_METHOD(UT_Signal_in_class_using)
		{
			auto signal = WS::Signal<void(std::string)>{};
			using signal_t = decltype(signal);
			using connection_t = signal_t::Connection_Guard;

			//signal-konsument
			struct SignalUser
			{
				connection_t connection_lamda;
				connection_t connection_std_bind;
				connection_t connection_operator;
				connection_t connection_functor;
				std::string lastFromLambda;
				std::string lastFromBind;
				std::string lastFromFunctor;
				std::string lastFromOperator;
				SignalUser( signal_t & signal)
				{
					struct functor
					{
						SignalUser & item;
						functor(SignalUser & item):item(item){}

						void operator()(std::string const & value){return item.SignalCallbackFunctor(value);}
					};
					//testweise 3 arten eine methoden bei signal zu registieren um das signal verarbeiten
					connection_lamda	= signal.connect([this](std::string value){this->SignalCallbackLambda(value);});
					connection_std_bind	= signal.connect(std::bind(&SignalUser::SignalCallbackBind, this, std::placeholders::_1));
					connection_functor	= signal.connect(functor{*this});
					//connection_operator = signal.connect(*this);//copy-ctor=delete  error C2280: 'UTSignal::UTSignal::UT_Signal_in_class_using::SignalUser::SignalUser(const UTSignal::UTSignal::UT_Signal_in_class_using::SignalUser &)': attempting to reference a deleted function
					connection_operator = signal.connect(std::reference_wrapper(*this));//reference_wrapper sonst wuerde kopie von this angelegt werden
				}

				void operator()(std::string const & value)
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:") + value).c_str() );
					lastFromOperator = value;
				}
				void SignalCallbackLambda(std::string value)
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:") + value).c_str() );
					lastFromLambda = value;
				}
				void SignalCallbackBind(std::string value)
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:") + value).c_str() );
					lastFromBind = value;
				}
				void SignalCallbackFunctor(std::string value)
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:") + value).c_str() );
					lastFromFunctor = value;
				}
			};

			char buf[20];
			Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.callbacks.size(),buf,10)).c_str() );
			Assert::IsTrue( signal.callbacks.size()==0 );
			signal("hallo1");
			{
				SignalUser ConnectToSignal(signal);
				Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.callbacks.size(),buf,10)).c_str() );
				Assert::IsTrue( signal.callbacks.size()==4 );
				signal("hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromLambda=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromBind=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromOperator=="hallo2");
				ConnectToSignal.connection_lamda.disconnect();
				Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.callbacks.size(),buf,10)).c_str() );
				Assert::IsTrue( signal.callbacks.size()==3 );
				signal("hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromLambda=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromFunctor=="hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromBind=="hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromOperator=="hallo2_1");
			}
			Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.callbacks.size(),buf,10)).c_str() );
			Assert::IsTrue( signal.callbacks.size()==0 );
			signal("hallo3");
		}
		TEST_METHOD(UT_tostring)
		{

			auto i=4;
			{
				char buf[40];

				Logger::WriteMessage( "char buffer" );
				Logger::WriteMessage( tostring( short( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned short( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( int( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned int( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( __int8( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned __int8( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( __int64( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned __int64( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( long long( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned long long( ++i ), buf, 10 ) );
			}

			{
				wchar_t buf[40];

				Logger::WriteMessage( "wchar_t buffer" );
				Logger::WriteMessage( tostring( short( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned short( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( int( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned int( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( __int8( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned __int8( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( __int64( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned __int64( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( long long( ++i ), buf, 10 ) );
				Logger::WriteMessage( tostring( unsigned long long( ++i ), buf, 10 ) );
			}

		}	
	};
}
