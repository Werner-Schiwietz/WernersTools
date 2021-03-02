#pragma once
//Copyright (c) 2020 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

//alles im namespace WS
//WS::Cache mit key und value
//- erg�nzend kann ein validkey_type angegeben werden, der ggf. den Wert der �ber den Key gecacht wird, ung�ltig macht.
//  der default-validkey_type ist Pure_key. Mit Pure_key bleibt der Wert g�ltig, bis er gel�scht wird
//  wenn der Wert nur eine bestimmte Zeit g�ltig sein soll kann die Spezialisierung WS::CacheDuration, welche Duration_key nutzt verwendet werden
//- der Zugriff ist per mutex_type (default-std::mutex) synchronisiert. Sollte keine Synchronisierung n�tig sein, weil single-thread, kann WS::null_mutex f�r sperrenlose Funktion sorgen
//
//Beispielcode und Unittest in UT_Cache.cpp

#include <mutex>
#include <map>
#include <optional>
#include <chrono>

namespace WS
{
	//f�r singlethreaded nichts tuender mutex
	struct null_mutex
	{
		void lock(){ }
		void unlock(){ }
	};
	#pragma region key-types  
	template<typename key_type,int duration_value,typename duration_type=std::chrono::milliseconds>struct Duration_key
	{
		static_assert(duration_value>0);
		using key_t = key_type;
		using duration_t = duration_type;
		decltype(std::chrono::system_clock::now()+duration_t{duration_value}) valid_till;
		key_t key;

		Duration_key()=delete;
		Duration_key( key_t key) : key(std::move(key)), valid_till(std::chrono::system_clock::now() + valid_duration()){}

		bool operator<( Duration_key const & r ) const {return  this->key<r.key;}
		bool is_valid() const {return std::chrono::system_clock::now() <= this->valid_till;} 
		bool is_invalid() const {return !is_valid();}

		static constexpr duration_t valid_duration(){return duration_t{duration_value};}
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

		mutex_t			mutex;
		data_t			data;
		Cache(){}

		//h�lt lock, liefert iterator
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
			ret_value.iter = this->data.find(validkey_t{key});

			if( ret_value.iter==data.end() )
				ret_value.locked.unlock();
			else if( ret_value.iter->first.is_invalid() )
			{
				this->data.erase(ret_value.iter);
				ret_value.locked.unlock();
			}

			return ret_value;
		}
		//ohne bleibenden lock als kopie
		std::optional<value_type>	Get( key_t const & key )
		{
			if( auto item = get_locked_item( key ) )
				return {item.iter->second};
			return {};
		}
		//wie get_locked_item. h�lt return-wert lock und liefert iterator
		auto Set( key_t const & key, value_t value )
		{
			decltype(get_locked_item( key )) ret_value{ WS::lock_guard{this->mutex} };
			ret_value.iter = this->data.insert_or_assign(validkey_t{key}, std::move(value)).first;

			return ret_value;
		}

		void clear(){data.clear();}
		auto clear( key_t key ){return data.erase(key);}
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
	#pragma endregion 
}
