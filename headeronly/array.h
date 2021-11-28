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

#include <array>

//wie std::array, nur operator[] mit bounds-check

namespace WS
{
	template <typename T, size_t _Size> 
	class array : public std::array<T,_Size>
	{	
	public:
		using base_t			= std::array<T,_Size>;
		using value_type		= typename base_t::value_type;
		using size_type			= typename base_t::size_type;
		using difference_type	= typename base_t::difference_type;
		using pointer			= typename base_t::pointer;
		using const_pointer		= typename base_t::const_pointer;
		using reference			= typename base_t::reference;
		using const_reference	= typename base_t::const_reference;
		//using base_t::base_t;//ctor der basis direkt nutzen, hat nur keinen

		_NODISCARD _CONSTEXPR17 reference operator[](size_type index) & 
		{
			return this->at(index);
		}
		_NODISCARD constexpr const_reference operator[](_In_range_(0, _Size - 1) size_type _Pos) const &
		{
			return this->at(index);
		}
	};

#pragma region ctor aus <array> std::array-ctor kopiert
#if _HAS_CXX17
	template <class _First, class... _Rest>
	array(_First, _Rest...) -> array<typename std::_Enforce_same<_First, _Rest...>::type, 1 + sizeof...(_Rest)>;
#endif // _HAS_CXX17
#pragma endregion

}
