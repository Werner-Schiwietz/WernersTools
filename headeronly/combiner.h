#pragma once
//Copyright (c) 2020 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

#include "combiner_last.h"

#include <optional>//WS::ICombiner
#include <deque>

namespace WS
{
	//Combiner f�r Signal
	template<typename T> struct combiner_and
	{	//combiner_and liefert alle r�ckgabewerte mit operator& verkn�pft in std::optional<T> value
		using id_t = size_t;//typename Signal_trait<T()>::id_t;
		std::optional<T> value;
		auto operator()() { return this->value; }
		auto & operator()( T const & v_in, id_t  ={} ) &
		{
			if( this->value.has_value() )
				this->value.value() = this->value.value() & v_in;//T=bool -> mit &= warning C4805: '&': unsafe mix of type 'bool' and type 'int' in operation
			else
				this->value = v_in;
			return *this;
		}
		auto operator()( T const & v_in, id_t id={} ) && { return operator()( v_in, id ); }
	};
	template<typename T> struct combiner_or
	{	//combiner_and liefert alle r�ckgabewerte mit operator| verkn�pft in std::optional<T> value
		using id_t = size_t;//typename Signal_trait<T()>::id_t;
		std::optional<T> value;
		auto operator()() { return this->value; }
		auto & operator()( T const & v_in, id_t ={} ) &
		{
			if( this->value.has_value() )
				this->value.value() = this->value.value() | v_in;//T=bool -> mit |= warning C4805: '|': unsafe mix of type 'bool' and type 'int' in operation
			else
				this->value = v_in;
			return *this;
		}
		auto  operator()( T const & v_in, id_t id={} ) && { return operator()( v_in, id ); }
	};
	template<typename T> struct combiner_all
	{	//combiner_and liefert alle r�ckgabewerte in std::deque<std::pair<T,id_t>> value. der id_t ist die id, die beim Signal.connect im Connection_Guard geliefert wurde
		using id_t = size_t;//typename Signal_trait<T()>::id_t;
		std::deque<std::pair<T, id_t>> value;
		auto operator()() { return this->value; }
		auto & operator()( T const & v_in, id_t id={} ) &
		{
			this->value.push_back( {v_in,id} );
			return *this;
		}
		auto  operator()( T const & v_in, id_t id={} ) && { return operator()( v_in, id ); }
	};
}


