#pragma once
//Copyright (c) 2024 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 


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


namespace WS
{
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
	/// _node helper-funktionen für den xml import/export per pugi
	/// </summary>
	namespace _node
	{
		template<typename T> concept std_pair_type = std::is_same<T,std::pair<typename T::first_type,typename T::second_type>>::value;
		template<typename T> auto IsStdPair(unsigned long) -> std::false_type;
		template<std_pair_type T> auto IsStdPair(int) -> std::true_type;
		template<typename T> static bool constexpr IsStdPair_v = decltype(IsStdPair<T>(0))::value;


		template<typename T> concept std_basis_string_type = std::is_same<T,std::string>::value || std::is_same<T,std::wstring>::value;
		template<typename T> auto IsStdBasisString(unsigned long) -> std::false_type;
		template<std_basis_string_type T> auto IsStdBasisString(int) -> std::true_type;
		template<typename T> static bool constexpr IsStdBasisString_v = decltype(IsStdBasisString<T>(0))::value;

		template<typename T> concept container_type = 
			not IsStdBasisString_v<T> //std-strings wären sonst auch container, haben aber ihre spezialisierung
			&& requires(typename std::remove_cvref_t<T> c) 
			{ 
				{(void)c.begin()}; 
				{(void)c.end()}; 
				{(void)c.cbegin()}; 
				{(void)c.cend()};
				{(void)c.size()};
				//{(void)c.emplace_back( std::declval<typename decltype(c)::value_type>() )};//mit push_back ist std::string auch ein container, das soll er aber nicht
				{(void)c.push_back( std::declval<typename decltype(c)::value_type>() )};
			};
		template<typename T> auto IsContainer(unsigned long) -> std::false_type;
		template<container_type T> auto IsContainer(int) -> std::true_type;
		template<typename T> static bool constexpr IsContainer_v = decltype(IsContainer<T>(0))::value;


		/// <summary>
		/// _node::HasMethod_Load prüft ob es eine T::load-methode mit parameter pugi::xml_node nutzbar(public) gibt
		/// </summary>
		template<typename T> auto HasMethod_load(unsigned long) -> std::false_type;
		template<typename T> auto HasMethod_load(int) -> decltype(WS::decllval<T>().load(WS::decllval<pugi::xml_node>(),(PUGIXML_CHAR const*)nullptr) , std::true_type{});
		template<typename T> static bool constexpr HasMethod_load_v = decltype(HasMethod_load<T>(0))::value;

		/// <summary>
		/// _node::HasMethod_Save prüft ob es eine T::save-methode mit parameter pugi::xml_node nutzbar(public) gibt
		/// </summary>
		template<typename T> auto HasMethod_Save(unsigned long) -> std::false_type;
		template<typename T> auto HasMethod_Save(int) -> decltype(WS::decllval<T const>().save(WS::decllval<pugi::xml_node const>(),(PUGIXML_CHAR const*)nullptr) , std::true_type{});
		template<typename T> static bool constexpr HasMethod_Save_v = decltype(HasMethod_Save<T>(0))::value;


		/// <summary>
		/// _node::Has_Load_ctor prüft ob es eine T::T (ctor) mit parameter pugi::xml_node nutzbar(public) gibt
		/// </summary>
		template<typename T> auto Has_Load_ctor(unsigned long) -> std::false_type;
		template<typename T> auto Has_Load_ctor(int) -> decltype( T{WS::decllval<pugi::xml_node>(),(PUGIXML_CHAR const*)nullptr},std::bool_constant< std::is_constructible<T,pugi::xml_node,PUGIXML_CHAR const*>::value && !std::is_trivially_constructible<T,pugi::xml_node,PUGIXML_CHAR const*>::value>{});
		template<typename T> static bool constexpr Has_Load_ctor_v = decltype(Has_Load_ctor<T>(0))::value;

		/// <summary>
		/// pugi-from-node-getter konvertiert vom text des datenknoten in den gewünschten datentype 
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
				t.load(node,node.name());
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
		template<> auto getter<double>(pugi::xml_node const & node)
		{
			return node.text().as_double();
		}
		template<> auto getter<float>(pugi::xml_node const & node)
		{
			return node.text().as_float();
		}
		template<> auto getter<pugi::string_t>(pugi::xml_node const & node)
		{
			return node.text().as_string();
		}
	


		template<typename T> bool setter( pugi::xml_node & node, T const & dest )
		{
			if constexpr (std::is_enum<T>::value)
			{
				return  node.text().set( tostring<TCHAR>(dest).get() );
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
				static_assert(WS::dependent_false<T>,"spezialisierung für T fehlt");
			}
		}
		template<> bool setter<bool>( pugi::xml_node & node, bool const & dest )
		{
			return node.text().set(dest);
		}
		template<> bool setter<double>( pugi::xml_node & node, double const & dest )
		{
			std::locale mylocale{"C"};//formatierung sicherstellen
			pugistringstream ss;
			ss.imbue(mylocale);
			ss << std::hexfloat << dest;//als hex schreiben damit lesen sicher den selben wert liefert
			return node.text().set(ss.str().c_str());

			//vielleicht wäre es besser den floatingpoint-wert selbst in einen string zu wandeln
			//return node.text().set(dest,20);
		}
		template<> bool setter<float>(pugi::xml_node & node, float const & dest)
		{
			std::locale mylocale{"C"};//formatierung sicherstellen
			pugistringstream ss;
			ss.imbue(mylocale);
			ss << std::hexfloat << dest;//als hex schreiben damit lesen sicher den selben wert liefert
			return node.text().set(ss.str().c_str());

			//vielleicht wäre es besser den floatingpoint-wert selbst in einen string zu wandeln
			//return node.text().set(dest,10);
		}
		template<> bool setter<pugi::string_t>( pugi::xml_node & node, pugi::string_t const & dest )
		{
			return node.text().set(dest.c_str());
		}
		//zu diesem setter gibt es kein getter gegenstück, wer sollte den speicher anlegen/freigeben
		template<> bool setter<pugi::char_t const *>( pugi::xml_node & node, pugi::char_t const * const & dest )
		{
			return node.text().set(dest);
		}
		//zu diesem setter gibt es kein getter gegenstück, wer sollte den speicher anlegen/freigeben
		template<typename T, size_t size> bool setter( pugi::xml_node & node, T const (&dest)[size] )
		{
			return node.text().set(dest);
		}
	}//namespace _node


	template<typename T> bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name)
	{
		if constexpr (_node::HasMethod_load_v<T>)
		{
			//ASSERT( stringcmp(name,dest.node_name())==0);
			return dest.load(container,name);
		}
		else if constexpr (_node::IsContainer_v<T>)
		{
			for( auto child : container.children(name))
			{
				typename T::value_type value{};
				if( from_node<typename T::value_type>(child,value,name) )
					dest.push_back( value );
			}
			return true;
		}
		else if constexpr ( _node::IsStdPair_v<T> )
		{
			if( auto node = container.child(name) )
			{
				bool ret_v = true;

				ret_v |= from_node( node, dest.first, _T("_1") );
				ret_v |= from_node( node, dest.second, _T("_2") );

				return ret_v;
			}
			return false;
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

	template<typename T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name)
	{
		if constexpr ( _node::HasMethod_Save_v<T> )
		{
			//wenn es die save-methode gibt, diese jetzt aufrufen
			return dest.save(container,name);
		}
		else if constexpr (_node::IsContainer_v<T>)
		{
			for( auto value : dest )
			{
				to_node<typename T::value_type>(container,value,name);
			}
			return true;
		}
		else if constexpr ( _node::IsStdPair_v<T> )
		{
			if( auto node = container.append_child( name ) )
			{
				bool ret_v = true;

				ret_v |= to_node( node, dest.first, _T("_1") );
				ret_v |= to_node( node, dest.second, _T("_2") );

				return ret_v;
			}
			return false;
		}
		else
		{
			if( auto node = container.append_child( name ) )
				return _node::setter( node, dest );
			return false;
		}
	}
}
