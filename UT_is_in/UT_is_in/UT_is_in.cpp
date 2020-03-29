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
	template<typename char_t>char_t const * begin(char_t const * p){return p;}
	template<typename char_t>char_t const * end(char_t const * p){return p+stringlen(p);}
}

namespace UTisin
{
	TEST_CLASS(UTisin)
	{
	public:
		
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
		//TEST_METHOD(UT_char_array_per_begin)
		//{
		//	char const * text="ace";

		//	//using geht_nicht_als_std::begin;
		//	//using geht_nicht_als_std::end;
		//	//using ::begin;
		//	//using ::end;
		//	using std::begin;
		//	using std::end;

		//	//static_assert(WS::exist_begin<decltype(text)>::value, "begin(char const *) existiert");
		//	auto b = begin(text);
		//	Assert::IsTrue( *b++ == 'a');
		//	Assert::IsTrue( *b++ == 'c');
		//	Assert::IsTrue( *b++ == 'e');
		//	Assert::IsTrue( b == end(text));

		//	//Assert::IsTrue( WS::is_in('a', text ) );
		//	//Assert::IsTrue( WS::is_in('c', text ) );
		//	//Assert::IsTrue( WS::is_in('e', text ) );
		//	//Assert::IsFalse( WS::is_in('\0', text ) );
		//	//Assert::IsFalse( WS::is_in('b', text ) );
		//}

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
