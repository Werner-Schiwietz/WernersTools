#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <algorithm>

#include "..\..\headeronly\parse_helper.h"
#include "..\..\headeronly\iterator_access.h"

#include "..\..\headeronly\SignatureTest.h"

#include <string>
#include <guiddef.h>
#include <rpc.h>

#pragma warning(push,4)

template<typename T> T xxxx()
{
	static_assert(false);
}


namespace UTPaserFkt
{
	template<typename container_t
			,typename function_t
			, int = ( WS::canCall<function_t,bool(decltype(*std::declval<container_t>().begin()))>::value?1:0
				    + WS::canCall<function_t,container_t(container_t&)>::value?2:0 )
				>
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
			Assert::IsTrue( WS::canCall<decltype(fn),void(int)>::value );//der int-parameter wird gecastet, keine ahnung, wie man das prüfen kann
		}
		TEST_METHOD( UT_test_using_callable_function )
		{
			{
				auto to_parse = WS::iterator_access( "hallo welt" );
				auto erg = eat( to_parse, []( char const & ) { return true; } );
				Assert::IsFalse( to_parse );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg == WS::iterator_access( "hallo welt" ) );
			}
			{
				auto to_parse = WS::iterator_access( "hallo welt" );
				auto erg = eat( to_parse, []( WS::_iterator_access<char const *> & container ) 
				{ 
					WS::_iterator_access<char const *> retvalue {container.begin(),container.begin()+5}; 
					container.begin()+=5;
					return retvalue;
				} );
				Assert::IsTrue( to_parse == WS::iterator_access( " welt" ) );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg == WS::iterator_access( "hallo" ) );
			}
			{
				auto to_parse = WS::iterator_access( "hallo welt" );
				//auto erg = eat( to_parse, []( std::wstring & container ) { return std::wstring{}; } );
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
			Assert::IsTrue( erg==WS::parse_error::invalid_escape_sequence);
			Assert::IsTrue( erg.eaten_till_error == WS::iterator_access( "hal" ) );
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
					//Assert::IsTrue( erg==WS::iterator_access( "hallo" ) ); //error, ging in früherer compilerversion
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
			Assert::IsTrue( erg.eaten == WS::iterator_access( "hallo" ) );
			Assert::IsTrue( erg.left == '\'' );
			Assert::IsTrue( erg.right == '\'' );
			Assert::IsTrue( toparse == WS::iterator_access( " 'welt'" ) );
		}
		TEST_METHOD( eat_positiv_different_first_last )
		{
			auto toparse = WS::iterator_access( "[[hallo\\]] 'welt'" );
			auto erg = WS::eat_flanked( toparse, WS::left_type('['), WS::right_type(']'), WS::escape_type('\\') );
			Assert::IsTrue( erg );
			Assert::IsTrue( erg.eaten == WS::iterator_access( "[hallo\\]" ) );
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
			return WS::remove_escape( without_flank.eaten, escape );
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
				auto erg = WS::eat_space( toparse );
				Assert::IsTrue( toparse == WS::iterator_access("hallo") );
				Assert::IsTrue( erg );
				Assert::IsTrue( erg == WS::iterator_access("   \t") );
				erg = WS::skip_space( toparse );
				Assert::IsFalse( erg );
			}
			{
				auto toparse = WS::iterator_access( L"   \thallo" );
				auto erg = WS::eat_space( toparse );
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
				Assert::IsTrue( erg.error==WS::parse_error::interger_overflow );
			}
		}
		TEST_METHOD( UT_parse_guid )
		{
			auto eat_guid = [](auto guidstring)
			{
				auto toparse = guidstring;

				using iterator_t = decltype(guidstring)::iterator_t;
				struct rettype_eat_guid : WS::rettype_eat<iterator_t>
				{
					using base_t = WS::rettype_eat<iterator_t>;
					_GUID  guid {};

					rettype_eat_guid(WS::_iterator_access<iterator_t> & container) : base_t{container,container.begin(),container.begin()} {};

					operator _GUID const &() const { return guid; }
				}ret_value{toparse};

				if( auto erg = WS::eat(toparse,L'{'); !erg )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}

				if( auto erg = WS::eat_integer<unsigned _int32,16>(toparse); !erg || erg.eaten.len()!=8 )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				else 
					ret_value.guid.Data1 = erg.value;

				if( auto erg = WS::eat(toparse,L'-'); !erg )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				if( auto erg = WS::eat_integer<unsigned _int16,16>(toparse); !erg || erg.eaten.len()!=4 )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				else 
				{
					ret_value.guid.Data2 = erg.value;
				}

				if( auto erg = WS::eat(toparse,L'-'); !erg )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				if( auto erg = WS::eat_integer<unsigned _int16,16>(toparse); !erg || erg.eaten.len()!=4 )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				else 
					ret_value.guid.Data3 = erg.value;

				if( auto erg = WS::eat(toparse,L'-'); !erg )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				auto erg3_1 = WS::eat_integer<unsigned _int16,16>(toparse);
				if( !erg3_1 || erg3_1.eaten.len()!=4 )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				if( auto erg = WS::eat(toparse,L'-'); !erg )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				auto erg3_2 = WS::eat_integer<unsigned _int64,16>(toparse);
				if( !erg3_2 || erg3_2.eaten.len()!=12 )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}
				if( auto erg = WS::eat(toparse,L'}'); !erg )
				{
					ret_value.eaten_till_error = ret_value.eaten;
					return ret_value;
				}

				ret_value.guid.Data4[0]= static_cast<unsigned char>(erg3_1.value>>8);
				ret_value.guid.Data4[1]= static_cast<unsigned char>(erg3_1.value);
				auto v = erg3_2.value;
				for( int i=6; i --> 0; )
				{
					ret_value.guid.Data4[i+2]= static_cast<unsigned char>(v);
					v>>=8;
				}

				return ret_value;
			};
			//GUID guid; 
			auto guidtext		= L"{12345678-9ABC-DEF0-1234-56789ABCDEF0}hallo";
			auto guidtextpure	= L"12345678-9ABC-DEF0-1234-56789ABCDEF0";
			auto toparse = WS::iterator_access(guidtext);
			{
				auto erg = eat_guid(toparse);
				Assert::IsTrue( erg );
				_GUID guid{};

				UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<std::remove_const_t<std::remove_pointer_t<decltype(guidtextpure)>>*>(guidtextpure)),&guid);
				Assert::IsTrue( erg.guid == guid );
			}
		}
	};

}
