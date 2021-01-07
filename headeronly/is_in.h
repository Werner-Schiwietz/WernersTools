#pragma once
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

#include <algorithm>

#include "is_.h"


#pragma warning( push,4)

#ifndef ASSERT
#	if !defined(_ASSERT) 
#		define ASSERT(x) assert(x)
#	else
#		define ASSERT(x) _ASSERT(x)
#	endif
#endif

namespace WS
{
	template<typename T> struct bereich_t
	{
		T lower;
		T upper;
		bereich_t( T lower, T upper ):upper(upper),lower(lower){ASSERT(lower<upper||!(upper<lower));}
		bool is_in(T value) const { return !(value<this->lower || this->upper<value); }
	};
	template<typename T>bereich_t<T> bereich( T const & lower, T const & upper ){return bereich_t<T>(lower,upper);}//weil bereich(1,2) schoener als bereich_t<int>(1,2) aussieht
	template<typename T>bool operator<( WS::bereich_t<T> const & bereich, T const & value )
	{
		return bereich.upper < value;
	}
	template<typename T>bool operator<( T const & value, WS::bereich_t<T> const & bereich )
	{
		return value < bereich.lower;
	}
}

namespace WS
{
	//container
	template <typename value_t, typename U> auto is_in( value_t const & gesucht, U const & other ) 
		-> std::enable_if_t<WS_exist::begin_v<U> || WS_exist::begin_WS_v<U> || WS_exist::begin_glbNS_v<U> || WS_exist::begin_std_v<U> || WS_has_method::begin_v<U>,bool>
	{
		if constexpr ( WS_has_method::begin_v<U> )
		{
			auto b = other.begin();
			auto e = other.end();
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_v<U> )
		{
			auto b = begin(other);
			auto e = end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_WS_v<U> )
		{
			auto b = WS::begin(other);
			auto e = WS::end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_glbNS_v<U> )
		{
			auto b = ::begin(other);
			auto e = ::end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_std_v<U> )
		{
			auto b = std::begin(other);
			auto e = std::end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_has_method::begin_v<U> )
		{
			auto b = other.begin();
			auto e = other.end();
			return std::find(b,e,gesucht)!=e;
		}
	}
	//bereich
	template<typename value_t> bool is_in( value_t const & value, WS::bereich_t<value_t> const & bereich ){return bereich.is_in( value );}
	//array, sonderbehandulung char-type-array
	template<typename value_t, size_t size> bool is_in( value_t const & value, value_t const (&values)[size] )
	{
		if constexpr ( WS::is_char_type_v<value_t> )
		{
			//std::basic_string_view ist so nicht nullterminiert, geht nicht
			//return is_in(value,std::basic_string_view<value_t>(values,size));
			auto b = WS::begin<value_t,size>(values);
			auto e = WS::end<value_t,size>(values);
			return std::find(b,e,value)!=e;
		}
		else
		{
			for( auto const & item : values )
				if( value == item )
					return true;
		}
		return false;
	}	
	//vergleich gleicher werte
	template<typename value_t> bool is_in( value_t const & value, value_t const & vergleichoperand ){return value == vergleichoperand;}
	//variadic
	template<typename value_t, typename vergleichoperand_t, typename... others> bool  is_in( value_t const & value, vergleichoperand_t const & vergleichoperand, others const & ... Rest )
	{
		return is_in( value, vergleichoperand ) || is_in( value, Rest ... );
	}
}

#pragma warning( pop )