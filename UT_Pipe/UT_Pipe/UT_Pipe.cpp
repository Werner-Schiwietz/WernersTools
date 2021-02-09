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

#include  <atlstr.h> //CString
#include  <future>
#include  <functional>
#include  <queue>
#include  <optional>
#include  <memory>

#include "..\..\headeronly\pipe.h"

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

            auto worker = []( my_data && data )
            {      //doing-something

                CString str;
                str.Format( _T("%6d working on data "), data.value );
                Logger::WriteMessage(str);

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(2us);
            };

            {
                auto pipe = WS::make_pipe<my_data>( worker );

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
                pipe.member->end_worker();//destructer macht nur detach, dadurch kann der qworker länger arbeiten als die eigentliche pipe lebt
            }
            Logger::WriteMessage("<- UT_pipe");
        }
        TEST_METHOD(UT_pipe_Logger)
        {
            Logger::WriteMessage("-> UT_pipe_Logger");
            using my_data = CString;

            auto worker = []( my_data && data )
            {      //doing-something

                CString str;
                str.Format( _T("worker -> '%s'"), data.GetString() );
                Logger::WriteMessage(str);
            };

            {
                auto pipe = WS::make_pipe<my_data>( worker );
                pipe.AddData( "Hallo");
                pipe.AddData( "Welt");
                pipe.AddData( "");
                pipe.AddData( "Ende und aus");

                //ohne end_worker wird zumeist nichts ausgegeben
                pipe.member->end_worker();//destructer macht nur detach, dadurch kann der qworker länger arbeiten als die eigentliche pipe lebt
            }
            Logger::WriteMessage("<- UT_pipe_Logger");
        }
    };
}



