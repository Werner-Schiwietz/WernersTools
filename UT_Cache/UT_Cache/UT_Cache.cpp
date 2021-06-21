#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\cache.h"

#include "..\..\headeronly\char_helper.h"

#include <string>
#include <iostream>
#include <future>

#pragma warning(push,4)
namespace
{
	//cout umleitung////////////////////////////////////
	class LoggerStreambuf : public std::streambuf
	{
	public:
		virtual int_type overflow( int_type c = EOF ) {
			static std::mutex mutex{};
			auto locked = std::lock_guard(mutex);
			static std::map<std::thread::id,std::string> buf;
			if( c != EOF )
			{
				if( c != '\n' )
					buf[std::this_thread::get_id()] += static_cast<char>(c);
				else
				{
					Logger::WriteMessage( buf[std::this_thread::get_id()].c_str() );
					buf.erase(std::this_thread::get_id());
				}
			}
			return c;
		}
	};
	template<typename streambuf_t=LoggerStreambuf>
	class Cout2Output
	{
		streambuf_t dbgstream;
		std::streambuf *default_stream;

	public:
		Cout2Output() {
			default_stream = std::cout.rdbuf( &dbgstream );
		}

		~Cout2Output() {
			std::cout.rdbuf( default_stream );
		}
	};
	//cout umleitung////////////////////////////////////

#pragma region Laufzeitüberwachung in der Testumgebung
	class time_watch
	{

		std::chrono::system_clock::time_point	last_end_time;
		std::thread								watchdog{};
		enum{uninitializied,runing,ready}		watchdoc_status{uninitializied};
		std::atomic_bool						zeitueberschreitung{false};
		std::function<void(void)>				zeitueberschreitungscallback{};

		static void watchdog_fn(time_watch * timewatch)
		{
			while(timewatch->watchdoc_status!=ready)
			{
				if( timewatch->watchdoc_status==runing )
				{
					if (timewatch->last_end_time < std::chrono::system_clock::now())
					{
						timewatch->zeitueberschreitung = true;
						if (timewatch->zeitueberschreitungscallback)
							timewatch->zeitueberschreitungscallback();
						timewatch->watchdoc_status = ready;
						return;
					}
				}
				std::this_thread::yield();
			}
		}
	public:
		template <class _Rep, class _Period> time_watch(std::chrono::duration<_Rep, _Period> const & dura, std::function<void(void)> callback )
			: zeitueberschreitungscallback(callback)
		{
			this->watchdog = std::thread(watchdog_fn,this);//überwachnungsthread für das MS-testsystem eigentlich ungeeeignet. es kommt mit laufenden threads nicht klar

			last_end_time = std::chrono::time_point_cast<decltype(last_end_time)::duration>(std::chrono::system_clock::now() + dura);
			watchdoc_status = runing;
		}
		template <class _Rep, class _Period> time_watch(std::chrono::duration<_Rep, _Period> const & dura) : time_watch(dura, std::function<void(void)>{}){}
		time_watch(){}
		~time_watch()
		{
			if(watchdoc_status==runing)
				watchdoc_status = ready;
			if(watchdoc_status==ready)
				watchdog.join();
		}
		operator bool() const { return this->zeitueberschreitung==false;}
	};
#pragma endregion
}


namespace
{
	struct mykey 
	{	using value_t = int;
	value_t value{};
	mykey(value_t value):value(value){}
	bool operator<(mykey const & r ) const { return this->value < r.value; }
	};
	struct mystring : std::string
	{
		using base_t = std::string;
		~mystring()
		{
		}
		mystring() : base_t() 
		{
		}
		template<typename T>mystring(T value) : base_t(std::move(value))
		{
		}
		mystring(base_t r) : base_t(std::move(r))
		{
		}
		mystring(mystring const & r) : base_t(r)
		{
		}
		mystring(mystring && r) : base_t(std::move(r))
		{
		}
		mystring& operator=(mystring const & r) & 
		{
			base_t::operator=(r);
			return *this;
		}
		mystring& operator=(mystring && r) & 
		{ 
			base_t::operator=((base_t&&)r);
			return *this;
		}
	};
}

namespace UTCache
{
	TEST_CLASS(UT_Verstaendnis)
	{
		TEST_METHOD(UT_Valid_Data)
		{
			using key_t=int;
			using value_t=std::unique_ptr<int>;
			std::map<key_t,value_t> map;

			auto v5=map.insert_or_assign(5,std::make_unique<int>(5));
			Assert::IsTrue( *v5.first->second == 5 );
			auto v3=map.insert_or_assign(3,std::make_unique<int>(3));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 3 );
			auto v9=map.insert_or_assign(9,std::make_unique<int>(9));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 3 );
			Assert::IsTrue( *v9.first->second == 9 );
			auto v7=map.insert_or_assign(7,std::make_unique<int>(7));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 3 );
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			map.erase(3);
			Assert::IsTrue( *v5.first->second == 5 );
			//Assert::IsTrue( v3.first );//no way checking if valid
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			v3 = map.insert_or_assign(3,std::make_unique<int>(33));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 33 );
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			map.insert_or_assign(3,std::make_unique<int>(333));//undefined behavior ? but v3 is valid with correct value
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 333 );
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			for(key_t i=0;i<1000;++i)
			{
				map.insert_or_assign(i,std::make_unique<int>(i));
				Assert::IsTrue( *v5.first->second == 5 );
				Assert::IsTrue( *v3.first->second == 333 || *v3.first->second==3);
				Assert::IsTrue( *v9.first->second == 9 );
				Assert::IsTrue( *v7.first->second == 7 );
			}

		}
		TEST_METHOD(UT_Duration_180ms)
		{
			using namespace std::literals::chrono_literals;

			std::chrono::milliseconds xx(5);

			auto breakafter = 180ms;
			std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();

			int i;
			auto stoptime = std::chrono::system_clock::now() + breakafter;
			for(i=0;;++i)
			{
				if( stoptime <= std::chrono::system_clock::now() )
					break;
			}
		}
		TEST_METHOD(UT_Duration2_150ms)
		{
			auto till = std::chrono::system_clock::now() + std::chrono::milliseconds{150};
			while(std::chrono::system_clock::now() <= till )
			{}
		}
		TEST_METHOD(UT_Key_200ms)
		{
			using namespace std::literals::chrono_literals;
			auto start_time {std::chrono::system_clock::now()};

			WS::Pure_key<int> key_t2{6};
			WS::Duration_key<int,200> key_t1{5};
			for( auto duration = std::chrono::system_clock::now() - start_time; key_t1.is_valid(); duration = std::chrono::system_clock::now() - start_time)
			{
				Assert::IsTrue( duration < 201ms );
				Assert::IsTrue( key_t2.is_valid() );
			}
			auto duration = std::chrono::system_clock::now() - start_time;
			Assert::IsTrue( 200ms <= duration );
		}
		TEST_METHOD(UT_ret_shared_ptr)
		{
			struct A
			{
				std::shared_ptr<int> data { new int{5} };

				std::shared_ptr<int> &		get_data1(){ return data;}
				std::shared_ptr<int> const & get_data1() const { return data;}
				std::shared_ptr<int> &		get_data2(){ return data;}
				std::shared_ptr<int const> const & get_data2() const { return data;}
			};

			A a;
			A const & const_ref_a = a;
			Assert::IsTrue( *a.get_data1() == 5 );
			*const_ref_a.get_data1() = 6;//von wegen const. der shared-pointer ist const, nicht dessen inhalt
			Assert::IsTrue( *a.get_data1() == 6 );

			//*const_ref_a.get_data2() = 7;//error C3892: 'const_ref_a': you cannot assign to a variable that is const
		}
		TEST_METHOD(UT_mutex_atomicflag__deadlock_detect)
		{
			Cout2Output<> coutumleiten{};
			WS::mutex_atomicflag mutex{};

			auto locked = WS::lock_guard(mutex);
			try
			{
				auto locked2 = WS::lock_guard(mutex);
				Assert::Fail(L"exception erwartet");
			}
			catch( std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}
		TEST_METHOD(UT_recursive_mutex_atomicflag)
		{
			Cout2Output<> coutumleiten{};
			WS::recursive_mutex_atomicflag mutex{};

			Assert::IsFalse(mutex.islocked());
			Assert::IsTrue(mutex.refcount() == 0);
			{
				auto locked = WS::lock_guard(mutex);
				Assert::IsTrue(mutex.islocked());
				Assert::IsTrue(mutex.refcount() == 1);
				{
					auto locked2 = WS::lock_guard(mutex);
					Assert::IsTrue(mutex.islocked());
					Assert::IsTrue(mutex.refcount() == 2);
				}
				Assert::IsTrue(mutex.islocked());
				Assert::IsTrue(mutex.refcount() == 1);
			}
			Assert::IsFalse(mutex.islocked());
			Assert::IsTrue(mutex.refcount() == 0);
		}
		TEST_METHOD(UT_recursive_mutex_atomicflag_im_thread)
		{
			constexpr size_t durchgaenge = 50;
			constexpr size_t anzahlthreads = 20;//mehr als 3 macht auf unseren kisten keinen sinn, die laufen gar nicht an
			using namespace std::chrono_literals;
			auto dauer_eines_durchgangs = 20ms;

			Cout2Output<> coutumleiten{};
			std::map<std::thread::id,std::atomic_size_t> counter{};
			WS::recursive_mutex_atomicflag mutex{};
			bool stop_threads{false};
			auto thread_fn = [&]()->bool
			{
			#define ret_if_false(value){if((value)==false)return false;}
				auto id = std::this_thread::get_id();
				while(stop_threads==false)
				{
					{
						auto locked = WS::lock(mutex);
						ret_if_false( mutex.lockingthread()==id );
						auto locked2 = WS::lock(mutex);//recursive_mutex, sonst deadlock-exception
						ret_if_false( mutex.lockingthread()==id );

						++counter[id];

						//expliciter unlock geht, geht auch mehrmals. ist hier sicher nicht nötig
						Assert::IsTrue( locked.unlock());
						Assert::IsFalse(locked.unlock());
					}
					using namespace std::chrono_literals;
					//std::this_thread::sleep_for(20us);//zumindest kurz nicht gelockt halten, sonst  könnte es zu dazu kommen dass nur noch ein thread läuft die anderen warten, warum auch immer
					std::this_thread::yield();
				}
				return true;
			};

			for(auto i=0; i<durchgaenge; ++i )
			{
				stop_threads = false;
				counter.clear();

				std::future<bool> threads[anzahlthreads];
				for( auto &t : threads )
					t = std::async(thread_fn);

				time_watch timewatch{dauer_eines_durchgangs*2.9,[&](){stop_threads=true;}};//wenn schleife zu lange braucht wird test rot und läuft nicht endlos, dazu müssen abr die threads beendet werden, sonst hängt Assert::

				std::this_thread::sleep_for(dauer_eines_durchgangs);
				stop_threads = true;
				std::cout << std::this_thread::get_id() << " stop threads refcount:" << mutex.refcount() <<  std::endl;

				{
					for(;;)
					{
						Assert::IsTrue(timewatch,L"methode läuft zu lange");

						if(auto locked = WS::try_lock(mutex);locked.is_locked())
						{
							std::cout << "stop " << anzahlthreads << " threads" << std::endl;
							for (auto& [id, counterper_thread] : counter)
								std::cout << " thread:" << id << " counter:" << counterper_thread << std::endl;
							break;
						}
					}
				}

				for( auto &t : threads )
					Assert::IsTrue(t.get());
			}
		}
	};
	TEST_CLASS(UTCache)
	{
	public:

		TEST_METHOD(cache_int_mystring)
		{
			{
				std::map<mykey,mystring> map;
				mystring str{"Hallo"};
				auto ptr = str.c_str();
				map[1] = std::move(str);
				//Assert::IsTrue( ptr == map[1].c_str() );
				ptr = map[1].c_str();
				std::map<mykey,mystring>::iterator iter = map.begin();
				Assert::IsTrue( ptr == iter->second.c_str() );
			}
			auto cache = WS::Cache<mykey,mystring>{};

			Assert::IsFalse( cache.Get(2).has_value() );
			Assert::IsTrue( cache.Set(2,"hallo").iter->second == "hallo" );
			Assert::IsTrue( cache.Set(3,"welt").iter->second == "welt" );
			auto v1 = cache.Get(1);
			Assert::IsFalse( v1.has_value() );
			auto v2 = cache.Get(2);//Get() liefert ggf. kopie im optional, braucht so aber auch keinen lock
			Assert::IsTrue( v2.has_value() );
			Assert::IsTrue( v2.value() == "hallo" );
			Assert::IsTrue( cache.Set(2,"Hallo").iter->second == "Hallo" );
			auto v2item = cache.get_locked_item(2);
			Assert::IsTrue( v2item );
			Assert::IsTrue( v2item.iter->second == "Hallo" );
			v2item.locked.unlock();//ohne deadlock-exception

			auto v1item = cache.get_locked_item(1);
			Assert::IsFalse( v1item );
		}
		TEST_METHOD(cache_int_mystring_valid_75ms)
		{
			auto cache = WS::CacheDuration<mykey,mystring,75>{};

			Assert::IsFalse( cache.Get(2).has_value() );
			cache.Set(3,"welt");
			auto start_time = std::chrono::system_clock::now();
			cache.Set(2,"hallo");
			while( cache.Get(2).has_value() ){}
			Assert::IsTrue( std::chrono::system_clock::now() - start_time > decltype(cache)::validkey_t::valid_duration() );
		}
		TEST_METHOD(cache_int_mystring_valid_DurationRefresh)
		{
			constexpr int valid = 100'000;
			auto cache = WS::CacheDurationRefresh<mykey,mystring,valid,std::chrono::microseconds>{};

			Assert::IsFalse( cache.Get(2).has_value() );
			cache.Set(3,"welt");
			auto start_time = std::chrono::system_clock::now();
			char buf1[20];
			char buf2[20];
			int sleep = valid / 50;
			int counter = 0;
			cache.Set(2,"hallo");
			while( cache.Get(2).has_value() )
			{
				//Logger::WriteMessage( (std::string(__FUNCTION__)+" Sleep:"+tostring(sleep,buf,10)+"µs").c_str());
				//(void)cache.Get(2);
				if( ((++counter)%5)==0 )
					sleep*=2;
				std::this_thread::sleep_for(std::chrono::microseconds(sleep));
			}
			Logger::WriteMessage( (std::string(__FUNCTION__)+" counter:"+tostring(counter,buf1,10)+" Sleep:"+tostring(sleep,buf2,10)+"µs").c_str());
			Assert::IsTrue( counter==30 ); //&& sleep>=valid );
		}
		TEST_METHOD(cache_int_mystring_validkey_maker)
		{
			int validator=5;
			auto make_validkey = [&](mykey key)
			{
				struct keyhandler
				{
					using validator_t = std::remove_reference_t<decltype(validator)>;
					using key_t = mykey;
					validator_t const & validator_value;
					validator_t soll_validator_value;

					key_t key;

					keyhandler()=delete;
					keyhandler( key_t key, validator_t const & validator_value) : key(std::move(key)), validator_value(validator_value),soll_validator_value(validator_value){}

					bool operator<( keyhandler const & r ) const {return  this->key<r.key;}
					bool is_valid() const 
					{
						return validator_value==soll_validator_value;
					} 
					bool is_invalid() const {return !is_valid();}
				};
				return keyhandler(std::move(key),validator);
			};

			auto cache = WS::Cache<mykey,mystring,std::mutex,decltype(make_validkey(std::declval<mykey>()))>{make_validkey};

			Assert::IsFalse( cache.Get(2).has_value() );
			for( auto i=5; i --> 0;)
			{
				cache.Set( 3, "welt" );
				cache.Set( 2, "hallo" );
				for( int i2=100; i2--> 0; )
				{
					Assert::IsTrue( cache.Get( 2 ).has_value() );
				}
				++validator;
				Assert::IsFalse( cache.Get( 2 ).has_value() );
				Assert::IsFalse( cache.Get( 3 ).has_value() );
			}
		}
	};
}
