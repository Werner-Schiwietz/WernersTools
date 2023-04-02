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

#include "iterator_access.h"
#include "char_helper.h"

//ein paar hilfs-funktion zum parsen
//mit dem ziel so wenig zu kopieren wie möglich
//eat                                               //genau die items in der reihenfolge, oder das eine item
//eat_if                                            //ein zeichen wenn bedingung erfüllt ist
//eat_while                                         //solange wie bedingung erfüllt ist
//eat_oneof                                         //ein zeichen wenn es in der liste ist
//eat_integer                                       //konvertiert digits zu integer
//eat_flanked                                       //entfernt das flankierende zeichenpaar. escapesequenzen bleiben allerdings stehen. alles eingebettet ist z.B. \"hallo\" in "\"hallo\"" oder "hallo" in ["hallo"]
//remove_escape                                     //entfernt aus den ergebnis von eat_flanked ggf. die escape-sequenzen
//make_flanked                                      //bettet in zeichenpaar ein und fügt ggf escape-zeichen ein
//remove_flank                                      //entfernt das flankierende zeichenpaar und escapesequenzen


namespace WS
{
    enum class parse_error
    {
        none
        , incomplete                                                //vorzeitiges ende
        , tillitem_not_found
        , invalid_escape_sequence
        , integer_overflow                                          //digits pasen nicht in den datentyp             
        , length                                              
        , no_match                                                  //kein zeichen matched
        , out_of_range                                              //wert ungültig
        , delimiter
        , left_without_right
    };
    template<typename T> struct rettype_eat
    {
        using item_t = T;
        using this_t = rettype_eat<T>;
        _iterator_access<T> eaten;
        _iterator_access<T> eaten_till_error;
        parse_error error = parse_error::no_match;

        operator bool() { return this->error == parse_error::none; }
        bool operator !() { return this->error != parse_error::none; }
        operator _iterator_access<T>() { return eaten; }
        operator parse_error() { return error; }

        rettype_eat(){}//no_match
        rettype_eat( _iterator_access<T> & aliveInOut, typename _iterator_access<T>::iterator_t & begin, typename _iterator_access<T>::iterator_t & end )
            : rettype_eat( aliveInOut, begin, end, parse_error::none ) {}
        rettype_eat( _iterator_access<T> & aliveInOut, typename _iterator_access<T>::iterator_t & begin, typename _iterator_access<T>::iterator_t & end, parse_error error ) : error(error)
        {
            if( error==parse_error::none )
            {
                eaten = aliveInOut;
                eaten.begin() = begin;
                eaten.end() = end;

                aliveInOut.begin() = end;//bis dahin ist es ausgewertet
            }
            else
            {
                eaten_till_error = aliveInOut;
                eaten_till_error.begin() = begin;
                eaten_till_error.end() = end;
            }
        }
    };

    template<typename T> bool _eat_unchecked( _iterator_access<T> & container, typename _iterator_access<T>::value_t const & item )
    {
        if( *container.begin() == item )
        {
            ++container.begin();
            return true;
        }
        return false;
    }          

    //erstes item wegnehmen
    template<typename T> bool eat( _iterator_access<T> & container )
    {
        if( *container.begin() != *container.end() )
        {
            ++container.begin();
            return true;
        }
        return false;
    }          

    template<typename T> bool eat( _iterator_access<T> & container, typename _iterator_access<T>::value_t const & item )
    {
        if( container.begin() != container.end() )
            return _eat_unchecked( container, item );
        return false;
    }

    //return empty or
    template<typename T, typename U> rettype_eat<T> eat( _iterator_access<T> & container_in, _iterator_access<U> const & items )
    {
        auto container = container_in;
        auto error = parse_error::no_match;

        for( auto const & item : items )
        {
            if(eat(container,item) == false)
                return rettype_eat<T>{ container_in,container_in.begin(),container.begin(),error };
            else
                error = parse_error::incomplete;
        }


        rettype_eat<T> retvalue{ container_in, container_in.begin(), container.begin() };
        container_in = container;
        return retvalue;
    }

    template<typename T> bool _eat_oneof_unchecked( _iterator_access<T> & ) { return false; }
    template<typename T, typename ... items_t> bool _eat_oneof_unchecked( _iterator_access<T> & container, typename _iterator_access<T>::value_t const & item, items_t ... items )
    {
        if( _eat_unchecked( container, item ) )
            return true;
        return _eat_oneof_unchecked( container, items ... );
    }          
    template<typename T, typename ... items_t> _iterator_access<T> eat_oneof( _iterator_access<T> & container, items_t ... items )
    {
        _iterator_access<T> retvalue{container.begin(),container.begin(),container.rvalue_lifetime_extender};

        if( container.begin()!=container.end() )
            if( _eat_oneof_unchecked( container, items ... ) )
                retvalue.end() = container.begin();

        return retvalue;
    }
    template<typename T, typename ... items_t> _iterator_access<T> eat_oneof( _iterator_access<T> & container, _iterator_access<T> items )
    {
        auto begin = container.begin();

        if( container.begin()!=container.end() )
            for( auto & item : items)
                if( _eat_unchecked( container, item ) )
                    return _iterator_access<T> {begin,container.begin(),container.rvalue_lifetime_extender};

        return _iterator_access<T> {begin,begin,container.rvalue_lifetime_extender};
    }

    template<typename T> rettype_eat<T> eat_till( _iterator_access<T> & container_in, typename _iterator_access<T>::value_t const & till_item, typename _iterator_access<T>::value_t const & escape_item )
    {
        auto container = container_in;

        while( container )
        {
            auto cont_esc = container;
            if( eat( cont_esc, escape_item ) )//escape-sequence
            {
                if( eat_oneof( cont_esc, till_item, escape_item ) )
                    container.begin()=cont_esc.begin();
                else
                    return rettype_eat<T>{container_in, container_in.begin(), container.begin(), parse_error::invalid_escape_sequence};
            }
            else if( eat( cont_esc, till_item ) )//
            {
                rettype_eat<T> retvalue{container_in, container_in.begin(), container.begin()};
                container_in = container;
                return retvalue;
            }
            else
                ++container.begin();
        }

        return rettype_eat<T> { container_in, container_in.begin(), container.begin(), parse_error::tillitem_not_found };
    }


    template<typename T> using flanked_t = typename _iterator_access<T>::value_t;
    template<typename T> using left_t = typename _iterator_access<T>::value_t;
    template<typename T> using right_t = typename _iterator_access<T>::value_t;
    template<typename T> using escape_t = typename _iterator_access<T>::value_t;
    template<typename T> auto const & flanked_type	( T const & item )	{ return item; }
    template<typename T> auto const & left_type		( T const & item )	{ return item; }
    template<typename T> auto const & right_type	( T const & item )	{ return item; }
    template<typename T> auto const & escape_type	( T const & item )	{ return item; }

    template<typename T> struct rettype_eat_flanked : rettype_eat<T>
    {
        using base_t = rettype_eat<T>;

        left_t<T> left{};
        right_t<T> right{};
        _iterator_access<T> value;//ohne flanke

        operator bool(){return this->error==parse_error::none;}

        auto && setLeft ( left_t<T>  v ) && { this->left =v; return std::move(*this); }
        auto && setRight( right_t<T> v ) && { this->right=v; return std::move(*this); }

        using base_t::base_t;

        rettype_eat_flanked( rettype_eat<T> && r) : base_t(r){}
    };
    template<typename T> rettype_eat_flanked<T> _eat_flanked( _iterator_access<T> & container, right_t<T> const & right_item, escape_t<T> const & escape_item )
    {
        rettype_eat_flanked<T> retvalue = eat_till( container, right_item, escape_item );
        eat( container, right_item );
        return std::move(retvalue).setRight(right_item);
    }
    template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> & container_in, left_t<T> const & left_item, right_t<T> const & right_item, escape_t<T> const & escape_item )
    {
        auto container = container_in;
        if( eat( container, left_item ) )
        {
            auto erg = _eat_flanked( container, right_item, escape_item ).setLeft(left_item);
            if( erg )
            {
                erg.value = _iterator_access<T>{erg.eaten.begin(),container.begin()-1,erg.eaten.rvalue_lifetime_extender};
                erg.eaten = _iterator_access<T>{container_in.begin(),container.begin(),erg.eaten.rvalue_lifetime_extender} ;
                container_in = container;
            }
            return erg;
        }
        return {container,container.begin(),container.begin(),parse_error::no_match};
    }
    template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> && container_in, left_t<T> const & left_item, right_t<T> const & right_item, escape_t<T> const & escape_item )
    {
        return eat_flanked( container_in, left_item, right_item, escape_item );
    }
    template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> & container, flanked_t<T> const & flank_item, escape_t<T> const & escape_item )
    {
        return eat_flanked( container, left_t<T>(flank_item), right_t<T>(flank_item), escape_item );
    }
    template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> && container, flanked_t<T> const & flank_item, escape_t<T> const & escape_item )
    {
        return eat_flanked( container, left_t<T>(flank_item), right_t<T>(flank_item), escape_item );
    }
    template<typename T> rettype_eat_flanked<T>  eat_flanked( _iterator_access<T> & container_in, typename _iterator_access<T> const & till_items, escape_t<T> const & escape_item )
    {
        auto container = container_in;
        if( auto erg_first_last_item = eat_oneof( container, till_items ) )
        {
            auto retvalue = _eat_flanked( container, *erg_first_last_item.begin(), escape_item );
            if( retvalue )
                container_in = container;
            return std::move(retvalue).setLeft(*erg_first_last_item.begin());
        }
        return {container_in, container_in.begin(),container_in.begin(),parse_error::no_match};
    }
    
    template <typename container_t, typename value_t> container_t& append( container_t & container, value_t value )
    {
        return container += value;
    }
    template <typename container_t, typename iterator_t> container_t& append( container_t & container, iterator_t first, iterator_t last )
    {
        return container += container_t{first, last};
    }
    template<typename container_t, typename T> container_t make_flanked( _iterator_access<T> parse, left_t<T> left, right_t<T> right, escape_t<T> escape)
    {
        container_t retvalue;
        using value_t = typename _iterator_access<T>::value_t;

        append( retvalue, left );

        for(;parse;)
        {
            auto part = eat_while( parse, [right, escape]( value_t const & value ) { return value!=right && value!=escape; } );
            if( part )
                append( retvalue, part.begin(), part.end() );
            if( parse )
            {
                append( retvalue, escape );
                append( retvalue, *parse.begin()++ );
            }
        }
        return append( retvalue, right );
    }

    template<typename container_t, typename T> container_t make_flanked( _iterator_access<T> parse, flanked_t<T> const & flank_item, escape_t<T> escape)
    {
        return make_flanked<container_t>( parse, flank_item, flank_item, escape);
    }

    template<typename iterator_t>  _iterator_access<decltype(&*std::declval<iterator_t>())> remove_flank( _iterator_access<iterator_t> parse, left_t<iterator_t> const & left_item, right_t<iterator_t> const & right_item, escape_t<iterator_t> const & escape_item )
    {
        return remove_escape( eat_flanked( parse, left_item, right_item, escape_item ).value, escape_item );
    }

    template<typename iterator_t>  _iterator_access<decltype(&*std::declval<iterator_t>())> remove_flank( _iterator_access<iterator_t> parse, flanked_t<iterator_t> const & flank_item, escape_t<iterator_t> const & escape_item )
    {
        return remove_escape( eat_flanked( parse, flank_item, flank_item, escape_item ), escape_item );
    }


    //entfernt escape-char. macht kopie nur, wenn sie nötig ist
    template<typename iterator_t>  _iterator_access<decltype(&*std::declval<iterator_t>())> remove_escape( _iterator_access<iterator_t> parse, escape_t<iterator_t> escape)
    {
        using retvalue_t = _iterator_access<decltype(&*std::declval<iterator_t>())>;
        using value_t = typename _iterator_access<iterator_t>::value_t;
        using buffer_t = std::basic_string<value_t>;//funktioniert nur, wenn

        _iterator_access<iterator_t>	retvalue{parse.begin(),parse.begin(),parse.rvalue_lifetime_extender};//copy rvalue_lifetime_extender
        buffer_t						char_buffer;//nur wenn nötig umkopieren

        //bei nötiger veränderung char_buffer benutzen
        auto append =[&]( _iterator_access<iterator_t> parse )
        {
            if( parse.empty() )
                return;
            if( retvalue.empty() )
                retvalue = parse;
            else if( retvalue.end() == parse.begin() )
                retvalue.end() = parse.end();
            else
            {	//veränderung -> umkopieren
                if( char_buffer.empty() )
                    if( retvalue )
                        char_buffer = buffer_t{retvalue.begin(),retvalue.end()};
                char_buffer.append( parse.begin(), parse.end() );
            }
        };

        while(parse)
        {
            auto part = eat_while( parse, [escape]( value_t const & value ) { return value!=escape; } );
            append( part );

            if( parse )
            {
                auto b = ++parse.begin();//wg parameter reihenfolge ++ nicht als fn-parameter
                auto e = ++parse.begin();//wg parameter reihenfolge ++ nicht als fn-parameter
                append( iterator_access( b, e ) );
            }
        }

        if( char_buffer.empty() )
        {
            auto b = &*retvalue.begin();
            auto e = b+retvalue.len();
            return retvalue_t{b,e,retvalue.rvalue_lifetime_extender};
        }

        {
            auto temp = iterator_access( std::move( char_buffer ) );//rvalue_lifetime_extender anlegen
            auto b = &*temp.begin();
            auto e = b+temp.len();//*temp.end() geht nicht
            return retvalue_t{b,e,temp.rvalue_lifetime_extender};
        }
    }

    template<typename T, typename function_t> _iterator_access<T> _eat_if_unckecked( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const &)
    {
        _iterator_access<T> retvalue{container.begin(),container.begin(), container.rvalue_lifetime_extender};

        if( function( *container.begin() ) )
            retvalue.end() = ++container.begin();

        return retvalue;
    }          
    template<typename T, typename function_t> _iterator_access<T> eat_if( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const &)
    {
        if( container.begin()!=container.end() )
            return _eat_if_unckecked( container, function );

        return {container.begin(),container.begin(),container.rvalue_lifetime_extender};
    }
    template<typename T, typename function_t> _iterator_access<T> eat_while( _iterator_access<T> & container, function_t function ) //function_t signature bool(T const&)
    {
        _iterator_access<T> retvalue( container.begin(), container.begin(), container.rvalue_lifetime_extender );
        while( eat_if( container, function ) )
            ++retvalue.end();
        return retvalue;
    }

    template<int radix=10,typename T> rettype_eat<T> eat_digitcount( _iterator_access<T> & container, size_t chars_min, size_t chars_max )
    {
        _iterator_access<T> toparse = container;

        size_t gelesen = 0;
        {
            auto is_digit = [](std::remove_pointer_t<T> ch)->bool{ return digit<radix>(ch); };
            for( ;gelesen<chars_max; ++gelesen )
            {
                if( !eat_if( toparse, is_digit ) )
                    break;
            }
        }

        WS::parse_error error = WS::parse_error::none;
        if( gelesen<chars_min )
        {
            if( gelesen )
                error = WS::parse_error::length;
            else
                error = WS::parse_error::no_match;
        }
        auto retvalue = rettype_eat<T>{ container, container.begin(), toparse.begin(), error };
        return retvalue;
    }
    template<int radix=10,typename T> rettype_eat<T> eat_digitcount( _iterator_access<T> & container, size_t items )
    {
        return eat_digitcount<radix>( container, items, items );
    }

    template<typename integer_t,typename T> struct rettype_eat_integer : rettype_eat<T>
    {
        using base_t = rettype_eat<T>;
        integer_t value = integer_t{0};

        rettype_eat_integer() {};
        rettype_eat_integer(_iterator_access<T> & container) : base_t{container,container.begin(),container.begin()} {};

        operator integer_t(){ return value; }
    };
    template<typename integer_t,int radix=10,typename T> rettype_eat_integer<integer_t,T> eat_integer( _iterator_access<T> & container) noexcept(false)//wirft bei ueberlauf exception
    {
        rettype_eat_integer<integer_t,T> ret_value{container};
        ret_value.error = parse_error::no_match;

        while( ret_value.eaten.end() != container.end() )
        {
            if( auto erg = digit<radix>(*ret_value.eaten.end()) )
            {
                ret_value.error = parse_error::none;//mindestens eine Zahl gefunden

                auto old = ret_value.value;
                ret_value.value = ret_value.value * radix + static_cast<integer_t>(erg.value);
                if( ret_value.value<old )
                {
                    ret_value.error = parse_error::integer_overflow;
                    ret_value.eaten_till_error = ret_value.eaten;
                    return ret_value;
                    //throw std::out_of_range( __FUNCTION__ " overflow" );
                }
                ++ret_value.eaten.end();
            }
            else
                break;
        }

        container.begin() = ret_value.eaten.end();

        return ret_value;
    }
    template<typename integer_t,typename T> rettype_eat_integer<integer_t,T> eat_integer( _iterator_access<T> && container ) noexcept(false)//wirft bei ueberlauf exception
    {
        return eat_integer<integer_t>( container );
    }
    template<typename integer_t,int radix=10,typename T> rettype_eat_integer<integer_t,T> eat_integercount( _iterator_access<T> & container, size_t chars_min, size_t chars_max )
    {
        auto toparse = container;
        if( auto erg = eat_digitcount<radix>( toparse, chars_min, chars_max ) )
        {
            auto erg2 = eat_integer<integer_t,radix>( erg.eaten );
            if( erg2 )
                container = toparse;
            else if( erg2.error==parse_error::no_match ) 
                erg2.error = parse_error::none;
            return erg2;
        }
        return {};
    }
    template<typename integer_t,int radix=10,typename T> rettype_eat_integer<integer_t,T> eat_integercount( _iterator_access<T> & container, size_t items )
    {
        return eat_integercount<integer_t,radix>( container, items, items );
    }

    template<typename T> auto eat_space( _iterator_access<T> & container )
    {
        using is_t = bool(*)(typename _iterator_access<T>::value_t);
        return eat_while( container, (is_t)&WS::isspace );
    }
    template<typename T> struct rettype_skip
    {
        _iterator_access<T> eaten;
        operator bool() { return true; }
        operator _iterator_access<T>() { return eaten; }
        operator parse_error() { parse_error::none; }

        rettype_skip() {};
        rettype_skip(_iterator_access<T> eaten) : eaten(eaten) {};
    };
    template<typename T> rettype_skip<T> skip_space( _iterator_access<T> & container )
    {
        return eat_space( container );
    }
}
