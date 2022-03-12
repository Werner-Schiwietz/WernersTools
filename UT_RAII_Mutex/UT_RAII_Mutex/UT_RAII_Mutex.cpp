#include "pch.h"
#include "CppUnitTest.h"


#include <future>


#include "..\..\headeronly\semaphore.h"
#include "..\..\headeronly\raii_mutex.h"
#include "..\..\headeronly\mutex_atomicflag.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#pragma warning(push,4)


namespace WS
{
	struct _foo{};
	int foo(_foo){return 7;}
	int foo(int){return 6;}
	int foo(raii_mutex<std::recursive_timed_mutex> &  ){return 5;}
	int foo(raii_mutex<std::timed_mutex> &  ){return 4;}
}




int xxxx = []()->int
{
	WS::raii_mutex<std::timed_mutex> m;
	[[maybe_unused]]auto x1 = foo(WS::_foo{});//compiliert wegen ADL
											  //[[maybe_unused]]auto x2 = foo(int{5});//error int ist kein type aus namespace
	[[maybe_unused]]auto x3 = foo(m);//compiliert wegen ADL

	(void)std::lock_guard{m};
	(void)std::scoped_lock{m};
	(void)lock(m);//lock + unlock //warum übersetzt das. es gibt kein "using namespace WP"
	(void)WS::lock(m);//lock + unlock
	auto locked = WS::lock(m);
	[[maybe_unused]]decltype(locked)::base_t & basetype_locked = locked;
	//locked.release();//error C2280: deleted, aber mit genügend "kriminelle energie" siehe UT_evil_things

	return 0;
}();

enum class deadlocktestStatus{uninitialised,running,firstlocked,ready,exception} ;
template<typename mutex_t>struct doublelock
{
	deadlocktestStatus& status;
	doublelock( deadlocktestStatus& status):status(status){ this->status=deadlocktestStatus::uninitialised;}
	void operator()(void)
	{
		mutex_t mutex;
		status=deadlocktestStatus::running;
		auto locked = WS::lock(mutex);
		status=deadlocktestStatus::firstlocked;
		try
		{
			(void)WS::lock(mutex);
			status=deadlocktestStatus::ready;
		}
		catch(...)
		{
			status=deadlocktestStatus::exception;
		}
	}
};

template<typename mutex_t>void lock(size_t runden )
{
	mutex_t mutex;

	for(auto i=runden; i-->0; )
	{
		auto x=std::lock_guard{mutex};x;
	}
}
template<typename mutex_t>void createarray_and_lock(size_t size )
{
	auto mutexPtr= std::make_unique<mutex_t[]>(size);
	auto guardPtr= std::make_unique<std::unique_lock<mutex_t>[]>(size);
	for(auto i=size; i --> 0 ;)
	{
		guardPtr[i]=std::unique_lock<mutex_t>{mutexPtr[i]};
	}
}


namespace UT_WP_TimedMutex
{
	TEST_CLASS(UT_WP_timed_mutex)
	{

	public:
		TEST_CLASS_INITIALIZE(Init)
		{
		}
		TEST_METHOD(UT_timed_mutex_lockit)
		{
			WS::raii_mutex<std::recursive_timed_mutex> m;

			(void)lock(m);//lock + unlock 
			(void)lock(m);//lock + unlock
						  //Assert::IsTrue( lock(m) );//lock + unlock // error C2280: 'WS::unique_lock<WS::raii_mutex<std::recursive_timed_mutex>>::operator bool(void) noexcept const &': attempting to reference a deleted function
			{
				auto locked = lock(m);
				Assert::IsTrue(locked);//lock + unlock
			}
			if( auto locked = lock(m) )
			{
				auto locked2 = try_lock(m);
				Assert::IsTrue( locked2 );//rekursiv_lock
			}
			//if( lock(m) )//error C2280: 'WS::unique_lock<WS::raii_mutex<std::recursive_timed_mutex>>::operator bool(void) noexcept const &': attempting to reference a deleted function
			{
				//Assert::IsTrue( try_lock(m) );//error C2280: 'WS::unique_lock<WS::raii_mutex<std::recursive_timed_mutex>>::operator bool(void) noexcept const &': attempting to reference a deleted function
			}
		}
		TEST_METHOD(UT_timed_mutex_lock_failed)
		{
			WS::raii_mutex<std::recursive_timed_mutex> m;
			WS::Semaphore sema;
			WS::Semaphore sema2;

			//traceln("mainThread:",std::this_thread::get_id());

			//thread1 hält lock bis sema auf running gesetzt wird
			auto thread1  = 
				std::async(	std::launch::async,
							[&]()
			{
				//traceln("thread1:",std::this_thread::get_id());
				auto locked = lock(m);
				sema2.set_running();
				Assert::IsTrue(locked.owns_lock());
				sema.set_blocked_and_wait();
				Assert::IsTrue(locked.owns_lock());
			}
			);

			sema2.wait();

			auto x = WS::try_lock(m);
			auto toboolhelper=[](auto&&locked){return (bool)locked;};
			Assert::IsFalse( toboolhelper(WS::try_lock(m)) );
			Assert::IsFalse( toboolhelper(WS::try_lock_until(m,std::chrono::steady_clock::now()+std::chrono::milliseconds{50})) );
			Assert::IsFalse( toboolhelper(WS::try_lock_for(m,std::chrono::milliseconds{50})) );

			{
				//sema ist block, also kann thread2 keinen lock setzen
				auto thread2 = std::async(	std::launch::async
										   , [&](std::chrono::milliseconds millisec)->bool
				{
					auto locked = try_lock_for(m,millisec);
					//traceln("thread1 locked ", (bool)locked);
					//Assert::IsFalse( locked );//Assert nicht thread-safe lol
					return locked;
				}
										   , std::chrono::milliseconds{10}
				);
				thread2.wait();
				Assert::IsFalse(thread2.get());
			}
			{
				//thread2 wartet lange genung, um den lock setzen zu können
				auto thread2 = std::async(	std::launch::async
										   , [&](std::chrono::milliseconds millisec)->bool
				{
					auto locked = try_lock_for(m,millisec);
					//traceln("thread2 locked ", (bool)locked);
					//Assert::IsTrue( locked );//Assert nicht thread-safe lol
					return locked;
				}
										   , std::chrono::milliseconds{2'000}//ggf. lange genug warten
				);

				using namespace std::literals;
				// or using namespace std::literals::chrono_literals;
				// or using namespace std::chrono_literals;
				std::this_thread::sleep_for(200ms);

				sema.set_running();//thread1 gibt lock frei
								   //thread2.wait();
				Assert::IsTrue(thread2.get());//get result
			}
			thread1.wait();
			//Assert::Fail();
		}
		TEST_METHOD(UT_samethread_deadlock)
		{
			deadlocktestStatus status{};
			using namespace std::literals;

			{
				auto worker = std::async(	std::launch::async, doublelock<std::mutex>{status} );
				auto waittingstate = worker.wait_for(200ms);
				Assert::IsTrue( waittingstate==std::future_status::ready);
				Assert::IsTrue( status==deadlocktestStatus::exception);
			}
			{
				auto worker = std::async(	std::launch::async, doublelock<std::recursive_mutex>{status} );
				auto waittingstate = worker.wait_for(200ms);
				Assert::IsTrue( waittingstate==std::future_status::ready);
				Assert::IsTrue( status==deadlocktestStatus::ready);
			}
			{
				auto worker = std::thread( doublelock<std::timed_mutex>{status} );
				std::this_thread::sleep_for(200ms);
				worker.detach();//dem ist nicht zu helfen
				Assert::IsTrue(status==deadlocktestStatus::firstlocked);//timed_mutex in MS implementierung erzeugt am 2022-01-09(10.0.22000.0) deadlock
			}
			{
				auto worker = std::async(	std::launch::async, doublelock<std::recursive_timed_mutex>{status} );
				auto waittingstate = worker.wait_for(200ms);
				Assert::IsTrue( waittingstate==std::future_status::ready);
				Assert::IsTrue( status==deadlocktestStatus::ready);
			}
		}
		TEST_METHOD(UT_evil_things)
		{
			WS::raii_mutex<std::timed_mutex> m;			

			auto locked = WS::lock(m);

			//dinge, die du nie tun sollltest
			//locked.release();//deleted, aber mit genügend "kriminelle energie"
			locked.decltype(locked)::base_t::release(); //lol
			locked = WS::try_lock(m);
			Assert::IsFalse(locked);
			//auto mutex_pointer = locked.decltype(locked)::base_t::release();//übersetzt nicht ???
			//aber wer C++ kann
			//never do things like this
			struct EsGehtDoch : decltype(m)
			{
				using base_t = decltype(m);
				void unlock() { base_t::base_t::unlock(); }
			};
			EsGehtDoch* imevil = (EsGehtDoch*)(&m);
			imevil->unlock();

			locked = WS::try_lock(m);
			Assert::IsTrue(locked);
		}

		TEST_METHOD(UT_100000_WS_raii_mutex_std_recursive_mutex)
		{
			lock<WS::raii_mutex<std::recursive_mutex>>(100'000);
		}
		TEST_METHOD(UT_100000_std_recursive_mutex)
		{
			lock<std::recursive_mutex>(100'000);
		}
		TEST_METHOD(UT_100000_WS_raii_mutex_WS_recursive_mutex_atomicflag)
		{
			lock<WS::raii_mutex<WS::recursive_mutex_atomicflag>>(100'000);
		}
		TEST_METHOD(UT_100000_WS_recursive_mutex_atomicflag)
		{
			lock<WS::recursive_mutex_atomicflag>(100'000);
		}
		TEST_METHOD(UT_100000_WS_raii_mutex_std_mutex)
		{
			lock<WS::raii_mutex<std::mutex>>(100'000);
		}
		TEST_METHOD(UT_100000_std_mutex)
		{
			lock<std::mutex>(100'000);
		}
		TEST_METHOD(UT_100000_WS_raii_mutex_WS_mutex_atomicflag)
		{
			lock<WS::raii_mutex<WS::mutex_atomicflag>>(100'000);
		}
		TEST_METHOD(UT_100000_WS_mutex_atomicflag)
		{
			lock<WS::mutex_atomicflag>(100'000);
		}

		TEST_METHOD(UT_with_100000000_std_mutex)
		{
			constexpr size_t size = 100'000'000;
			using mutex_t=std::mutex;
			createarray_and_lock<mutex_t>(size);
		}
		TEST_METHOD(UT_with_100000000_WS_mutex_atomicflag)
		{
			constexpr size_t size = 100'000'000;
			using mutex_t=WS::mutex_atomicflag;
			createarray_and_lock<mutex_t>(size);
		}

	};
}
