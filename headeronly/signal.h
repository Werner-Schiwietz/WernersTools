#pragma once
///Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

//WS::Signal ähnlich boost::Signals2::Signal

//header 
//headeronly\signal.h				diese datei
//headeronly\mutex_automicflag.h	mutex fuer loak_guard nutzt atomic_flag
//headeronly\combiner_last.h		default_combiner
//evtl. noch
//headeronly\combiner.h	//optinal. da sind noch andere combiner, die man evtl. brauchen könnte

//usage siehe UT_signal.cpp
//	auto signal = WS::Signal<signatur[,combiner_t]>{}
//	auto connection_guard = signal.connect(funktion_or_[,priority]);
//	combiner_t erg = signal([funktions_parameter[,...]]); 
//
//	auto event_sender = WS::Signal<int(int,char const *, signatur_parameter...){,combiner_type]>{};//default combiner ist WS::combiner_last. Der liefert den Rückgabewert der letzten gerufenen Funktion siehe combiner.h und combiner_last.h
//	auto connection_guard = event_sender.connect(function_pointer[,prio]);//oder alles was in std::function<ret_t(params...)> geht
//		//prio im Bereich 1..0xff wobei 0xff default-prio mit geringeter priorität ist. Damit kann die Reihenfolge der fFunktionsaufrufe gesteuert werden
//	auto combiner = event_sender(signatur_parameter...);
/*
TEST_METHOD(UT_demo)
{
	using signal_t = WS::Signal<bool(std::string const &, bool&)>;
	auto  signal = signal_t{};//anlegen des signal-objekt
		
	struct SignalUser //callback-objekt mit autolink zum signal
	{
		using signal_t = signal_t;
		signal_t::Connection_Guard connection;
		SignalUser(SignalUser const &) = delete;//um sicher zu gehen, dass keine kopie angelegt werden kann
		SignalUser( signal_t  & signal ) {this->connection = signal.connect(std::reference_wrapper(*this));}//reference_wrapper sonst wuerde kopie von this angelegt werden

		bool operator()(std::string const & value,bool & verarbeitet)//diese funktion wird von signal gerufen
		{
			Logger::WriteMessage( (std::string(__FUNCTION__ " value:'") + value + "' verarbeitet=" + (verarbeitet?"true":"false")).c_str() );
			#pragma warning(suppress:6282)
			return verarbeitet=true;//als verarbeite kennzeichnen
		}
	} signaluser{signal};//verknüpft signal im ctor per connect mit signaluser 

	bool verarbeitet = false;
	auto combiner_wenn_von_interesse = signal("hallo", verarbeitet);//ruft alle verbundenen funktionen
	Assert::IsTrue(verarbeitet);
	Assert::IsTrue(combiner_wenn_von_interesse());// liefert ergebnis des default-combiner combiner_last<bool>
}
*/

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

		struct id_type//gibt den bits der id_t besondere bedeutung
		{
			using id_t								= id_t;
			using prio_t							= unsigned __int8;
			static prio_t constexpr default_prio	= 0xff;//ist schwächste prio

			union 
			{
				id_t _id;
				struct parts_t
				{
					id_t lfdnr		: sizeof(id_t)*8 - 8;
					id_t prio		: 8;
				}parts;
			};


			id_type(id_t nr,unsigned __int8 prio=0xff):_id(nr) //stärkste prio 1 -> 2 -> 3 -> ... ff  schwach, 0 invalid
			{ 
				if( this->parts.prio == 0 )
				{
					if( prio==0 )
						throw std::length_error( __FUNCTION__ " prio ist 0");
					this->parts.prio = prio;
				}
				else if( prio!=0xff )
					throw std::length_error( __FUNCTION__ " prio ist nicht defaultwert");
			}

			operator id_t const &() const &	// lfdnr + prio
			{
				return this->_id;
			}
			id_t id() const
			{
				return this->parts.lfdnr;
			}
			id_t id_offset(int offset) const
			{
				decltype(this->parts) _parts{this->parts.lfdnr + offset};//bitfield überlauf 0xfff'fff + 1 soll 0 werden nicht 0x1'000'000
				return _parts.lfdnr;
			}
			prio_t prio() const //stärkste prio 1 -> 2 -> 3 -> ... ff  schwach, 0 invalid
			{
				return this->parts.prio;
			} 
		};
		using prio_t = typename id_type::prio_t;
		#pragma region Guards
		struct _Guard
		{	//_Guard für Connection_ und Block_ 
			using Signal_t=Signal<return_type(parameter_types...),combiner_t>;
			id_t					id		= 0;
			Signal_t *				signal	= nullptr;
			_Guard() noexcept {}
			_Guard(_Guard const &) = delete;
			_Guard(_Guard && r) noexcept {swap(r);}
			_Guard& operator=(_Guard const &)& = delete;
			_Guard& operator=(_Guard && r ) & noexcept {_Guard{std::move(r)}.swap(*this);return *this;}

			_Guard(Signal_t* signal, id_t id) : signal(signal), id(id){}

			void swap(_Guard & r)
			{
				std::swap(this->id, r.id);
				std::swap(this->signal, r.signal);
			}
			operator bool(){return this->signal && id;}
		};
		struct Connection_Guard : _Guard
		{	//Connection_Guard entfernt automatisch im dtor die callback-funktion aus der Signal-Funktionspointer-Verwaltung
			//using _Guard::_Guard;
			Connection_Guard(){}
			Connection_Guard(Connection_Guard const &) = delete;
			Connection_Guard(Connection_Guard && r) noexcept : _Guard(std::move(r)){}
			Connection_Guard& operator=(Connection_Guard const &) & = delete;
			Connection_Guard& operator=(Connection_Guard && r ) & noexcept { Connection_Guard{std::move(r)}.swap(*this);return *this; }
			Connection_Guard(Signal_t* signal, id_t id) noexcept : _Guard(signal, id){}

			~Connection_Guard(){try{disconnect();}catch(...){}}//ohne try ggf app-abort

			[[nodiscard]]
			id_t release(){signal=nullptr;return id;}//der auf aufrufer verantwortet den disconnect selbst
			void disconnect();
		};
		struct Block_Guard : _Guard
		{	//Block_Guard entfernt automatisch im dtor die die Blockade für den slot 
			using _Guard::_Guard;
			Block_Guard(){}
			Block_Guard(Block_Guard const &) = delete;
			Block_Guard(Block_Guard && r) noexcept : _Guard(std::move(r)){}
			Block_Guard& operator=(Block_Guard const &) & = delete;
			Block_Guard& operator=(Block_Guard && r ) & noexcept { Block_Guard{std::move(r)}.swap(*this);return *this; }
			Block_Guard(Signal_t* signal, id_t id) : _Guard(signal, id){}


			~Block_Guard(){try{unblock();}catch(...){}}//ohne try ggf app-abort

			[[nodiscard]]
			id_t release(){signal=nullptr;return id;}//der auf aufrufer verantwortet den disconnect selbst
			void unblock();
		};
		#pragma endregion
		Signal( ) {}
		auto operator()( parameter_types... args)
		{
			std::lock_guard<decltype(locker)> const lock{locker};
			if constexpr(std::is_same<return_t,void>::value==false)
			{
				combiner_type combiner{};
				for(auto &[prio,callbacks] : this->prio_callbacks)
				{
					for( auto & [id, fn_item] : callbacks )
					{
						if( fn_item.blocked==false ) 
							combiner( fn_item.fn(args...), id_type{id,prio} );									
						//combiner( fn( std::forward<parameter_types>( args )... ), id_type{id,prio} );	//mit std::forward schlägt UT_Signal_in_class_using fehl, wegen std::string als parameter der moved würde
					}
				}
				return combiner;
			}
			else for(auto &[prio,callbacks] : this->prio_callbacks)
			{
				for( auto & [id, fn_item] : callbacks )
				{
					if( fn_item.blocked==false )
						fn_item.fn( args... );
					//fn( std::forward<parameter_types>( args )... );//mit std::forward schlägt UT_Signal_in_class_using fehl, wegen std::string als parameter der moved würde
				}
			}
		}

		template<typename fn_in_t> [[nodiscard]] Connection_Guard	connect( fn_in_t fn, typename id_type::prio_t prio = id_type::default_prio ) //für nutzer wie fn_in_t::operator()(...)  evtl connect(std::reference_wrapper(*this)) verwenden, da 'fn' immer kopiert wird
		{ 
			std::lock_guard<decltype(locker)> const lock{locker};
			auto id = next_id(prio);
			prio_callbacks[id.prio()][id.id()] = {fn};
			return {this,id};
		}
		void														disconnect( id_t id )
		{
			auto parts = id_type(id);
			std::lock_guard<decltype(locker)> const lock{locker};

			if( prio_callbacks[parts.prio()].erase(parts.id()) != 1 )
				throw std::invalid_argument(__FUNCTION__ " id war ungültig");
		}
		[[nodiscard]] Block_Guard									block( Connection_Guard const & connection)
		{
			std::lock_guard<decltype(locker)> const lock{locker};
			if( connection.signal!=this )throw std::runtime_error(__FUNCTION__ " connection.signal!=this");

			auto id = id_type{connection.id};
			auto & callbacks = prio_callbacks[id.prio()];
			auto iter = callbacks.find(id.id());
			if( iter != callbacks.end() )
			{
				if( iter->second.blocked==false )
				{
					iter->second.blocked = true;
					return{this,id};
				}
			}
			return {};
		}
		void														unblock( id_t id)
		{
			std::lock_guard<decltype(locker)> const lock{locker};

			auto id_blocked = id_type{id};
			auto & callbacks = prio_callbacks[id_blocked.prio()];
			auto iter = callbacks.find(id_blocked.id());
			if( iter != callbacks.end() )
			{
				if( iter->second.blocked==false )
					throw std::runtime_error(__FUNCTION__ " nicht geblocked");
				iter->second.blocked = false;
			}
		}

		struct fn_item
		{
			fn_t fn;
			bool blocked=false;
		};
		std::map<typename id_type::prio_t,std::map<typename id_type::id_t,fn_item>> prio_callbacks;//Funktionspointer-Verwaltung
	private:
		id_type next_id(typename prio_t prio)
		{
			if(locker.islocked()==false)
				throw std::runtime_error(__FUNCTION__ " lock fehlt");

			id_t id = 1;
			auto const & callbacks = prio_callbacks[prio];
			auto iter = callbacks .rbegin();
			if( iter!=callbacks.rend())
				id = id_type{(*iter).first}.id_offset(1);//bei 0xfff'fff evtl. ueberlauf
			if( id==0 )//überlauf? id0 ist invalid
			{
				id = 1;
				for( auto [used_id, fn_item] : callbacks )//lücke suchen
				{
					if( id==used_id && fn_item.fn )
					{
						if( ++id==0 )
							throw std::runtime_error( "zuviele signal-user" );
					}
					else
						break;
				}
			}
			return {id,prio};
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
	template<typename ret_t,typename...args,typename combiner_t> void Signal<ret_t(args...),combiner_t>::Block_Guard::unblock( )
	{
		if( this->signal )
		{
			this->signal->unblock(this->id);
			this->signal=nullptr;
		}
	}
}