#pragma once
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

#include "mutex_automicflag.h"
#include "combiner_last.h"


#include <functional>
#include <map>

namespace WS
{
	template<typename signatur>struct Signal_trait;
	template<typename return_type, typename ... parameter_types>struct Signal_trait<return_type(parameter_types...)>
	{
		using signatur_t		= return_type(parameter_types...);
		using return_t          = return_type;
		using tuple_t			= std::tuple<parameter_types...>;
		using fn_t              = std::function<return_type(parameter_types...)>;
		using id_t				= size_t;
	};

	template<typename signatur,typename combiner_t=combiner_last<Signal_trait<signatur>::return_t>>struct Signal;
	template<typename return_type, typename ... parameter_types,typename combiner_type>struct Signal<return_type(parameter_types...),typename combiner_type> : Signal_trait<return_type(parameter_types...)>
	{
		using combiner_t=combiner_type;
		struct Connection_Guard 
		{	//Connection_Guard entfernt automatisch im dtor die callback-funktion aus der Signal-Funktionspointer-Verwaltung
			using Signal_t=Signal<return_type(parameter_types...),combiner_t>;
			id_t					id		= 0;
			Signal_t *				signal	= nullptr;
			~Connection_Guard(){disconnect();}
			Connection_Guard(){}
			Connection_Guard(Connection_Guard const &) = delete;
			Connection_Guard(Connection_Guard && r) {swap(r);}
			Connection_Guard& operator=(Connection_Guard const &)& = delete;
			Connection_Guard& operator=(Connection_Guard && r )&{Connection_Guard{std::move(r)}.swap(*this);return *this;}

			Connection_Guard(Signal_t* signal, id_t id) : signal(signal), id(id){}

			void swap(Connection_Guard & r)
			{
				std::swap(this->id, r.id);
				std::swap(this->signal, r.signal);
			}

			[[nodiscard]]
			id_t release(){signal=nullptr;return id;}//der auf aufrufer verantwortet den disconnect selbst
			void disconnect();
		};

		Signal( ) {}
		template<typename fn_in_t> [[nodiscard]] Connection_Guard connect( fn_in_t fn ) //für nutzer wie fn_in_t::operator()(...)  evtl connect(std::reference_wrapper(*this)) verwenden, da 'fn' immer kopiert wird
		{ 
			std::lock_guard<decltype(locker)> const lock{locker};
			auto id = next_id();
			callbacks[id] = fn;
			return {this,id};
		}
		auto operator()( parameter_types... args)
		{
			std::lock_guard<decltype(locker)> const lock{locker};
			if constexpr(std::is_same<return_t,void>::value==false)
			{
				combiner_type combiner{};
				for(auto &[id,fn] : this->callbacks)
					combiner( fn(args...), id );
				//return_value = fn(std::forward<parameter_types>(args)...);
				return combiner;
			}
			else for(auto & [id,fn] : this->callbacks)
				fn(args...);
		}
		void disconnect( id_t id )
		{
			std::lock_guard<decltype(locker)> const lock{locker};
			if( callbacks.erase(id) != 1 )
				throw std::invalid_argument(__FUNCTION__ " id war ungültig");
		}

		std::map<id_t,fn_t> callbacks;//Funktionspointer-Verwaltung
	private:
		id_t next_id()
		{
			if(locker.islocked()==false)
				throw std::runtime_error(__FUNCTION__ " lock fehlt");

			id_t id = 1;
			auto iter = callbacks.rbegin();
			if( iter!=callbacks.rend())
				id = (*iter).first + 1;
			if( id==0 )//überlauf? id0 ist invalid
			{
				id = 1;
				for( auto [used_id, callback] : callbacks )//lücke suchen
				{
					if( id==used_id && callback )
					{
						if( ++id==0 )
							throw std::runtime_error( "zuviele signal-user" );
					}
					else
						break;
				}
			}
			return id;
		}
		mutex_atomicflag	locker {};	
	};

	template<typename ret_t,typename...args,typename combiner_t> void Signal<ret_t(args...),combiner_t>::Connection_Guard::disconnect( )
	{
		if( this->signal )
		{
			this->signal->disconnect(this->id);
			this->signal=nullptr;
		}
	}
}