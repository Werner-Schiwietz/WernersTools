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

#include <regex>
#include <map>

#pragma push_macro("_VALIDCHARS_")
#pragma push_macro("_WORT_")
#pragma push_macro("_LEER_")
#pragma push_macro("_REST_")
#pragma push_macro("_NUMMERN_") 

#define _VALIDCHARS_ "a-zA-Z\xC0-\xD6\xD8-\xF6\xF8-\xFF"
#define _WORT_ "([_" _VALIDCHARS_ "][_0-9" _VALIDCHARS_ "]*)"
#define _LEER_ "\\s"
#define _REST_ ".*"
#define _NUMMERN_ "(\\d+)"


template<typename enum_t>auto parseEnumInitlist( char  const * liste )
	{
		struct
		{
			using enum_type=enum_t;
			bool success;
			std::multimap<enum_t,std::string> values;
			char const * errorposition;

			operator bool() { return success; }
			bool operator !() { return !success; }
			operator decltype(values)&() { return values; }
		} retvalue;

		std::map<std::string, enum_t> mapKnown;


		std::regex regex( _LEER_"*" _WORT_  _LEER_"*" "(=" _LEER_"*" "(" _NUMMERN_ "|" _WORT_ "))?" _LEER_"*" "(" _REST_ ")" );

		std::regex regexDelimiter( "," _LEER_"*" "(.*)" );

		std::cmatch match;
		auto first = liste;
		auto last = first + strlen( first );

		int runingvalue = 0;

		while( std::regex_match( first, last, match, regex ) )//finde erstes/nächstes _WORT_
		{
			if( match[4].matched )
				runingvalue = atoi( match[3].first );
			else if( match[5].matched )
			{
				auto iter=mapKnown.find( match[5].str() );
				if( iter==mapKnown.end() )
					break;//error
				runingvalue = iter->second;
			}

			mapKnown.insert( std::make_pair( match[1].str(), enum_t(runingvalue) ) );
			retvalue.values.insert( std::make_pair( enum_t(runingvalue++),match[1].str() ) );

			first = match[6].first;
			if( std::regex_match( first, last, match, regexDelimiter ) )//finde trenner
			{
				first = match[1].first;
			}
			else
				break;
		}
		retvalue.success = first == last;
		retvalue.errorposition = first!=last ? first : nullptr;
		return retvalue;

	}

#define MAKE_ENUM( name, ... )																																				\
	struct name {																																							\
			enum enum_t{ __VA_ARGS__ } value;																																\
			name(){}																																						\
			name( enum_t value ) : value(value){}																															\
			static constexpr char const * type_name(){ return #name;}																										\
			static std::multimap<enum_t, std::string> const & Map(){static std::multimap<enum_t, std::string> map =parseEnumInitlist<enum_t>(#__VA_ARGS__); return map;}	\
			static std::string string(enum_t key) {auto iter=Map().find(key);if(iter==Map().end())return std::string();return iter->second;}								\
			std::string string()const {auto iter=Map().find(this->value);if(iter==Map().end())return std::string();return iter->second;}									\
			bool operator==( name const & r )const{return this->value==r.value;}																							\
			bool operator!=( name const & r )const{return this->value!=r.value;}																							\
			bool operator<( name const & r )const{return this->value<r.value;}																								\
	}

template<typename structenum_t> auto GetEnumValue(LPCSTR text) 
{
	struct return_t
	{ 
		bool							success = false;
		typename structenum_t::enum_t	value;
		operator bool()							{return success;}
		bool operator!()						{return !success;}
		operator structenum_t::enum_t	()		{return value;}
		auto toEnumType()						{return value;}
		bool operator==( return_t const & r )	{return value==r.value;}
	} retvalue;
	for( auto & pair : structenum_t::Map() )
		if( strcmp( pair.second.c_str(), text )==0 )
		{
			retvalue.success = true;
			retvalue.value = pair.first;
			break;
		}
	return retvalue;
}		
template<typename structenum_t> auto GetEnumValue(CStringA text) 
{
	return GetEnumValue<structenum_t>(text.GetString()); 
}
template<typename structenum_t> auto GetEnumValue(CStringW text) 
{
	CStringA atext( text );
	return GetEnumValue<structenum_t>(atext.GetString()); 
}


#undef _VALIDCHARS_ 
#undef _WORT_ 
#undef _LEER_ 
#undef _REST_ 
#undef _NUMMERN_ 

#pragma pop_macro("_VALIDCHARS_")
#pragma pop_macro("_WORT_")
#pragma pop_macro("_LEER_")
#pragma pop_macro("_REST_")
#pragma pop_macro("_NUMMERN_") 
