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
//		if( sema.is_running()==false )
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
		std::mutex					wait_mutex{};
		std::condition_variable		cv{};
		std::atomic_bool			running{false};
		std::atomic<size_t>			waiting{0};//anzahl wartender threads, f�r pulse n�tig

		mutable std::mutex			state_mutex{};//dient haupts�chlich der vermeidung von race-condition
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
		void set_blocked_and_wait()//statt set_blocked und wait unabh�ngig mit gefahr einer race condition
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
		void pulse() //jeder wartender thread soll gestartet werden, der n�chste wait im thread blockiert wieder. also definiert einmalige ausf�hrung.
		{
			auto pulse_lock=_lock_state_mutex();//blockiert neue wait-aufrufe und damit das hochz�hlen von waiting
			if( _is_running()==false )
			{
				size_t counter = 0;size_t const threads = waiting;//basteln, weil notify_all manchmal nicht die gew�nschte wirkung erzielt
				_set_running();
				while( waiting )//alle wartenten m�ssen gestartet worden sein
				{
					if( ++counter > threads )
					{
						cv.notify_all();//???geht doch manchmal einer verloren, also nochmal ansto�en!!!
						counter=0;
					}
					std::this_thread::yield();//nicht n�tig, schadet aber auch nicht
				}
				_set_blocked();
			}
		}
		#pragma endregion 

		size_t	Waiting(){auto lock=_lock_state_mutex();return waiting;}//liefert anzahl der wartenden threads
		void	notify_all(){ cv.notify_all(); };//sollten wartende threads nicht gestartet werden, kann mit notify_all() der ansto� erneute ausgel�st werden. k.A. warum das manchmal n�tig ist
	private:
		//funktionen ohne eigenen lock sind private
		bool _is_running() const {return running;}
		void _set_running(){running=true;cv.notify_all();}
		void _set_blocked(){running=false;}
		void _wait( WS::lock_guard<decltype(state_mutex)> & pulse_lock)
		{
			if(running==false)
			{
				auto condition = [this,pulse_lock=std::move(pulse_lock)]() mutable //ohne mutable lambda klappt das mit dem verschieben des pulse_lock nicht, da die capture-parameter const w�ren
				{
					pulse_lock.unlock();//setzt beim ersten aufruf der check_funktion den mutext zur�ck
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

