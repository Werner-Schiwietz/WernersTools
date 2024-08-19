#pragma once
///Copyright (c) 2023 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 



/// <summary>
/// auto WS::to_underlying( type_name value ) liefert den integralen wert des enums(muss ein enum sein)
/// auto WS::to_integral( type_name value ) liefert den integralen wert type_name muss ein integraler-type oder ein enum sein
/// </summary>
namespace WS
{
	template<class _Ty> std::add_lvalue_reference_t<_Ty> decllval() noexcept;
	template<class _Ty> std::add_rvalue_reference_t<_Ty> declrval() noexcept;

	template<typename T> auto HasMethod_to_underlying(unsigned long) -> std::false_type;
	template<typename T> auto HasMethod_to_underlying(int) -> decltype(WS::decllval<T const>().to_underlying() , std::true_type{});
	template<typename T> static bool constexpr HasMethod_to_underlying_v = decltype(HasMethod_to_underlying<T>(0))::value;

	//std::to_underlying gibt es erst mit c++23
	template<typename enum_t>auto to_underlying( enum_t value )
	{
		static_assert(std::is_enum_v<enum_t>,"enum_t ist kein enum");
		return static_cast<std::underlying_type_t<enum_t>>(value);
	}
	template<typename T>auto to_integral( T value )
	{
		if constexpr ( std::is_enum_v<T> )
		{
			return to_underlying(value);
		}
		else if constexpr ( HasMethod_to_underlying_v<T> )
		{
			static_assert(std::is_integral_v<std::remove_reference_t<decltype(value.to_underlying())>>, "T::to_underlying() liefert keinen integralen Datentyp" );
			return value.to_underlying();
		}
		else
		{
			static_assert(std::is_integral_v<T>,"T ist kein integraler Datentyp");
			return value;
		}
	}
	template<typename T>struct integral
	{
		using type=decltype(WS::to_integral<T>(std::declval<T>()));
	};
	template<typename T>
	using integral_t = typename integral<T>::type;
}//namespace WS
