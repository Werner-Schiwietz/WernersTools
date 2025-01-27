#pragma once
///Copyright (c) 2025 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

#include <map>
#include <vector>
#include <deque>
#include <mutex>
#include "pipe.h"

namespace WS
{
    //für singlethreaded nichts tuender mutex
    struct null_mutex
    {
        void lock(){ }
        void unlock(){ }
    };

    /// <summary>
    /// liefert value_t komplett als key
    /// </summary>
    /// <typeparam name="value_t"></typeparam>
    template<typename value_t> struct GetValueAsKey
    {
        value_t const & operator()( value_t const & v ){return v;}
    };
}
namespace WS
{
    /// <summary>
    /// container-wrapper der einen unsortierenden container verwaltet und per std::map items schnell findet
    /// container muss wie std::vector die funktionen
    ///                   size
    ///                   clear
    ///                   push_back
    ///                   operator[]
    ///                   at
    ///                   erase(iterator)
    /// und traits
    ///                   value_type
    ///                   size_type
    /// bereitstellen
    /// </summary>
    /// <typeparam name="container_type">std::vector std::deque</typeparam>
    /// <typeparam name="pred_getKeyType">funktion(sobjekt) welches aus value_t einen key liefert</typeparam>
    /// <typeparam name="mutex_type"></typeparam>
    template<typename container_type,typename pred_getKeyType=WS::GetValueAsKey<typename container_type::value_type>,typename mutex_type=std::mutex>
    class ContainerMitSuche
    {
    public:
        using container_t = container_type;
        using value_t = typename container_type::value_type;
        using index_t = typename container_type::size_type;
        using mutex_t = typename mutex_type;
        using findkey_t = typename std::remove_cvref_t<decltype(pred_getKeyType{}(std::declval<value_t>()))>;

        virtual ~ContainerMitSuche(){}
        ContainerMitSuche(){}
        ContainerMitSuche( pred_getKeyType fnGetKeyType) : GetKey(fnGetKeyType){}//ctor mit key-funktion, z.b. eine lambda

    public:
        bool valid_index( index_t index ) const { return index>=0 && index<this->daten.size(); }

        void clear()
        {
            auto locked=std::lock_guard(this->mutex);
            this->daten.clear();
            this->map.clear();
        }

        index_t size() const { return this->daten.size(); }
        index_t push_back( value_t && v )
        {
            auto locked=std::lock_guard(this->mutex);
            auto index = this->daten.size();
            this->map[GetKey(v)].push_back(index);
            this->daten.push_back(std::move(v));
            return index;
        }
        index_t push_back( value_t const & v )
        {
            return push_back(value_t{v});
        }
        index_t push_back_unique( value_t && v )
        {
            auto locked=std::lock_guard(this->mutex);
            if( _find(GetKey(v)) )
                return index_t(-1);
            auto index = this->daten.size();
            this->map[GetKey(v)].push_back(index);
            this->daten.push_back(std::move(v));
            return index;
        }
        index_t push_back_unique( value_t const & v )
        {
            return push_back_unique(value_t{v});
        }

        decltype(auto) operator[](index_t index)
        {
            auto locked=std::lock_guard(this->mutex);
            ASSERT(valid_index(index));
            return this->daten[index];//?gefährlich ref auf array ohne lock zurückliefern? ggf decltype(auto) zu auto ändern
        }
        decltype(auto) operator[](index_t index) const
        {
            auto locked=std::lock_guard(this->mutex);
            ASSERT(valid_index(index));
            return this->daten[index];//?gefährlich ref auf array ohne lock zurückliefern? ggf decltype(auto) zu auto ändern
        }
        decltype(auto) at(index_t index)
        {
            auto locked=std::lock_guard(this->mutex);
            ASSERT(valid_index(index));
            return this->daten.at(index);//?gefährlich ref auf array ohne lock zurückliefern? ggf decltype(auto) zu auto ändern
        }
        decltype(auto) at(index_t index) const
        {
            auto locked=std::lock_guard(this->mutex);
            ASSERT(valid_index(index));
            return this->daten.at(index);//?gefährlich ref auf array ohne lock zurückliefern? ggf decltype(auto) zu auto ändern
        }

        void erase_at( index_t index)
        {
            auto locked=std::lock_guard(this->mutex);
            ASSERT(valid_index(index));
            this->daten.erase(this->daten.begin()+index);
            _newmap();
        }

        //liefert einen pointer auf std:vector<index_t> mit allen einträgen, die die key-bedingung erfüllen, oder nullptr, wenn es keine ergebnismenge gibt
        auto findall( findkey_t const & key ) const
        {
            auto locked=std::lock_guard(this->mutex);
            return _findall(key);
        }
        //wenn der gelieferte wert ein Valid_index ist, ist der wert mit dem key genau einmal enthalten. in eValid steht ggf warum es kein index geleifert werden kann
        struct find_ret_t
        {
            enum valid_t {valid,key_not_found,key_ambiguous} eValid;
            index_t index;

            operator index_t() const{ return index; }
            explicit operator bool() const { return eValid==valid_t::valid; }   
        }
        find( findkey_t const & key ) const
        {
            auto locked=std::lock_guard(this->mutex);
            return _find(key);
        }

    private:
        //findall ohne eigenen lock
        auto _findall( findkey_t const & key ) const
        {
            auto iter = this->map.find(key);
            if( iter != this->map.end() )
            {
                return &iter->second;
            }
            return (decltype(&iter->second))nullptr;
        }
        find_ret_t _find( findkey_t const & key ) const
        {
            if( auto pAllIndex = _findall(key) )
                if( pAllIndex->size()==1)
                {
                    return find_ret_t(find_ret_t::valid,(*pAllIndex)[0]);
                }
                else
                    return {find_ret_t::key_ambiguous,(*pAllIndex)[0]};
            return {find_ret_t::key_not_found,index_t(-1)};
        }
        void _newmap()
        {
            //auto locked=std::lock_guard(this->mutex);
            this->map.clear();
            index_t index = 0;
            for(auto const & datum : this->daten )
            {
                this->map[this->GetKey(datum)].push_back(index);
                ++index;
            }
        }

        pred_getKeyType GetKey{};//funktion(objekt), liefert den key des value
        container_t daten;
        std::map<findkey_t,std::vector<index_t>> map;
        mutable mutex_t mutex{};
    };

    /// <summary>
    /// wie ContainerMitSuche nur push_back wird asynchron im thread ausgeführt
    /// </summary>
    /// <typeparam name="pred_getKeyType"></typeparam>
    /// <typeparam name="mutex_type"></typeparam>
    /// <typeparam name="container_type"></typeparam>
    template<typename container_type,typename pred_getKeyType=WS::GetValueAsKey<typename container_type::value_type>,typename mutex_type=std::mutex>
    class ContainerMitSucheAsync : public ContainerMitSuche<typename container_type,typename pred_getKeyType,typename mutex_type>
    {
    public:
        using base_t = ContainerMitSuche<typename container_type,typename pred_getKeyType,typename mutex_type>;
        using container_t = typename base_t::container_t;
        using value_t = typename base_t::value_t;
        using index_t = typename base_t::index_t;
        using mutex_t = typename base_t::mutex_t;
        using findkey_t = typename base_t::findkey_t;
        ContainerMitSucheAsync() 
            : base_t()
            , pipe(make_pipe<value_t>( [&](value_t && v){base_t::push_back(std::move(v));})){}
        ContainerMitSucheAsync( pred_getKeyType fnGetKeyType) 
            : base_t(fnGetKeyType) 
            , pipe(make_pipe<value_t>( [&](value_t && v){base_t::push_back(std::move(v));})){}//ctor mit key-funktion, z.b. eine lambda

        void push_back( value_t && v )
        {
            pipe.AddData( std::move(v) );
        }
        auto pending() const {return this->pipe.pending();}
    private:
        WS::Pipe<value_t> pipe;
    };
    /// <summary>
    /// wie ContainerMitSuche nur push_back wird asynchron im thread ausgeführt
    /// </summary>
    /// <typeparam name="pred_getKeyType"></typeparam>
    /// <typeparam name="mutex_type"></typeparam>
    /// <typeparam name="container_type"></typeparam>
    template<typename container_type,typename pred_getKeyType=WS::GetValueAsKey<typename container_type::value_type>,typename mutex_type=std::mutex>
    class ContainerMitSucheUniqueAsync : public ContainerMitSuche<typename container_type,typename pred_getKeyType,typename mutex_type>
    {
    public:
        using base_t = ContainerMitSuche<typename container_type,typename pred_getKeyType,typename mutex_type>;
        using container_t = typename base_t::container_t;
        using value_t = typename base_t::value_t;
        using index_t = typename base_t::index_t;
        using mutex_t = typename base_t::mutex_t;
        using findkey_t = typename base_t::findkey_t;
        ContainerMitSucheUniqueAsync() 
            : base_t()
            , pipe(make_pipe<value_t>( [&](value_t && v){base_t::push_back_unique(std::move(v));})){}
        ContainerMitSucheUniqueAsync( pred_getKeyType fnGetKeyType) 
            : base_t(fnGetKeyType) 
            , pipe(make_pipe<value_t>( [&](value_t && v){base_t::push_back_unique(std::move(v));})){}//ctor mit key-funktion, z.b. eine lambda

        void push_back_unique( value_t && v )
        {
            pipe.AddData( std::move(v) );
        }
        auto pending() const {return this->pipe.pending();}
    private:
        WS::Pipe<value_t> pipe;
    };
}

