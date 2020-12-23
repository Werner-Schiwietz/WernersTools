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
//!!!VORSICHT die meisten WS::Semaphore-Methoden liefern auch den lock_guard auf den state_mutex zurück. Also den Rückgabewert nicht merken, oder ggf. unlock aufrufen
//	wichtige funktionen
//		WS::Semaphore::signaled(notify=Notify::all) or set_running. enum Notify als none, one, all gibt an, welche Notify-Funktion auf die condition_variable nach signaled gerufen wird
//		WS::Semaphore::blocked() or set_running
//		WS::Semaphore::wait() //if blocked
//oder als taktgeber
//		WS::Semaphore::pulse()//jeder wartende thread wird gestartet. nächste wait wartet garantiert, also genau ein durchlauf von wait zu wait in jedem thread. pulse kommt zum aufrufer zurück, wenn alle threads laufen, nicht wenn sie fertig sind
//		WS::Semaphore::wait() //if blocked
//Fragen, für UnitTest. Ansonsten wird man die nicht brauchen.
//		WS::Semaphore::is_signaled() or WS::Semaphore() or (bool)WS::Semaphore
//		WS::Semaphore::is_blocked()

// beispiel
//void einfaches_daten_synchronisations_beispiel ()//ein datenlieferant und ein worker
//{
//	WS::Semaphore		sema;//blocked
//	bool				ready{false};
//	std::atomic<size_t> counter{0};
//
//
//	auto worker = std::thread( [&]()
//	{	//thread zählt von 0 - 100 und legt sich dann schlafen
//		for(;ready==false;)//wenn true beendet sich der thread
//			if( ++counter == 100 )
//				sema.set_blocked_and_wait();//wichtig statt blocked();wait(); was zur race-condition führen kann. Es ginge auch wait( blocked() );
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
//pulse beispiel in UT_semaphore.cpp

#include "mutex_automicflag.h"
#include "return_type.h"

#include <atomic>
#include <mutex>
#include <condition_variable>

namespace WS
{
	class Semaphore
	{
	public:
		using count_t				= size_t;
	private:
		using mutex_t				= std::mutex;
		using condition_variable_t	= std::condition_variable;
		enum class Notify{none,one,all};

		mutex_t						wait_mutex{};
		condition_variable_t		cv{};
		std::atomic_bool			running{false};
		std::atomic<count_t>		waiting{0};//anzahl wartender threads, für pulse nötig

		mutable mutex_t				state_mutex{};//dient hauptsächlich der vermeidung von race-condition

	public:
		#pragma region return_types
		struct count_return_t
		{
			count_t				counter;
			lock_guard<mutex_t> locked;

			operator lock_guard<mutex_t>&&() &  {return std::move(locked);}
			operator lock_guard<mutex_t>  () && {return std::move(locked);}
			operator count_t() const {return counter;}
		};
		struct bool_lock : compare_bool
		{
			bool_lock(bool value,lock_guard<mutex_t> locked) : value(value),locked(std::move(locked)){} 
			bool				value;
			lock_guard<mutex_t>	locked;

			operator lock_guard<mutex_t>&&() &  {return std::move(locked);}
			operator lock_guard<mutex_t>  () && {return std::move(locked);}
			bool to_bool() const override{return value;}
		};
		#pragma endregion

		#pragma region status der semaphore running oder blocked
		auto is_signaled(lock_guard<mutex_t>locked) const	{return bool_lock{_is_signaled(),std::move(locked)};}// !! liefert auch den lock_guard zurück !!
		auto is_signaled() const							{return is_signaled(lock(this->state_mutex));}// !! liefert auch den lock_guard zurück !!
		auto operator()()  const							{return is_signaled();}// !! liefert auch den lock_guard zurück !!
		operator bool ()   const							{return is_signaled();}
		auto is_blocked(lock_guard<mutex_t>locked) const	{return bool_lock{_is_blocked(),std::move(locked)};}// !! liefert auch den lock_guard zurück !!
		auto is_blocked() const								{return is_signaled(lock(this->state_mutex));}// !! liefert auch den lock_guard zurück !!
		#pragma endregion

		#pragma region blocking methoden
		auto reset(lock_guard<mutex_t>locked)				{_set_blocked();return std::move(locked);}
		auto reset()										{return reset(lock(this->state_mutex));}
		auto blocked(lock_guard<mutex_t>locked)				{return reset(std::move(locked));}
		auto blocked()										{return reset();}
		#pragma endregion 
		void set_blocked_and_wait()//statt blocked und wait unabhängig mit gefahr einer race condition
		{
			_wait( blocked() );
		}
		void wait( lock_guard<mutex_t>locked )
		{
			_wait( std::move(locked) );
		}
		void wait( )//es wird ggf gewartet, bis die Semaphore im running-mode ist, per set_running() oder pulse() egal
		{
			_wait( lock(this->state_mutex) );
		}

		#pragma region running methoden
		count_return_t	signaled(Notify eNotify,lock_guard<mutex_t>locked)	{return {_set_signaled_and_wait_till_all_running(eNotify),std::move(locked)};}//liefert die anzahl der wartenden threads UND DEN LOCK. also vorsicht, den return-wert nicht aufheben
		auto			signaled(Notify eNotify)							{return signaled(eNotify,lock(this->state_mutex));}
		auto			signaled()											{return signaled(Notify::all,lock(this->state_mutex));}
		auto			set_running(Notify eNotify)							{return signaled(eNotify);}
		auto			set_running()										{return signaled(Notify::all);}

		count_return_t pulse(lock_guard<mutex_t>locked) //jeder wartender thread soll gestartet werden, der nächste wait im thread blockiert wieder. also definiert einmalige ausführung.
		{
			count_t	started_thread_count{0};
			if( _is_blocked() )
			{
				started_thread_count = _set_signaled_and_wait_till_all_running(Notify::all);
				_set_blocked();
			}
			return {started_thread_count,std::move(locked)};
		}
		auto pulse(){return pulse(lock(this->state_mutex));}//blockiert neue wait-aufrufe und damit das hochzählen von waiting}
		#pragma endregion 

		count_return_t	Waiting(lock_guard<mutex_t>locked){return {waiting,std::move(locked)};}//liefert anzahl der wartenden threads
		auto			Waiting(){ return Waiting(lock(this->state_mutex)); }

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
		bool _is_signaled() const {return running;}
		bool _is_blocked()  const {return !running;}
		void _set_signaled(Notify eNotify){running=true;notify(eNotify);}
		void _set_blocked(){running=false;}
		void _wait( lock_guard<decltype(state_mutex)> pulse_lock)
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
		count_t _set_signaled_and_wait_till_all_running(Notify const eNotify)
		{
			if( _is_signaled()==false )
			{
				count_t const threads_waiting = this->waiting;//weil notify_all manchmal nicht die gewünschte wirkung erzielt. die anzahl wartenden threads vor signaled merken und weitergeben
				_set_signaled(eNotify);
				_wait_till_all_running( threads_waiting, eNotify);
				return threads_waiting;
			}
			return 0;
		}
	};
}

