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

#include <type_traits>

namespace std
{
	template <class _Ty, class _Alloc>class vector;
	template <class _Kty, class _Pr, class _Alloc>class set;
	template <typename>class shared_ptr;
	template <typename,typename>class unique_ptr;
}
namespace WS
{
	//idee von https://stackoverflow.com/questions/49904809/template-function-for-detecting-pointer-like-dereferencable-types-fails-for-ac Answer 4
	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type can_dereference(unsigned long);
	template <typename T> auto can_dereference(int) -> decltype( * std::declval<T>(), std::true_type{} );		//kann dereferenziert werden
																												
	template <typename T> using is_dereferenceable = decltype(can_dereference<T>(0));
	template <typename T> static auto const is_dereferenceable_v = is_dereferenceable<T>::value;

	template <typename> std::false_type has_pointer_op(unsigned long);//helper, ohne definition
	template <typename T> auto has_pointer_op(int) -> decltype( std::declval<T>().operator->(), std::true_type{} );//hat operator-> //helper, ohne definition
																												
	//template <typename T> using is_pointerable = std::integral_constant<bool,decltype(has_pointer_op<T>(0))::value>;
	template <typename T> using is_pointerable = std::integral_constant<bool,decltype(has_pointer_op<T>(0))::value | std::is_pointer<T>::value>;
	//template <typename T> using is_pointerable = decltype(has_pointer_op<T>(0));
	template <typename T> static auto const is_pointerable_v = is_pointerable<T>::value;
}

namespace WS
{
	template<typename T> struct is_shared_ptr : std::false_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> const> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> volatile> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> const volatile> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T>&> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> const &> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> volatile &> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> const volatile &> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> &&> : std::true_type{};
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> const &&> : std::true_type{};//d�mlich, const rvalue, aber sonst roter test UT_is_shared_ptr_const
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> volatile &&> : std::true_type{};//d�mlich
	template<typename T> struct is_shared_ptr<std::shared_ptr<T> const volatile &&> : std::true_type{};//d�mlich
	template<typename T> static auto const is_shared_ptr_v = is_shared_ptr<T>::value;
}


namespace WS
{
	template <typename T,typename =void> struct is_std_vector : std::false_type { };
	template <typename T> struct is_std_vector<T,typename std::enable_if<std::is_same<std::decay_t<T>,std::vector<typename std::decay_t<T>::value_type, typename std::decay_t<T>::allocator_type>>::value>::type> : std::true_type {};
	template <typename T> static auto const is_std_vector_v = is_std_vector<T>::value;
	
	template <typename T,typename =void> struct is_std_set : std::false_type { };
	template <typename T> struct is_std_set< T, typename std::enable_if< std::is_same<std::decay_t<T>,std::set< typename std::decay_t<T>::value_type, typename std::decay_t<T>::key_compare, typename std::decay_t<T>::allocator_type > >::value >::type > : std::true_type {};
	template <typename T> static auto const is_std_set_v = is_std_set<T>::value;
}