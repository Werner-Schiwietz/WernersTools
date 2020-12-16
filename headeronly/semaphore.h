#pragma once
///Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

//header 
//headeronly\semaphore.h			diese datei
//
//headeronly\mutex_automicflag.h	mutex fuer lock_guard nutzt atomic_flag
//<atomic>
//<mutex>
//<condition_variable>


//usage siehe UT_semaphore.cpp 
//	wichtige funktionen
//		WS::Semaphore::set_running()
//		WS::Semaphore::set_blocked()
//		WS::Semaphore::wait() //if blocked
	//oder als taktgeber
//		WS::Semaphore::wait() //if blocked
//		WS::Semaphore::pulse()


// beispiel
//void einfaches_daten_synchronisations_beispiel ()
//{
//	WS::Semaphore		sema;//blocked
//	bool				ready{false};
//	std::atomic<size_t> counter{0};
//
//
//	auto worker = std::thread( [&]()
//	{	//thread zählt von 0 - 100 und legt sich dann schlafen
//		for(;ready==false;)
//			if( ++counter == 100 )
//				sema.set_blocked_and_wait();//wichtig statt set_blocked();wait(); was zur race-condition führen kann
//	});
//
//	size_t counter_inner{20000};//20'000 mal bis 100 zählen lassen, super anspruchsvoll
//	while( ready==false )
//	{
//		//wenn worker nichts mehr zu tun hat mit neuen daten(counter=0) aufwecken
//		if( sema.is_running()==false )
//		{
//			counter=0;
//			if( --counter_inner == 0 )
//				ready=true;//threads beenden
//
//			sema.set_running();
//		}
//		else
//			sema.notify_all();//eine notification geht scheinbar schonmal verloren. wenn man lange genug wartet werden vom system notifications ausgelöst. ist aber lästig
//	}
//
//	worker.join();
//}

#include "mutex_automicflag.h"

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace WS
{
	class Semaphore
	{
		std::mutex					wait_mutex{};
		std::condition_variable		cv{};
		std::atomic_bool			running{false};
		std::atomic<size_t>			waiting{0};//anzahl wartender threads, für pulse nötig

		mutable std::mutex			state_mutex{};//dient hauptsächlich der vermeidung von race-condition
		auto _lock_state_mutex() const
		{
			return WS::lock_guard<decltype(this->state_mutex)>(this->state_mutex);
		}

	public:
		#pragma region status der semaphore runing oder blocked
		bool is_running() const {auto locked=_lock_state_mutex();return _is_running();}
		bool operator()() const {return is_running();}
		operator bool () const {return is_running();}
		#pragma endregion

		#pragma region blocking methoden
		void reset()		{auto lock=_lock_state_mutex();_set_blocked();}
		void set_blocked()	{auto lock=_lock_state_mutex();_set_blocked();}
		#pragma endregion 
		void set_blocked_and_wait()//statt set_blocked und wait unabhängig mit gefahr einer race condition
		{
			auto lock=_lock_state_mutex();
			_set_blocked();
			_wait( lock );
		}

		void wait( )//es wird ggf gewartet, bis die Semaphore im running-mode ist, per set_running() oder pulse() egal
		{
			auto pulse_lock =_lock_state_mutex();
			_wait( pulse_lock );
		}

		#pragma region running methoden
		void signaled()		{auto lock=_lock_state_mutex();_set_running();}
		void set_running()	{auto lock=_lock_state_mutex();_set_running();}
		void pulse() //jeder wartender thread soll gestartet werden, der nächste wait im thread blockiert wieder. also definiert einmalige ausführung.
		{
			auto pulse_lock=_lock_state_mutex();//blockiert neue wait-aufrufe und damit das hochzählen von waiting
			if( _is_running()==false )
			{
				size_t counter = 0;size_t const threads = waiting;//basteln, weil notify_all manchmal nicht die gewünschte wirkung erzielt
				_set_running();
				while( waiting )//alle wartenten müssen gestartet worden sein
				{
					if( ++counter > threads )
					{
						cv.notify_all();//???geht doch manchmal einer verloren, also nochmal anstoßen!!!
						counter=0;
					}
					std::this_thread::yield();//nicht nötig, schadet aber auch nicht
				}
				_set_blocked();
			}
		}
		#pragma endregion 

		size_t	Waiting(){auto lock=_lock_state_mutex();return waiting;}//liefert anzahl der wartenden threads
		void	notify_all(){ cv.notify_all(); };//sollten wartende threads nicht gestartet werden, kann mit notify_all() der anstoß erneute ausgelöst werden. k.A. warum das manchmal nötig ist
	private:
		//funktionen ohne eigenen lock sind private
		bool _is_running() const {return running;}
		void _set_running(){running=true;cv.notify_all();}
		void _set_blocked(){running=false;}
		void _wait( WS::lock_guard<decltype(state_mutex)> & pulse_lock)
		{
			if(running==false)
			{
				auto condition = [this,pulse_lock=std::move(pulse_lock)]() mutable //ohne mutable lambda klappt das mit dem verschieben des pulse_lock nicht, da die capture-parameter const wären
				{
					pulse_lock.unlock();//setzt beim ersten aufruf der check_funktion den mutext zurück
					return this->_is_running(); 
				};
				std::unique_lock<std::mutex> lk( wait_mutex );
				++waiting;
				cv.wait( lk, std::ref(condition) );
				--waiting;
			}
		}
	};
}

