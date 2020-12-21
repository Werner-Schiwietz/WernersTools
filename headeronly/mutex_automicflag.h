#pragma once
//Copyright (c) 2020 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

#include <atomic>

					//usage 
					// mutex_atomicflag	locker {};
					// als WS::lock_guard
					// auto auto_release_lock =  WS::lock_guard<mutex_atomicflag>{locker};//der ctor ruft locker.lock() der dtor locker.unlock()
					// oder als std::lock_guard
					// auto auto_release_lock = std::lock_guard<mutex_atomicflag>{locker};//der ctor ruft locker.lock() der dtor locker.unlock()

namespace WS
{
	//WS::lock_guard wie std::lock_guard. zusätzlich is_locked() unlock() und WS::lock_guard kann per rvalue-ref verschoben werden
	template <class mutex_type> class lock_guard 
	{ 
	public:
		using mutex_t = mutex_type;

		explicit lock_guard(mutex_type& mutex) // construct and lock
			: mutex(&mutex) 
		{
			this->mutex->lock();
			this->locked = true;
		}
		lock_guard(lock_guard const &) = delete;
		lock_guard& operator=(lock_guard const &) = delete;
		lock_guard(lock_guard && r) {swap(r);}
		lock_guard& operator=(lock_guard && r) & {lock_guard{std::move(r)}.swap(*this);return *this;}
		void swap( lock_guard & r )
		{
			std::swap( this->locked, r.locked );
			std::swap( this->mutex, r.mutex );
		}

		~lock_guard() noexcept {unlock();}
		void unlock()
		{
			if(is_locked())
			{
				this->locked=false;
				//#pragma warning(suppress:26110)//unsinn warning C26110: Caller failing to hold lock 'this->mutex' before calling function 'std::_Mutex_base::unlock'.
				this->mutex->unlock();
				this->mutex = nullptr;
			}
		}
		bool is_locked() const { return locked && this->mutex;}
	private:
		lock_guard(){}
		bool			locked	{false};
		mutex_t *		mutex	{nullptr};
	};
	template <class mutex_type> [[nodiscard]]lock_guard<mutex_type> lock( mutex_type & mutex){ return lock_guard<mutex_type>{mutex};}


	class mutex_atomicflag
	{
		std::atomic_flag flag{};
	public:
		void lock(){ while (flag.test_and_set());}
		void unlock(){ flag.clear(); }
		bool islocked()//nicht atomic, nur zu kontrollzwecken
		{ 
			//std::atomic_flag::test() erst mit c++20
			bool retvalue = flag.test_and_set();
			if(retvalue==false)
				flag.clear(); 
			return retvalue;
		}
	};
}