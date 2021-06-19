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

#include "pch.h"
#include "CppUnitTest.h"

#include <atlstr.h> //CString
#include <queue>
#include <optional>
#include <memory>
#include <chrono>

#include "..\..\headeronly\pipe.h"
#include "..\..\headeronly\char_helper.h"
#include "..\..\headeronly\mutex_atomicflag.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#ifndef LOG_STD
#define LOG_STD catch(...){}
#endif




namespace BasisUnitTests
{
    TEST_CLASS(Pipe_Tester)
    {
    public:
        TEST_METHOD(UT_pipe)
        {
            Logger::WriteMessage("-> UT_pipe");
            struct my_data{int value;};
            int counter{0};

            auto threadworker = []( my_data && data )
            {      //doing-something

                CString str;
                str.Format( _T("%6d working on data "), data.value );
                Logger::WriteMessage(str);

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(2us);
            };

            {
                auto pipe = WS::make_pipe<my_data>( threadworker );

                using namespace std::chrono_literals;
                auto waiting = 0us;
                //std::this_thread::sleep_for(2000ms);

                while( counter<100 )
                {
                    CString str;
                    str.Format( _T("%6d AddData"), ++counter );
                    Logger::WriteMessage(str);

                    if( (counter%10) == 0 )
                        ++waiting;
                    std::this_thread::sleep_for(waiting);

                    pipe.AddData(my_data{counter});
                    if( auto pending = pipe.pending() )
                    {
                        str.Format( _T("  %6d pending"), pending.count() );
                        Logger::WriteMessage(str);
                    }
                }
                pipe.member->processdata_endworker();//destructor macht nur soft-terminate und detach, dadurch kann der threadworker länger arbeiten als die eigentliche pipe lebt
            }
            Logger::WriteMessage("<- UT_pipe");
        }
        TEST_METHOD(UT_pipe_Logger)
        {
            Logger::WriteMessage("-> UT_pipe_Logger");
            using my_data = CString;

            auto threadworker = []( my_data && data )
            {      //doing-something

                CString str;
                str.Format( _T("threadworker -> '%s'"), data.GetString() );
                Logger::WriteMessage(str);
            };

            {
                auto pipe = WS::make_pipe<my_data>( threadworker );
                pipe.AddData( "Hallo");
                pipe.AddData( "Welt");
                pipe.AddData( "");
                pipe.AddData( "Ende und aus");

                //ohne processdata_endworker wird zumeist nichts ausgegeben
                pipe.member->processdata_endworker();//destructor macht nur soft-terminate und detach, dadurch kann der threadworker länger arbeiten als die eigentliche pipe lebt
            }
            Logger::WriteMessage("<- UT_pipe_Logger");
        }
        TEST_METHOD(UT_using_not_threadsafe_service_from_threads)
        {   //threadworker-thread ist nicht thread-safe, wird aber aus verschiedenen threads mit daten beschickt.
            //das muss synchronisiert werden

            #pragma region die daten, die dem Service übergeben werden
            struct working_data
            {
                int id;
                std::chrono::microseconds working_time;
                int & ergebnis;
                working_data( int & ergebnis, int id, std::chrono::microseconds working_time ) : ergebnis(ergebnis), id(id), working_time(working_time) {}
            };
            #pragma endregion 

            #pragma region der Service
            std::atomic_flag not_threadsafe_indikator{};//sollten mehr als ein thred laufen, wird exception geworfen. das kann aber nie passieren.
            auto service_not_threadsafe = [&](working_data working_on)
            {
                if( not_threadsafe_indikator.test_and_set() )
                    throw std::runtime_error{ __FUNCTION__ " soll nicht threadsafe sein"};
                std::this_thread::sleep_for( working_on.working_time );
                working_on.ergebnis = working_on.id%2;//irgendetwas prüfbares
                not_threadsafe_indikator.clear();
            };
            #pragma endregion 
            
            #pragma region die Pipe die die Daten zeitlich(FIFO) geordnet dem Service übergibt
            using pipe_data_processed_t = WS::pipe_data_processed<working_data>;
            WS::Pipe pipe = WS::make_pipe<pipe_data_processed_t>(WS::service_not_threadsafe_handler<working_data>{service_not_threadsafe});
            #pragma endregion 
    
            #pragma region code des threads der daten an den nicht threadsafen Service übergeben muss
            WS::Semaphore alle_gleichzeitig_anlaufen_lassen;alle_gleichzeitig_anlaufen_lassen.set_blocked();
            auto threadworker = [&](std::chrono::milliseconds working_time, int id)
            {
                int ergebnis = -1;
                char buf[20];
                Logger::WriteMessage( (std::string{"-> threadworker:"} + tostring(id,buf,10)).c_str() );
                WS::Semaphore daten_verarbeitet{};
                alle_gleichzeitig_anlaufen_lassen.wait();
                pipe.AddData( WS::pipe_data_processed(daten_verarbeitet,pipe_data_processed_t::working_data_t{ergebnis,id,working_time}) );//hier wird der service per pipe mit daten beschickt. Es ist sichergestellt, dass die Daten hinter einander(FIFO) abgearbeitet werden
                daten_verarbeitet.wait();
                Logger::WriteMessage((std::string{"<- threadworker:"} + tostring(id,buf,10) + " ergbnis wie erwartet:" + (ergebnis==id%2?"true":"false")).c_str() );
            };
            #pragma endregion 

            #pragma region hier werden die threads, die daten an den service schicken gestartet
            using namespace std::literals::chrono_literals;
            std::vector<std::future<void>> threads;
            int lfd=0;
            threads.push_back( std::async(threadworker,10ms,++lfd) );
            threads.push_back( std::async(threadworker,1ms,++lfd) );
            threads.push_back( std::async(threadworker,5ms,++lfd) );
            threads.push_back( std::async(threadworker,100ms,++lfd) );
            threads.push_back( std::async(threadworker,5ms,++lfd) );
            #pragma endregion 

            #pragma region alle threads zeitgleich anlaufen lassen und auf deren beendigung warten
            alle_gleichzeitig_anlaufen_lassen.set_running();
            for( auto & thread : threads )
                thread.wait();
            #pragma endregion 
        }
    };
}



