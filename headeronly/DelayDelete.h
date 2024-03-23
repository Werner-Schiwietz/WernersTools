#pragma once
///Copyright (c) 2024 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

#include <memory>
#include <map>
#include <chrono>
#include <any>
#include <mutex>

namespace WP
{
    /// <summary>
    /// WP::DelayDelete zerst�rt Objekte mit Zeitverz�gerung, sp�testen beim Beenden des Programms,
    /// fr�hesten nach Ablauf der duration-zeit
    /// 
    /// nutzt std:any, ben�tig mindestens c++17
    /// </summary>
    /// <typeparam name="duration_t">std::chrono::??? duration-type z.b. seconds, microsoconds, ...</typeparam>
    /// <typeparam name="default_duration">die zeitspanne in duration_t</typeparam>
    template<int default_duration,typename duration_t=std::chrono::seconds> class DelayDelete
    {
        using value_t = std::pair<decltype(std::chrono::high_resolution_clock::now()),std::any>;
        std::multimap<decltype(std::chrono::high_resolution_clock::now()),std::any> todelete;
        std::mutex mutex{};
    public:
        DelayDelete(){};
        DelayDelete(DelayDelete const &)=delete;
        DelayDelete(DelayDelete &&)=delete;
        DelayDelete& operator=(DelayDelete const &)=delete;
        DelayDelete& operator=(DelayDelete &&)=delete;

        /// <summary>
        /// Clean() macht die deletes auf objekte deren zeit abgelaufen ist. liefert, wieviele objekt noch in der queue sind
        /// </summary>
        /// <returns>Anzahl der Objekte, die noch zerst�rt werden m�ssen</returns>
        size_t Clean()
        {
            auto locked = std::lock_guard{this->mutex};
            if( auto iter=this->todelete.begin(); iter!=todelete.end())
            {
                auto now = std::chrono::high_resolution_clock::now();
                while( iter!=this->todelete.end() )
                {
                    auto &[validTill,pointer] = *iter;
                    if( validTill < now )
                    {
                        iter = this->todelete.erase(iter);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            return todelete.size();
        }

        /// <summary>
        /// CleanAll() zerst�rt alle Objekte, unabh�ngig ob Zeit abgelaufen ist
        /// </summary>
        /// <returns>anzahl der objekte die zerst�rt wurden</returns>
        size_t CleanAll()
        {
            auto locked = std::lock_guard{this->mutex};
            auto ret_value = todelete.size();
            todelete.clear();
            return ret_value;
        }

        /// <summary>
        /// Add(T*,duration) f�gt Objekt der Verwaltung hinzu 
        /// </summary>
        /// <typeparam name="T"> Datentyp der zerst�rt werden soll</typeparam>
        /// <typeparam name="duration_t"> std::chrono:: duration-types</typeparam>
        /// <param name="pToDelete">pointer auf das objekt</param>
        /// <param name="duration">die zeitspanne wie lange das objekt mindestens weiter lebt</param>
        template<typename T, typename duration_t> void Add( T* pToDelete, duration_t duration )
        {
            Clean();
            auto locked = std::lock_guard{this->mutex};
            todelete.insert( value_t{std::chrono::high_resolution_clock::now() + duration, std::any(std::shared_ptr<T>{pToDelete})} );//std::any braucht copyable und unique_ptr ist nicht copyable
        }

        /// <summary>
        /// Add(T*) f�gt Objekt der Verwaltung mit default-duration hinzu
        /// </summary>
        /// <typeparam name="T"> Datentyp der zerst�rt werden soll</typeparam>
        /// <param name="pToDelete">pointer auf das objekt</param>
        template<typename T> void Add( T* pToDelete )
        {
            Add( pToDelete, duration_t{default_duration} );
        }
    };
}
