#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\mutex_automicflag.h"
#include "..\..\headeronly\return_type.h"

#include <mutex>
#include <map>
#include <optional>

#include <string>

namespace WS
{
	template<typename key_type>struct validate_cache_key
	{
		using key_t = key_type;
		bool operator()( key_t const & key_in_cache,key_t const & key_new ){key_in_cache;key_new;return true;}
	};
	struct null_mutex//für singlethreaded nichts tuender mutex
	{
		bool try_lock(){return true;}
		void lock(){ }
		void unlock(){ }
	};
	template<typename key_type,typename value_type, typename validate_key_type=validate_cache_key<key_type>, typename mutex_type=std::mutex> struct Cache
	{
		using key_t				= key_type;
		using value_t			= value_type;
		using validate_key_t	= validate_key_type;
		using mutex_t			= mutex_type;
		using data_t			= std::map<key_t,value_t>;

		mutex_t			mutex;
		data_t			data;
		validate_key_t	validate_key{};
		Cache(){}
		Cache(validate_key_t validate_key):validate_key(std::move(validate_key)){}

		auto get_locked_item( key_t const & key )
		{
			struct r_t : WS::compare_bool
			{
				using iter_t = decltype(std::declval<data_t>().find(key));
				WS::lock_guard<mutex_t> locked;
				value_t * value_ptr{};

				r_t( WS::lock_guard<mutex_t> locked, value_t * value_ptr ) : locked(std::move(locked)),value_ptr(value_ptr) {}
				r_t( WS::lock_guard<mutex_t> locked ) : locked(std::move(locked)) {}

			private:
				bool to_bool()const override{return locked.is_locked();}
			}ret_value{WS::lock_guard{this->mutex}};
			
			auto & [key_ptr, value_ptr] = _get_value_ptr(key);
			
			if( key_ptr==nullptr || validate_key(*key_ptr,key)==false )
				ret_value.locked.unlock();
			else
				ret_value.value_ptr = value_ptr;

			return ret_value;
		}
		std::optional<value_type>	Get( key_t const & key )
		{
			auto item = get_locked_item( key );
			if( item )
				return {*item.value_ptr};
			return {};
		}
		auto Set( key_t const & key, value_t value )
		{
			decltype(get_locked_item( key )) ret_value{WS::lock_guard{this->mutex}};
			ret_value.value_ptr = &(data[key] = std::move(value));

			return ret_value;
		}
	private:
		std::pair<key_t const *,value_t*>  _get_value_ptr(key_t const & key)
		{
			auto iter = data.find(key);
			if(iter!=data.end())
				return {&iter->first, &iter->second};
			return {};
		};
	};
	template<typename key_type,typename value_type, typename validate_key_type=validate_cache_key<key_type>> struct CacheSingleThread : Cache<key_type,value_type,validate_key_type,null_mutex>
	{
		using Cache::Cache;
	};
}

namespace
{
	struct mystring : std::string
	{
		using base_t = std::string;
		~mystring()
		{
		}
		mystring() : base_t() 
		{
		}
		template<typename T>mystring(T value) : base_t(std::move(value))
		{
		}
		mystring(base_t r) : base_t(std::move(r))
		{
		}
		mystring(mystring const & r) : base_t(r)
		{
		}
		mystring(mystring && r) : base_t(std::move(r))
		{
		}
		mystring& operator=(mystring const & r) & 
		{
			base_t::operator=(r);
			return *this;
		}
		mystring& operator=(mystring && r) & 
		{ 
			base_t::operator=((base_t&&)r);
			return *this;
		}
	};
}

namespace UTCache
{
	TEST_CLASS(UTCache)
	{
	public:
		
		TEST_METHOD(cache_int_mystring)
		{
			{
				std::map<int,mystring> map;
				mystring str{"Hallo"};
				auto ptr = str.c_str();
				map[1] = std::move(str);
				//Assert::IsTrue( ptr == map[1].c_str() );
				ptr = map[1].c_str();
				std::map<int,mystring>::iterator iter = map.begin();
				Assert::IsTrue( ptr == iter->second.c_str() );
			}
			auto cache = WS::Cache<int,mystring>{};

			Assert::IsFalse( cache.Get(2).has_value() );
			Assert::IsTrue( *cache.Set(2,"hallo").value_ptr == "hallo" );
			auto v1 = cache.Get(1);
			Assert::IsFalse( v1.has_value() );
			auto v2 = cache.Get(2);//Get() liefert ggf. kopie im optional, braucht so aber auch keinen lock
			Assert::IsTrue( v2.has_value() );
			Assert::IsTrue( v2.value() == "hallo" );
			Assert::IsTrue( *cache.Set(2,"Hallo").value_ptr == "Hallo" );
			auto v2item = cache.get_locked_item(2);
			Assert::IsTrue( v2item );
			Assert::IsTrue( v2item.value_ptr );
			Assert::IsTrue( *v2item.value_ptr == "Hallo" );
			v2item.locked.unlock();//ohne deadlock-exception

			auto v1item = cache.get_locked_item(1);
			Assert::IsFalse( v1item );
		}
	};
}
