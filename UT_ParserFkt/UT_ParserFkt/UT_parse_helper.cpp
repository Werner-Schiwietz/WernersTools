#include "pch.h"
#include "CppUnitTest.h"

// /permissive- /Zc:twoPhase- 
//char * test_permissive = "xxx";//mit /permissive- compile-error

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <algorithm>

#include "..\..\headeronly\parse_helper.h"
#include "..\..\headeronly\iterator_access.h"

#include "..\..\headeronly\SignatureTest.h"
#include "..\..\headeronly\is_in.h"

#include <functional>
#include <string>
#include <guiddef.h>
#include <rpc.h>
#include <vector>

#pragma warning(push,4)

template<typename T> T X()
{
	static_assert(false);
	return T{};
}

namespace WS
{
	template<typename iterator_t> auto eat_guid( WS::_iterator_access<iterator_t> & guidstring, typename WS::_iterator_access<iterator_t>::value_t delimiter= WS::_iterator_access<iterator_t>::value_t( '-' ) )
	{
		auto toparse = guidstring;
		using value_t = WS::_iterator_access<iterator_t>::value_t;
		auto left_parentheses = value_t{'{'};
		auto right_parentheses = value_t{'}'};

		//using iterator_t = std::remove_reference_t<decltype(guidstring)>::iterator_t;
		struct rettype_eat_guid : WS::rettype_eat<iterator_t>
		{
			using base_t = WS::rettype_eat<iterator_t>;
			_GUID	guid{};
			bool	with_parenthese = false;

			rettype_eat_guid( WS::_iterator_access<iterator_t> & container ) : base_t{container,container.begin(),container.begin()} {}

			operator _GUID const & () const { return guid; }
		}ret_value{toparse};

		ret_value.with_parenthese = WS::eat( toparse, left_parentheses );
		ret_value.eaten.end()=toparse.begin();

		if( auto erg = WS::eat_integer<unsigned _int32, 16>( toparse ); !erg || erg.eaten.len()!=8 )
		{
			ret_value.error = parse_error::length;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		else
		{
			ret_value.guid.Data1 = erg.value;
		}
		ret_value.eaten.end()=toparse.begin();

		if( auto erg = WS::eat( toparse, delimiter ); !erg )
		{
			ret_value.error = parse_error::delimiter;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		ret_value.eaten.end()=toparse.begin();

		if( auto erg = WS::eat_integer<unsigned _int16, 16>( toparse ); !erg || erg.eaten.len()!=4 )
		{
			ret_value.error = parse_error::length;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		else
		{
			ret_value.guid.Data2 = erg.value;
		}
		ret_value.eaten.end()=toparse.begin();

		if( auto erg = WS::eat( toparse, delimiter ); !erg )
		{
			ret_value.error = parse_error::delimiter;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		ret_value.eaten.end()=toparse.begin();

		if( auto erg = WS::eat_integer<unsigned _int16, 16>( toparse ); !erg || erg.eaten.len()!=4 )
		{
			ret_value.error = parse_error::length;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		else
			ret_value.guid.Data3 = erg.value;
		ret_value.eaten.end()=toparse.begin();

		if( auto erg = WS::eat( toparse, delimiter ); !erg )
		{
			ret_value.error = parse_error::delimiter;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		ret_value.eaten.end()=toparse.begin();

		auto erg3_1 = WS::eat_integer<unsigned _int16, 16>( toparse );
		if( !erg3_1 || erg3_1.eaten.len()!=4 )
		{
			ret_value.error = parse_error::length;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		ret_value.eaten.end()=toparse.begin();

		if( auto erg = WS::eat( toparse, delimiter ); !erg )
		{
			ret_value.error = parse_error::delimiter;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		ret_value.eaten.end()=toparse.begin();

		auto erg3_2 = WS::eat_integer<unsigned _int64, 16>( toparse );
		if( !erg3_2 || erg3_2.eaten.len()!=12 )
		{
			ret_value.error = parse_error::length;
			ret_value.eaten_till_error = ret_value.eaten;
			return ret_value;
		}
		ret_value.eaten.end()=toparse.begin();

		if( ret_value.with_parenthese )
		{
			if( auto erg = WS::eat( toparse, right_parentheses ); !erg )
			{
				ret_value.error = WS::parse_error::left_without_right;
				ret_value.eaten_till_error = ret_value.eaten;
				return ret_value;
			}
			ret_value.eaten.end()=toparse.begin();
		}

		ret_value.guid.Data4[0]= static_cast<unsigned char>(erg3_1.value>>8);
		ret_value.guid.Data4[1]= static_cast<unsigned char>(erg3_1.value);
		auto v = erg3_2.value;
		for( int i=6; i--> 0; )
		{
			ret_value.guid.Data4[i+2]= static_cast<unsigned char>(v);
			v>>=8;
		}

		guidstring.begin()=toparse.begin();
		return ret_value;
	}
	template<typename iterator_t> auto eat_guid( WS::_iterator_access<iterator_t> && guidstring, typename WS::_iterator_access<iterator_t>::value_t delimiter= WS::_iterator_access<iterator_t>::value_t( '-' ) )
	{
		return eat_guid( guidstring, delimiter );
	}
	template<typename char_t> auto eat_guid( char_t const * guidstring )
	{
		auto toparse = WS::iterator_access( guidstring );
		return eat_guid( toparse );
	}
}
 
namespace UT_char_helper
{
	template<typename char_t> bool operator==( digit_range<char_t> const & l, digit_range<char_t> const & r )
	{
		return 
			l.lower == r.lower &&
			l.upper == r.upper &&
			l.start_char == r.start_char;
	}

	TEST_CLASS(UT_digit)
	{
		TEST_METHOD(UT_digit_range_list)
		{
			#undef ns
			#define ns ::
			//#define ns ::new_digit::
			{
				decltype(auto) rangelist = ns digit_def_ranges<10,char>();
				auto iter = rangelist.begin();
				Assert::IsTrue( *iter == digit_range{0,10,'0'} );
				Assert::IsTrue( ++iter == rangelist.end() );
			}
			{
				decltype(auto) rangelist = ns digit_def_ranges<12,char>();
				auto iter = rangelist.begin();
				Assert::IsTrue( *iter == digit_range{0,10,'0'} );
				Assert::IsTrue( *++iter == digit_range{10,36,'A'} );
				Assert::IsTrue( *++iter == digit_range{10,36,'a'} );
				Assert::IsTrue( ++iter == rangelist.end() );
			}
			{
				decltype(auto) rangelist = ns digit_def_ranges<10,wchar_t>();
				auto iter = rangelist.begin();
				Assert::IsTrue( *iter == digit_range{0,10,L'0'} );
				Assert::IsTrue( ++iter == rangelist.end() );
			}


			auto test_initilizer_list_as_parameter = []( auto container)
			{
				return container.size();
			};

			Assert::IsTrue( 1==test_initilizer_list_as_parameter( ns digit_def_ranges<8,char>() ) );
			Assert::IsTrue( 3==test_initilizer_list_as_parameter( ns digit_def_ranges<12,char>() ) );


		}
		TEST_METHOD(digit)
		{
			#undef ns
			#define ns ::
			//#define ns ::new_digit::

			#pragma region digit 
			{
				constexpr unsigned int radix = 10;
				auto numeric_one='\x1';
				auto lowest = '0';
				auto highest = '9';
				{
					auto erg = ns digit(lowest);
					Assert::IsTrue( erg );
					Assert::IsTrue( erg.value==0 );
				}
				{
					auto erg = ns digit(highest);
					Assert::IsTrue( erg );
					Assert::IsTrue( erg.value==radix-1 );
				}
				{
					auto erg = ns digit(lowest-numeric_one);
					Assert::IsFalse( erg );
				}
				{
					auto erg = ns digit(highest+numeric_one);
					Assert::IsFalse( erg );
				}
			}
			#pragma endregion

			#pragma region digit<10> 
			{
				constexpr unsigned int radix = 10;
				auto numeric_one='\x1';
				auto lowest = '0';
				auto highest = '9';
				{
					auto erg = ns digit<radix>(lowest);
					Assert::IsTrue( erg );
					Assert::IsTrue( erg.value==0 );
				}
				{
					auto erg = ns digit<radix>(highest);
					Assert::IsTrue( erg );
					Assert::IsTrue( erg.value==radix-1 );
				}
				{
					auto erg = ns digit<radix>(lowest-numeric_one);
					Assert::IsFalse( erg );
				}
				{
					auto erg = ns digit<radix>(highest+numeric_one);
					Assert::IsFalse( erg );
				}
			}
		#pragma endregion
			#pragma region digit<1> 
				{
					constexpr unsigned int radix = 2;
					auto lowest = L'0';
					auto highest = L'1';
					auto numeric_one=L'\x1';
					{
						auto erg = ns digit<radix>(lowest);
						Assert::IsTrue( erg );
						Assert::IsTrue( erg.value==0 );
					}
					{
						auto erg = ns digit<radix>(highest);
						Assert::IsTrue( erg );
						Assert::IsTrue( erg.value==radix-1 );
					}
					{
						auto erg = ns digit<radix>(lowest-numeric_one);
						Assert::IsFalse( erg );
					}
					{
						auto erg = ns digit<radix>(highest+numeric_one);
						Assert::IsFalse( erg );
					}
				}
			#pragma endregion
			#pragma region digit<12> 
				{
					constexpr unsigned int radix = 12;
					unsigned char numeric_one='\x1';
					unsigned char lowest = '0';
					unsigned char highest = 'B';
					{
						auto erg = ns digit<radix>(lowest);
						Assert::IsTrue( erg );
						Assert::IsTrue( erg.value==0 );
					}
					{
						auto erg = ns digit<radix>(highest);
						Assert::IsTrue( erg );
						Assert::IsTrue( erg.value==radix-1 );
					}
					{
						auto erg = ns digit<radix>(lowest-numeric_one);
						Assert::IsFalse( erg );
					}
					{
						auto erg = ns digit<radix>(highest+numeric_one);
						Assert::IsFalse( erg );
					}
				}
			#pragma endregion
			#pragma region digit<16> 
				{
					constexpr unsigned int radix = 16;
					auto numeric_one=L'\x1';
					auto lowest = L'0';
					auto highest = L'f';
					{
						auto erg = ns digit<radix>(lowest);
						Assert::IsTrue( erg );
						Assert::IsTrue( erg.value==0 );
					}
					{
						auto erg = ns digit<radix>(highest);
						Assert::IsTrue( erg );
						Assert::IsTrue( erg.value==radix-1 );
					}
					{
						auto erg = ns digit<radix>(lowest-numeric_one);
						Assert::IsFalse( erg );
					}
					{
						auto erg = ns digit<radix>(highest+numeric_one);
						Assert::IsFalse( erg );
					}
				}
			#pragma endregion

		}
		TEST_METHOD(KaktovikischesZahlensystem)
		{
			#undef ns
			#define ns ::
			//#define ns ::new_digit::

			constexpr wchar_t Null{L'A'};//keine ahnung was die 0 im kaktovikischen Zahlensystem ist, also nehmen wir einfach A

			auto ranges = {digit_range<wchar_t>{0,20,Null}};

			constexpr unsigned int radix = 20;
			wchar_t numeric_one=L'\x1';
			wchar_t lowest = Null;
			wchar_t highest = wchar_t{Null+19};

			for( unsigned int num=0; num<radix; ++num )
			{
				auto erg = ns digit<radix>( wchar_t(lowest+num), ranges );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.value==num );
			}
			{
				auto erg = ns digit<radix>(lowest-numeric_one);
				Assert::IsFalse( erg );
			}
			{
				auto erg = ns digit<radix>(highest+numeric_one);
				Assert::IsFalse( erg );
			}
		}
	};
}
namespace UTParserFkt
{
	template<typename container_t,typename function_t> constexpr int eater_fn = 
		( WS::canCall<function_t,bool(decltype(*std::declval<container_t>().begin()))>::value?1:0
		  + WS::canCall<function_t,container_t(container_t&)>::value?2:0 );

	template<typename container_t,typename function_t, int = eater_fn<container_t,function_t>>
	struct _eat
	{
		static container_t call( container_t &container, function_t fn)
		{
			static_assert(false, "funktion hat falsche Signatur");
			return container_t{};
		}
	};
	template<typename container_t, typename function_t>
	struct _eat<container_t,function_t,1>
	{
		static container_t call( container_t & container, function_t fn )
		{
			container_t retvalue{ container.begin(), container.begin()};
			while( container.begin()!=container.end() 
				   && fn( *container.begin() ) )
			{
				retvalue.end() = ++container.begin();
			}
			
			return retvalue;
		}
	};
	template<typename container_t, typename function_t>
	struct _eat<container_t,function_t,2>
	{
		static container_t call( container_t &container, function_t fn)
		{
			return fn( container );
		}
	};

	template<typename container_t, typename function_t>
	static container_t eat( container_t & container, function_t fn )
	{
		return _eat<container_t, function_t>::call( container, fn );
	}
	TEST_CLASS(UT_CanCall)
	{
	public:
		TEST_METHOD(CanCall_negativ1)
		{
			auto fn = [&]( bool ) { return false; };
			Assert::IsFalse( WS::canCall<decltype(fn),void(bool)>::value );
		}
		TEST_METHOD(CanCall_positiv1)
		{
			auto fn = [&]( bool ) { };
			Assert::IsTrue( WS::canCall<decltype(fn),void(bool)>::value );
			Assert::IsTrue( WS::canCall<decltype(fn),void(int)>::value );//der int-parameter wird gecastet, keine ahnung, wie man das pr�fen kann
		}
		TEST_METHOD( UT_test_using_callable_function )
		{
			{
				auto to_parse = WS::iterator_access( "hallo welt" );
				auto fn_eat_all = []( char const & ) { return true; }; //eat all
				static_assert( UTParserFkt::eater_fn<decltype(to_parse),decltype(fn_eat_all)> == 1 );

				auto erg = eat( to_parse, fn_eat_all );//eat all
				Assert::IsFalse( to_parse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg == WS::iterator_access( "hallo welt" ) );
			}
			{
				auto to_parse = WS::iterator_access( "hallo welt" );
				auto fn_eat_5_chars = []( WS::_iterator_access<char const *> & container ) 
				{ //eat first 5 chars
					WS::_iterator_access<char const *> retvalue {container.begin(),container.begin()+5}; 
					container.begin()+=5;
					return retvalue;
				};
				static_assert( UTParserFkt::eater_fn<decltype(to_parse),decltype(fn_eat_5_chars)> == 2 );

				auto erg = eat( to_parse, fn_eat_5_chars );
				Assert::IsTrue( to_parse == WS::iterator_access( " welt" ) );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg == WS::iterator_access( "hallo" ) );
			}
			{
				[[maybe_unused]]auto to_parse = WS::iterator_access( "hallo welt" );
				auto fn = []( [[maybe_unused]]std::wstring & container ) { return std::wstring{}; }; //falsche signatur
				constexpr int x = UTParserFkt::eater_fn<decltype(to_parse),decltype(fn)>;
				static_assert( x!=1 && x!=2 );
				//[[maybe_unused]]auto erg = eat( to_parse, []( [[maybe_unused]]std::wstring & container ) { return std::wstring{}; } );//error C2338: static_assert failed: 'funktion hat falsche Signatur'
			}
		}


	};
	TEST_CLASS(UT_OneChar)
	{
	public:
		
		TEST_METHOD(eat_negativ)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			Assert::IsFalse(WS::eat( toparse, 'H' ));
			Assert::IsTrue( len1 == toparse.len() );
		}
		TEST_METHOD(eat_positiv)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			Assert::IsTrue(WS::eat( toparse, 'h' ));
			Assert::IsTrue( len1 == toparse.len() + 1 );
		}
		TEST_METHOD(eat_positiv_all)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			auto len = len1-len1;//0
			Assert::IsTrue(WS::eat( toparse, 'h' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'a' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'l' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'l' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue(WS::eat( toparse, 'o' ));
			Assert::IsTrue( len1 == ++len+toparse.len() );
			Assert::IsTrue( len1 == len );
		}
	};
	TEST_CLASS(UT_OneOf)
	{
	public:

		TEST_METHOD(eat_oneof_negativ)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			Assert::IsFalse(WS::eat_oneof( toparse, 'w', 'W' ));
			Assert::IsTrue( len1 == toparse.len() );
		}
		TEST_METHOD(eat_oneof_positiv1)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			auto erg = WS::eat_oneof( toparse, 'H', 'h' );
			Assert::IsTrue(erg);
			Assert::IsTrue(erg==WS::iterator_access( "h" ) );
			Assert::IsTrue( len1 == toparse.len() + 1 );
		}
		TEST_METHOD(eat_oneof_positiv2)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			auto erg = WS::eat_oneof( toparse, WS::iterator_access("Hh") );
			Assert::IsTrue(erg);
			Assert::IsTrue(erg==WS::iterator_access( "h" ) );
			Assert::IsTrue( len1 == toparse.len() + 1 );
		}
	};
	TEST_CLASS(UT_till)
	{
	public:

		TEST_METHOD(eat_till_negativ_till_end_open)
		{
			auto toparse = WS::iterator_access( "'hallo" );
			char begin_end_item = '\'';
			char escape_item = '\\';

			Assert::IsTrue( eat( toparse, begin_end_item ) );
			auto erg = eat_till( toparse, begin_end_item, escape_item );
			Assert::IsFalse( erg );
			Assert::IsTrue( erg==WS::parse_error::tillitem_not_found );
			Assert::IsTrue( erg.eaten_till_error == WS::iterator_access( "hallo" ) );
		}
		TEST_METHOD(eat_till_negativ_till_invalid_escape)
		{
			auto toparse = WS::iterator_access( R"('hal\lo)" );
			char begin_end_item = '\'';
			char escape_item = '\\';

			Assert::IsTrue( eat( toparse, begin_end_item ) );
			auto erg = eat_till( toparse, begin_end_item, escape_item );
			Assert::IsFalse( erg );
			Assert::IsTrue( erg==WS::parse_error::tillitem_not_found);
			Assert::IsTrue( erg.eaten_till_error == WS::iterator_access( "hal\\lo" ) );
		}
		TEST_METHOD(eat_till_positive)
		{
			auto toparse = WS::iterator_access( "'hallo welt', how are you " );
			char begin_end_item = '\'';
			char escape_item = '\\';

			Assert::IsTrue( eat( toparse, begin_end_item ) );
			auto erg = eat_till( toparse, begin_end_item, escape_item );
			Assert::IsTrue( erg );
			Assert::IsTrue( erg==WS::parse_error::none);
			Assert::IsTrue( erg.eaten == WS::iterator_access( "hallo welt" ) );
			Assert::IsTrue( toparse == WS::iterator_access( "', how are you " ) );
		}
		TEST_METHOD(eat_till_positive_with_escape)
		{
			auto toparse = WS::iterator_access( R"('hallo\' welt\\', how are you)" );
			char begin_end_item = '\'';
			char escape_item = '\\';

			Assert::IsTrue( eat( toparse, begin_end_item ) );
			auto erg = eat_till( toparse, begin_end_item, escape_item );
			Assert::IsTrue( erg );
			Assert::IsTrue( erg==WS::parse_error::none);
			Assert::IsTrue( erg.eaten == WS::iterator_access( R"(hallo\' welt\\)" ) );
			Assert::IsTrue( toparse == WS::iterator_access( "', how are you" ) );
		}
	};

	TEST_CLASS( UT_flanked )
	{
	public:

		TEST_METHOD( eat_positiv1 )
		{
			auto toparse = WS::iterator_access( "'hallo' 'welt'" );
			if( auto erg1=WS::eat_oneof( toparse, '"', '\'' ) )
				if( auto erg=WS::eat_till( toparse, *erg1.begin(), '\\' ) )
				{
					Assert::IsTrue( WS::eat( toparse, *erg1.begin() ) );
					Assert::IsTrue( erg.eaten==WS::iterator_access( "hallo" ) ); //so geht es
					//Assert::IsTrue( ((decltype(erg.eaten))erg)==WS::iterator_access( "hallo" ) ); //so geht es
					//Assert::IsTrue( erg==WS::iterator_access( "hallo" ) ); //error, ging in fr�herer compilerversion
					return;
				}

			Assert::Fail( L"tja" );
		}
		TEST_METHOD( eat_positiv_empty )
		{
			auto toparse = WS::iterator_access( "''" );
			auto erg = WS::eat_flanked( toparse, WS::flanked_type('\''), WS::escape_type('\\') );
			Assert::IsTrue( erg.error == WS::parse_error::none );
			Assert::IsTrue( erg );
			Assert::IsTrue( toparse == WS::iterator_access( "" ) );
		}
		TEST_METHOD( eat_negativ_same_first_last )
		{
			auto toparse = WS::iterator_access( "'hallo welt" );
			auto erg = WS::eat_flanked( toparse, WS::flanked_type('\''), WS::escape_type('\\') );
			Assert::IsFalse( erg );
			//Assert::IsTrue( erg.eaten_till_error == WS::iterator_access( "hallo" ) );
			Assert::IsTrue( erg.error == WS::parse_error::tillitem_not_found );
			Assert::IsTrue( toparse == WS::iterator_access( "'hallo welt" ) );
		}
		TEST_METHOD( eat_positiv_same_first_last )
		{
			auto toparse = WS::iterator_access( "'hallo' 'welt'" );
			auto erg = WS::eat_flanked( toparse, WS::flanked_type('\''), WS::escape_type('\\') );
			Assert::IsTrue( erg );
			Assert::IsTrue( erg.value == WS::iterator_access( "hallo" ) );
			Assert::IsTrue( erg.left == '\'' );
			Assert::IsTrue( erg.right == '\'' );
			Assert::IsTrue( toparse == WS::iterator_access( " 'welt'" ) );
		}
		TEST_METHOD( eat_positiv_different_first_last )
		{
			auto toparse = WS::iterator_access( "[[hallo\\]] 'welt'" );
			auto erg = WS::eat_flanked( toparse, WS::left_type('['), WS::right_type(']'), WS::escape_type('\\') );
			Assert::IsTrue( erg );
			Assert::IsTrue( erg.value == WS::iterator_access( "[hallo\\]" ) );
			Assert::IsTrue( erg.left == '[' );
			Assert::IsTrue( erg.right == ']' );
			Assert::IsTrue( toparse == WS::iterator_access( " 'welt'" ) );
		}
		TEST_METHOD( eat_positiv_oneof_first_last )
		{
			auto toparse = WS::iterator_access( "'hallo' 'welt'" );
			auto erg = WS::eat_flanked( toparse, WS::iterator_access("\"'"), WS::escape_type('\\') );
			Assert::IsTrue( erg );
			Assert::IsTrue( erg.eaten == WS::iterator_access( "hallo" ) );
			Assert::IsTrue( erg.left == '\'' );
			Assert::IsTrue( erg.right == '\'' );
			Assert::IsTrue( toparse == WS::iterator_access( " 'welt'" ) );
		}
		TEST_METHOD(make_flanked)
		{
			auto erg = WS::make_flanked<std::string>( WS::iterator_access("Hallo"), WS::left_type('['), WS::right_type(']'), WS::escape_type('\\') );
			Assert::IsTrue( erg=="[Hallo]" );

			erg = WS::make_flanked<std::string>( WS::iterator_access("Ha[ll]o\\Welt"), '[', ']', '\\' );
			Assert::IsTrue( erg=="[Ha[ll\\]o\\\\Welt]" );

		}
		template<typename char_t> auto make_eat_remove_helper( char_t const * text, char_t left, char_t right, char_t escape )
		{
			auto flanked_out = WS::make_flanked<std::basic_string<char_t>>( WS::iterator_access(text), left,right,escape );
			auto without_flank = WS::eat_flanked( WS::iterator_access( std::move(flanked_out) ), left,right,escape );
			return WS::remove_escape( without_flank.value, escape );
		}
		TEST_METHOD(remove_escape_LPCSTR)
		{
			auto erg1 = WS::remove_escape( WS::iterator_access("hallo"), '\\' );
			Assert::IsTrue( erg1==WS::iterator_access( "hallo" ) );
			auto erg2 = WS::remove_escape( WS::iterator_access(std::string("hallo")), '\\' );
			Assert::IsTrue( erg2==WS::iterator_access( "hallo" ) );
		}
		TEST_METHOD(iterator_access__append)
		{
			auto buf = WS::iterator_access("Hallo Welt");
			auto erg = buf.left( 1 );
			Assert::IsTrue( erg == WS::iterator_access("H") );
			erg += buf.mid( 1, 1 );
			Assert::IsTrue( erg == WS::iterator_access("Ha") );
			erg += buf.mid( 2, 2 );
			Assert::IsTrue( erg == WS::iterator_access("Hall") );

			try
			{
				erg += buf.mid( 5, 1 );
				Assert::Fail( L"exception erwartet, l.end()!=r.begin()" );
			}
			catch(...){}
		}
		TEST_METHOD(remove_flank_std_string_with_change)
		{
			auto ursprung = std::wstring(LR"("Hallo"\Welt)");
			auto flanke = WS::flanked_type(L'"');
			auto escape = WS::escape_type(L'\\');
			{
				auto flanked = WS::make_flanked<std::basic_string<wchar_t>>( WS::iterator_access(std::wstring(ursprung)), flanke, escape );
				auto ohne_flanke = WS::remove_flank( WS::iterator_access(flanked), flanke, flanke, escape );
				Assert::IsTrue( WS::iterator_access(ursprung)==ohne_flanke);
				Assert::IsTrue( has_value(ohne_flanke.rvalue_lifetime_extender) );
			}
		}

		TEST_METHOD(remove_flank_std_string_without_change)
		{
			auto ursprung = std::wstring(LR"(Hallo Welt)");
			auto flanke = WS::flanked_type(L'"');
			auto escape = WS::escape_type(L'\\');
			{
				auto flanked = WS::make_flanked<std::basic_string<wchar_t>>( WS::iterator_access(std::wstring(ursprung)), flanke, escape );
				auto ohne_flanke = WS::remove_flank( WS::iterator_access(flanked), flanke, flanke, escape );
				Assert::IsTrue( WS::iterator_access(ursprung)==ohne_flanke);
				Assert::IsFalse( has_value(ohne_flanke.rvalue_lifetime_extender) );
			}
		}

		TEST_METHOD(remove_flank_LPCTSR_with_change)
		{
			auto ursprung = LR"("Hallo"\Welt)";
			auto flanke = WS::flanked_type(L'"');
			auto escape = WS::escape_type(L'\\');
			{
				auto flanked = WS::make_flanked<std::basic_string<wchar_t>>( WS::iterator_access(ursprung), flanke, escape );
				auto ohne_flanke = WS::remove_flank( WS::iterator_access(flanked), flanke, flanke, escape );
				Assert::IsTrue( WS::iterator_access(ursprung)==ohne_flanke);
				Assert::IsTrue( has_value(ohne_flanke.rvalue_lifetime_extender) );
			}
		}

		TEST_METHOD(remove_flank_LPCTSR_without_change)
		{
			auto ursprung = LR"(Hallo Welt)";
			auto flanke = WS::flanked_type(L'"');
			auto escape = WS::escape_type(L'\\');
			{
				auto flanked = WS::make_flanked<std::basic_string<wchar_t>>( WS::iterator_access(ursprung), flanke, escape );
				auto ohne_flanke = WS::remove_flank( WS::iterator_access(flanked), flanke, flanke, escape );
				Assert::IsTrue( WS::iterator_access(ursprung)==ohne_flanke);
				Assert::IsFalse( has_value(ohne_flanke.rvalue_lifetime_extender) );
			}
		}
		TEST_METHOD(remove_flanked_with_change)
		{
			{
				auto ursprung = R"("Hallo"\Welt\)";
				auto erg = make_eat_remove_helper( ursprung, WS::left_type( '"' ), WS::right_type( '"' ), WS::escape_type( '\\' ) );
				Assert::IsTrue( erg ==WS::iterator_access(ursprung) );
			}
			{
				auto ursprung = R"(Hallo "heile" Welt)";
				auto erg = make_eat_remove_helper( ursprung, WS::left_type( '"' ), WS::right_type( '"' ), WS::escape_type( '\\' ) );
				Assert::IsTrue( erg ==WS::iterator_access(ursprung) );
			}
		}
		TEST_METHOD(remove_flanked_without_change)
		{
			{
				{
					auto ursprung = R"(Hallo Welt)";
					auto erg = make_eat_remove_helper( ursprung, WS::left_type( '"' ), WS::right_type( '"' ), WS::escape_type( '\\' ) );
					Assert::IsTrue( erg ==WS::iterator_access(ursprung) );
				}
				{
					auto ursprung = R"()";
					auto erg = make_eat_remove_helper( ursprung, WS::left_type( '"' ), WS::right_type( '"' ), WS::escape_type( '\\' ) );
					Assert::IsTrue( erg ==WS::iterator_access(ursprung) );
				}
			}
		}
	};

	TEST_CLASS( UT_find_replace )
	{
		TEST_METHOD(find_replace)
		{
			auto toparse = WS::iterator_access( R"(hallo welt, how is die welt today)" );
			using iterator_t = decltype(toparse)::iterator_t;

			using vecitem_t = std::pair<WS::_iterator_access<iterator_t>,WS::_iterator_access<iterator_t>>;
			std::vector<vecitem_t> find_replace_values
			{ 
				vecitem_t{WS::iterator_access( R"(welt)" ),WS::iterator_access( R"(world)" )} ,
				vecitem_t{WS::iterator_access( R"(die)" ),WS::iterator_access( R"(the)" )} 
			};

			auto fn = [& find_replace_values]( WS::_iterator_access<iterator_t> toparse ) -> WS::find_replace_t<iterator_t> 
			{
				while( toparse )
				{
					auto akt = toparse;
					for( auto const & [find,replace] : find_replace_values )
					{
						if( WS::eat(toparse,find) )
						{
							WS::find_replace_t<iterator_t> retvalue;
							retvalue.found=akt;
							retvalue.found.end() = toparse.begin();
							retvalue.newvalue = replace;
							return retvalue;
						}
					}
					eat(toparse);
				}
				return {};
			};

			//auto convertiert1 = WS::find_replace( toparse, fn );

			{
				auto erg = fn(toparse);
				Assert::IsTrue(erg);
			}

			auto convertiert = WS::find_replace( toparse, std::function<WS::find_replace_t<iterator_t>(WS::_iterator_access<iterator_t>)>(fn) );
			Assert::IsTrue(convertiert);
			Assert::IsTrue(convertiert == WS::iterator_access(R"(hallo world, how is the world today)") );

		}
	};
	TEST_CLASS(UT_Chars)
	{
	public:

		TEST_METHOD(eat_negativ)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			auto erg = WS::eat( toparse, WS::iterator_access( "haL" ));
			Assert::IsFalse(erg);
			Assert::IsTrue( len1 == toparse.len() );
			Assert::IsTrue( erg.eaten_till_error==toparse.left( 2 ) );
		}
		TEST_METHOD(eat_positiv)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len();
			Assert::IsTrue(WS::eat( toparse, WS::iterator_access("hal") ));
			Assert::IsTrue( len1 == toparse.len() + 3 );
		}
		TEST_METHOD(eat_positiv_all)
		{
			auto toparse = WS::iterator_access( "hallo" );
			auto len1 = toparse.len(); len1;
			Assert::IsTrue(WS::eat( toparse, WS::iterator_access("hal") ));
			Assert::IsTrue(WS::eat( toparse, WS::iterator_access("lo") ));
			Assert::IsTrue( toparse.len()==0 );
		}
	};
	TEST_CLASS( UT_eat_while )
	{
	public:

		TEST_METHOD( eat_negativ )
		{
			auto fnIsdigit = []( auto Char ) { return isdigit( Char ); };
			auto toparse = WS::iterator_access( "Hallo" );
			Assert::IsTrue( 0==WS::eat_while( toparse, fnIsdigit ).len() );
			Assert::IsTrue( toparse == WS::iterator_access("Hallo") );
		}
		TEST_METHOD( eat_positiv )
		{
			auto fnIsdigit = []( auto Char ) { return isdigit( Char ); };
			{
				auto toparse = WS::iterator_access( "123Hallo" );
				Assert::IsTrue( 3==WS::eat_while( toparse, fnIsdigit ).len() );
				Assert::IsTrue( toparse == WS::iterator_access("Hallo") );
			}
			{
				auto toparse = WS::iterator_access( L"123Hallo" );
				Assert::IsTrue( 3==WS::eat_while( toparse, fnIsdigit ).len() );
				Assert::IsTrue( toparse == WS::iterator_access(L"Hallo") );
			}
		}
		TEST_METHOD( skip_space )
		{
			{
				auto toparse = WS::iterator_access( "   \thallo" );
				auto erg = WS::skip_space( toparse );
				Assert::IsTrue( toparse == WS::iterator_access("hallo") );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.eaten == WS::iterator_access("   \t") );
				erg = WS::skip_space( toparse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.eaten.empty() );
			}
			{
				auto toparse = WS::iterator_access( L"   \thallo" );
				auto erg = WS::skip_space( toparse );
				Assert::IsTrue( toparse == WS::iterator_access(L"hallo") );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.eaten == WS::iterator_access(L"   \t") );
				erg = WS::skip_space( toparse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.eaten.empty() );
			}
		}
		TEST_METHOD( eat_space )
		{
			{
				auto toparse = WS::iterator_access( "   \thallo" );
				auto erg = WS::eat_spaces( toparse );
				Assert::IsTrue( toparse == WS::iterator_access("hallo") );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg == WS::iterator_access("   \t") );
				erg = WS::skip_space( toparse );
				Assert::IsFalse( erg );
			}
			{
				auto toparse = WS::iterator_access( L"   \thallo" );
				auto erg = WS::eat_spaces( toparse );
				Assert::IsTrue( toparse == WS::iterator_access(L"hallo") );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg == WS::iterator_access(L"   \t") );
				erg = WS::eat_space( toparse );
				Assert::IsFalse( erg );
			}
		}
	};
	TEST_CLASS( UT_eat_komplex )
	{
	public:
		TEST_METHOD( UT_eat_integercount )
		{
			auto toparse = WS::iterator_access( L"Hallo" );
			{
				auto erg = WS::eat_integercount<int>( toparse, 2 );
				Assert::IsFalse( erg );
				erg = WS::eat_integercount<int>( toparse, 0,2 );
				Assert::IsTrue( erg );
			}
			toparse = WS::iterator_access( L"12345" );
			{
				auto erg = WS::eat_integercount<int,16>( toparse, 2 );
				Assert::IsTrue(erg);
				Assert::IsTrue(erg.value==0x12);
				erg = WS::eat_integercount<int,16>( toparse, 2 );
				Assert::IsTrue(erg);
				Assert::IsTrue(erg.value==0x34);
				erg = WS::eat_integercount<int,16>( toparse, 2 );
				Assert::IsFalse(erg);
				Assert::IsTrue(toparse==WS::iterator_access(L"5"));
			}
			toparse = WS::iterator_access( L"123456" );
			{
				auto erg = WS::eat_integercount<int>( toparse, 1, 3 );
				Assert::IsTrue(erg);
				Assert::IsTrue(erg.value==123);
				erg = WS::eat_integercount<int>( toparse, 1, 2 );
				Assert::IsTrue(erg);
				Assert::IsTrue(erg.value==45);
				erg = WS::eat_integercount<int,16>( toparse, 2, 3 );
				Assert::IsFalse(erg);
				Assert::IsTrue(toparse==WS::iterator_access(L"6"));
			}
		}
		TEST_METHOD( UT_eat_integercount_overflow )
		{
			auto toparse = WS::iterator_access( L"12345" );

			{
				auto erg = WS::eat_integercount<__int8,10>( toparse, 5 );
				Assert::IsFalse(erg);
				Assert::IsTrue(toparse==WS::iterator_access(L"12345"));
			}
		}
		TEST_METHOD( UT_eat_integer_negativ )
		{
			{
				auto toparse = WS::iterator_access( L"Hallo" );
				auto erg = WS::eat_integer<int>( toparse );
				Assert::IsFalse( erg );
			}
			{
				auto toparse = WS::iterator_access( L"" );
				auto erg = WS::eat_integer<int>( toparse );
				Assert::IsFalse( erg );
			}
			{
				WS::_iterator_access<char *> toparse;
				auto erg = WS::eat_integer<unsigned short>( toparse );
				Assert::IsFalse( erg );
			}
		}
		TEST_METHOD( UT_eat_integer_positiv )
		{
			{
				auto toparse = WS::iterator_access( L"123Hallo" );
				auto erg = WS::eat_integer<int>( toparse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.value==123 );
				Assert::IsTrue( toparse == WS::iterator_access( L"Hallo" ) );
			}
			{
				auto toparse = WS::iterator_access( "123" );
				auto erg = WS::eat_integer<unsigned short>( toparse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.value==123 );
			}
		}
		TEST_METHOD( UT_eat_integer_positiv_rvalue_lifetime_extender_test )
		{
			{
				auto erg = WS::eat_integer<unsigned short>( WS::iterator_access( std::wstring(L"123Hallo") ) );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.value == 123 );
			}
		}
		TEST_METHOD( UT_eat_integer_negativ_rvalue_lifetime_extender_test )
		{
			{
				auto erg = WS::eat_integer<short>( WS::iterator_access( std::wstring(L"-123Hallo") ) );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.value == -123 );
			}
		}		
		TEST_METHOD( UT_stringto_int_mit_vorzeichen )
		{
			{
				for( auto text : {"123","+123","-123"," - 123","+"} )
				{
					auto toparse = WS::iterator_access( text );
					WS::skip_space(toparse);
					bool negative=false;
					if(auto erg=WS::eat_oneof( toparse, '-', '+' ) )
					{
						negative = *erg.begin()=='-';
					}
					WS::skip_space(toparse);

					short value;
					if( auto erg = WS::eat_integer<short>(toparse) )
					{
						value = erg.value;
						Assert::IsTrue( value == 123 );
						if( negative )
							value = -value;
					}
					else
					{
						Assert::IsTrue( stringcmp( text, "+" )==0 );
					}
				}
				auto erg = WS::eat_integer<short>( WS::iterator_access( std::wstring(L"- 123Hallo") ) );
				Assert::IsTrue( erg );//eat_integer wertet keine  vorzeichen aus
				Assert::IsTrue( erg.value == -123 );
			}
		}		
		TEST_METHOD( UT_eat_integer_positiv_overflow )
		{
			{
				auto toparse = WS::iterator_access( std::wstring(L"1234") );//with rvalue_lifetime_extender test
				auto x = toparse;
#pragma warning(suppress:4996)
				auto xi = *x.GetNextEntry();
				Assert::IsTrue( xi==L'1' );
#pragma warning(suppress:4996)
				xi = *x.GetNextEntry();
				Assert::IsTrue( xi==L'2' );
#pragma warning(suppress:4996)
				xi = *x.GetNextEntry();
				Assert::IsTrue( xi==L'3' );
#pragma warning(suppress:4996)
				xi = *x.GetNextEntry();
				Assert::IsTrue( xi==L'4' );
				Assert::IsTrue( x.empty() );
//#pragma warning(suppress:4996)
//				xi = *x.GetNextEntry();

				auto erg = WS::eat_integer<__int8>( toparse );
				Assert::IsFalse( erg );
				Assert::IsTrue( erg.error==WS::parse_error::integer_overflow );
			}
		}
		TEST_METHOD( UT_parse_guid )
		{
			//GUID guid; 
			auto guidtext		= L"{12345678-9ABC-DEF0-1234-56789ABCDEF0}hallo";
			auto guidtextpure	= L"12345678-9ABC-DEF0-1234-56789ABCDEF0";

			{
				auto erg = WS::eat_guid(guidtextpure);
				Assert::IsTrue( erg );
				Assert::IsFalse( erg.with_parenthese );
				auto toparse = WS::iterator_access(guidtext);
				erg = WS::eat_guid(toparse);
				Assert::IsTrue( erg );
				Assert::IsTrue( erg.with_parenthese );
				Assert::IsTrue( toparse==WS::iterator_access(L"hallo") );
				toparse = WS::iterator_access(guidtext+1);
				erg = WS::eat_guid(toparse);
				Assert::IsTrue( erg );
				Assert::IsFalse( erg.with_parenthese );
				Assert::IsTrue( toparse==WS::iterator_access(L"}hallo") );

				_GUID guid{};
				auto erg2=UuidFromStringW(reinterpret_cast<RPC_WSTR>(const_cast<std::remove_const_t<std::remove_pointer_t<decltype(guidtextpure)>>*>(guidtextpure)),&guid);
				Assert::IsTrue(erg2==RPC_S_OK);

				Assert::IsTrue( erg.guid == guid );
				Assert::IsTrue( guid == erg );
			}

			{
				auto toparse = WS::iterator_access( "12345678-9ABC-DEF0-1234-56789ABCDEF0}" );
				auto erg = WS::eat_guid( toparse );
				Assert::IsTrue( erg );
				Assert::IsTrue( *toparse == '}' );
			}
		}
		TEST_METHOD( UT_parse_guid_failed )
		{
			{
				auto guidtest = "12345678-0ABC-DEF0-1234-56789ABCDEF0";

				_GUID guid{};
				//RPC_S_OK RPC_S_INVALID_STRING_UUID
				auto erg2 = UuidFromStringA(reinterpret_cast<RPC_CSTR>(const_cast<std::remove_const_t<std::remove_pointer_t<decltype(guidtest)>>*>(guidtest)),&guid);
				Assert::IsTrue(erg2==RPC_S_OK);
			}
			{
				auto guidtest = "12345678-ABC-DEF0-1234-56789ABCDEF0";

				_GUID guid{};
				auto erg2 = UuidFromStringA(reinterpret_cast<RPC_CSTR>(const_cast<std::remove_const_t<std::remove_pointer_t<decltype(guidtest)>>*>(guidtest)),&guid);
				Assert::IsFalse(erg2==RPC_S_OK);

				auto erg = WS::eat_guid( "12345678-ABC-DEF0-1234-56789ABCDEF0" );
				Assert::IsFalse(erg);
				Assert::IsTrue(erg.error==WS::parse_error::length);
			}
		}
	};
}

