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

#include <memory>

#pragma warning( push,4)

/// Dokumentation der WS::?const_cast Funktionen
/// 
/// toggleconst_cast macht aus const value ein value das nicht const ist und umgekeht
/// bei pointern (raw/shared) wird nicht der pointer const, sondern das value auf den der pointer zeigt
/// 
/// notconst_cast erwarte ein T const bzw T const * oder std::shared_ptr<T const> und nimmt das const weg. Sonst wie toggleconst_cast 
/// compilefehler, wenn T nicht const ist
/// 
/// toconst_cast erwarte ein T bzw T *  oder std::shared_ptr<T> und macht T zuu const. Sonst wie toggleconst_cast 
/// compilefehler, wenn T const ist
/// 
/// 
/// 
/// usage aus UT_const_cast
///	struct A
///	{
///		int v=5;
///	
///		int const & GetValue() const &
///		{
///			return v;
///		}
///		int & GetValue() &
///		{
///			//cast mit richtungsangabe. gibt compilefehler, wenn keine ver�nderung n�tig ist
///			return WS::notconst_cast(WS::toconst_cast(*this).GetValue());
///		}
///	};

///	struct B
///	{
///		int v=5;
///		int const & GetValue() const &
///		{
///		return v;
///		}
///		int & GetValue() &
///		{
///		return WS::toggleconst_cast(WS::toggleconst_cast(this)->GetValue());
///		}
///	};
/// 
namespace WS
{
	template<typename T> auto toggleconst_cast( T & value) -> std::enable_if_t<std::is_const_v<T> && std::is_pointer<T>::value==false, std::remove_const_t<T> &>
	{
		return const_cast<std::remove_const_t<T>&>(value);
	}
	template<typename T> auto toggleconst_cast( T & value) -> std::enable_if_t<std::is_const<T>::value==false && std::is_pointer<T>::value==false, T const &>
	{
		return value;
	}
	template<typename T> auto toggleconst_cast( T && value) -> std::enable_if_t<std::is_const_v<T> && std::is_pointer<T>::value==false, std::remove_const_t<T> &&>
	{
		return const_cast<std::remove_const_t<T>&&>(value);
	}
	template<typename T> auto toggleconst_cast( T && value) -> std::enable_if_t<std::is_const<T>::value==false && std::is_pointer<T>::value==false, T const &&>
	{
		return std::move(value);
	}

	template<typename T> auto notconst_cast( T & value) -> std::enable_if_t<std::is_const_v<T>, std::remove_const_t<T> &>
	{
		return toggleconst_cast(value);
	}
	template<typename T> auto toconst_cast( T & value) -> std::enable_if_t<std::is_const<T>::value==false, T const &>
	{
		return value;
	}
	template<typename T> auto notconst_cast( T && value) -> std::enable_if_t<std::is_const_v<T>, std::remove_const_t<T> &&>
	{
		return toggleconst_cast(std::move(value));
	}
	template<typename T> auto toconst_cast( T && value) -> std::enable_if_t<std::is_const<T>::value==false, T const &&>
	{
		return std::move(value);
	}

	#pragma region raw-pointer
	template<typename T> auto toggleconst_cast( T * value) -> std::enable_if_t<std::is_const_v<T>, std::remove_const_t<std::remove_pointer_t<T>> *>
	{
		return const_cast<std::remove_const_t<std::remove_pointer_t<T>>*>(value);
	}
	template<typename T> auto toggleconst_cast( T * value) -> std::enable_if_t<std::is_const<T>::value==false, std::remove_pointer_t<T> const *>
	{
		return value;
	}
	template<typename T> auto notconst_cast( T * value) -> std::enable_if_t<std::is_const_v<T>, std::remove_const_t<T> *>
	{
		return toggleconst_cast(value);
	}
	template<typename T> auto notconst_cast( T * value) -> std::enable_if_t<std::is_const_v<T> == false, std::remove_const_t<T> *> = delete;//T to T ist unerw�nscht
	template<typename T> auto toconst_cast( T * value) -> std::enable_if_t<std::is_const<T>::value==false, T const *>
	{
		return value;
	}
	template<typename T> auto toconst_cast( T * value) -> std::enable_if_t<std::is_const<T>::value, T const *> = delete;//T const to T const ist unerw�nscht
#pragma endregion 

	#pragma region std::shared_ptr
	template<typename T> auto toggleconst_cast( std::shared_ptr<T> & value ) -> std::enable_if_t<std::is_const_v<T>, std::shared_ptr<std::remove_const_t<T>>>
	{
		return std::const_pointer_cast<std::remove_const_t<T>>(value);
	}
	template<typename T> auto toggleconst_cast( std::shared_ptr<T> & value ) -> std::enable_if_t<std::is_const<T>::value==false, std::shared_ptr<T const>>
	{
		return std::const_pointer_cast<T const>( value );
	}
	template<typename T> auto notconst_cast( std::shared_ptr<T> & value ) -> std::enable_if_t<std::is_const_v<T>, std::shared_ptr<std::remove_const_t<T>>>
	{
		return std::const_pointer_cast<std::remove_const_t<T>>(value);
	}
	template<typename T> auto notconst_cast( std::shared_ptr<T> & value ) -> std::enable_if_t<std::is_const_v<T> == false, std::shared_ptr<std::remove_const_t<T>>> = delete;//T to T ist unerw�nscht
	//{
	//	static_assert( false, "T ist nicht const");
	//	return std::const_pointer_cast<std::remove_const_t<T>>(value);
	//}
	template<typename T> auto toconst_cast( std::shared_ptr<T> & value ) -> std::enable_if_t<std::is_const<T>::value==false, std::shared_ptr<T const>>
	{
		return std::const_pointer_cast<T const>( value );
	}
	template<typename T> auto toconst_cast( std::shared_ptr<T> & value ) -> std::enable_if_t<std::is_const<T>::value, std::shared_ptr<T const>> = delete;//T const to T const ist unerw�nscht
	//{
	//	static_assert( false, "T ist schon const");
	//	return std::const_pointer_cast<T const>( value );
	//}
#pragma endregion 

}


#pragma warning( pop )