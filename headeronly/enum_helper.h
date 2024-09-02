#pragma once
//Copyright (c) 2024 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

#include "char_helper.h"

/// convertieren enum_t <-> char_type (const) * kann spezialisiert werden
/// per tostring<char_t,enum_t>(enum_t)
/// bzw stringto<enum_t,char_t>(char_t const*)
/// 
/// es wird der numerische wert als string zur basis 10 geliefert
/// spezialisierungen k�nnen richtige text liefern, siehe UTLoadSavePUGI.cpp
/// 
template<typename T> concept enum_type = std::is_enum_v<T>;//c++20
template<char_type char_t,int radix = 10,enum_type enum_type> WS::auto_ptr<char_t const[]> tostring(enum_type value )
{
	constexpr size_t chars = WS::bits<enum_type>() + 8;
	auto buf = std::unique_ptr<char_t[]>{ new char_t[chars]{} };//buffer ohne const anlegen
	tostring(WS::to_underlying(value), buf.get(), chars, radix );
	return buf;
}
template<enum_type enum_type,char_type char_t> enum_type stringto(char_t const * psz )
{
	return static_cast<enum_type>(::stringto<std::underlying_type_t<enum_type>>(psz));
}

