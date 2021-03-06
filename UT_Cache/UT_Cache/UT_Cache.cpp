#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\mutex_automicflag.h"
#include "..\..\headeronly\return_type.h"
#include "..\..\headeronly\cache.h"

#include <string>


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
		TEST_METHOD(UT_Duration_180ms)
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
		TEST_METHOD(UT_Duration2_150ms)
		{
			auto till = std::chrono::system_clock::now() + std::chrono::milliseconds{150};
			while(std::chrono::system_clock::now() <= till )
			{}
		}
		TEST_METHOD(UT_Key_200ms)
		{
			using namespace std::literals::chrono_literals;
			auto start_time {std::chrono::system_clock::now()};

			WS::Pure_key<int> key_t2{6};
			WS::Duration_key<int,200> key_t1{5};
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
			Assert::IsTrue( std::chrono::system_clock::now() - start_time > decltype(cache)::validkey_t::valid_duration() );
		}
		TEST_METHOD(cache_int_mystring_validkey_maker)
		{
			int validator=5;
			auto make_validkey = [&](mykey key)
			{
				struct keyhandler
				{
					using validator_t = std::remove_reference_t<decltype(validator)>;
					using key_t = mykey;
					validator_t const & validator_value;
					validator_t soll_validator_value;

					key_t key;

					keyhandler()=delete;
					keyhandler( key_t key, validator_t const & validator_value) : key(std::move(key)), validator_value(validator_value),soll_validator_value(validator_value){}

					bool operator<( keyhandler const & r ) const {return  this->key<r.key;}
					bool is_valid() const 
					{
						return validator_value==soll_validator_value;
					} 
					bool is_invalid() const {return !is_valid();}
				};
				return keyhandler(std::move(key),validator);
			};

			auto cache = WS::Cache<mykey,mystring,std::mutex,decltype(make_validkey(std::declval<mykey>()))>{make_validkey};

			Assert::IsFalse( cache.Get(2).has_value() );
			for( auto i=5; i --> 0;)
			{
				cache.Set( 3, "welt" );
				cache.Set( 2, "hallo" );
				for( int i=100; i--> 0; )
				{
					Assert::IsTrue( cache.Get( 2 ).has_value() );
				}
				++validator;
				Assert::IsFalse( cache.Get( 2 ).has_value() );
				Assert::IsFalse( cache.Get( 3 ).has_value() );
			}
		}
	};
}
