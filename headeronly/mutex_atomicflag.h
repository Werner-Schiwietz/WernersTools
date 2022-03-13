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

//VORSICHT: ich konnte keine Vorteile gegenüber std::mutex erkennen. std::mutex ist sogar schneller.Es können auch sehr viele angelegt werden (im UT_RAII_Mutex 100'000'000)
					//usage 
					// mutex_atomicflag	locker {};
					// als WS::lock_guard, der kann per move weitergegeben werden
					// auto auto_release_lock =  WS::lock_guard<mutex_atomicflag>{locker};//der ctor ruft locker.lock() der dtor locker.unlock()
					// oder als std::lock_guard
					// auto auto_release_lock = std::lock_guard<mutex_atomicflag>{locker};//der ctor ruft locker.lock() der dtor locker.unlock()


#include <atomic>
#include <thread>

namespace WS
{
	//WS::lock_guard wie std::lock_guard. zusätzlich is_locked() unlock() und WS::lock_guard kann per rvalue-ref verschoben werden
	template <typename mutex_type> class lock_guard 
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
		bool unlock()
		{
			if(is_locked())
			{
				this->locked=false;
				//#pragma warning(suppress:26110)//unsinn warning C26110: Caller failing to hold lock 'this->mutex' before calling function 'std::_Mutex_base::unlock'.
				this->mutex->unlock();
				this->mutex = nullptr;
				return true;
			}
			return false;
		}
		bool is_locked() const { return locked && this->mutex;}
		operator bool() const{return is_locked();}
	private:
		template<typename mutex_type> friend class trylock_guard;
		lock_guard(){}
		bool			locked	{false};
		mutex_t *		mutex	{nullptr};
	};
	template <typename mutex_type> class trylock_guard : public lock_guard<mutex_type>
	{
	public:
		using lock_guard<mutex_type>::lock_guard;
		explicit trylock_guard(mutex_type& mutexin ) // construct and lock
		{
			this->mutex = &mutexin;
			this->locked = this->mutex->try_lock();
		}
	};
	template <class mutex_type> [[nodiscard]]lock_guard<mutex_type> lock( mutex_type & mutex){ return lock_guard<mutex_type>{mutex};}
	template <class mutex_type> [[nodiscard]]lock_guard<mutex_type> try_lock( mutex_type & mutex){ return trylock_guard<mutex_type>{mutex};}

	class mutex_atomicflag
	{
		std::atomic_flag	flag{};
		std::thread::id		locking_thread{};

	public:
		bool try_lock()
		{
			if(  flag.test_and_set()==true )
				return false;

			this->locking_thread = std::this_thread::get_id();
			return true;
		}
		void lock()
		{ 
			if( flag.test_and_set()==true )
			{
				if( this->locking_thread == std::this_thread::get_id() )
				{
					struct deadlock_exception : std::exception{using std::exception::exception;};
					throw deadlock_exception{__FUNCTION__ " deadlock detected"};
				}
				while( this->flag.test_and_set() ){std::this_thread::yield();}
			}
			//merken, welcher thread den lock hält
			this->locking_thread = std::this_thread::get_id();
		}
		void unlock(){ this->locking_thread=decltype(this->locking_thread){};this->flag.clear(); }
		#pragma region nicht atomic, nur zu kontrollzwecken
		bool islocked()//nicht atomic, nur zu kontrollzwecken
		{ 
			//std::atomic_flag::test() erst mit c++20
			bool retvalue = this->flag.test_and_set();
			if(retvalue==false)
				this->flag.clear(); 
			return retvalue;
		}
		#pragma endregion 
	};
	class recursive_mutex_atomicflag
	{
		std::atomic_flag	flag{};
		std::thread::id		locking_thread{};
		std::atomic_size_t	ref_count{};
		void addref()		{++this->ref_count;}
		bool releaseref()	
		{
			if( --this->ref_count == 0 )
			{
				this->locking_thread = decltype(this->locking_thread){};
				this->flag.clear();
				return true;
			}
			return false;
		}
	public:
		bool try_lock()
		{
			if( flag.test_and_set()==true )
			{
				if( this->locking_thread != std::this_thread::get_id() )
				{
					return false;
				}
			}

			addref();
			this->locking_thread = std::this_thread::get_id();
			return true;
		}
		void lock()
		{ 
			while( try_lock()==false )
				std::this_thread::yield();
		}
		void unlock()
		{ 
			if( releaseref() == false )
			{	
				//using namespace std::chrono_literals;
				//std::this_thread::sleep_for(20us);
			}
		}
		#pragma region nicht atomic, nur zu kontrollzwecken
		bool islocked()//nicht atomic, nur zu kontrollzwecken
		{ 
			//std::atomic_flag::test() erst mit c++20
			bool retvalue = this->flag.test_and_set();
			if(retvalue==false)
				this->flag.clear(); 
			return retvalue;
		}
		size_t refcount() const {return this->ref_count;}
		auto lockingthread() const {return this->locking_thread;}
		#pragma endregion 
	};
}