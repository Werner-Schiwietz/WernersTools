#pragma once
///Copyright (c) 2021 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

/// Funktionsweise
///
/// 
/// threads schieben die zu verarbeitenden daten in die pipe.
/// der thread der pipe verarbeitet die daten der reihe(oder nach priorität) in dem die daten der working-function übergeben wird.
/// im normalfall asyncron, aber es kann auch auf die verarbeitung gewartet werden, wenn z.b. der return-value wichtig ist


/// usage am einfachsten
/// auto pipe = WS::make_pipe<working_data_type>( std::function<void(working_data_type&&) );
/// pipe.AddData( working_data_type{} );
/// 
/// oder, wenn auf die verarbeitung gewartet werden muss
/// 
/// auto pipe = WS::make_syncro_pipe<working_data_type>( std::function<void(working_data_type&&) );
/// WS::process_data_and_wait( pipe, working_data_type{} );

///header 
///headeronly\pipe.h				diese datei
///headeronly\semaphore.h			
///headeronly\mutex_atomicflag.h	mutex fuer lock_guard nutzt atomic_flag
///headeronly\dependent_false.h		false für static_assert wenn twoPhase-compiling , sonst permissive- compile-error
///<atomic>
///<mutex>
///<condition_variable>

#include  <future>
#include  <functional>
#include  <queue>
#include  <optional>
#include  <memory>

#include "semaphore.h"
#include "dependent_false.h"

namespace WS
{
	template<typename data_type> struct fifo //nicht sychronisierter fifo-buffer
	{
		using data_t = data_type;
		template<typename ... not_used_ts>void AddData( data_t && data, not_used_ts && ... )
		{
			this->data.push( std::move(data) );
			static_assert(sizeof...(not_used_ts)==0);
		}
		std::optional<data_t> PopData()
		{
			if(this->size())//sind zu verarbeitende daten da
			{
				auto value = std::move(this->data.front());//daten holen
				this->data.pop();//daten aus pool entfernen
				return value;
			}
			return {};
		}
		auto size() const{ return data.size();}
	private:
		std::queue<data_type> data;
	};
	template<typename data_type,typename prio_type=size_t> struct fifo_with_prio //nicht sychronisierter, erweiterter fifo-buffer, kleinere prio haben höhere priorität
	{
		using prio_t = prio_type;
		using data_t = data_type;
		struct mydata_t
		{
			prio_t prio;
			data_t data;

			////hier geht es nur um sortierung, nicht um die inhalte
			//bool operator<(mydata_t const & r)const{return this->prio<r.prio;}
			//bool operator==(mydata_t const & r)const{return this->prio==r.prio;}
			//bool operator!=(mydata_t const & r)const{return this->prio!=r.prio;}
		};

		template<typename ... not_used_ts>void AddData( data_t && data, prio_t prio=prio_t{},not_used_ts && ... )
		{
			static_assert(sizeof...(not_used_ts)==0);
			auto iter = this->data.begin();
			//for(;iter!=this->data.end() && iter->prio <= prio; ++iter )//TODO kann man schneller machen
			for(;iter!=this->data.end() && !(prio<iter->prio); ++iter )//TODO kann man schneller machen
			{}
			this->data.insert(iter,mydata_t{prio,std::move(data)});
		}
		std::optional<data_t> PopData()
		{
			auto iter = this->data.begin();
			if(iter!=this->data.end())//sind zu verarbeitende daten da
			{
				auto value = std::move((*iter).data);//daten holen
				this->data.erase(iter);//daten aus pool entfernen
				for(auto & v : this->data )
				{
					if(v.prio)
						--v.prio;//prio der alten daten erhöhen
				}
				return value;
			}
			return {};
		}
		auto size() const{ return data.size();}
	private:
		std::deque<mydata_t> data;
	};

	template<typename data_type,typename datapool_type=WS::fifo<data_type>>
	struct Pipe
	{
		using data_t        = data_type;
		using datapool_t    = datapool_type;

		template<typename pipedata_type> struct Data 
		{
			using pipedata_t = pipedata_type;
			using mutex_t = std::mutex;
			enum enumThreadState{running,process_and_terminate,terminate,blocked};
			std::atomic<enumThreadState> threadstate{enumThreadState::running};
			WS::Semaphore       semaphore;//ohne daten wird verarbeitungsthread schlafen gelegt
			mutex_t		        mutex;
			std::thread         thread_working;
			pipedata_type		data_pool;

			void terminate_detach_worker(enumThreadState newState=enumThreadState::terminate)//es werden ggf. nicht alle pending-datas verarbeitet
			{
				if( this->threadstate==enumThreadState::running || this->threadstate==enumThreadState::blocked )
				{
					this->threadstate = newState;
					this->semaphore.set_running(Semaphore::Notify::none);
					this->thread_working.detach();
				}
			}
			void processdata_endworker(enumThreadState newState=enumThreadState::process_and_terminate)//wartet, bis alle daten verarbeitet sind und thread sich beendet hat
			{
				if( this->threadstate==enumThreadState::running || this->threadstate==enumThreadState::blocked )
				{
					if( newState!=enumThreadState::process_and_terminate && newState!=enumThreadState::terminate )
						throw std::invalid_argument( __FUNCTION__ " entweder enumThreadState::process_and_terminate oder enumThreadState::terminate erwartet");
					this->threadstate = newState;
					this->semaphore.set_running();//daten verarbeiten
					this->thread_working.join();
				}
			}

			auto pending(WS::lock_guard<mutex_t> locked)
			{
				struct pending_ret_t : WS::compare_bool
				{
					WS::lock_guard<mutex_t> locked;
					size_t                  datasets = 0;

					pending_ret_t( WS::lock_guard<mutex_t> && locked, size_t datasets ) : locked(std::move(locked)), datasets(datasets){}
					auto count() const {return datasets;}
					bool to_bool() const override{ return datasets>0; }
				}ret_value( std::move(locked), this->data_pool.size() );
				return ret_value;
			}
			auto pending()
			{
				return pending(WS::lock_guard(this->mutex));
			}

			std::optional<data_t> PopData()
			{
				auto looked = WS::lock_guard(this->mutex);
				auto data = this->data_pool.PopData();
				auto moredata = pending(std::move(looked));//moredata hält den look
				if( moredata == false )
					this->semaphore.set_blocked();
				return data;
			}
			template<typename ... addional_ts> void AddData( data_t && data, addional_ts && ... params )
			{
				auto looked = std::lock_guard(this->mutex);
				data_pool.AddData( std::move(data),std::forward<addional_ts>(params)...);
				if(this->threadstate==enumThreadState::running)
					this->semaphore.set_running();//ggf. worker laufen lassen
			}
		};
		using data_syncro_t = Data<datapool_t>;

		using enumThreadState = typename data_syncro_t::enumThreadState;
		using data_ptr_t = std::shared_ptr<data_syncro_t>;
		data_ptr_t member{std::make_shared<data_syncro_t>()};

		~Pipe()
		{
			member->terminate_detach_worker();
			//member->end_worker();
		}
		Pipe(Pipe const& ) = delete;
		Pipe( std::function<void(typename Pipe<data_t,datapool_t>::data_ptr_t,std::function<void(data_t&&)>)> fn, std::function<void(data_t&&)> worker )
		{
			this->member->semaphore.set_blocked();
			this->member->thread_working = std::thread( fn, this->member, worker );
		}
		Pipe( std::function<void(typename Pipe<data_t,datapool_t>::data_ptr_t)> fn )
		{
			static_assert(dependent_false<data_type>,"untestet function");//maybe it works without this line
			this->member->semaphore.set_blocked();
			this->member->thread_working = std::thread( fn, std::ref(*this) );
		}


		template<typename ... addional_ts> 
		void AddData( data_t && data, addional_ts && ... params ){this->member->AddData( std::move(data),std::forward<addional_ts>(params)... );}//params ggf wenn nicht einfacher fifo benutzt wird zusätzliche daten
		std::optional<data_t> PopData(){return this->member->PopData();}
		
	#pragma region funktionen zum testen
		auto pending(){return member->pending();}//liefert ob, und wieviele daten zur verarbeitung bereitstehen. VORSICHT ret_value hält lock_guard
		void set_processing_blocked()//verarbeitung auch mit daten blockieren
		{
			this->member->threadstate = enumThreadState::blocked;
			this->member->semaphore.set_blocked();
		}
		void set_processing()//verarbeitung mit daten ermöglichen
		{
			this->member->threadstate = enumThreadState::running;
			if( auto erg=this->member->pending() )
				this->member->semaphore.set_running();
		}
	#pragma endregion

	};
	template<typename pipe_t>void std_pipe_function_processor(typename pipe_t::data_ptr_t pipedata, std::function<void(typename pipe_t::data_t&&)> worker )
	{
		using data_t = typename pipe_t::data_t;
		while(pipedata->threadstate!=Pipe<data_t>::enumThreadState::terminate)
		{
			if( auto data = pipedata->PopData() )
			{
				//daten verarbeiten
				worker(std::move(data.value()));
			}
			else if( pipedata->threadstate==Pipe<data_t>::enumThreadState::process_and_terminate )
			{
				//fertig
				break;
			}
			else
			{
				//keine daten mehr da. schlafen legen, mit neuen daten werden wir geweckt
				//pipedata->semaphore.set_blocked_and_wait();
				pipedata->semaphore.wait();
				//if(auto erg = pipedata->pending())
				//{
				//	erg.count();
				//}
			}
		}
		//end-thread
	}
	template<typename data_t,typename datapool_t=WS::fifo<data_t>> 
	auto make_pipe( std::function<void(data_t&&)> worker_function )
	{
		using pipe_t = Pipe<data_t,datapool_t>;
		return pipe_t{std_pipe_function_processor<pipe_t>, worker_function};
	}


	#pragma region nicht threadsafe datenverarbeitung per pipe synchronisieren. siehe UT_Pipe.cpp-UT_using_not_threadsafe_service_from_threads
	template<typename working_data_type> struct pipe_data_process
	{
		using working_data_t = working_data_type;
		using semaphore_t = WS::Semaphore;
		semaphore_t&    data_processed;
		bool            data_processed_isvalid{false};
		working_data_t  data;

		pipe_data_process()=delete;
		pipe_data_process(pipe_data_process const&)=delete;
		pipe_data_process(pipe_data_process && r) noexcept : data_processed(r.data_processed), data(std::move(r.data)){std::swap(data_processed_isvalid,r.data_processed_isvalid);}
		pipe_data_process& operator=(pipe_data_process const&)=delete;
		pipe_data_process& operator=(pipe_data_process &&)=delete;
		pipe_data_process(semaphore_t& data_processed, working_data_t data ) noexcept : data_processed(data_processed), data(std::move(data)) 
		{
			this->data_processed.set_blocked();
			this->data_processed_isvalid = true;
		}
		virtual ~pipe_data_process()
		{
			if( this->data_processed_isvalid )
			{
				this->data_processed.set_running();
			}
		}
	};
	template<typename working_data_type> struct pipe_data_processed_handler
	{   
		std::function<void(working_data_type)> worker;
		pipe_data_processed_handler(std::function<void(working_data_type)> worker) : worker(worker){}
		void operator()( pipe_data_process<working_data_type> working_on)
		{
			worker(std::move(working_on.data));
		}
	};

	template<typename data_t,typename datapool_t=WS::fifo<pipe_data_process<data_t>>> 
	auto make_syncro_pipe( std::function<void(data_t&&)> worker_function )
	{
		return WS::make_pipe<pipe_data_process<data_t>,datapool_t>(WS::pipe_data_processed_handler<data_t>{worker_function});
	}
	template<typename working_data_type> void process_data_and_wait( Pipe<pipe_data_process<working_data_type>> & pipe, working_data_type data )// die pipe wurde mit make_syncro_pipe erzeugt
	{
		auto daten_verarbeitet = WS::Semaphore {};  //hier die semaphore für AddData
		pipe.AddData( pipe_data_process<working_data_type>{daten_verarbeitet,std::move(data)} );//hier wird der service per pipe mit daten beschickt. Es ist sichergestellt, dass die Daten hinter einander(FIFO) abgearbeitet werden
		(daten_verarbeitet).wait();//warten, bis genau dieser datensatz im parallelen thread verarbeitet wurde. im ergebnis steht das ergebins der datenverarbeitung
	}
	#pragma endregion
}//namespace WS

