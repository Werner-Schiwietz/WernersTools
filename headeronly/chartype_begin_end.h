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

#include "is_char_type.h"

namespace WS
{
	//???? ambiguous wenn das so gemacht wird??
	//template<typename char_t,size_t size> auto begin(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){return ar;}
	//template<typename char_t,size_t size> auto end(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{})
	//{
	//	size_t index=0;
	//	for( ; index<size && ar[index]; ++index ) {}
	//	return &ar[0]+index;
	//}	
	//
	//\evtl. 0-terminierte string-arrays. nicht 0-terminierte liefern als end die erste adresse hinter dem array
	template<typename char_t,size_t size> auto begin(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){return ar;}
	template<typename char_t,size_t size> auto end(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{})
	{
		size_t index=0;
		for( ; index<size && ar[index]; ++index ) {}
		return &ar[0]+index;
	}
	template<typename char_t,size_t size> auto begin(char_t const (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{}){return ar;}
	template<typename char_t,size_t size> auto end(char_t const (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{})
	{
		size_t index=0;
		for( ; index<size && ar[index]; ++index ) {}
		return &ar[0]+index;
	}
	//\0-terminierte strings, wenn nicht \0-terminiert undefineirtes ergebnis
	template<typename char_t> auto begin(char_t const * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{}){return p;}
	template<typename char_t> auto end(char_t const * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{}){while(*p)++p;return p;}
	template<typename char_t> auto begin(char_t * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){return p;}
	template<typename char_t> auto end(char_t * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){while(*p)++p;return p;}
}

