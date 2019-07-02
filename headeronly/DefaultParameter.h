#pragma once
#error "Bitte keinen include machen. headeronly\DefaultParameter.h enthaelt keinen ausführbaren code, nur Infos" 

#include <tuple>


namespace WP
{
	//wenn eine funktion mehr als einen defaultparameter hat, wird es in C++ laestig
	//hier ein versuch von n default-werten bei bedarf nur genau die angeben zu müssen, die geaendert werden sollen
	//z.b. usage
	//definition void funktion( WP::VorlageZumKopieren defaultParam=WP::VorlageZumKopieren() );
	//aufruf	1. funktion(); //nutzt die default-werte
	//			2. funktion( WP::VorlageZumKopieren().Set<VorlageZumKopieren::N::Name>( value1 ).Set<VorlageZumKopieren::N::Hausnummer>(5));//veraendert 1. und 3. default-wert. 2. bleibt default. N:: koennte auch weggelassen werden, da es kein enum class ist
	//
	//
	//Kopievorlage des templateparameter. hier BITTE NIE aendern. Kopieren und an anderer Stelle Anpassungen machen
	struct VorlageZumKopieren											//!!!!!!!!!!!!!!!eigenen namen vergeben
	{
		using this_t = VorlageZumKopieren;								//!!!!!!!!!!!!!!!eigenen namen 
		enum //class
		N {
								Name, Strasse, Hausnummer				//!!!!!!!!!!!!!!!!namen fuer zugriff
		};	
		std::tuple<
								char const *, char const *, int			//!!!!!!!!!!!!!!!!datentypen der werte. reihenfolge muss der des enum N entsprechen
		>values;
		this_t() : values( 
								"<Name>", "<Straße>", 0					//!!!!!!!!!!!!liste der defaultwerte in der reihenfolge des enum N
		){}

		//hier nichts aendern
		template<this_t::N index> auto & get()& //referenz_qualifier bringt hier keinen nutzen, tut aber auch nicht weh
		{
			return std::get<static_cast<std::size_t>(index)>(this->values);
		}
		template<this_t::N index> auto const & get() const & //referenz_qualifier bringt hier keinen nutzen, tut aber auch nicht weh
		{
			return std::get<static_cast<std::size_t>(index)>(this->values);
		}
		template<this_t::N index> auto && get() && //referenz_qualifier bringt hier keinen nutzen, tut aber auch nicht weh
		{
			return std::move(std::get<static_cast<std::size_t>(index)>(this->values));
		}
		template<this_t::N index, typename value_t> this_t & set( value_t && value ) &
		{
			get<index>() = std::forward<value_t>(value);
			return *this;
		}
		template<this_t::N index, typename value_t> this_t && set( value_t && value ) &&
		{
			get<index>() = std::forward<value_t>(value);
			return std::move(*this);
		}
		//hier nichts aendern
	};	
}
#endif