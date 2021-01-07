#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
namespace WS
{
	template<typename T>	struct _is_char_type			: std::false_type{};
	template<>				struct _is_char_type<char>		: std::true_type{};
	template<>				struct _is_char_type<wchar_t>	: std::true_type{};
	template<typename T>	using is_char_type = _is_char_type<std::remove_cv_t<std::remove_reference_t<T>>>;
	template<typename T>	static bool constexpr is_char_type_v = is_char_type<T>::value;

	static_assert( is_char_type_v<char> );
	static_assert( is_char_type_v<char const> );
	static_assert( is_char_type_v<char const *> == false );
	static_assert( is_char_type_v<char *> == false );
	static_assert( is_char_type_v<char const * const > == false );
	static_assert( is_char_type_v<char const &> == true );
	static_assert( is_char_type_v<const char> );
	static_assert( is_char_type_v<wchar_t> );
	static_assert( is_char_type_v<unsigned char> == false );
	static_assert( std::is_same_v<char,__int8> == true );//???
	static_assert( is_char_type_v<__int8> == std::is_same_v<char,__int8> );


	//???? ambiguous wenn das so gemacht wird??
	//template<typename char_t,size_t size> auto begin(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){return ar;}
	//template<typename char_t,size_t size> auto end(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{})
	//{
	//	size_t index=0;
	//	for( ; index<size && ar[index]; ++index ) {}
	//	return &ar[0]+index;
	//}	template<typename char_t,size_t size> auto begin(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){return ar;}
	template<typename char_t,size_t size> auto end(char_t (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{})
	{
		size_t index=0;
		for( ; index<size && ar[index]; ++index ) {}
		return &ar[0]+index;
	}
	template<typename char_t,size_t size> auto begin(char_t const (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{}){return ar;}
	template<typename char_t,size_t size> auto end(char_t const (&ar)[size]) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{})
	{
		size_t index=0;
		for( ; index<size && ar[index]; ++index ) {}
		return &ar[0]+index;
	}
	//\0-terminierte strings
	template<typename char_t> auto begin(char_t const * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{}){return p;}
	template<typename char_t> auto end(char_t const * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t const *>{}){while(*p)++p;return p;}
	template<typename char_t> auto begin(char_t * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){return p;}
	template<typename char_t> auto end(char_t * p) noexcept ->decltype( std::enable_if_t<is_char_type_v<char_t>, char_t *>{}){while(*p)++p;return p;}
}


namespace WS_test
{
	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type __exist_std_begin(unsigned long);
	template <typename T> auto __exist_std_begin(int) -> decltype( std :: begin( std::declval<T>() ), std::true_type{} );		//kann dereferenziert werden
	template <typename T> using _exist_std_begin = decltype(__exist_std_begin<T>(0));
	template <typename T> static auto const _exist_std_begin_v = _exist_std_begin<T>::value;

	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type __exist_WS_begin(unsigned long);
	template <typename T> auto __exist_WS_begin(int) -> decltype( WS :: begin( std::declval<T>() ), std::true_type{} );		//kann dereferenziert werden
	template <typename T> using _exist_WS_begin = decltype(__exist_WS_begin<T>(0));
	template <typename T> static auto const _exist_WS_begin_v = _exist_WS_begin<T>::value;

	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type __exist_begin(unsigned long);
	template <typename T> auto __exist_begin(int) -> decltype( :: begin( std::declval<T>() ), std::true_type{} );		//kann dereferenziert werden
	template <typename T> using _exist_begin = decltype(__exist_begin<T>(0));
	template <typename T> static auto const _exist_begin_v = _exist_begin<T>::value;
}
namespace WS_has_method
{
	//SFINAE: Substitution Failure Is Not An Error
	template <typename> std::false_type _begin(unsigned long);
	template <typename T> auto _begin(int) -> decltype( std::declval<T>().begin(), std::true_type{} );		//kann dereferenziert werden
	template <typename T> using begin = decltype(_begin<T>(0));
	template <typename T> static auto const begin_v = begin<T>::value;
}

namespace UTisin
{
	TEST_CLASS(UTisin)
	{
	public:
		TEST_METHOD(UT_find_begin)
		{
			{
				using namespace WS_test;
				static_assert( _exist_std_begin_v<int> == false );
				static_assert( _exist_std_begin_v<char const *> == false );
				static_assert( _exist_std_begin_v<decltype("hallo")> == true );
				decltype("hallo") x{};x;
				static_assert( _exist_std_begin_v<decltype(x)> == true );
				static_assert( _exist_WS_begin_v<char const *> == true );
				static_assert( _exist_WS_begin_v<char *> == true );

				static_assert( WS_has_method::begin_v<int> == false );
				static_assert( WS_has_method::begin_v<char const *> == false );
				static_assert( WS_has_method::begin_v<decltype("hallo")> == false );
				static_assert( WS_has_method::begin_v<std::vector<int>> == true );

				//static_assert( _exist_WS_begin_v<decltype(x)> == true ); //weil ...
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
			Assert::IsTrue( WS::is_in('\0', "ace" ) );
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
			short gesucht = 5;
			Assert::IsTrue( WS::is_in(gesucht, 1,2,3,4,5,6 ) );
			Assert::IsFalse( WS::is_in(gesucht, 1,2,3,4,6 ) );
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
	};
}
