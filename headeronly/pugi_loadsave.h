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


/// from_node / to_node
/// funktionen die das schreiben in XML und lesen aus XML per pugpxml vereinheitlichen, siehe UTLoadSavePUGI.cpp


#include "pugixml.hpp"

#include "Auto_Ptr.h"
#include "enum_helper.h"

#include <string>
#include <sstream>
#include <iomanip>

#undef __CONCAT
#define __CONCAT(x, y) x ## y
#undef _CONCAT
#define _CONCAT(x, y)  __CONCAT(x, y)

#ifndef TCHAR
#	ifdef UNICODE
#		ifndef PUGIXML_WCHAR_MODE
#			error definiere doch bitte PUGIXML_WCHAR_MODE in den projektdateien
#		endif
#		define TCHAR wchar_t
#		define _T(x) _CONCAT(L,x)
#	else
#		ifdef PUGIXML_WCHAR_MODE
#			error entferne doch bitte PUGIXML_WCHAR_MODE aus den projektdateien
#		endif
#		define TCHAR char
#		define _T(x) x
#	endif
#endif

using pugistringstream = std::basic_stringstream<pugi::char_t, std::char_traits<pugi::char_t>, std::allocator<pugi::char_t>>;

//NAME_AND_STR macht aus parameter n, "n" bzw n, L"n". n ist z.b. platzhalter des namens eines member
#define NAME_AND_STR(n) n, _T(#n)


template<typename T,typename fn_type> bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name, fn_type getter )
{
	if( auto node = container.child(name) )
	{
		dest = getter(node);
		return true;
	}
	return false;
}

/// <summary>
/// _node helper-funktionen f�r den xml import/export per pugi
/// </summary>
namespace _node
{
	/// <summary>
	/// _node::HasMethod_Load pr�ft ob es eine T::load-methode mit parameter pugi::xml_node nutzbar(public) gibt
	/// </summary>
	template<typename T> auto HasMethod_load(unsigned long) -> std::false_type;
	template<typename T> auto HasMethod_load(int) -> decltype(WS::decllval<T>().load(WS::decllval<pugi::xml_node>()) , std::true_type{});
	template<typename T> static bool constexpr HasMethod_load_v = decltype(HasMethod_load<T>(0))::value;

	/// <summary>
	/// _node::HasMethod_Save pr�ft ob es eine T::save-methode mit parameter pugi::xml_node nutzbar(public) gibt
	/// </summary>
	template<typename T> auto HasMethod_Save(unsigned long) -> std::false_type;
	template<typename T> auto HasMethod_Save(int) -> decltype(WS::decllval<T>().save(WS::decllval<pugi::xml_node const>()) , std::true_type{});
	template<typename T> static bool constexpr HasMethod_Save_v = decltype(HasMethod_Save<T>(0))::value;


	/// <summary>
	/// _node::Has_Load_ctor pr�ft ob es eine T::T (ctor) mit parameter pugi::xml_node nutzbar(public) gibt
	/// </summary>
	template<typename T> auto Has_Load_ctor(unsigned long) -> std::false_type;
	template<typename T> auto Has_Load_ctor(int) -> decltype( T{WS::decllval<pugi::xml_node>()},std::bool_constant< std::is_constructible<T,pugi::xml_node>::value && !std::is_trivially_constructible<T,pugi::xml_node>::value>{});
	template<typename T> static bool constexpr Has_Load_ctor_v = decltype(Has_Load_ctor<T>(0))::value;

	/// <summary>
	/// pugi-from-node-getter konvertiert vom text des datenknoten in den gew�nschten datentype 
	/// z.zt. implementierte Datentypen (sollte einfach erweiterbar sein)
	/// bool(wird als true/false gespeichert)
	/// Integrale 8 - 64 bit
	/// enum-datentypen die brauchen die funktionen stringto/tostring
	/// pugi::string_t oder ein assigneable-string 
	/// double presion 20
	/// float  presion 10
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="node"></param>
	/// <returns>der passende datentype</returns>
	template<typename T> auto getter(pugi::xml_node const & node)
	{
		if constexpr (Has_Load_ctor_v<T>)
		{
			return T{node};
		}
		else if constexpr (HasMethod_load_v<T>)
		{
			T t{};
			t.load(node);
			return t;
		}
		else if constexpr (std::is_enum<T>::value)
		{
			return stringto<T,pugi::char_t>(node.text().as_string());
		}
		else if constexpr (std::is_assignable_v<T,PUGIXML_CHAR const*>)
		{
			return static_cast<T>(node.text().as_string());
		}
		else if constexpr (std::is_unsigned_v<T>)
		{
			if constexpr (WS::bits<T>()==64)
				return node.text().as_ullong();
			else if constexpr (WS::bits<T>()==32)
				return node.text().as_uint();
			else if constexpr (WS::bits<T>()==16)
				return static_cast<unsigned __int16>(node.text().as_uint());
			else if constexpr (WS::bits<T>()==8)
				return static_cast<unsigned __int8>(node.text().as_uint());
		}
		else if constexpr (std::is_signed_v<T>)
		{
			if constexpr (WS::bits<T>()==64)
				return node.text().as_llong();
			else if constexpr (WS::bits<T>()==32)
				return node.text().as_int();
			else if constexpr (WS::bits<T>()==16)
				return static_cast<__int16>(node.text().as_int());
			else if constexpr (WS::bits<T>()==8)
				return static_cast<__int8>(node.text().as_int());
		}
	}
	template<> auto getter<bool>(pugi::xml_node const & node)
	{
		return node.text().as_bool();
	}
	template<> auto getter<pugi::string_t>(pugi::xml_node const & node)
	{
		return node.text().as_string();
	}
	template<> auto getter<double>(pugi::xml_node const & node)
	{
		return node.text().as_double();
	}
	template<> auto getter<float>(pugi::xml_node const & node)
	{
		return node.text().as_float();
	}


	template<typename T> bool setter( pugi::xml_node & node, T const & dest )
	{
		if constexpr (std::is_enum<T>::value)
		{
			return  node.text().set( tostring<TCHAR,T>(dest).get() );
		}
		else if constexpr (std::is_assignable_v<T,PUGIXML_CHAR const*>)
		{
			return node.text().set( static_cast<PUGIXML_CHAR const*>(dest) );
		}
		else if constexpr (std::is_integral_v<T>)
		{
			return node.text().set( dest );
		}
		else
		{
			static_assert(WS::dependent_false<T>,"spezialisierung f�r T fehlt");
		}
	}
	template<> bool setter<bool>( pugi::xml_node & node, bool const & dest )
	{
		return node.text().set(dest);
	}
	template<> bool setter<pugi::string_t>( pugi::xml_node & node, pugi::string_t const & dest )
	{
		return node.text().set(dest.c_str());
	}
	template<> bool setter<double>( pugi::xml_node & node, double const & dest )
	{
		std::locale mylocale{"C"};//formatierung sicherstellen
		pugistringstream ss;
		ss.imbue(mylocale);
		ss << std::hexfloat << dest;//als hex schreiben damit lesen sicher den selben wert liefert
		return node.text().set(ss.str().c_str());

		//vielleicht w�re es besser den floatingpoint-wert selbst in einen string zu wandeln
		//return node.text().set(dest,20);
	}
	template<> bool setter<float>(pugi::xml_node & node, float const & dest)
	{
		std::locale mylocale{"C"};//formatierung sicherstellen
		pugistringstream ss;
		ss.imbue(mylocale);
		ss << std::hexfloat << dest;//als hex schreiben damit lesen sicher den selben wert liefert
		return node.text().set(ss.str().c_str());

		//vielleicht w�re es besser den floatingpoint-wert selbst in einen string zu wandeln
		//return node.text().set(dest,10);
	}
}//namespace _node


template<typename T> bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name)
{
	if constexpr (_node::Has_Load_ctor_v<T>)
	{
		//ASSERT( stringcmp(name,dest.node_name())==0);
		return dest.load(container);
	}
	else 
	{
		if( auto node = container.child(name) )
		{
			dest = _node::getter<T>(node);
			return true;
		}
		return false;
	}
}

//T ben�tigt die (statische) methode node_name(), ansonsten muss der node-name als parameter mitgegeben werden
template<typename T> bool from_node( pugi::xml_node const & container, T &dest)
{
	return from_node( container, dest, dest.node_name() );
}

template<typename T> bool to_node( pugi::xml_node & container, T &dest, TCHAR const* name)
{
	if constexpr ( _node::HasMethod_Save_v<T> )
	{
		//wenn es die save-methode gibt, diese jetzt aufrufen
		return dest.save(container);
	}
	else
	{
		if( auto node = container.append_child( name ) )
			return _node::setter( node, dest );
		return false;
	}
}

//T ben�tigt die (statische) methode node_name(), ansonsten muss der node-name als parameter mitgegeben werden
template<typename T> bool to_node( pugi::xml_node & container, T &dest)
{
	return to_node( container, dest, dest.node_name() );
}