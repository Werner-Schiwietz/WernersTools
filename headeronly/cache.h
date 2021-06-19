#pragma once

//alles im namespace WS
//WS::Cache mit key und value
//- ergänzend kann ein validkey_type angegeben werden, der ggf. den Wert, der über den Key gecacht wird, ungültig macht.
//  der default-validkey_type ist Pure_key. Mit Pure_key bleibt der Wert gültig, bis er gelöscht wird
//  wenn der Wert nur eine bestimmte Zeit gültig sein soll kann die Spezialisierung WS::CacheDuration verwendet werden. WS::CacheDuration nutzt Duration_key.
//	komplexere valid-Prüfungen können per validkey_maker realisiert werden. Siehe dazu UnitTest cache_int_mystring_validkey_maker
//- der Zugriff ist per mutex_type (default-std::mutex) synchronisiert. Sollte keine Synchronisierung nötig sein, weil single-thread, kann WS::null_mutex für sperrenlose Funktion sorgen
//
//Beispielcode und Unittest in UT_Cache.cpp

#include "return_type.h"
#include "mutex_atomicflag.h"

#include <mutex>
#include <map>
#include <optional>
#include <chrono>
#include <functional>

namespace WS
{
	//für singlethreaded nichts tuender mutex
	struct null_mutex
	{
		void lock(){ }
		void unlock(){ }
	};
	template<typename validkey_t,typename key_t> validkey_t _validkey_maker(key_t key){return validkey_t{std::move(key)};}
	#pragma region key-types  
	template<typename key_type,int duration_value,typename duration_type=std::chrono::milliseconds>struct Duration_key
	{
		static_assert(duration_value>0);
		using key_t = key_type;
		using duration_t = duration_type;
		mutable decltype(std::chrono::system_clock::now()+duration_t{duration_value}) valid_till;
		key_t key;

		Duration_key()=delete;
		Duration_key( key_t key) : key(std::move(key)), valid_till(std::chrono::system_clock::now() + valid_duration()){}

		bool operator<( Duration_key const & r ) const {return  this->key<r.key;}
		virtual bool is_valid() const {return std::chrono::system_clock::now() <= this->valid_till;} 
		bool is_invalid() const {return !is_valid();}

		static constexpr duration_t valid_duration(){return duration_t{duration_value};}
	};

	//DurationRefresh_key vaild_till wird bei jedem positiven valid() neu ermittelt, also die valid-zeit verlängert
	template<typename key_type,int duration_value,typename duration_type=std::chrono::milliseconds>struct DurationRefresh_key : Duration_key<key_type,duration_value,duration_type>
	{
		using Duration_key::Duration_key;
		bool is_valid() const override 
		{
			if( std::chrono::system_clock::now() <= this->valid_till )
			{
				this->valid_till = std::chrono::system_clock::now() + valid_duration();
				return true;
			}
			return false;
		} 
	};
	template<typename key_type>struct Pure_key
	{
		using key_t = key_type;
		key_t key;

		Pure_key()=delete;
		Pure_key( key_t const & key) : key(key){}

		bool operator<( Pure_key const & r ) const {return  this->key < r.key;}
		bool is_valid() const {return true;} 
		bool is_invalid() const {return !is_valid();}
	};
	#pragma endregion

	template<typename key_type,typename value_type, typename mutex_type=std::mutex,typename validkey_type=Pure_key<key_type>> struct Cache
	{
		using key_t				= key_type;

		using validkey_t		= validkey_type;
		using value_t			= value_type;
		using mutex_t			= mutex_type;
		using data_t			= std::map<validkey_t,value_t>;
		using validkey_maker_t	= std::function<validkey_t(key_type)>;

		mutex_t				mutex;
		data_t				data;
		validkey_maker_t	validkey_maker;
	public:
		Cache( validkey_maker_t validkey_maker=_validkey_maker<validkey_t,key_t> ) : validkey_maker(std::move(validkey_maker)) {}

		//hält lock, liefert iterator
		auto get_locked_item( key_t const & key )
		{
			struct r_t: WS::compare_bool
			{
				WS::lock_guard<mutex_t> locked;
				data_t::iterator iter;

				r_t( WS::lock_guard<mutex_t> locked) : locked(std::move(locked)) {}
			private:
				bool to_bool() const override{return locked.is_locked();}
			}ret_value{WS::lock_guard{this->mutex}};
			ret_value.iter = this->data.find(validkey_maker(std::move(key)));

			if( ret_value.iter==data.end() )
				ret_value.locked.unlock();
			else if( ret_value.iter->first.is_invalid() )
			{
				this->data.erase(ret_value.iter);
				_clear_invalid();
				ret_value.locked.unlock();
			}

			return ret_value;
		}
		//ohne bleibenden lock als kopie
		std::optional<value_type>	Get( key_t key )
		{
			if( auto item = get_locked_item( std::move(key) ) )
				return {item.iter->second};
			return {};
		}
		//wie get_locked_item. hält return-wert lock und liefert iterator
		auto Set( key_t const & key, value_t value )
		{
			decltype(get_locked_item( key )) ret_value{ WS::lock_guard{this->mutex} };
			ret_value.iter = this->data.insert_or_assign(validkey_maker(key), std::move(value)).first;

			return ret_value;
		}

		void clear()
		{
			auto locked=std::lock_guard(this->mutex);
			data.clear();
		}
		auto clear( key_t key )
		{
			auto locked=std::lock_guard(this->mutex);
			return data.erase(validkey_maker(key));
		}
		void clear_invalid()
		{
			auto locked=std::lock_guard(this->mutex);
			_clear_invalid();
		}
	private:
		void _clear_invalid()
		{
			for( auto iter = this->data.begin(); iter!=this->data.end();  )
			{
				if(iter->first.is_invalid())
					iter = this->data.erase(iter);
				else
					++iter;
			}
		}
	};
	#pragma region cache spezialisierungen
	template<typename key_type,typename value_type,typename validkey_type=Pure_key<key_type>> struct CacheSingleThread : Cache<key_type,value_type,null_mutex,validkey_type>
	{
		using Cache::Cache;
	};
	template<typename key_type,typename value_type,int duration_value,typename duration_type=std::chrono::milliseconds, typename mutex_type=std::mutex> struct CacheDuration : Cache<key_type,value_type,mutex_type,Duration_key<key_type,duration_value,duration_type>>
	{
		using Cache::Cache;
	};
	template<typename key_type,typename value_type,int duration_value,typename duration_type=std::chrono::milliseconds, typename mutex_type=std::mutex> struct CacheDurationRefresh : Cache<key_type,value_type,mutex_type,DurationRefresh_key<key_type,duration_value,duration_type>>
	{
		using Cache::Cache;
	};
	#pragma endregion 
}
