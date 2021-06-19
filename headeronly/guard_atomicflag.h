#pragma once
//Copyright (c) 2021 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

///usage 
/// auto autorestore = WS::test_and_set( atomiflag );
/// if(autorestore==false)//VORSICHT, genau anders herum als std::atomic_flag::test_and_set()
///		set hat NICHT geklappt, siehe std::atomic_flag::test_and_set
/// set hat geklappt, reset wird automatisch mit dtor von guard gemacht
///oder besser
///	if( auto autorestore = WS::test_and_set( atomiflag ) )
///	{
///		was zu tun ist, wenn der set erfolgreich war.
///	}

///RAII_atomic_flag_set_reset bzw atomic_flag_guard
///ruft auf den atomic_flag test_and_set() und liefert true, wenn atomic_flag vor aufruf false war.
///der destruktor stellt den zustand vor der ver�nderung wieder her.
///usage z.B. if( auto raii = RAII_atomic_flag_set_reset{this->in_PrepareData} ) //die if-teil beding ist erf�llt, wenn atomic_flag vor aufruf false war


#include <atomic>

namespace WS
{
	class RAII_atomic_flag_set_reset
	{
		bool				oldValue{true};
		std::atomic_flag*	to_clear{};
	public:
		RAII_atomic_flag_set_reset(){}
		RAII_atomic_flag_set_reset(std::atomic_flag & to_clear) : to_clear(&to_clear),oldValue(to_clear.test_and_set()){}
		RAII_atomic_flag_set_reset(RAII_atomic_flag_set_reset const &) = delete;
		RAII_atomic_flag_set_reset(RAII_atomic_flag_set_reset && r){swap(r);}
		RAII_atomic_flag_set_reset& operator=(RAII_atomic_flag_set_reset const &) = delete;
		RAII_atomic_flag_set_reset& operator=(RAII_atomic_flag_set_reset && r)&{RAII_atomic_flag_set_reset(std::move(r)).swap(*this);return *this;}
		void swap(RAII_atomic_flag_set_reset & r)
		{
			std::swap(this->oldValue,r.oldValue);
			std::swap(this->to_clear,r.to_clear);
		}
		~RAII_atomic_flag_set_reset()
		{
			clear();
		}
		void clear()
		{
			if(this->oldValue==false && this->to_clear)
			{
				this->oldValue = true;
				this->to_clear->clear();
				this->to_clear=nullptr;
			}
		}
		operator bool() const {return this->oldValue==false;}
	};
	using atomic_flag_guard = RAII_atomic_flag_set_reset;
	[[nodiscard]]atomic_flag_guard test_and_set( std::atomic_flag& flag){ return atomic_flag_guard{flag};}
}