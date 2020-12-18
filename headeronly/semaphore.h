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
//		if( sema.is_signaled()==false )
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
		using mutex_t				= std::mutex;
		using condition_variable_t	= std::condition_variable;
		using count_t				= size_t;
		enum class Notify{none,one,all};

		mutex_t						wait_mutex{};
		condition_variable_t		cv{};
		std::atomic_bool			running{false};
		std::atomic<count_t>		waiting{0};//anzahl wartender threads, für pulse nötig

		mutable mutex_t				state_mutex{};//dient hauptsächlich der vermeidung von race-condition

	public:
		#pragma region status der semaphore runing oder blocked
		bool is_signaled() const {auto locked=lock(this->state_mutex);return _is_signaled();}
		bool operator()()  const {return  is_signaled();}
		operator bool ()   const {return  is_signaled();}
		bool is_blocked()  const {return !is_signaled();}
		#pragma endregion

		#pragma region blocking methoden
		void reset()		{auto locked=lock(this->state_mutex);_set_blocked();}
		void set_blocked()	{auto locked=lock(this->state_mutex);_set_blocked();}
		#pragma endregion 
		void set_blocked_and_wait()//statt set_blocked und wait unabhängig mit gefahr einer race condition
		{
			auto locked=lock(this->state_mutex);
			_set_blocked();
			_wait( locked );
		}
		void wait( )//es wird ggf gewartet, bis die Semaphore im running-mode ist, per set_running() oder pulse() egal
		{
			auto locked =lock(this->state_mutex);
			_wait( locked );
		}

		#pragma region running methoden
		void signaled(Notify eNotify)		{auto locked=lock(this->state_mutex);_set_signaled_and_wait_till_all_running(eNotify);}
		void signaled()						{signaled(Notify::all);}
		void set_running(Notify eNotify)	{signaled(eNotify);}
		void set_running()					{signaled(Notify::all);}
		void pulse() //jeder wartender thread soll gestartet werden, der nächste wait im thread blockiert wieder. also definiert einmalige ausführung.
		{
			auto locked=lock(this->state_mutex);//blockiert neue wait-aufrufe und damit das hochzählen von waiting
			if( _is_blocked() )
			{
				_set_signaled_and_wait_till_all_running(Notify::all);
				_set_blocked();
			}
		}
		#pragma endregion 

		count_t	Waiting(){auto locked=lock(this->state_mutex);return waiting;}//liefert anzahl der wartenden threads

		void	notify(Notify notify)
		{
			switch(notify)
			{
			case Notify::one:
				cv.notify_one(); 
				break;
			case Notify::all:
				cv.notify_all(); 
				break;
			}
		}
		void	notify_all(){ notify(Notify::all); };//sollten wartende threads nicht gestartet werden, kann mit notify_all() der anstoß erneute ausgelöst werden. k.A. warum das manchmal nötig ist

	private:
		//funktionen ohne eigenen lock sind private
		bool _is_signaled() const {return  running;}
		bool _is_blocked()  const {return !running;}
		void _set_signaled(Notify eNotify){running=true;notify(eNotify);}
		void _set_blocked(){running=false;}
		void _wait( lock_guard<decltype(state_mutex)> & pulse_lock)
		{
			if(running==false)
			{
				auto condition = [this,pulse_lock=std::move(pulse_lock)]() mutable //ohne mutable lambda klappt das mit dem verschieben des pulse_lock nicht, da die capture-parameter const wären
				{
					bool running = this->_is_signaled();
					if(  running==false && pulse_lock.is_locked() )
						++waiting;//erster aufruf, wir warten. genau einmal waiting++
					else if( running && pulse_lock.is_locked()==false )
						--waiting;//nicht erster aufruf und wir warten nicht mehr

					pulse_lock.unlock();//setzt beim ersten aufruf der check_funktion den mutext zurück, die semaphore ist wieder frei für veränderung, egal ob gewartet wird, oder nicht
					return running;
				};
				std::unique_lock<mutex_t> lk( wait_mutex );
				cv.wait( lk, std::ref(condition) );
			}
		}

		void _wait_till_all_running( count_t const threads_waiting, Notify const eNotify)
		{
			if( eNotify!=Notify::none )
			{
				count_t counter = 0; 
				while( this->waiting )//wartet noch ein thread
				{
					if( eNotify==Notify::one && this->waiting<threads_waiting )
						return;//mind. einer ist losgelaufen

					if( ++counter > threads_waiting )//warten wir schon lange?
					{
						notify( eNotify );//???geht doch manchmal einer verloren, also nochmal anstoßen!!!
						counter=0;//evtl. läuft das ewig. dann hängt ein thread oder wurde unsachgemäß beendet
					}
					std::this_thread::yield();//nicht nötig, schadet aber auch nicht
				}
			}
		}
		void _set_signaled_and_wait_till_all_running(Notify const eNotify)
		{
			if( _is_signaled()==false )
			{
				count_t const threads_waiting = this->waiting;//weil notify_all manchmal nicht die gewünschte wirkung erzielt. die anzahl wartenden threads vor signaled merken und weitergeben
				_set_signaled(eNotify);
				_wait_till_all_running( threads_waiting, eNotify);
			}
		}
	};
}

