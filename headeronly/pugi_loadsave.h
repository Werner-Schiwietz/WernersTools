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
#		define __T(x) L ## x
#		define _T(x) __T(x)
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
	template <typename first_t, typename...rest_t, size_t... indices>
	auto _get_rest(std::tuple<first_t, rest_t...>& tuple, std::index_sequence<indices...>)
	{
		return std::tie(std::get<indices + 1>(tuple)...);
	}
	template <typename first_t, typename...rest_t, size_t... indices>
	auto _get_rest(std::tuple<first_t, rest_t...> const & tuple, std::index_sequence<indices...>)
	{
		return std::tie(std::get<indices + 1>(tuple)...);
	}

	/// <summary>
	/// get_rest() liefert einen tuple ohne das erste element, also um ein element verkürzten tuple mit referenzen auf den orginal-tuple
	/// MSC könnte auch std::tuple::_Get_rest() nutzen
	/// </summary>
	/// <typeparam name="first_t"></typeparam>
	/// <typeparam name="...rest_t"></typeparam>
	/// <param name="tuple"></param>
	/// <returns></returns>
	template <typename first_t, typename...rest_t> auto
	get_rest(std::tuple<first_t, rest_t...>& tuple)
	{
		return _get_rest(tuple, std::make_index_sequence<sizeof...(rest_t)>());
	}
	template <typename first_t, typename...rest_t> auto
		get_rest(std::tuple<first_t, rest_t...> const & tuple)
	{
		return _get_rest(tuple, std::make_index_sequence<sizeof...(rest_t)>());
	}
}

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

		template <typename> struct is_tuple : std::false_type {};
		template <typename ...T> struct is_tuple<std::tuple<T...>> : std::true_type {};
		template<typename T> concept std_tuple_type = is_tuple<T>::value;

		template<typename T> concept std_basis_string_type = std::is_same<T,std::string>::value || std::is_same<T,std::wstring>::value;
		template<typename T> auto IsStdBasisString(unsigned long) -> std::false_type;
		template<std_basis_string_type T> auto IsStdBasisString(int) -> std::true_type;
		template<typename T> static bool constexpr IsStdBasisString_v = decltype(IsStdBasisString<T>(0))::value;

		template<typename T> concept container_type = 
			not IsStdBasisString_v<T> //std-strings wären sonst auch container, die haben aber ihre spezialisierung
			&& requires(typename std::remove_cvref_t<T> c) 
			{ 
				{(void)c.begin()}; 
				{(void)c.end()}; 
				{(void)c.cbegin()}; 
				{(void)c.cend()};
				{(void)c.size()};
				{(void)c.push_back( std::declval<typename decltype(c)::value_type>() )};
			};
		template<typename T> auto IsContainer(unsigned long) -> std::false_type;
		template<container_type T> auto IsContainer(int) -> std::true_type;
		template<typename T> static bool constexpr IsContainer_v = decltype(IsContainer<T>(0))::value;

		template<typename T> concept map_type = 
			not IsStdBasisString_v<T> //std-strings wären sonst auch container, die haben aber ihre spezialisierung
			&& requires(typename std::remove_cvref_t<T> m) 
		{ 
			{(void)m.begin()}; 
			{(void)m.end()}; 
			{(void)m.cbegin()}; 
			{(void)m.cend()};
			{(void)m.size()};
			{(void)m.insert( std::declval<typename decltype(m)::value_type>() )};
		};
		template<typename T> auto IsMap(unsigned long) -> std::false_type;
		template<map_type T> auto IsMap(int) -> std::true_type;
		template<typename T> static bool constexpr IsMap_v = decltype(IsMap<T>(0))::value;


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
		/// liefert node wenn dessen name name ist, oder dessen erstes child mit name name
		/// </summary>
		/// <param name="node"></param>
		/// <param name="name"></param>
		/// <returns></returns>
		auto nodename( pugi::xml_node const & node, PUGIXML_CHAR const* name )
		{
			if( stringcmp(node.name(),name)==0 )
				return node;
			else
				return node.child(name);
		}

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
				//return false;
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


	 //forward specialisierungen, die können sich kreuz und quer gegenseitig aufrufen
	template<_node::container_type T>	bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name);
	template<_node::map_type T>			bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name);
	template<_node::std_tuple_type T>	bool from_node(pugi::xml_node const & container, T &dest, TCHAR const* name);
	template<_node::std_pair_type T>	bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name);

	template<typename T> bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name)
	{
		if constexpr (_node::HasMethod_load_v<T>)
		{
			//ASSERT( stringcmp(name,dest.node_name())==0);
			return dest.load(container,name);
		}
		else 
		{
			if( auto node = _node::nodename(container,name) )
			{
				dest = _node::getter<T>(node);
				return true;
			}
			return false;
		}
	}
	template<_node::std_pair_type T> bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name)
	{
		bool ret_v = false;
		if( auto node = _node::nodename(container,name) )
		{
			ret_v = true;
			ret_v |= from_node( node, dest.first, _T("_1") );
			ret_v |= from_node( node, dest.second, _T("_2") );

		}
		return ret_v;
	}
	template<_node::container_type T> bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name)
	{
		dest.clear();
		for( auto child : container.children(name))
		{
			typename T::value_type value{};
			if( from_node<typename T::value_type>(child,value,name) )
				dest.push_back( value );
		}
		return true;
	}
	template<_node::map_type T>			bool from_node( pugi::xml_node const & container, T &dest, TCHAR const* name)
	{
		dest.clear();
		if( auto node = _node::nodename(container,name) )
		{
			auto valuename = _T("mapvalue");
			for( auto child : node.children(valuename) )
			{
				std::pair<typename T::key_type,typename T::mapped_type> value{};
				if( from_node(child,value,valuename) )
					dest.insert( value );
			}
			return true;
		}
		return false;
	}
	template<_node::std_tuple_type T> bool _from_node_tuple_helper(pugi::xml_object_range<pugi::xml_named_node_iterator> nodes, T &dest, TCHAR const* name)
	{
		if( nodes.begin() != nodes.end() )
		{
			bool ret_v = from_node( *nodes.begin(), std::get<0>(dest), name );

			if constexpr ( std::tuple_size_v<T> > 1  )
			{
				nodes = decltype(nodes){ ++nodes.begin(), nodes.end() };
				auto dest2 = WS::get_rest(dest);
				ret_v |= _from_node_tuple_helper(nodes, dest2, name);
			}
			return ret_v;
		}

		return false;
	}
	template<_node::std_tuple_type T> bool from_node(pugi::xml_node const & container, T &dest, TCHAR const* name)
	{
		pugi::xml_node node = _node::nodename(container,name);
		if( node != container )
			dest = T{};

		if( node )
		{
			return _from_node_tuple_helper( node.children(_T("value")), dest, _T("value") );
		}
		return false;
	}


	//forward specialisierungen, die können sich kreuz und quer gegenseitig aufrufen
	template<_node::std_pair_type T>  bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name);
	template<_node::std_tuple_type T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name);
	template<_node::container_type T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name);
	template<_node::map_type T>		  bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name);

	template<typename T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name)
	{
		if constexpr ( _node::HasMethod_Save_v<T> )
		{
			//wenn es die save-methode gibt, diese jetzt aufrufen
			return dest.save(container,name);
		}
		else
		{
			if( auto node = container.append_child( name ) )
				return _node::setter( node, dest );
			return false;
		}
	}
	template<_node::std_pair_type T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name)
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
	template<_node::std_tuple_type T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name)
	{
		//static_assert(false,"not implemented");
		pugi::xml_node node;
		if( stringcmp(container.name(),name)==0 )
			node = container;
		else
		{
			node = container.append_child( name );
		}

		if( node )
		{
			bool ret_v = to_node( node, std::get<0>(dest), _T("value")) ;
			if constexpr ( std::tuple_size_v<T> > 1 )
				ret_v |= to_node( node, WS::get_rest(dest), name) ;

			return ret_v;
		}
		return false;
	}
	template<_node::container_type T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name)
	{
		for( auto value : dest )
		{
			to_node<typename T::value_type>(container,value,name);
		}
		return true;
	}
	template<_node::map_type T> bool to_node( pugi::xml_node & container, T const &dest, TCHAR const* name)
	{
		if( auto node = container.append_child( name ) )
		{
			auto valuename = _T("mapvalue");
			for( auto value : dest )
			{
				to_node<typename T::value_type>(node,value,valuename);
			}
			return true;
		}
		return false;
	}

}
