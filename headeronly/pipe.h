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

//header 
//headeronly\pipe.h					diese datei
//headeronly\semaphore.h			
//headeronly\mutex_atomicflag.h	mutex fuer lock_guard nutzt atomic_flag
//<atomic>
//<mutex>
//<condition_variable>


#include  <future>
#include  <functional>
#include  <queue>
#include  <optional>
#include  <memory>

#include "semaphore.h"

namespace WS
{
    template<typename data_type>
    struct Pipe
    {
        using data_t = data_type;
        using mutex_t = std::mutex;
        struct Data
        {
            enum enumThreadState{running,process_and_terminate,terminate} threadstate{enumThreadState::running};
            WS::Semaphore       semaphore;
            std::mutex          mutex;
            std::thread         thread_working;
            std::queue<data_t>  data_pool;

            void terminate_detach_worker(enumThreadState newState=enumThreadState::terminate)//es werden ggf. nicht alle pending-datas verarbeitet
            {
                if( this->threadstate==enumThreadState::running )
                {
                    this->threadstate = newState;
                    this->semaphore.set_running(Semaphore::Notify::none);
                    this->thread_working.detach();
                }
            }
            void processdata_endworker(enumThreadState newState=enumThreadState::process_and_terminate)//wartet, bis alle daten verarbeitet sind und thread sich beendet hat
            {
                if( this->threadstate==enumThreadState::running )
                {
                    if( newState!=enumThreadState::process_and_terminate && newState!=enumThreadState::terminate )
                        throw std::invalid_argument( __FUNCTION__ " entweder enumThreadState::process_and_terminate oder enumThreadState::terminate erwartet");
                    this->threadstate = newState;
                    this->semaphore.set_running();//daten verarbeiten
                    this->thread_working.join();
                }
            }

            auto pending()
            {
                struct pending_ret_t : WS::compare_bool
                {
                    WS::lock_guard<mutex_t> locked;
                    size_t                  datasets = 0;

                    pending_ret_t( WS::lock_guard<mutex_t> && locked, size_t datasets ) : locked(std::move(locked)), datasets(datasets){}
                    auto count() const {return datasets;}
                    bool to_bool() const override{ return datasets>0; }
                }ret_value( WS::lock_guard(this->mutex), this->data_pool.size() );
                return ret_value;
            }

            std::optional<data_t> PopData()
            {
                auto looked = std::lock_guard(this->mutex);
                if(this->data_pool.size())//sind zu verarbeitende daten da
                {
                    auto data = std::move(this->data_pool.front());//daten holen
                    this->data_pool.pop();//daten aus pool entfernen
                    return data;
                }
                return {};
            }
            void AddData( data_t && data )
            {
                auto looked = std::lock_guard(this->mutex);
                data_pool.push( std::move(data) );
                semaphore.set_running();//ggf. worker laufen lassen
            }
        };
        using enumThreadState = typename Data::enumThreadState;
        using data_ptr_t = std::shared_ptr<Data>;
        data_ptr_t member{std::make_shared<Data>()};

        ~Pipe()
        {
            member->terminate_detach_worker();
            //member->end_worker();
        }
        Pipe( std::function<void(Pipe<data_t>::data_ptr_t,std::function<void(data_t&&)>) > fn, std::function<void(data_t&&)> worker )
        {
            this->member->semaphore.set_blocked();
            this->member->thread_working = std::thread( fn, this->member, worker );
        }
        Pipe( std::function<void(Pipe<data_t>::data_ptr_t)> fn )
        {
            this->member->semaphore.set_blocked();
            this->member->thread_working = std::thread( fn, std::ref(*this) );
        }

        auto pending(){return member->pending();}//VORSICHT ret_value hält lock_guard

        std::optional<data_t> PopData(){return this->member->PopData();}
        void AddData( data_t && data ){this->member->AddData( std::move(data) );}
    };

    #pragma region nicht threadsafe datenverarbeitung per pipe synchronisieren. siehe UT_Pipe.cpp-UT_using_not_threadsafe_service_from_threads
    template<typename working_data_type> struct pipe_data_processed
    {
        using working_data_t = working_data_type;
        using semaphore_t = WS::Semaphore;
        semaphore_t&    data_processed;
        bool            data_processed_isvalid{false};
        working_data_t  data;

        pipe_data_processed()=delete;
        pipe_data_processed(pipe_data_processed const&)=delete;
        pipe_data_processed(pipe_data_processed && r) noexcept : data_processed(r.data_processed), data(std::move(r.data)){std::swap(data_processed_isvalid,r.data_processed_isvalid);}
        pipe_data_processed& operator=(pipe_data_processed const&)=delete;
        pipe_data_processed& operator=(pipe_data_processed &&)=delete;
        pipe_data_processed(semaphore_t& data_processed, working_data_t data ) noexcept : data_processed(data_processed), data(data) 
        {
            this->data_processed.set_blocked();
            this->data_processed_isvalid = true;
        }
        virtual ~pipe_data_processed()
        {
            if( this->data_processed_isvalid )
            {
                this->data_processed.set_running();
            }
        }
    };
    template<typename working_data_type> struct service_not_threadsafe_handler
    {   
        std::function<void(working_data_type)> worker;
        service_not_threadsafe_handler(std::function<void(working_data_type)> worker) : worker(worker){}
        void operator()( pipe_data_processed<working_data_type> working_on)
        {
            worker(std::move(working_on.data));
        }
    };
    #pragma endregion

    template<typename data_t>void std_pipe_function_processor(typename Pipe<data_t>::data_ptr_t pipedata, std::function<void(data_t&&)> worker )
    {
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
                pipedata->semaphore.set_blocked_and_wait();
            }
        }
        //end-thread
    }
    template<typename data_t> auto make_pipe( std::function<void(data_t&&)> worker_function )
    {
        return Pipe<data_t>{std_pipe_function_processor<data_t>, worker_function};
    }
}

