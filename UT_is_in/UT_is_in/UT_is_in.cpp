#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// /permissive- /Zc:twoPhase- 
//char * error =  "hallo";// mit /permissive- /Zc:twoPhase- error C2440: 'initializing': cannot convert from 'const char [6]' to 'char *'

#include <vector>
//#include <string>

#pragma warning(push,4)
#include "..\..\headeronly\is_in.h"
#include "..\..\headeronly\char_helper.h"

//namespace geht_nicht_als_std//???
namespace std
{
	//template<typename char_t,size_t size>char_t const * begin(char_t const (&p)[size]);//{return p;}
	//template<typename char_t,size_t size>char_t const * end(char_t const (&p)[size]);//{return p+size;}
	//template<typename char_t>char_t const * begin(char_t const * p){return p;}
	//template<typename char_t>char_t const * end(char_t const * p){return p+stringlen(p);}
}



namespace WS_test
{
	//container
	template <typename value_t, typename U> auto is_in( value_t const & gesucht, U const & other ) 
		-> std::enable_if_t<WS_exist::begin_v<U> || WS_exist::begin_WS_v<U> || WS_exist::begin_glbNS_v<U> || WS_exist::begin_std_v<U> || WS_has_method::begin_v<U>,bool>
	{
		if constexpr ( WS_has_method::begin_v<U> )
		{
			auto b = other.begin();
			auto e = other.end();
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_v<U> )
		{
			auto b = begin(other);
			auto e = end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_WS_v<U> )
		{
			auto b = WS::begin(other);
			auto e = WS::end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_glbNS_v<U> )
		{
			auto b = ::begin(other);
			auto e = ::end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_exist::begin_std_v<U> )
		{
			auto b = std::begin(other);
			auto e = std::end(other);
			return std::find(b,e,gesucht)!=e;
		}
		else if constexpr ( WS_has_method::begin_v<U> )
		{
			auto b = other.begin();
			auto e = other.end();
			return std::find(b,e,gesucht)!=e;
		}
	}
	//bereich
	template<typename value_t> bool is_in( value_t const & value, WS::bereich_t<value_t> const & bereich ){return bereich.is_in( value );}
	//array, sonderbehandulung char-type-array
	template<typename value_t, size_t size> bool is_in( value_t const & value, value_t const (&values)[size] )
	{
		if constexpr ( WS::is_char_type_v<value_t> )
		{
			//std::basic_string_view ist so nicht nullterminiert, geht nicht
			//return is_in(value,std::basic_string_view<value_t>(values,size));
			auto b = WS::begin<value_t,size>(values);
			auto e = WS::end<value_t,size>(values);
			return std::find(b,e,value)!=e;
		}
		else
		{
			for( auto const & item : values )
				if( value == item )
					return true;
		}
		return false;
	}	
	//vergleich gleicher werte
	template<typename value_t> bool is_in( value_t const & value, value_t const & vergleichoperand ){return value == vergleichoperand;}
	//variadic
	template<typename value_t, typename vergleichoperand_t, typename... others> bool  is_in( value_t const & value, vergleichoperand_t const & vergleichoperand, others const & ... Rest )
	{
		return WS_test::is_in( value, vergleichoperand ) || WS_test::is_in( value, Rest ... );
	}
}

namespace UTisin
{
	//\evtl. 0-terminierte string-arrays. nicht 0-terminiert liefern als end die erste adresse hinter dem array
	template<typename char_t,size_t size> auto begin(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<WS::is_char_type_v<char_t>, char_t *>{}){return ar;}
	template<typename char_t,size_t size> auto end(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<WS::is_char_type_v<char_t>, char_t *>{})
	{
		size_t index=0;
		for( ; index<size && ar[index]; ++index ) {}
		return &ar[0]+index;
	}
	template<typename char_t,size_t size> auto begin(char_t const (&ar)[size]) noexcept ->decltype( std::enable_if_t<WS::is_char_type_v<char_t>, char_t const *>{}){return ar;}
	template<typename char_t,size_t size> auto end(char_t const (&ar)[size]) noexcept ->decltype( std::enable_if_t<WS::is_char_type_v<char_t>, char_t const *>{})
	{
		size_t index=0;
		for( ; index<size && ar[index]; ++index ) {}
		return &ar[0]+index;
	}

	TEST_CLASS(UTisin)
	{
	public:
		TEST_METHOD(UT_neu_is_in)
		{
			Assert::IsTrue(WS_test::is_in(3,3));
			Assert::IsFalse(WS_test::is_in(3,4));
			Assert::IsTrue(WS_test::is_in(3,std::vector{4,3,5,6}));
			Assert::IsFalse(WS_test::is_in(2,std::vector{4,3,5,6}));

			wchar_t text[5];
			#pragma warning(suppress:4996)
			wcsncpy(text,L"Hallo",5);//nicht nullterminiert
			Assert::IsTrue(WS_test::is_in(L'a',text));
			Assert::IsFalse(WS_test::is_in(L'\0',text));
			Assert::IsFalse(WS_test::is_in(L'w',text));
			Assert::IsTrue(WS_test::is_in('a',"hallo"));
			Assert::IsFalse(WS_test::is_in('\0',"hallo"));
			Assert::IsTrue(WS::is_in('a',"hallo"));
			Assert::IsFalse(WS::is_in('\0',"hallo"));//seit 2021-01-07 false
			Assert::IsTrue(WS_test::is_in(3,WS::bereich(2,6)));
			Assert::IsFalse(WS_test::is_in(31,WS::bereich(2,6)));
			//beriche müssen schon selben type haben
			//Assert::IsTrue(WS_test::is_in(3,WS::bereich<short>(2,6)));//error C2672: 'WS_test::is_in': no matching overloaded function found
			Assert::IsTrue(WS_test::is_in(3i16,WS::bereich<short>(2,6)));
			Assert::IsFalse(WS_test::is_in(31i16,WS::bereich<short>(2,6)));
			Assert::IsFalse(WS_test::is_in(31i16,WS::bereich<short>(2,6)));

			Assert::IsTrue( WS_test::is_in('a', {'a','c','e'} ) );
			Assert::IsTrue( WS_test::is_in('c', {'a','c','e'} ) );
			Assert::IsTrue( WS_test::is_in('e', {'a','c','e'} ) );
			Assert::IsFalse( WS_test::is_in('b', {'a','c','e'} ) );

			Assert::IsTrue( WS_test::is_in(5, 1,3,5,7,9) );
			Assert::IsFalse( WS_test::is_in(5, 1) );
			//Assert::IsFalse( WS_test::is_in(5) );//error C2672: 'WS_test::is_in': no matching overloaded function found
		}
		TEST_METHOD(UT_find_begin)
		{
			{
				static_assert( WS_exist::begin_std_v<int> == false );
				static_assert( WS_exist::begin_std_v<char const *> == false );
				static_assert( WS_exist::begin_std_v<decltype("hallo")> == true );

				decltype("hallo") x{};x;
				static_assert( WS_exist::begin_std_v<decltype(x)> == true );
				static_assert( WS_exist::begin_WS_v<char const *> == true );
				static_assert( WS_exist::begin_WS_v<char *> == true );

				static_assert( WS_exist::begin_v<decltype(x)> == false );//(void)::begin(x);
				static_assert( WS_exist::begin_v<char const *> == false );
				static_assert( WS_exist::begin_v<char *> == false );

				static_assert( WS_has_method::begin_v<int> == false );
				static_assert( WS_has_method::begin_v<char const *> == false );
				static_assert( WS_has_method::begin_v<decltype("hallo")> == false );
				static_assert( WS_has_method::begin_v<std::vector<int>> == true );

				//static_assert( WS_exist::begin_WS_v<decltype(x)> == true ); //weil ...
				//decltype( WS :: begin( x ) ) xx;//error C2668: 'WS::begin': ambiguous call to overloaded function
				decltype( WS :: begin<char const,6>( x ) ) px;px;

				auto b = std::begin("hallo");b;//ruft array-begin, dann kann es aber kein begin( T const * ) mehr geben, sonst kommt bei verwendung mit array-parameter ambiguous-error, sihe oben
				auto e = std::end("hallo");e;

				//b = WS::begin("hallo");//error C2668: 'WS::begin': ambiguous call to overloaded function "Microsoft Visual Studio Enterprise 2019 Version 16.8.3"
				b = WS::begin<char const,_countof(x)>(x);
				b = WS::begin<char const,_countof("hallo")>("hallo");
				e = WS::end<char const,_countof("hallo")>("hallo");
				Assert::IsTrue( std::distance(b,e) != _countof("hallo") );
				Assert::IsTrue( *e==0 );
				e = std::end("hallo");
				Assert::IsTrue( std::distance(b,e) == _countof("hallo") );
			}
		}

		TEST_METHOD(UT_int_liste)
		{
			Assert::IsTrue( WS::is_in(1,1,3,5,7) );
			Assert::IsTrue( WS::is_in(5,1,3,5,7) );
			Assert::IsTrue( WS::is_in(7,1,3,5,7) );
			Assert::IsFalse( WS::is_in(9,1,3,5,7) );
		}
		TEST_METHOD(UT_int_Bereich)
		{
			Assert::IsTrue( WS::is_in(1,WS::bereich(1,3),WS::bereich(5,7)) );
			Assert::IsTrue( WS::is_in(5,WS::bereich(1,3),WS::bereich(5,7)) );
			Assert::IsTrue( WS::is_in(7,WS::bereich(1,3),WS::bereich(5,7)) );
			Assert::IsFalse( WS::is_in(9,WS::bereich(1,3),WS::bereich(5,7)) );

			Assert::IsFalse( WS::is_not_in(1,WS::bereich(1,3),WS::bereich(5,7)) );
			Assert::IsFalse( WS::is_not_in(5,WS::bereich(1,3),WS::bereich(5,7)) );
			Assert::IsFalse( WS::is_not_in(7,WS::bereich(1,3),WS::bereich(5,7)) );
			Assert::IsTrue( WS::is_not_in(9,WS::bereich(1,3),WS::bereich(5,7)) );

		}
		TEST_METHOD(UT_int_Bereich_und_liste)
		{
			Assert::IsTrue( WS::is_in(1,1,3,WS::bereich(5,7)) );
			Assert::IsTrue( WS::is_in(5,WS::bereich(1,3),5,7) );
			Assert::IsTrue( WS::is_in(7,WS::bereich(1,3),WS::bereich(5,7)) );
			Assert::IsFalse( WS::is_in(9,1,WS::bereich(3,5),7) );
		}
		TEST_METHOD(UT_char_array_per_initilizer)
		{
			Assert::IsTrue( WS::is_in('a', {'a','c','e'} ) );
			Assert::IsTrue( WS::is_in('c', {'a','c','e'} ) );
			Assert::IsTrue( WS::is_in('e', {'a','c','e'} ) );
			Assert::IsFalse( WS::is_in('b', {'a','c','e'} ) );
		}
		TEST_METHOD(UT_char_array_per_array)
		{
			Assert::IsTrue( WS::is_in('a', "ace" ) );
			Assert::IsTrue( WS::is_in('c', "ace" ) );
			Assert::IsTrue( WS::is_in('e', "ace" ) );
			Assert::IsFalse( WS::is_in('\0', "ace" ) );//seit 2021-01-07 false
			Assert::IsFalse( WS::is_in('b', "ace" ) );
		}
		TEST_METHOD(UT_char_array_per_begin)
		{
			char const * text="ace";

			//using geht_nicht_als_std::begin;
			//using geht_nicht_als_std::end;
			//using ::begin;
			//using ::end;
			using WS::begin;
			using WS::end;
			using std::begin;
			using std::end;

			//static_assert(WS::exist_begin<decltype(text)>::value, "begin(char const *) existiert");
			//auto bx = begin((unsigned char const*)text);bx =bx;
			auto b = begin(text);
			b=b;
			Assert::IsTrue( *b++ == 'a');
			Assert::IsTrue( *b++ == 'c');
			Assert::IsTrue( *b++ == 'e');
			Assert::IsTrue( b == end(text));

			//Assert::IsTrue( WS::is_in('a', text ) );
			//Assert::IsTrue( WS::is_in('c', text ) );
			//Assert::IsTrue( WS::is_in('e', text ) );
			//Assert::IsFalse( WS::is_in('\0', text ) );
			//Assert::IsFalse( WS::is_in('b', text ) );
		}

		TEST_METHOD(UT_char_per_container)
		{
			Assert::IsTrue( WS::is_in('a', std::vector<char>{'a','c','e'} ) );
			Assert::IsTrue( WS::is_in('c', std::vector<char>{'a','c','e'} ) );
			Assert::IsTrue( WS::is_in('e', std::vector<char>{'a','c','e'} ) );
			Assert::IsFalse( WS::is_in('b', std::vector<char>{'a','c','e'} ) );
		}
		TEST_METHOD(UT_short_int)
		{
			//short gesucht = 5;
			//Assert::IsTrue( WS::is_in(gesucht, 1,2,3,4,5,6 ) );//error C2672: 'is_in': no matching overloaded function found
			//Assert::IsFalse( WS::is_in(gesucht, 1,2,3,4,6 ) );//error C2672: 'is_in': no matching overloaded function found
		}
		TEST_METHOD(UT_nullterminiert_pointervergleich)
		{
			auto const * text = "hallo welt";
			
			Assert::IsTrue( WS::is_in(text, (decltype(text))"hallo",(decltype(text))"welt",(decltype(text))"hallo welt" ) );//pointervergleich
			Assert::IsTrue( WS::is_in(text, (decltype(text))"hallo",(decltype(text))"welt",(decltype(text))"hallo" " " "welt" ) );//pointervergleich
		}
		TEST_METHOD(UT_nullterminiert_pointervergleich_per_new)
		{
			std::string text = "hallo welt";

			Assert::IsFalse( WS::is_in(text.c_str(), (decltype(text.c_str()))"hallo",(decltype(text.c_str()))"welt",(decltype(text.c_str()))"hallo welt" ) );//pointervergleich
			Assert::IsFalse( WS::is_in(text.c_str(), (decltype(text.c_str()))"hallo",(decltype(text.c_str()))"welt",(decltype(text.c_str()))"hallo" " " "welt" ) );//pointervergleich
		}
		template<typename char_t>struct text_wrapper
		{
			char_t const * ptr;
			text_wrapper(char_t const * ptr):ptr(ptr){}
			bool operator==( char_t const * r ) const{ return stringcmp(this->ptr,r)==0; }
			bool operator==( text_wrapper const & r ) const{ return stringcmp(this->ptr,r.ptr)==0; }
		};
		TEST_METHOD(UT_nullterminiert_textwrapper)
		{
			std::string text = "hallo welt";

			auto x = text_wrapper(text.c_str());//geht mit c++17 c++14-> error C2955: 'UTisin::UTisin::text_wrapper': use of class template requires template argument list
			//auto x = text_wrapper<char>(text.c_str());
			Assert::IsTrue( WS::is_in(text_wrapper{text.c_str()}, text_wrapper{"hallo"},(text_wrapper<char>)"welt",text_wrapper{"hallo welt"} ) );
			Assert::IsFalse( WS::is_in(text_wrapper{text.c_str()}, text_wrapper{"hallo"},(text_wrapper<char>)"welt",text_wrapper{"hallo  welt"} ) );
		}
		TEST_METHOD(UT_string_sz)
		{
			char const * ptext = "hallo welt";
			std::string text = ptext;

			Assert::IsTrue( WS::is_in( text, std::string{ptext} ));
			Assert::IsTrue( WS::is_in( text, ptext ));
		}
		TEST_METHOD(UT_string_vector_sz)
		{
			char const * ptext = "hallo welt";
			std::string text = ptext;
			std::vector<char const *> vec{"hallo","welt"};
			Assert::IsFalse( WS::is_in( text, vec ));
			Assert::IsTrue( WS::is_in( text, vec, ptext ));
			vec.push_back( ptext );
			Assert::IsTrue( WS::is_in( text, vec ));

		}
	};
}
