#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\semaphore.h"

#include <sstream>

void einfaches_daten_synchronisations_beispiel ()
{
	WS::Semaphore		sema;//blocked
	bool				ready{false};
	std::atomic<size_t> counter{0};
	

	auto worker = std::thread( [&]()
	{	//thread zählt von 0 - 100 und legt sich dann schlafen
		for(;ready==false;)
			if( ++counter == 100 )
				sema.set_blocked_and_wait();//wichtig statt blocked();wait(); was zur race-condition führen kann. Es ginge auch wait( blocked() );
	});

	size_t counter_inner{20000};//20'000 mal bis 100 zählen lassen, super anspruchsvoll
	while( ready==false )
	{
		//wenn worker nichts mehr zu tun hat mit neuen daten(counter=0) aufwecken
		if( sema.is_signaled()==false )
		{
			counter=0;
			if( --counter_inner == 0 )
				ready=true;//threads beenden

			sema.set_running();
		}
		//else
		//	sema.notify_all();//eine notification geht scheinbar schonmal verloren. wenn man lange genug wartet werden vom system notifications ausgelöst. ist aber lästig
		std::this_thread::yield();//mit yield ca. 45% schneller ??
	}

	worker.join();
}


namespace 
{
	void out( char const * text ) 
	{
		Logger::WriteMessage(text);
	}
	void out( wchar_t const * text ) 
	{
		Logger::WriteMessage(text);
	}

	//#pragma warning(suppress:4455)
	//short operator"" sh(unsigned long long int v) noexcept {return short(v);}
}

namespace UTSemaphore
{
	TEST_CLASS(UTSemaphore)
	{
	public:
		TEST_METHOD(Beispiel1)
		{
			einfaches_daten_synchronisations_beispiel();
		}
		TEST_METHOD(_1000_pulse_auf_2000_threads)
		{
			WS::Semaphore		sema;
			bool				ready{false};
			std::atomic<size_t> counter{0};
			std::atomic<size_t>	threads_running{0};

			size_t anzahl_pulse		= 1000;
			size_t anzahl_threads	= 2000;//über 3030 threads kam es bei ersten tests zu "Microsoft C++ exception: std::system_error ..."

			auto fn = [&]
			{
				//std::ostringstream ss;
				for(;ready==false;)
				{
					//ss << std::this_thread::get_id() << " blocking";out(ss.str().c_str());ss.str("");
					sema.wait();//wartet bis sema in running-state  versetzt wird bzw. hier pulse ausgeführt wird. pulse soll wait-blockade genau einmal lösen. der nächste aufruf von wait blockiert bis zum nächsten pulse
					++counter;//das ist die arbeit die der thread ausführt. kurz, aber immer hin klar definiert
					//ss << std::this_thread::get_id() << " running";out(ss.str().c_str());ss.str("");
				}
				--threads_running;
			};
			auto start_thread = [&]
			{
				try
				{
					std::thread(fn).detach();
					++threads_running;
				}
				catch( std::exception & e)
				{
					auto str = std::string(__FUNCTION__ " ") +   e.what();str;
					//out( str.c_str() );
				}
				catch(...)
				{
					out( __FUNCTION__ " exception" );
				}
			};
			using namespace std::chrono_literals;
			Assert::IsTrue( counter==0);

			Assert::IsTrue( sema.pulse() == 0 );
			Assert::IsTrue( counter==0);

			start_thread();
			while( sema.Waiting()!=1)
				std::this_thread::yield();

			decltype(sema)::count_t started_thread_count = sema.pulse();//nicht auto, sonst wird der lock nicht freigegeben
			while(sema.Waiting()!=started_thread_count)std::this_thread::yield();//pulse() wartet bis thread läuft, nicht bis die arbeit gemacht ist.
			Assert::IsTrue( counter==started_thread_count);
			counter=0;


			for( size_t i=1; i<anzahl_threads; ++i )
				start_thread();

			//MessageBox(nullptr,L"wait",L"", 0);

			Assert::IsTrue( threads_running == anzahl_threads, L"es konnten nicht alle threads gestartet werden" );

			while( sema.Waiting()!=threads_running)
				std::this_thread::yield();//warten bis alle im definierten warte-zustand sind

			for( auto i=anzahl_pulse; i --> 0; )
			{
				started_thread_count = sema.pulse();//funktioniert nur, wenn kein thread per therminate oder sonstige krummen dinge beendet wurde
				Assert::IsTrue( started_thread_count == anzahl_threads );

				while( sema.Waiting()!=threads_running)
					std::this_thread::yield();//warten bis alle wieder im definierten warte-zustand sind

				Assert::IsTrue( counter==threads_running );

				//std::ostringstream ss;ss << "loop " << i;out(ss.str().c_str());

				counter=0;
			}

			//aufräumen, alle threads beenden lassen
			ready=true;
			sema.set_running();
			while( threads_running )
				std::this_thread::yield();
		}
		TEST_METHOD( lock_at_100_mal_20000 )
		{
			WS::Semaphore		sema;
			bool				ready{false};
			std::atomic<size_t>	counter{0};

			sema.set_running();
			auto threadfunction = [&]
			{
				for(;ready==false;)
				{
					if( ++counter == 100 )
					{
						sema.set_blocked_and_wait();//wichtig statt set_blocked();wait(); was zur race-condition führt
					}
					if( counter >= 100 )
						throw std::runtime_error(__FUNCTION__ " counter sollte wieder bei 0 sein");//ohne try catch abort
				}
			};
			auto t1 = std::thread(threadfunction);
			auto releasethreadfunction = [&]
			{
				size_t counter_inner{20000};
				while( ready==false )
				{
					if( sema.is_signaled()==false )
					{
						counter=0;
						if( --counter_inner == 0 )
							ready=true;//threads beenden

									   //Assert::IsTrue( sema.Waiting()==1);Der aktive Testlauf wurde abgebrochen. Grund: Der Testhostprozess ist abgestürzt.
						if( sema.Waiting() != 1)
							throw std::runtime_error(__FUNCTION__ " muss hier 1 liefern");

						sema.set_running();
					}
					else
					{
						using namespace std::chrono_literals;
						//std::this_thread::sleep_for(30us);//???mit 1 us oder yield gehen scheinbar die notify_all verloren???
						//std::this_thread::yield();
						if(01)//notify_all sollte nicht nötig sein. so ein mist. Hat doch set_running() schon einmal gemacht
							sema.notify_all();//eine notification geht scheinbar schonmal verloren, wenn man lange genug wartet werden von system notifications ausgelöst. ist aber lästig
					}
				}
			};
			auto t2 = std::thread(releasethreadfunction);

			t1.join();
			t2.join();
		}
		TEST_METHOD( inc_100_mal_20000 )
		{
			for( std::atomic<size_t>	counter{0}; counter<100*20000; )
			{
				++counter;
			}
		}
		TEST_METHOD( move_lock )
		{
			std::mutex	mutex{};
			auto lock = WS::lock(mutex);
			Assert::IsTrue( lock.is_locked() );
			decltype(lock) lock2 = std::move(lock);
			Assert::IsFalse( lock.is_locked() );
			Assert::IsTrue( lock2.is_locked() );
		}
		TEST_METHOD( std__reference_wrapper2 )
		{
			{
				int a = 3;
				int b = 4;
				auto refa = std::ref(a);
				auto refb = std::ref(b);
				Assert::IsTrue(refa < refb);
			}
			{
				struct A
				{
					int v;
					bool operator<( A const & r) const {return v<r.v;}
					//friend bool operator<( std::reference_wrapper<A> const & l, A const & r) {return l.get().v<r.v;}
				};
				A a { 3 };
				A b { 4 }; 
				auto refa = std::ref(a);
				auto refb = std::ref(b);
				//Assert::IsTrue(refa < refb);//error C2678: binary '<': no operator found which takes a left-hand operand of type 'std::reference_wrapper<UTSemaphore::UTSemaphore::std__reference_wrapper2::A>' (or there is no acceptable conversion)
				Assert::IsTrue(refa.get() < refb);
			}
		}
		TEST_METHOD( std__reference_wrapper1 )
		{
			struct A
			{
				A()=default;
				A(A const &)=delete;
				int foo(){return 5;}
				auto& operator()(A & v){return v;}
			};

			A a;
			Assert::IsTrue( a.foo() == 5 );
			std::reference_wrapper<A> ra = a;
			//Assert::IsTrue( ra.foo() == 5 );//error C2039: 'foo': is not a member of 'std::reference_wrapper<UTSemaphore::UTSemaphore::std__reference_wrapper1::A>'
			Assert::IsTrue( ra.get().foo() == 5 );
			static_cast<decltype(ra)::type&>(ra).foo();
			A{}(ra).foo();
			//A(ra).foo();
			
		}
		TEST_METHOD( is_signale_blocked )
		{
			WS::Semaphore sema;

			{
				auto issignaled = sema.is_signaled();//hält ergebnis und LOCK
				Assert::IsTrue(  issignaled==false && sema.is_blocked(issignaled) );//LOCK an is_blocked weiter geben
				sema.signaled();
				issignaled = sema.is_signaled();
				Assert::IsTrue(  issignaled && sema.is_blocked(issignaled)==false );
			}

			if( auto issignaled = sema.is_signaled(); issignaled==false || sema.is_blocked(issignaled) )
			{
				Assert::Fail();
			}
			sema.blocked();
			if( auto issignaled = sema.is_signaled(); issignaled || sema.is_blocked(issignaled)==false )
			{
				Assert::Fail();
			}

		}
	};
}
