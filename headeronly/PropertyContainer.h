#pragma once

#include <map>

//#include <any>
#include "boost\any.hpp"

//die PropertyList verwaltet Properties. Ein Property-Datentyp muss value_t definiert haben und ueber PropertyID<type>() muss ein constexpr PropertyID_type (z.zt. char const *) zu ermitteln sein
//usage siehe BasisUnitTests\UT_PropertyContainer.cpp

//PropertyID_type ist z.Zt. ein pointer, der zur laufzeit des programm konstant sein muss. typeid(T).name() scheint diese anforderung zu erfüllen
//sollte die Write/Read  funktionalität benutzt werden müssen 
//		ReadData(stream, std::unique_ptr<PropertyID_chartype []> & ) und
//			WriteData(stream, PropertyID_type & ) benutzbar sein
//		ReadData(stream,property_t::value_t &) und WriteData(stream,property_t::value_t const &) benutzbar sein
//		WriteData(stream,PropertyID_type)->ReadData(stream, std::unique_ptr<PropertyID_chartype []> & ) 
//			auch nach COMPILERVERSION-Änderung den gleichen Inhalt in PropertyID<property_t>() liefern
//			ob typeid(T).name() das leistet weiß ich nicht, wird sich zeigen. Bei compilerwechel, z.b. auf gcc, wird das sicher nicht funktionieren


namespace WS
{
	using PropertyID_chartype = char const;
	using PropertyID_type = PropertyID_chartype *;
	//using PropertyID_type = std::string;

	//namespace nsany = std;//vc\tools\msvc\14.11.25503\include\any(14): fatal error C1189: #error:  class any is only available with C++17.
	namespace nsany = boost;

	using PropertyList = std::map<PropertyID_type, nsany::any>;//vorsicht, key ist pointer, verglichen werden die adressen, nicht der inhalt
}

inline bool equPropertyID( WS::PropertyID_chartype * const & id1, WS::PropertyID_chartype * const & id2 )
{
	if( id1==nullptr || id2==nullptr )
		return false;
	return strcmp( id1, id2 ) == 0;
}
inline bool equPropertyID( std::string const & id1, std::string const & id2 )
{
	return  id1 == id2;
}

//mit dem define koennte man Properties in der minimalversion definieren
#define WP_PROPERTY_TYPE(type_name,type) \
	struct type_name \
	{ \
		using value_t = type; \
		/*static constexpr PropertyID_type PropertyID() { return #type_name; }  /*wenn vorhanden wird dieser aufruf statt typeid(T).name() benutzt*/ \
	};


//zu testzwecken
#define WP_PROPERTY_TYPE_WITH_PropertyID(type_name,type) \
	struct type_name \
	{ \
		using value_t = type; \
		static WS::PropertyID_type PropertyID() { return #type_name; }  /*wenn vorhanden wird dieser aufruf statt typeid(T).name() benutzt*/ \
		type_name() = delete; /*kein objekt dieser klasse anlegen lassen, wozu auch, stört aber auch nicht*/ \
	};

//aus https://stackoverflow.com/questions/30372941/check-if-a-class-has-a-static-member-function-of-a-given-signature
#define DEFINE_HAS_SIGNATURE(traitsName, funcName, signature)								\
	template <typename U> class traitsName													\
	{																						\
	private:																				\
		template<typename T, T> struct helper;												\
		template<typename T> static std::uint8_t  check(helper<signature, &funcName>*);     \
		template<typename T> static std::uint16_t check(...);								\
	public:																					\
		static																				\
		constexpr bool value = sizeof(check<U>(0)) == sizeof(std::uint8_t);					\
	}


namespace WS
{
	namespace dont_use_this
	{
		DEFINE_HAS_SIGNATURE(hasMemberFunction_PropertyID, T::PropertyID, WS::PropertyID_type (*)(void) ); //hat der zu pruefende typ die funktion PropertyID mit der angegebenen signatur. hier 'static PropertyID_type(void)'. constexpr wird scheinbar nicht ausgewertet, veraendert die signatur nicht
		//DEFINE_HAS_SIGNATURE(hasMemberFunction_PropertyID, T::PropertyID, WS::PropertyID_type (T::*)(void) const );//hat der zu pruefende typ die funktion PropertyID mit der angegebenen signatur. hier 'PropertyID_type(void) const' der Unterschied ist T::. ohne muss es eine statische methode oder funktion sein

		template<typename property_t, bool =hasMemberFunction_PropertyID<property_t>::value> struct PropertyID
		{
			PropertyID_type operator() ()//constexpr wird hier ignoriert
			{
				return property_t::PropertyID();
			}
		};
		template<typename property_t> struct PropertyID<property_t,false>
		{
			PropertyID_type operator() ()//constexpr wird hier ignoriert
			{
				return	typeid(property_t).name();	//die implementierung muss immer die gleiche adresse liefern, sonst funktioniert die PropertyList = std::map<char const */*PropertyID_type*/, nsany::any> nicht
													//und immer den gleichen string, auch in neuen compilerversionen, sonst funktioniert speichern/laden
			}
		};
	}
	template<typename property_t> constexpr PropertyID_type PropertyID()
	{
		return dont_use_this::PropertyID<property_t>()();
	}

	//*************************************
	//hier sind die interesanten funktionen
	//*************************************
	template<typename property_t> typename bool								Exists( PropertyList const & propertylist )
	{
		auto iter = propertylist.find( PropertyID<property_t>() );
		return iter != propertylist.end();
	}
	template<typename property_t> typename void								Erase( PropertyList & propertylist )
	{
		auto iter = propertylist.find( PropertyID<property_t>() );
		if(iter != propertylist.end() )
			propertylist.erase( iter );
	}

	template<typename property_t> typename property_t::value_t const &		GetPropertyValue( PropertyList const & propertylist )
	{
		auto iter = propertylist.find( PropertyID<property_t>() );
		if( iter != propertylist.end() )
			return nsany::any_cast<property_t::value_t const &>( iter->second );//wenn es den eintrag nicht gibt, oder falscher cast, fliegt exception
		
		throw std::invalid_argument( "Property nicht in der PropertyListe enthalten: " __FUNCSIG__ );
	}
	template<typename property_t> typename property_t::value_t &			GetPropertyValue( PropertyList& propertylist )
	{
		return const_cast<property_t::value_t &>( GetPropertyValue<property_t>( static_cast<PropertyList const &>(propertylist) ) );
	}
	template<typename property_t> typename property_t::value_t *			GetPropertyValue_pointer( PropertyList& propertylist ) noexcept
	{
		auto iter = propertylist.find( PropertyID<property_t>() );
		if( iter == propertylist.end() )
			return nullptr;
		return nsany::any_cast<property_t::value_t>( &(iter->second) );
	}
	template<typename property_t> typename property_t::value_t const *		GetPropertyValue_pointer( PropertyList const& propertylist ) noexcept
	{
		auto iter = propertylist.find( PropertyID<property_t>() );
		if( iter == propertylist.end() )
			return nullptr;
		return nsany::any_cast<property_t::value_t>( &(iter->second) );
	}

	template<typename property_t> typename void								SetPropertyValue( PropertyList& propertylist, typename property_t::value_t const & value )
	{
		propertylist[PropertyID<property_t>()] = value;
	}
	template<typename property_t> typename void								SetPropertyValue( PropertyList& propertylist, typename property_t::value_t && value )
	{
		propertylist[PropertyID<property_t>()] = std::move( value );
	}
}

//laden und speichern einer PropertyList
//usage siehe BasisUnitTests\UT_PropertyContainer.cpp 
//die benötigten ReadData und WriteData muesst ihr schon selbst bereit stellen. siehe BasisUnitTests\UT_PropertyContainer.cpp oder bbobjid.hpp
namespace WS 
{
	//WriteProperty schreibt nur etwas, wenn das Property vorhanden ist. Laden koennte etwas schwierig werden
	//also lieber anwender, benutze stattdessen WriteProperties, wenn du nicht genau weiß wie es funktioniert
	template<typename stream_t, typename property_t>	void				WriteProperty( stream_t stream, PropertyList const & propertylist ) 
	{
		if( Exists<property_t>( propertylist ) )
		{
			WriteData( stream, PropertyID<property_t>() );
			WriteData( stream, GetPropertyValue<property_t>(propertylist ) );
		}
	}
	namespace dont_use_this
	{
		template<typename stream_t, size_t count, typename first_property_t, typename ... rest_property_t> struct WritePropertyHelper
		{
			WritePropertyHelper( stream_t stream, PropertyList const & propertylist )
			{
				WriteProperty<stream_t,first_property_t>( stream, propertylist );
				WritePropertyHelper<stream_t,count-1,rest_property_t ...>( stream, propertylist );
			}
		};
		template<typename stream_t, typename first_property_t, typename ... rest_property_t> struct WritePropertyHelper<stream_t, 0, first_property_t, rest_property_t ...>
		{
			WritePropertyHelper( stream_t stream, PropertyList const & propertylist )
			{
				WriteProperty<stream_t,first_property_t>( stream, propertylist );
				
				WriteData( stream, static_cast<PropertyID_type>("") );//endekennung ist char const * = nullptr
			}
		};
	}
	template<typename stream_t, typename first_property_t, typename ... rest_property_t>
	void																	WriteProperties( stream_t stream, PropertyList const & propertylist )
	{
		dont_use_this::WritePropertyHelper<stream_t,sizeof...(rest_property_t), first_property_t, rest_property_t...>( stream, propertylist );//kann das ohne parameter nur ueber spezialisierung auflösen
	}
	
	namespace dont_use_this
	{
		template<typename stream_t, typename property_t>	bool				ReadProperty( stream_t stream, PropertyList & propertylist, PropertyID_type type_name ) 
		{
			if( equPropertyID(WS::PropertyID<property_t>(), type_name) )
			{
				property_t::value_t value;
				ReadData( stream, value );
				propertylist[PropertyID<property_t>()()] = std::move(value);
				//propertylist[type_name] = std::move(value);// [type_name] toedlich, adressen sind nicht gleich
				return true;
			}
			return false;
		}
	}
	//laden. vorsicht, das property muss auch wirklich an der stelle gespeichert worden sein (siehe kommentar zu WriteProperty)
	//es ist deutlich sicherer ReadProperties zu benutzen
	template<typename stream_t, typename property_t>	bool				ReadProperty( stream_t stream, PropertyList & propertylist ) 
	{
		std::unique_ptr< PropertyID_chartype [] > propertynamePtr;
		ReadData( stream, propertynamePtr );

		return dont_use_this::ReadProperty<stream_t, property_t>( stream, propertylist, propertynamePtr.get() );
	}	
	
	namespace dont_use_this
	{
		//ReadPropertyHelper sucht in den templteparametern den richtigen PropertyTyp
		template<typename stream_t, size_t count, typename first_property_t, typename ... rest_property_t> struct ReadPropertyHelper
		{
			bool value;
			ReadPropertyHelper( stream_t stream, PropertyList & propertylist, PropertyID_type type_name )
			{
				if( ReadProperty<stream_t,first_property_t>( stream, propertylist, type_name ) )
					value = true;
				else
					value = ReadPropertyHelper<stream_t,count-1,rest_property_t ...>( stream, propertylist, type_name ).value;
			}
		};
		template<typename stream_t, typename first_property_t, typename ... rest_property_t> struct ReadPropertyHelper<stream_t, 0, first_property_t, rest_property_t ...>
		{
			bool value;
			ReadPropertyHelper( stream_t stream, PropertyList & propertylist, PropertyID_type type_name )
			{
				value = ReadProperty<stream_t,first_property_t>( stream, propertylist, type_name );
			}
		};
	}
	//laden. Alle Properties die gespeichert wurden, müssen auch geladen werden, da ein ueberspringen nicht möglich ist
	//wenn ein fehlerauftritt liefert die funktion false. Vorsicht, die leseposition im stream ist ziemlich sicher veraendert
	template<typename stream_t, typename first_property_t, typename ... rest_property_t>
	bool																	ReadProperties( stream_t stream, PropertyList & propertylist )
	{
		bool value = true;
		while( value )//bis zum listen ende alle properties lesen
		{
			//id des naechsten property laden
			std::unique_ptr< PropertyID_chartype[] > PropertyID; 
			ReadData( stream, PropertyID );

			if( equPropertyID( PropertyID.get(),static_cast<PropertyID_type>("") ) )
				return value;//listenende
			else
				//suche property in templateparametern
				value = dont_use_this::ReadPropertyHelper<stream_t, sizeof...(rest_property_t), first_property_t, rest_property_t ...>( stream, propertylist, PropertyID.get() ).value;
		}
		return value;
	}
}

