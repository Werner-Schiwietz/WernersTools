#pragma once
#define is_is_char_type_defined
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


namespace WS
{
	template<typename T>	struct _is_char_type			: std::false_type{};
	template<>				struct _is_char_type<char>		: std::true_type{};
	template<>				struct _is_char_type<wchar_t>	: std::true_type{};
	template<typename T>	using is_char_type = _is_char_type<std::remove_cv_t<std::remove_reference_t<T>>>;
	template<typename T>	static bool constexpr is_char_type_v = is_char_type<T>::value;

	static_assert( is_char_type_v<char> );
	static_assert( is_char_type_v<char const> );
	static_assert( is_char_type_v<char const *> == false );
	static_assert( is_char_type_v<char *> == false );
	static_assert( is_char_type_v<char const * const > == false );
	static_assert( is_char_type_v<char const &> == true );
	static_assert( is_char_type_v<const char> );
	static_assert( is_char_type_v<wchar_t> );
	static_assert( is_char_type_v<unsigned char> == false );
	static_assert( std::is_same_v<char,__int8> == true );//???
	static_assert( is_char_type_v<__int8> == std::is_same_v<char,__int8> );
}