#pragma once
//Copyright (c) 2023 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

/// <summary>
/// WS::bits liefert die Anzahl bits eines integral/enum -datentyp
/// siehe auch std::numeric_limits<integral_type>::digits allerdings liefert WS::bits<T>() und WS::bits<unsigned T> den selben Wert
/// </summary>
/// 

/// <summary>
/// WS::rotr( T value, int bits )  
/// rotiert die Bits in einem integralen datentyp nach rechts, richtung least significant bit
/// WS::rotl( T value, int bits )
/// rotiert die Bits in einem integralen datentyp nach links, richtung  most significant bit
/// 
/// T	value		ist der wert dessen bits im kreis verschoben werden
/// int	bits		um wieviele bits im kreis gedreht wird. negative werte drehen die richtung um
/// </summary>


#include <type_traits>	//std::make_unsigned
#include <climits>		//std::numeric_limits

namespace WS
{
	template<typename integral_type>
	constexpr auto bits()
	{
		if constexpr ( std::is_enum_v<integral_type> )
		{
			return bits<std::underlying_type_t<integral_type>>();
		}
		else
		{
			static_assert(std::is_integral_v<integral_type>);
			return std::numeric_limits<std::make_unsigned_t<integral_type>>::digits;
		}
	}

	template<typename integral_type> integral_type rotr( integral_type value, int bits );
	template<typename integral_type> integral_type rotl( integral_type value, int bits )
	{
		if( bits < 0 ) return rotr(value,-bits);
		if( !value ) return value;

		while( bits >= WS::bits<integral_type>() )
			bits -= WS::bits<integral_type>();

		auto v1 = static_cast<integral_type>(value << bits);
		auto v2 = static_cast<integral_type>(static_cast<std::make_unsigned_t<integral_type>>(value) >> (WS::bits<integral_type>() - bits));
		auto v3 = v1 | v2;
		return v3;
	}
	template<typename integral_type> integral_type rotr( integral_type value, int bits )
	{
		if( bits < 0 ) return rotl(value,-bits);
		if( !value ) return value;

		while( bits >= WS::bits<integral_type>() )
			bits -= WS::bits<integral_type>();

		auto v1 = static_cast<integral_type>(static_cast<std::make_unsigned_t<integral_type>>(value) >> bits);
		auto v2 = static_cast<integral_type>(value << (WS::bits<integral_type>() - bits));
		auto v3 = v1 | v2;
		return v3;
	}
}
