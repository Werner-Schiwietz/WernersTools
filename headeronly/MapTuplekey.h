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

#include <map>
#include <tuple>

namespace WS
{
	template<typename value_t,typename ... key_types > class MapTuplekey : public std::map<std::tuple<key_types...>, value_t>
	{
	public:
		bool get(value_t & value_OutParam, std::tuple<key_types...> key )
		{
			auto iter = find(key);
			if (iter != end())
			{
				value_OutParam = iter->second;
				return true;
			}
			return false ;
		}
		auto get(value_t & value_OutParam, key_types ... keyvalues)
		{
			return get( value_OutParam, key(std::forward<key_types>(keyvalues)...) );
		}
		struct Get_type
		{
			bool found;
			value_t value;

			template<typename value_t>operator value_t() const//ohne template gaebe es compilefehler, wenn value_t bool waere. !!!bedeutet aber, wenn value_t bool ist muss explicit auf .value zugegriffen werden!!!!
			{
				return value;
			}
			operator bool() const
			{
				return found;
			}
		} get(std::tuple<key_types...> key)
		{
			Get_type retvalue;
			retvalue.found = get( retvalue.value, key);
			return retvalue;
		}
		auto get(key_types ... keyvalues)
		{
			return get( key(std::forward<key_types>(keyvalues)...) );
		}

		std::tuple<key_types...> make_key(key_types ... keyvalues)
		{
			return std::forward_as_tuple(keyvalues ...);
		}
		//value_t& operator[](key_types ... keyvalues)//mist, mag der compiler nicht
		auto& operator()(key_types ... keyvalues)
		{
			return operator[](key(keyvalues...));
		}
	};

	//wer nicht wei� was er tut, sollte diesen converter nie benutzen. der compiler kann euch hier nicht mehr helfen.
	template<typename ret_t=void*, typename fn_t> ret_t UnsafeCast(fn_t fn )
	{
		union Convert
		{
			ret_t		pv;
			fn_t		pfn;
		};
		Convert  convert;
		convert.pfn = fn;
		return convert.pv;
	};
}