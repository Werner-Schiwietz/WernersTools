#pragma once


#include <memory>
#include <atomic>
#include <thread>
#include <map>
#include <mutex>

#ifndef ASSERT
#	define ASSERT(x) ((void)0)
#endif // !ASSERT

//WS::SyncAccess<Datentyp,access_type> by Werner Schiwietz 2017-10-22 
//
//threadsicherer zugriff auf datenobjekte per WS::SyncAccess<Datentyp,access_type>
//schreibender zugriff blockiert ggf. neue read/write zugriffe, 
//lesender zugriff sollte sich gegeseitig, auch in verschiedenen threads, nicht blockieren. 
//lesender zugriff blockiert je nach access_type schreibenden zugriff
//
//access_type ist  strict_access oder thread_write (beide getestet) oder ggf. eine andere implementierung von WS::ICritical
	//access_type unabhaengig reine lesezugriffe sollten sich gegeseitig nie blockieren
	//strict_access: vorsicht,  wirft exception, wenn konkurrierend Zugriffe eines threads zu deadlock fuehren wuerde
	//thread_write: im selben thread koennen mehrere WriteAccess und ReadAccess gleichzeitig existieren und jederzeit erzeugt werden 
//
//
//using
//siehe BasisUnitTests\UT_SyncAccess.cpp
//	WS::SyncAccess<std::vector<int,strict_access>> data;	//verwaltetes objekt anlegen. hier wird ein std::vector als verwaltetes objekt verwendet
//	auto writeaccess = data.WriteAccessWait();				//warten, bis schreibender zugriff erfolgen kann (bei strict_access exception, wenn man sich selbst blockiert)
//	for( int i=4; i<30; ++i )								
//		(*writeaccess).push_back( i );						//daten in den vector schreiben

//	auto writeaccess = data.WriteAccessTry();				//versuch schreibender zugriff zu erhalten
//	if( writeaccess )										//pruefen, ob zugriff erfolgen kann
//		for( int i=4; i<30; ++i )						
//			(*writeaccess).push_back( i );					//daten in den vector schreiben

//	auto access = data.ReadAccessWait();					//warten, bis nur lesender zugriff erfolgen kann
//	for( auto & v : (*access))								//etwas mit den daten machen
//		trace(v);											

//	auto access = data.ReadAccessTry();						//versuch lesend zuzugreifen
//	if( access )											//pruefen, ob zugriff erfolgen kann
//		for( auto i=(*access).size(); i --> 0;)				//etwas mit den daten machen
//			trace(v[i]);						

namespace WS
{
	#undef _INTERFACE_FUNCTION_
	#define _INTERFACE_FUNCTION_ = 0
	class ICritical
	{
	public:
		struct Locked
		{
			ICritical * critical	= nullptr;
			bool		locked		= false;

			Locked() = default;
			Locked(ICritical * critical, bool locked) : critical(critical),	locked(locked){}
			Locked(Locked const & r) = delete;
			Locked(Locked && r) noexcept { swap( r ); }

			Locked& operator=(Locked const & r) = delete;
			Locked& operator=(Locked && r) & noexcept
			{
				Locked { std::move( r ) }.swap(*this);
				return *this;
			}

			void swap(Locked & r)
			{
				std::swap(this->critical, r.critical);
				std::swap(this->locked, r.locked);
			}
			~Locked()
			{
				try
				{
					release();
				}
				catch(...)
				{
				}
			}
			/*explicit*/ operator bool () const
			{
				return locked;
			}
			bool operator ! () const { return !operator bool(); }
			bool operator==( bool value ) const { return operator bool()==value; }
			bool operator()() const //functor statt operator bool
			{
				return locked;
			}
			void release();
		};
	public:
		virtual ~ICritical() {};

		virtual ICritical::Locked	try_lock() _INTERFACE_FUNCTION_;
		virtual ICritical::Locked	lock() _INTERFACE_FUNCTION_;
		virtual bool				is_locked() _INTERFACE_FUNCTION_;
	protected:
		virtual void				release() _INTERFACE_FUNCTION_;
	};
	#undef _INTERFACE_FUNCTION_
	#define _INTERFACE_FUNCTION_ override
	inline void ICritical::Locked::release()
	{
		if(critical && locked)
		{
			critical->release();
			locked = false;
			critical = nullptr;
		}
	}


	class Critical_atomic_flag : public ICritical
	{
		mutable std::atomic_flag	flag = ATOMIC_FLAG_INIT;
		//um deadlocks erkennen zu koennen
		using threadid = decltype(std::this_thread::get_id()) ;
		mutable threadid					writethreadid;

	public:
		~Critical_atomic_flag() {}

		//solange ICritical::Locked lebt ist es locked
		virtual ICritical::Locked	try_lock() _INTERFACE_FUNCTION_
		{
			auto retvalue = flag.test_and_set();
			if( retvalue==false )
				writethreadid = std::this_thread::get_id();

			return ICritical::Locked( this, !retvalue );
		}
		virtual ICritical::Locked	lock() _INTERFACE_FUNCTION_
		{
			if( flag.test_and_set()==1 )
			{
				if( writethreadid == std::this_thread::get_id() )
					throw std::runtime_error("deadlock erkannt");

				while(flag.test_and_set()==1)
					std::this_thread::yield();
			}
			writethreadid = std::this_thread::get_id();
			return ICritical::Locked( this, true );
		}
		virtual bool				is_locked() _INTERFACE_FUNCTION_
		{
			auto locked = try_lock();
		
			return locked==false;
		}
	protected:
		virtual void				release() _INTERFACE_FUNCTION_
		{
			//ASSERT(flag._My_flag);
			ASSERT( writethreadid == std::this_thread::get_id() );
			writethreadid = threadid();
			flag.clear();
		}
	};
	class Critical_recursive_mutex : public ICritical
	{
		mutable std::recursive_mutex	mutex;
		mutable std::atomic<int>		lock_count = 0;

	public:
		~Critical_recursive_mutex() {}

		//solange ICritical::Locked lebt ist es locked
		virtual ICritical::Locked	try_lock() _INTERFACE_FUNCTION_
		{
			auto retvalue = this->mutex.try_lock();
			if( retvalue )
				++lock_count;

			return ICritical::Locked( this, retvalue );
		}
		virtual ICritical::Locked	lock() _INTERFACE_FUNCTION_
		{
			this->mutex.lock();
			++lock_count;
			return ICritical::Locked( this, true );
		}
		virtual bool				is_locked() _INTERFACE_FUNCTION_
		{
			return lock_count>0;
		}

	protected:
		virtual void				release() _INTERFACE_FUNCTION_
		{
			--lock_count;
			this->mutex.unlock();
		}
	};

	using strict_access = Critical_atomic_flag;
	using thread_write = Critical_recursive_mutex;


	template<typename Resource_t,bool pointer=std::is_pointer<typename Resource_t>::value> struct ReadAccess_ValueType;

	template<typename Resource_t> struct ReadAccess_ValueType<Resource_t,true>
	{
		using type = std::remove_pointer_t<Resource_t> const * const;
		//typedef std::add_const_t<std::add_pointer_t<std::add_const_t<std::remove_pointer_t<Resource_t>>>> type;
	};
	template<typename Resource_t> struct ReadAccess_ValueType<typename Resource_t,false>
	{
		using type = Resource_t const;
	};
	template<typename Resource_t> using ReadAccess_ValueType_t=typename ReadAccess_ValueType<Resource_t>::type;

	template <typename Resource_t, typename ICritical_type, typename ReadAccess_t=ReadAccess_ValueType_t<Resource_t>> class SyncAccess
	{
	public:
		typedef Resource_t		Resource_type;
		typedef ReadAccess_t	ReadAccess_type;

	private:
		template <typename ResourceAccess_t>struct Access;
		struct Data
		{
			Resource_t						resource;
			mutable ICritical_type			writing;
			mutable std::atomic<size_t>		reading = 0;
			using threadid = decltype(std::this_thread::get_id()) ;
			struct counter{std::atomic<size_t> value=0;};
			mutable std::map<threadid,counter>	readthreadids;


			Data() = default;
			Data(Resource_t && resource) : resource(std::move(resource)) {}
			void release(Access<Resource_t> const *)
			{//writing wird beendet
				ASSERT(reading);
				--reading;
				--readthreadids[std::this_thread::get_id()].value;
			}
			void release(Access<ReadAccess_type> const *)
			{//reading wird beendet
				ASSERT(reading);
				--reading;
				--readthreadids[std::this_thread::get_id()].value;
			}

			bool other_thread_reading()
			{
				for( auto & v : readthreadids )
					if( v.second.value > 0 && v.first != std::this_thread::get_id() )
						return true;
				return false;
			}

		};
		template <typename ResourceAccess_t>struct Access
		{
			typedef ResourceAccess_t ResourceAccess_type;
			std::shared_ptr<Data>	data;
			ICritical::Locked		locked;
			~Access()
			{
				try
				{
					release();
				}
				catch (...)
				{
				}
			}
			Access() = default;
			Access(Access const &) = delete;
			Access(Access && r)
			{
				Access temp;
				temp.swap(r);
				temp.swap(*this);
			}
			Access& operator=(Access const &) = delete;
			Access& operator=(Access && r)
			{
				Access temp;
				temp.swap(r);
				temp.swap(*this);
				return *this;
			}
			Access(std::shared_ptr<Data> data) : data(data) {}
			Access(std::shared_ptr<Data> data, ICritical::Locked && locked)
				: data(data)
				, locked(std::move(locked))
			{}

			void swap(Access & r)
			{
				std::swap(this->data, r.data);
				std::swap(this->locked, r.locked);
			}
			ResourceAccess_type* operator->()
			{
				auto ptr = data.get();
				if( ptr==nullptr )
					throw std::runtime_error( __FUNCTION__ ": Zugriff auf gesperrte Resource" );//ohne diesen throw wird UT_SyncAccess::read2_thread_write und die App beendet
				return &ptr->resource;
			}
			ResourceAccess_type & operator *()
			{
				auto ptr = data.get();
				if( ptr==nullptr )
					throw std::runtime_error( __FUNCTION__ ": Zugriff auf gesperrte Resource" );//ohne diesen throw wird UT_SyncAccess::read2_thread_write und die App beendet
				return ptr->resource;
			}
			void release()
			{
				if (data)
				{
					data->release(this);
					data.reset();//shared_pointer wird nicht mehr gebraucht
				}
			}
			operator bool()
			{
				return data.get() != nullptr;
			}
		};
		std::shared_ptr<Data> data;
	public:
		SyncAccess() : data(std::make_shared<Data>()) {}
		SyncAccess(Resource_t && resource) : data(std::make_shared<Data>(std::move(resource))) {}

		//ReadAccess[Try|Wait] solange das rueckgegebene objekt lebt darf auf die resource lesen(const) zugegriffen werden 
		auto ReadAccessTry() const
		{
			auto locked = this->data->writing.try_lock();

			if( locked )
			{
				++this->data->reading;
				++this->data->readthreadids[std::this_thread::get_id()].value;
				auto retvalue = Access<ReadAccess_type>(data);//locked wird freigegeben
				return retvalue;
			}
			return Access<ReadAccess_type>();
		}
		auto ReadAccessWait() const
		{
			auto locked = this->data->writing.lock();//temporaer schreibzugriff, um nebenläufigen statrtenden schreibzugriff zu verhindern
			//kein schreibender zugriff mehr

			++this->data->reading;
			++this->data->readthreadids[std::this_thread::get_id()].value;
			auto retvalue = Access<ReadAccess_type>(data);
			return retvalue;//schreiblock wird durch locked freigegeben
		}

		//WritedAccess[Try|Wait] solange das rueckgegebene objekt lebt darf auf die resource zugegriffen werden 
		//wartet nicht, sondern liefert ggf. ein leeres Access<Resource_type> zurueck. pruefen per Access<Resource_type>::operator bool(). er liefert false, wenn kein zugriff erfolgen kann
		template <typename access_type=ICritical_type>Access<Resource_type> WriteAccessTry();
		template <>Access<Resource_type> WriteAccessTry<strict_access>()
		{
			auto locked = this->data->writing.try_lock();

			if (locked() && this->data->reading==0)
			{
				++this->data->reading;
				++this->data->readthreadids[std::this_thread::get_id()].value;

				auto retvalue = Access<Resource_type>(data,std::move(locked));//lock bleibt erhalten, solange retvalue lebt
				return retvalue;
			}
			else
				ASSERT( this->data->readthreadids[std::this_thread::get_id()].value == 0 && "writelock nicht moeglich, this_thread haelt readlock");
			return Access<Resource_type>();
		}
		template <>Access<Resource_type> WriteAccessTry<thread_write>()
		{
			auto locked = this->data->writing.try_lock();

			if (locked() && this->data->other_thread_reading()==false)
			{
				++this->data->reading;
				++this->data->readthreadids[std::this_thread::get_id()].value;

				auto retvalue = Access<Resource_type>(data,std::move(locked));//lock bleibt erhalten, solange retvalue lebt
				return retvalue;
			}
			else
				ASSERT( this->data->readthreadids[std::this_thread::get_id()].value == 0 && "writelock nicht moeglich, other_thread haelt readlock");
			return Access<Resource_type>();
		}

		template <typename access_type=ICritical_type>Access<Resource_type> WriteAccessWait();
		//strict_access warte bis alle anderen lese und schreibzugriffe beendet wurden
		template <>Access<Resource_type> WriteAccessWait<strict_access>()
		{
			decltype(this->data->writing.lock()) locked;
			
			while(locked()==false)
			{
				locked = this->data->writing.lock();
				//kein schreibender zugriff mehr

				if( this->data->reading )
				{
					if( this->data->readthreadids[std::this_thread::get_id()].value )
						throw std::runtime_error("deadlock erkannt. this_thread haelt readaccess");

					locked.release();//nochmal freigeben, sonst deadlockgefahr
					std::this_thread::yield();//sleep_for( std::chrono::milliseconds(1) );
				}
			}

			++this->data->reading;
			++this->data->readthreadids[std::this_thread::get_id()].value;
			auto retvalue = Access<Resource_type>(data,std::move(locked));//locked wird in Access verschoben. er bleibt bestehen, bis Access released wird

			return retvalue;
		}
		//thread_write verhindert zugriff, solange anderer thread read/writeaccess hat
		template <>Access<Resource_type> WriteAccessWait<thread_write>()
		{
			decltype(this->data->writing.lock()) locked;
			
			while(locked()==false)
			{
				locked = this->data->writing.lock();
				//kein schreibender zugriff mehr

				if(this->data->other_thread_reading())
				{	//warten, bis alle lesenden zugriffe beendet sind
					locked.release();	//wenn ich aber schreiblock halte, kann es ggf. zu deadlock fuehren, wenn anderer thread readlock hat und noch einen zweiten readlock anfordert
										//ohne diese release bleibt UT::blockierterwrite_thread_read haengen
					std::this_thread::yield();//sleep_for( std::chrono::milliseconds(1) );
					//traceln( "WriteAccessWait locked, retry");
				}
			}

			++this->data->reading;
			++this->data->readthreadids[std::this_thread::get_id()].value;
			auto retvalue = Access<Resource_type>(data,std::move(locked));//locked wird in Access verschoben. er bleibt bestehen, bis Access released wird

			return retvalue;
		}

		//Reinitialize nur aufrufen, wenn ein thread nicht ordnugsgemäß beendet wurde und ggf. ein lock  verloren ging
		//unerwünschte nebenwirkungen sind nicht ausgeschlossen, also nach möglichkeit den aufruf dieser methode vermeiden
		//die nutzdaten bleiben erhalten, aber die locks werden zurückgesetzt, bestehende accesslocks auf eine leere resource umgeleitet
		//return value false = es war keine aktion nötig(gut so); true = die zugriffssteuerung musste reinitialisiert werden, mindestens ein lock wurde bisher nicht freigegeben
		bool Reinitialize()
		{	
			if( this->data.use_count() > 1 )
			{
				std::shared_ptr<Data> data2 = std::make_shared<Data>();
				auto locked = data2->writing.lock();
				std::swap( data2, this->data );
				std::swap( data2->resource, this->data->resource );
				return true;
			}
			return false;
		}

	#ifdef _DEBUG
		size_t reading()
		{
			return this->data->reading;
		}
		bool writing()
		{
			return this->data->writing.is_locked();
		}
	#endif
	};
}//namespace WS
