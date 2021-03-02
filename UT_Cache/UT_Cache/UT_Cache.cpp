#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\mutex_automicflag.h"
#include "..\..\headeronly\return_type.h"

#include <mutex>
#include <map>
#include <optional>

#include <string>
#include <chrono>

namespace WS
{
	struct null_mutex//für singlethreaded nichts tuender mutex
	{
		bool try_lock(){return true;}
		void lock(){ }
		void unlock(){ }
	};
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
	template<typename key_type,typename value_type,int duration_value=0,typename duration_type=std::chrono::milliseconds> struct CacheSingleThread : Cache<key_type,value_type,null_mutex,duration_value,duration_type>
	{
		using Cache::Cache;
	};
	template<typename key_type,typename value_type,int duration_value,typename duration_type=std::chrono::milliseconds, typename mutex_type=std::mutex> struct CacheDuration : Cache<key_type,value_type,mutex_type,duration_value,duration_type>
	{
		using Cache::Cache;
	};
}

namespace
{
	struct mykey 
	{	using value_t = int;
		value_t value{};
		mykey(value_t value):value(value){}
		bool operator<(mykey const & r ) const { return this->value < r.value; }
	};
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
	TEST_CLASS(UT_Verstaendnis)
	{
		TEST_METHOD(UT_Valid_Data)
		{
			using key_t=int;
			using value_t=std::unique_ptr<int>;
			std::map<key_t,value_t> map;

			auto v5=map.insert_or_assign(5,std::make_unique<int>(5));
			Assert::IsTrue( *v5.first->second == 5 );
			auto v3=map.insert_or_assign(3,std::make_unique<int>(3));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 3 );
			auto v9=map.insert_or_assign(9,std::make_unique<int>(9));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 3 );
			Assert::IsTrue( *v9.first->second == 9 );
			auto v7=map.insert_or_assign(7,std::make_unique<int>(7));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 3 );
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			map.erase(3);
			Assert::IsTrue( *v5.first->second == 5 );
			//Assert::IsTrue( v3.first );//no way checking if valid
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			v3 = map.insert_or_assign(3,std::make_unique<int>(33));
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 33 );
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			map.insert_or_assign(3,std::make_unique<int>(333));//undefined behavior ? but v3 is valid with correct value
			Assert::IsTrue( *v5.first->second == 5 );
			Assert::IsTrue( *v3.first->second == 333 );
			Assert::IsTrue( *v9.first->second == 9 );
			Assert::IsTrue( *v7.first->second == 7 );
			for(key_t i=0;i<1000;++i)
			{
				map.insert_or_assign(i,std::make_unique<int>(i));
				Assert::IsTrue( *v5.first->second == 5 );
				Assert::IsTrue( *v3.first->second == 333 || *v3.first->second==3);
				Assert::IsTrue( *v9.first->second == 9 );
				Assert::IsTrue( *v7.first->second == 7 );
			}

		}
		TEST_METHOD(UT_Duration)
		{
			using namespace std::literals::chrono_literals;

			std::chrono::milliseconds xx(5);

			auto breakafter = 180ms;
			std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();

			int i;
			auto stoptime = std::chrono::system_clock::now() + breakafter;
			for(i=0;;++i)
			{
				if( stoptime <= std::chrono::system_clock::now() )
					break;
			}
		}
		TEST_METHOD(UT_Duration2)
		{
			auto till = std::chrono::system_clock::now() + std::chrono::milliseconds{150};
			while(std::chrono::system_clock::now() <= till )
			{}
		}
		TEST_METHOD(UT_Key)
		{
			using namespace std::literals::chrono_literals;
			auto start_time {std::chrono::system_clock::now()};

			WS::duration_key<int> key_t2{6};
			WS::duration_key<int,200> key_t1{5};
			for( auto duration = std::chrono::system_clock::now() - start_time; key_t1.is_valid(); duration = std::chrono::system_clock::now() - start_time)
			{
				Assert::IsTrue( duration < 201ms );
				Assert::IsTrue( key_t2.is_valid() );
			}
			auto duration = std::chrono::system_clock::now() - start_time;
			Assert::IsTrue( 200ms <= duration );
		}
		TEST_METHOD(UT_ret_shared_ptr)
		{
			struct A
			{
				std::shared_ptr<int> data { new int{5} };

				std::shared_ptr<int> &		get_data1(){ return data;}
				std::shared_ptr<int> const & get_data1() const { return data;}
				std::shared_ptr<int> &		get_data2(){ return data;}
				std::shared_ptr<int const> const & get_data2() const { return data;}
			};

			A a;
			A const & const_ref_a = a;
			Assert::IsTrue( *a.get_data1() == 5 );
			*const_ref_a.get_data1() = 6;//von wegen const. der shared-pointer ist const, nicht dessen inhalt
			Assert::IsTrue( *a.get_data1() == 6 );

			//*const_ref_a.get_data2() = 7;//error C3892: 'const_ref_a': you cannot assign to a variable that is const
		}
	};
	TEST_CLASS(UTCache)
	{
	public:
		
		TEST_METHOD(cache_int_mystring)
		{
			{
				std::map<mykey,mystring> map;
				mystring str{"Hallo"};
				auto ptr = str.c_str();
				map[1] = std::move(str);
				//Assert::IsTrue( ptr == map[1].c_str() );
				ptr = map[1].c_str();
				std::map<mykey,mystring>::iterator iter = map.begin();
				Assert::IsTrue( ptr == iter->second.c_str() );
			}
			auto cache = WS::Cache<mykey,mystring>{};

			Assert::IsFalse( cache.Get(2).has_value() );
			Assert::IsTrue( cache.Set(2,"hallo").iter->second == "hallo" );
			Assert::IsTrue( cache.Set(3,"welt").iter->second == "welt" );
			auto v1 = cache.Get(1);
			Assert::IsFalse( v1.has_value() );
			auto v2 = cache.Get(2);//Get() liefert ggf. kopie im optional, braucht so aber auch keinen lock
			Assert::IsTrue( v2.has_value() );
			Assert::IsTrue( v2.value() == "hallo" );
			Assert::IsTrue( cache.Set(2,"Hallo").iter->second == "Hallo" );
			auto v2item = cache.get_locked_item(2);
			Assert::IsTrue( v2item );
			Assert::IsTrue( v2item.iter->second == "Hallo" );
			v2item.locked.unlock();//ohne deadlock-exception

			auto v1item = cache.get_locked_item(1);
			Assert::IsFalse( v1item );
		}
		TEST_METHOD(cache_int_mystring_valid_75ms)
		{
			auto cache = WS::CacheDuration<mykey,mystring,75>{};

			Assert::IsFalse( cache.Get(2).has_value() );
			cache.Set(3,"welt");
			auto start_time = std::chrono::system_clock::now();
			cache.Set(2,"hallo");
			while( cache.Get(2).has_value() ){}
			Assert::IsTrue( std::chrono::system_clock::now() - start_time > decltype(cache)::duration_key_t::valid_duration() );
		}
	};
}
