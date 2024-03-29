#pragma once

//alles im namespace WS
//WS::Cache mit key und value
//- erg�nzend kann ein validkey_type angegeben werden, der ggf. den Wert, der �ber den Key gecacht wird, ung�ltig macht.
//  der default-validkey_type ist Pure_key. Mit Pure_key bleibt der Wert g�ltig, bis er gel�scht wird
//  wenn der Wert nur eine bestimmte Zeit g�ltig sein soll kann die Spezialisierung WS::CacheDuration verwendet werden. WS::CacheDuration nutzt Duration_key.
//	komplexere valid-Pr�fungen k�nnen per validkey_maker realisiert werden. Siehe dazu UnitTest cache_int_mystring_validkey_maker
//- der Zugriff ist per mutex_type (default-std::mutex) synchronisiert. Sollte keine Synchronisierung n�tig sein, weil single-thread, kann WS::null_mutex f�r sperrenlose Funktion sorgen
//
//Beispielcode und Unittest in UT_Cache.cpp

#include "return_type.h"
#include "mutex_atomicflag.h"

#include <mutex>
#include <map>
#include <optional>
#include <chrono>
#include <functional>
/*
* template<typename key_type,typename value_type, typename mutex_type=std::mutex,typename validkey_type=Pure_key<key_type>> 
* struct Cache
* -- ein cache verwaltet key-value-pairs threadsafe wenn mutex mit funktion wie std::mutex verwendet wird
*    mutex_type ein synchronisations-mutex (standard ist std::mutex) muss void lock() und void unlock() bereitstellen
*	 validkey_type factory-type erg�nzt den key um bool valid() (standard WS::Pure_key key immer valid)
* 
* Cache::Cache( ) 
*  -- ctor eines Cache
* 
* Cache::Cache( validkey_maker_t validkey_maker ) 
* -- ctor eines Cache mit parameter einer valid_key-factory mit signatur <validkey_type(key_type)>
* 
* auto Cache::Set( key_t const & key, value_t value )
* -- setzt key-value-pair. der r�ckgabewert sollte ignoriert werden. es ist ein lock_guard auf den mutex und ein iterator auf das verwaltete pair
* 
* std::optional<value_type>	Cache::Get( key_t key )
* --  liefert im std::optional den value, wenn key vorhanden und valid ist
* 
* auto get_locked_item( key_t key )
* -- liefert lock_guard und iterator aufs key-value-pair 
*    VORSICHT wegen lock_gurad deadlock gefahr

- Spezialisierungen die die verwendung etwas einfacher machen
- template<typename key_type,typename value_type,typename validkey_type=Pure_key<key_type>> 
- struct CacheSingleThread
- -- wie oben aber ohne threadsafe zu sein
-    validkey_type factory-type erg�nzt den key um bool valid() (standard WS::Pure_key key immer valid)
- 
- template<typename key_type,typename value_type,int duration_value,typename duration_type=std::chrono::milliseconds, typename mutex_type=std::mutex> 
- struct CacheDuration
- -- ein cache verwaltet key-value-pairs die eine gewisse zeit valid bleiben
-    das key-value-pair bleibt nach set duration_type{duration_value} valid
- 	 mutex_type ein synchronisations-mutex (standard ist std::mutex) muss void lock() und void unlock() bereitstellen
-
- template<typename key_type,typename value_type,int duration_value,typename duration_type=std::chrono::milliseconds, typename mutex_type=std::mutex> 
- struct CacheDurationRefresh
- -- wie CacheDuration, aber jeder Get() verl�ngert die valid-zeit


//struct WS::null_mutex f�r singlethreaded nichts tuender mutex

* valid-keys erg�nzen den id-key um valid-informationen
* template<typename key_type>
* struct WS::Pure_key -- der value zum key vom type key_type ist immer g�ltig
* 
* template<typename key_type,int duration_value,typename duration_type=std::chrono::milliseconds>
* struct WS::Duration_key   -- der value zum key vom type key_type bleibt duration_type{duration_value} g�ltig. also eine bestimmt zeit nach WS::Cache::Set
* 
* template<typename key_type,int duration_value,typename duration_type=std::chrono::milliseconds>
* struct DurationRefresh_key -- wie WS::Duration_key, aber valid-zeit wird mit jedem WS::Cache::Get auf now + duration_type{duration_value} verl�ngert
*/


namespace WS
{
	//f�r singlethreaded nichts tuender mutex
	struct null_mutex
	{
		void lock(){ }
		void unlock(){ }
	};

	//_validkey_maker template-factory um aus einem key einen key mit valid-pr�fung zu machen
	template<typename validkey_t,typename key_t> validkey_t _validkey_maker(key_t key){return validkey_t{std::move(key)};}
	#pragma region key-types  
	template<typename key_type,int duration_value,typename duration_type=std::chrono::milliseconds> struct Duration_key
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

	//DurationRefresh_key vaild_till wird bei jedem positiven valid() neu ermittelt, also die valid-zeit verl�ngert
	template<typename key_type,int duration_value,typename duration_type=std::chrono::milliseconds> struct DurationRefresh_key : Duration_key<key_type,duration_value,duration_type>
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

	//Pure_key(standard) key wird nie invalid
	template<typename key_type> struct Pure_key
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

		//h�lt lock, liefert iterator
		auto get_locked_item( key_t key ) 
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
		//ohne bleibenden lock, value als kopie
		std::optional<value_type>	Get( key_t key ) 
		{
			if( auto item = get_locked_item( std::move(key) ) )
				return {item.iter->second};
			return {};
		}
		//wie get_locked_item. h�lt return-wert lock und liefert iterator
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
