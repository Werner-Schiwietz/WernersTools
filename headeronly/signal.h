#pragma once

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
		std::map<id_t,fn_t> callbacks;//Funktionspointer-Verwaltung

		Signal( ) {}
		template<typename fn_in_t> 
		[[nodiscard]]
		Connection_Guard connect( fn_in_t fn ) //für nutzer wie fn_in_t::operator()(...)  evtl connect(std::reference_wrapper(*this)) verwenden, da 'fn' immer kopiert wird
		{ 
			std::lock_guard<decltype(locker)> const lock{locker};
			id_t id = ++last_id; 
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
			if( id )
			{
				std::lock_guard<decltype(locker)> const lock{locker};
				callbacks.erase(id);
			}
		}
	private:
		std::atomic<id_t>	last_id = 0;
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