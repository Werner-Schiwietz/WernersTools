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

#include <mutex>
#include <map>
#include <optional>
#include <chrono>

namespace WS
{
	//für singlethreaded nichts tuender mutex
	struct null_mutex
	{
		void lock(){ }
		void unlock(){ }
	};
	#pragma region key-types  
	template<typename key_type,int duration_value=0,typename duration_type=std::chrono::milliseconds>struct duration_key
	{
		using key_t = key_type;
		using duration_t = duration_type;
		decltype(std::chrono::system_clock::now()+duration_t{duration_value}) valid_till;
		key_t key;

		duration_key()=delete;
		duration_key( key_t key) : key(std::move(key)), valid_till(std::chrono::system_clock::now() + valid_duration()){}

		bool operator<( duration_key const & r ) const {return  this->key<r.key;}
		bool is_valid() const {return std::chrono::system_clock::now() <= this->valid_till;} 
		bool is_invalid() const {return !is_valid();}

		static constexpr duration_t valid_duration(){return duration_t{duration_value};}
	};
	template<typename key_type>struct duration_key<typename key_type,0>
	{
		using key_t = key_type;
		key_t key;

		duration_key()=delete;
		duration_key( key_t const & key) : key(key){}

		bool operator<( duration_key const & r ) const {return  this->key < r.key;}
		bool is_valid() const {return true;} 
		bool is_invalid() const {return !is_valid();}
	};
	#pragma endregion


	template<typename key_type,typename value_type, typename mutex_type=std::mutex,int duration_value=0,typename duration_type=std::chrono::milliseconds> struct Cache
	{
		using key_t				= key_type;
		using duration_key_t	= duration_key<key_t,duration_value,duration_type>;
		using value_t			= value_type;
		using mutex_t			= mutex_type;
		using data_t			= std::map<duration_key_t,value_t>;

		mutex_t			mutex;
		data_t			data;
		Cache(){}

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
			ret_value.iter = this->data.find(duration_key_t{key});

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
		//wie get_locked_item. hält return-wert lock und liefert iterator
		auto Set( key_t const & key, value_t value )
		{
			decltype(get_locked_item( key )) ret_value{ WS::lock_guard{this->mutex} };
			ret_value.iter = this->data.insert_or_assign(duration_key_t{key}, std::move(value)).first;

			return ret_value;
		}

		void clear(){data.clear();}
		auto clear( key_t key ){return data.erase(key);}
	};
	#pragma region cache spezialisierungen
	template<typename key_type,typename value_type,int duration_value=0,typename duration_type=std::chrono::milliseconds> struct CacheSingleThread : Cache<key_type,value_type,null_mutex,duration_value,duration_type>
	{
		using Cache::Cache;
	};
	template<typename key_type,typename value_type,int duration_value,typename duration_type=std::chrono::milliseconds, typename mutex_type=std::mutex> struct CacheDuration : Cache<key_type,value_type,mutex_type,duration_value,duration_type>
	{
		using Cache::Cache;
	};
	#pragma endregion 
}
