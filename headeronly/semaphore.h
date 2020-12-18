#pragma once
///Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

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
//	{	//thread z�hlt von 0 - 100 und legt sich dann schlafen
//		for(;ready==false;)
//			if( ++counter == 100 )
//				sema.set_blocked_and_wait();//wichtig statt set_blocked();wait(); was zur race-condition f�hren kann
//	});
//
//	size_t counter_inner{20000};//20'000 mal bis 100 z�hlen lassen, super anspruchsvoll
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
//			sema.notify_all();//eine notification geht scheinbar schonmal verloren. wenn man lange genug wartet werden vom system notifications ausgel�st. ist aber l�stig
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
		std::atomic<count_t>		waiting{0};//anzahl wartender threads, f�r pulse n�tig

		mutable mutex_t				state_mutex{};//dient haupts�chlich der vermeidung von race-condition

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
		void set_blocked_and_wait()//statt set_blocked und wait unabh�ngig mit gefahr einer race condition
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
		void pulse() //jeder wartender thread soll gestartet werden, der n�chste wait im thread blockiert wieder. also definiert einmalige ausf�hrung.
		{
			auto locked=lock(this->state_mutex);//blockiert neue wait-aufrufe und damit das hochz�hlen von waiting
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
		void	notify_all(){ notify(Notify::all); };//sollten wartende threads nicht gestartet werden, kann mit notify_all() der ansto� erneute ausgel�st werden. k.A. warum das manchmal n�tig ist

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
				auto condition = [this,pulse_lock=std::move(pulse_lock)]() mutable //ohne mutable lambda klappt das mit dem verschieben des pulse_lock nicht, da die capture-parameter const w�ren
				{
					bool running = this->_is_signaled();
					if(  running==false && pulse_lock.is_locked() )
						++waiting;//erster aufruf, wir warten. genau einmal waiting++
					else if( running && pulse_lock.is_locked()==false )
						--waiting;//nicht erster aufruf und wir warten nicht mehr

					pulse_lock.unlock();//setzt beim ersten aufruf der check_funktion den mutext zur�ck, die semaphore ist wieder frei f�r ver�nderung, egal ob gewartet wird, oder nicht
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
						notify( eNotify );//???geht doch manchmal einer verloren, also nochmal ansto�en!!!
						counter=0;//evtl. l�uft das ewig. dann h�ngt ein thread oder wurde unsachgem�� beendet
					}
					std::this_thread::yield();//nicht n�tig, schadet aber auch nicht
				}
			}
		}
		void _set_signaled_and_wait_till_all_running(Notify const eNotify)
		{
			if( _is_signaled()==false )
			{
				count_t const threads_waiting = this->waiting;//weil notify_all manchmal nicht die gew�nschte wirkung erzielt. die anzahl wartenden threads vor signaled merken und weitergeben
				_set_signaled(eNotify);
				_wait_till_all_running( threads_waiting, eNotify);
			}
		}
	};
}

