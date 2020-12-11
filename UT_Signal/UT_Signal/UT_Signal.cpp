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
			//WS::Signal<bool(int,int)>::Connection_Guard connection//besser nicht vor signal anlegen. signal würde vor dem guard zerstört werden!
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
			//WS::Signal<bool(int,int)>::Connection_Guard connection//besser nicht vor signal anlegen. signal würde vor dem guard zerstört werden!
			WS::Signal<bool(int,int)> signal;
			decltype(signal)::Connection_Guard connection;

			Assert::IsTrue( signal.prio_callbacks[0xff].size()==0 );
			connection = signal.connect(foo1);
			Assert::IsTrue( signal.prio_callbacks[0xff].size()==1 );
			connection = signal.connect(foo1);
			Assert::IsTrue( signal.prio_callbacks[0xff].size()==1 );
			connection = signal.connect(foo3);
			Assert::IsTrue( signal.prio_callbacks[0xff].size()==1 );
		}
		TEST_METHOD(UT_id_type_lfdid_uberlauf)
		{
			WS::Signal<void(void)>::id_type{0};
			auto id = WS::Signal<void(void)>::id_type{0}.id_offset(1);
			Assert::IsTrue(id==1);
			id = WS::Signal<void(void)>::id_type{0}.id_offset(-1);
			Assert::IsTrue(id==0xfff'fff);
			id = WS::Signal<void(void)>::id_type{0xfff'fff}.id_offset(+1);
			Assert::IsFalse(id);
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
					id5 = sig.connect(a).release();//wenn a zerstört wird bleibt der callback in sig erhalten. was passiert? sig legt kopie von a an, nicht das was beabsichtigt war
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
		TEST_METHOD(UT_Signal_standard_voidParameter)
		{
			size_t counter{0};
			WS::Signal<void()> sig1;
			WS::Signal<void(void)> sig2;
			sig1();
			sig2();
			Assert::IsTrue(counter==0);

			{
				auto connection1 = sig1.connect([&counter](void){++counter;});
				auto connection2 = sig2.connect([&counter](void){++counter;});
				sig1();
				Assert::IsTrue(counter==1);
				sig2();
				Assert::IsTrue(counter==2);
			}
			sig1();
			sig2();
			Assert::IsTrue(counter==2);
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
			auto constexpr def_prio = decltype(signal)::id_type::default_prio;

			Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
			Assert::IsTrue( signal.prio_callbacks[def_prio].size()==0 );
			signal("hallo1");
			{
				SignalUser ConnectToSignal(signal);
				Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
				Assert::IsTrue( signal.prio_callbacks[def_prio].size()==4 );
				signal("hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromLambda=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromBind=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromOperator=="hallo2");
				ConnectToSignal.connection_lamda.disconnect();
				Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
				Assert::IsTrue( signal.prio_callbacks[def_prio].size()==3 );
				signal("hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromLambda=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromFunctor=="hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromBind=="hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromOperator=="hallo2_1");
			}
			Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
			Assert::IsTrue( signal.prio_callbacks[def_prio].size()==0 );
			signal("hallo3");
		}
		TEST_METHOD(UT_Signal_in_class_using_mit_prio)
		{
			auto signal = WS::Signal<void(std::string,bool&)>{};
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

						void operator()(std::string const & value,bool& verarbeitet){return item.SignalCallbackFunctor(value,verarbeitet);}
					};
					//testweise 3 arten eine methoden bei signal zu registieren um das signal verarbeiten
					connection_lamda	= signal.connect([this](std::string value,bool& verarbeitet){this->SignalCallbackLambda(value,verarbeitet);});
					connection_std_bind	= signal.connect(std::bind(&SignalUser::SignalCallbackBind, this, std::placeholders::_1, std::placeholders::_2));
					connection_functor	= signal.connect(functor{*this},signal_t::prio_t{3});//wird vor default_prio ausgeführt
					//connection_operator = signal.connect(*this);//copy-ctor=delete  error C2280: 'UTSignal::UTSignal::UT_Signal_in_class_using::SignalUser::SignalUser(const UTSignal::UTSignal::UT_Signal_in_class_using::SignalUser &)': attempting to reference a deleted function
					connection_operator = signal.connect(std::reference_wrapper(*this),signal_t::prio_t{3});//reference_wrapper sonst wuerde kopie von this angelegt werden
				}

				void operator()(std::string const & value,bool & verarbeitet)
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:'") + value + "' verarbeitet=" + (verarbeitet?"true":"false")).c_str() );
					lastFromOperator = value;
				}
				void SignalCallbackLambda(std::string value,bool & verarbeitet)
				{
					Assert::IsFalse(verarbeitet);
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:'") + value + "' verarbeitet=" + (verarbeitet?"true":"false") + " wird nun verarbeitet").c_str() );
					verarbeitet=true;
					lastFromLambda = value;
				}
				void SignalCallbackBind(std::string value,bool & verarbeitet)
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:'") + value + "' verarbeitet=" + (verarbeitet?"true":"false")).c_str() );
					lastFromBind = value;
				}
				void SignalCallbackFunctor(std::string value,bool & verarbeitet)
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:'") + value + "' verarbeitet=" + (verarbeitet?"true":"false")).c_str() );
					lastFromFunctor = value;
				}
			};

			char buf[20];
			auto constexpr def_prio = decltype(signal)::id_type::default_prio;

			Logger::WriteMessage( (std::string("callback_count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
			Assert::IsTrue( signal.prio_callbacks[def_prio].size()==0 );
			bool verarbeitet = false;//die lambda verarbeitet die info
			signal("hallo1",verarbeitet );
			Assert::IsFalse(verarbeitet);
			{
				SignalUser ConnectToSignal(signal);
				Logger::WriteMessage( (std::string("callback[def_prio] count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
				Logger::WriteMessage( (std::string("callback[prio 3] count:") + tostring(signal.prio_callbacks[3].size(),buf,10)).c_str() );
				Assert::IsTrue( signal.prio_callbacks[def_prio].size()==2 );
				Assert::IsTrue( signal.prio_callbacks[3].size()==2 );
				signal("hallo2",verarbeitet=false);
				Assert::IsTrue(verarbeitet);
				Assert::IsTrue( ConnectToSignal.lastFromLambda=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromBind=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromOperator=="hallo2");
				ConnectToSignal.connection_lamda.disconnect();
				Logger::WriteMessage( (std::string("callback[def_prio] count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
				Logger::WriteMessage( (std::string("callback[prio 3] count:") + tostring(signal.prio_callbacks[3].size(),buf,10)).c_str() );
				Assert::IsTrue( signal.prio_callbacks[def_prio].size()==1 );
				Assert::IsTrue( signal.prio_callbacks[3].size()==2 );
				signal("hallo2_1",verarbeitet=false);
				Assert::IsFalse(verarbeitet);
				Assert::IsTrue( ConnectToSignal.lastFromLambda=="hallo2");
				Assert::IsTrue( ConnectToSignal.lastFromFunctor=="hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromBind=="hallo2_1");
				Assert::IsTrue( ConnectToSignal.lastFromOperator=="hallo2_1");
			}
			Logger::WriteMessage( (std::string("callback[def_prio] count:") + tostring(signal.prio_callbacks[def_prio].size(),buf,10)).c_str() );
			Logger::WriteMessage( (std::string("callback[prio 3] count:") + tostring(signal.prio_callbacks[3].size(),buf,10)).c_str() );
			Assert::IsTrue( signal.prio_callbacks[def_prio].size()==0 );
			Assert::IsTrue( signal.prio_callbacks[3].size()==0 );
			signal("hallo3",verarbeitet=false);
			Assert::IsFalse(verarbeitet);
		}
		TEST_METHOD(UT_Signal_in_class_using_block)
		{
			auto signal = WS::Signal<void(size_t&)>{};
			using signal_t = decltype(signal);
			using connection_t = signal_t::Connection_Guard;
			using block_t = signal_t::Block_Guard;
			block_t block;

			auto fn = [](size_t & counter){++counter;};

			auto connection1 = signal.connect(fn);
			auto connection2 = signal.connect(fn);
			auto connection3 = signal.connect(fn);

			size_t v=0;
			signal(v);
			Assert::IsTrue(v==3);

			{
				auto block2 =signal.block(connection2);

				v=0;
				signal(v);
				Assert::IsTrue(v==2);

				block =signal.block(connection1);

				v=0;
				signal(v);
				Assert::IsTrue(v==1);

				block =signal.block(connection3);//reuse blocker

				v=0;
				signal(v);
				Assert::IsTrue(v==1);
			}
			v=0;
			signal(v);
			Assert::IsTrue(v==2);
		}
		TEST_METHOD(UT_Signal_in_class_using_break_signaling)
		{
			auto signal = WS::Signal<void(size_t&)>{};
			using signal_t = decltype(signal);
			using connection_t = signal_t::Connection_Guard;

			size_t break_at = 0;
			
			connection_t connection1;
			auto fn = [&](size_t & counter)
			{
				if(++counter==break_at)
				{
					signal.break_signaling();
					//connection1.break_signaling();//welche der drei connections egal, der interne signalpointer ist entscheident
				}
			};

//			for( auto i=0;auto v :{1,2,3} )//c++20
//			{
//				++i;
//			}

			connection1 = signal.connect(fn);
			auto connection2 = signal.connect(fn);
			auto connection3 = signal.connect(fn);

			size_t v=0;
			signal(v);
			Assert::IsTrue(v==3);

			break_at=2;
			v=0;
			signal(v);
			Assert::IsTrue(v==2);

			break_at=0;
			v=0;
			signal(v);
			Assert::IsTrue(v==3);
		}
		TEST_METHOD(UT_Signal_destroy_with_connection)
		{
			auto signal_ptr = std::make_unique<WS::Signal<void(size_t&)>>();
			using signal_t = std::decay_t<decltype(*signal_ptr)>;
			using connection_t = signal_t::Connection_Guard;

			{
				connection_t connection1;
				auto fn = [&](size_t & counter)
				{
					++counter;
				};

				connection1 = signal_ptr->connect(fn);

				size_t v=0;
				(*signal_ptr)(v);
				Assert::IsTrue(v==1);
				Assert::IsTrue( connection1.signal!=nullptr );
				signal_ptr = nullptr;//zerstört signal obwohl connection1 noch einen signal-pointer hat. Problem?
				Assert::IsTrue( connection1.signal==nullptr );// no problem
			}
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
		TEST_METHOD(UT_Signal_id_t)
		{
			using id_type = typename WS::Signal<void(void)>::id_type;
			static_assert(sizeof(id_type)==sizeof(size_t));
			static_assert(sizeof(id_type::id_t)==sizeof(size_t));
			static_assert(sizeof(id_type::parts)==sizeof(size_t));
			static_assert(sizeof(id_type::prio_t)==sizeof(__int8));						//1Byte
			//static_assert(sizeof(decltype(id_type::parts_t::prio))==sizeof(size_t));	//aber in der  struct 4Byte

			auto id1=id_type{1};
			auto id2=id_type{1,1};
			size_t x1 = id1;x1;
			size_t x2 = id2;x2;
			Assert::IsFalse(id1==id2);
			Assert::IsTrue(id1==id1);
			Assert::IsTrue(id2==id2);
			Assert::IsTrue(id1.id()==id2.id());
			Assert::IsTrue(id1.prio()==0xff);
			Assert::IsTrue(id2.prio()==0x01);

			auto id3=id_type{0xcf'ffffff};//c ist prio
			Assert::IsTrue((id_type::id_t)id3==0xcf'fff'fff);
			Assert::IsTrue(id3.id()==0x00'fff'fff);
			Assert::IsTrue(id3.prio()==0xcf);
			try
			{
				id3=id_type{0xcf'fff'fff, 1};//cf ist prio
				Assert::Fail(L"exception erwartet");
			}
			catch(std::exception & e)
			{
				Logger::WriteMessage( (std::string(__FUNCTION__ " Line(" _LINE_ ")") +  e.what()).c_str() );
			}
			try
			{
				id3=id_type{0xcf'fff'fff, 0};//cf ist prio
				Assert::Fail(L"exception erwartet");
			}
			catch(std::exception & e)
			{
				Logger::WriteMessage( (std::string(__FUNCTION__ " Line(" _LINE_ ")") +  e.what()).c_str() );
			}
			id3=id_type{0xcf'fff'fff, 0xff};//cf ist prio, ff =default und wird ignoriert
			Assert::IsTrue((id_type::id_t)id3==0xcf'fff'fff);
			Assert::IsTrue(id3.id()==0x00'fff'fff);
			Assert::IsTrue(id3.prio()==0xcf);
		}
		TEST_METHOD(UT_demo)
		{
			using signal_t = WS::Signal<bool(std::string const &, bool&)>;
			auto  signal = signal_t{};//anlegen des signal-objekt
		
			struct SignalUser //callback-objekt mit autolink zum signal
			{
				using signal_t = signal_t;
				signal_t::Connection_Guard connection;
				SignalUser(SignalUser const &) = delete;//um sicher zu gehen, dass keine kopie angelegt werden kann
				SignalUser( signal_t  & signal ) {this->connection = signal.connect(std::reference_wrapper(*this));}//reference_wrapper sonst wuerde kopie von this angelegt werden

				bool operator()(std::string const & value,bool & verarbeitet)//diese funktion wird von signal gerufen
				{
					Logger::WriteMessage( (std::string(__FUNCTION__ " value:'") + value + "' verarbeitet=" + (verarbeitet?"true":"false")).c_str() );
					#pragma warning(suppress:6282)
					return verarbeitet=true;//als verarbeite kennzeichnen
				}
			} signaluser{signal};//verknüpft signal im ctor per connect mit signaluser 

			bool verarbeitet = false;
			auto combiner_wenn_von_interesse = signal("hallo", verarbeitet);//ruft alle verbundenen funktionen
			Assert::IsTrue(verarbeitet);
			Assert::IsTrue(combiner_wenn_von_interesse());// liefert ergebnis des default-combiner combiner_last<bool>
		}
	};
}
