//#pragma once
//Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 


/// usage C++17
/// z.b. per range based for
/// 
/// for( auto value : WS::laufvar(7,5) )
/// { doing von 7 -> incl. 5 }
/// 
/// or 
/// 
/// for( WS::laufvar value{unsigned __int8(0),unsigned __int8(255)}; value.isvalid(); ++value )
/// { doing }

#pragma once

#include <stdexcept>

namespace WS
{
    /// <summary>
    /// laufvar<laufvar_type> die laufvariabel vom typ laufvar_type läuft von start_wert bis einschließlich ende_wert. benutzt werden laufvar_type::operator++() bzw laufvar_type::operator--()
    /// beim zugriff auf einen "invalid()" wert wird eine std::out_of_range exception geworfen
    /// operator++ ruft next(), also intern ++ oder -- auf die zugrunde liegende laufvariable. es geht nur in eine richtung
    /// </summary>
    /// <typeparam name="laufvar_type"></typeparam>
    template<typename laufvar_type>
    struct laufvar
    {                using laufvar_t = laufvar_type;
    private:
        mutable bool                           valid = false;
        mutable laufvar_t i;                                                                     //i nimmt alle werte zwischen start_wert und ende_wert an, incl. der beiden grenzwerte.
        laufvar_t const                        start_wert;
        laufvar_t const                        ende_wert;
        bool const                                                   inc;//wenn true wird operator++ sonst operator-- benutzt. default ohne überlauf (also wenn e<s false sonst true)
    public:
        laufvar( laufvar_t start_wert, laufvar_t ende_wert, bool defaultdirection ) : start_wert(start_wert), ende_wert(ende_wert), i(start_wert), inc(defaultdirection?!(ende_wert<start_wert):(ende_wert<start_wert)), valid(true){}//inc default so, das ohne überlauf gearbeitet wird
        laufvar( laufvar_t start_wert, laufvar_t ende_wert ) : laufvar( start_wert,ende_wert, true ){}
        laufvar() : laufvar( laufvar_t{}, laufvar_t{} ){}
        laufvar( laufvar const & ) = default;

        laufvar_t const & to_underlying() const &
        {
            if( isvalid()==false )
                throw std::out_of_range{__FUNCTION__ " laufvar is invalid"};
            return i;
        }
        operator laufvar_t const &() const & {return to_underlying();}

        bool                                             next() const
        {
            if(i!=ende_wert)//ende erreicht -> wird invalid. geht nur so, anders würde laufvar<unsigned char>(0,255) nicht funktionieren
            {
                if( inc )
                    ++i; 
                else
                    --i;
            }
            else
                valid = false;
            return isvalid();
        }
        laufvar const & operator++() const & { next(); return *this; }
        laufvar&                   operator++() & { next(); return *this; }
        laufvar                                        operator++(int) const { auto retvalue{*this};next();return retvalue; }

        bool                                             isvalid() const{ return valid; }
        bool                                             isinvalid() const{ return !valid; }

        bool operator==(WS::laufvar<laufvar_t> const & r) const
        {
            //alle member vergleichen
            return    this->i           == r.i
                    , this->start_wert  == r.start_wert
                    , this->ende_wert   == r.ende_wert
                    , this->inc         == r.inc
                    , this->valid       == r.valid;
        }
        bool operator!=(WS::laufvar<laufvar_t> const & r) const { return !operator==(r); }

        template<typename laufvar_type> struct const_iterator
        {                using laufvar_t = laufvar_type;
        WS::laufvar<laufvar_t> laufvar;

        const_iterator(){};
        const_iterator(WS::laufvar<laufvar_t> laufvar) : laufvar(laufvar){}
        const_iterator(WS::laufvar<laufvar_t> laufvar,bool ) : laufvar(laufvar){this->laufvar.valid = false;}//für end

        bool operator==( const_iterator const & r ) const { return this->laufvar==r.laufvar; }
        bool operator!=( const_iterator const & r ) const { return this->laufvar!=r.laufvar; }
        const_iterator& operator++()
        {
            (void)laufvar.next();
            return *this;
        }
        const_iterator operator++(int)
        {
            auto retvalue = *this;
            ++(*this);
            return retvalue;
        }

        laufvar_t const & operator*() const
        {
            if( this->laufvar.isinvalid() )
                throw std::exception{ __FUNCTION__ " end defereced"};
            return this->laufvar.i;
        }
        laufvar_t const * operator->() const
        {
            if( this->laufvar.isinvalid() )
                throw std::exception{ __FUNCTION__ " end defereced"};
            return &this->laufvar.i;
        }
        };
        const_iterator<laufvar_t> begin() const{ return const_iterator<laufvar_t>{*this}; }
        const_iterator<laufvar_t> end() const{ return const_iterator<laufvar_t>{*this,false}; }
    };
}
