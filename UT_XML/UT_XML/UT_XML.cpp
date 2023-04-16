#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <iostream>
#include <iomanip>
#pragma region cout umleitung
//wcout umleitung////////////////////////////////////
template<typename char_t>
class LoggerStreambuf : public std::basic_streambuf<char_t, std::char_traits<char_t>>
{
public:
    using base_t = std::basic_streambuf<char_t, std::char_traits<char_t>>;
    using int_type = typename base_t::int_type;
    virtual int_type overflow( int_type c = EOF ) 
    {
        static std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t>> buf;
        if( c != EOF )
        {
            if( c != '\n' )
                buf += static_cast<decltype(buf)::traits_type::char_type>( c );
            else
            {
                Logger::WriteMessage( buf.c_str() );
                buf.clear();
            }
        }
        return c;
    }
};
template<typename streambuf_t=LoggerStreambuf<char>>
class Cout2Output
{
    streambuf_t dbgstream;
    std::streambuf *default_stream;

public:
    Cout2Output() {
        default_stream = std::cout.rdbuf( &dbgstream );
    }

    ~Cout2Output() {
        std::cout.rdbuf( default_stream );
    }
};
template<typename streambuf_t=LoggerStreambuf<wchar_t>>
class WCout2Output
{
    streambuf_t dbgstream;
    std::wstreambuf *default_stream;

public:
    WCout2Output() {
        default_stream = std::wcout.rdbuf( &dbgstream );
    }

    ~WCout2Output() {
        std::wcout.rdbuf( default_stream );
    }
};
//cout umleitung////////////////////////////////////
#pragma endregion

#include "..\..\experimental\headeronly\xml.h"

namespace UT_XML
{
    std::wstring xml{ 
        LR"(<?xml version="1.0" encoding="utf-8"?>)"
        LR"(<!--)"
        LR"(tree.natvis benutzerdefinierte Watchansicht zu WS::tree_t WS::node_t)"
        LR"(von Werner Schiwietz)"
        LR"(kann z.B. den Projektdateien hinzugefügt werden)"
        LR"(-->)"
        LR"(<AutoVisualizer xmlns="http://schemas.microsoft.com/vstudio/debugger/natvis/2010">)"
        LR"(<Type Name="WS::tree_t&lt;*&gt;::node_t">)"
        LR"(<DisplayString Condition="parent==0">{{value:{value} left:{left} right:{right} parent:&lt;NULL&gt;}}</DisplayString>)"
        LR"(<DisplayString >{{value:{value} left:{left} right:{right} parent:{(void*)parent}}}</DisplayString>)"
        LR"(<Expand>)"
        LR"(<Item Name="value">value</Item>)"
        LR"(<Item Name="left">*left</Item>)"
        LR"(<Item Name="right">*right</Item>)"
        LR"(<Item Name="parent">*parent</Item>)"
        LR"(<TreeItems>)"
        LR"(<!--<Size>_Mysize</Size>-->)"
        LR"(<HeadPointer>parent</HeadPointer>)"
        LR"(<LeftPointer>left._Mypair._Myval2</LeftPointer>)"
        LR"(<RightPointer>right._Mypair._Myval2</RightPointer>)"
        LR"(<ValueNode >value</ValueNode>)"
        LR"(</TreeItems>)"
        LR"(</Expand>)"
        LR"(</Type>)"
        LR"(<Type Name="WS::tree_t&lt;*&gt;">)"
        LR"(<DisplayString >{*root}</DisplayString>)"
        LR"(<Expand>)"
        LR"(<Item Name="root">*root</Item>)"
        LR"(<!--<TreeItems>)"
        LR"(--><!--<Size>_Mysize</Size>--><!--)"
        LR"(<HeadPointer>root._Mypair._Myval2</HeadPointer>)"
        LR"(<LeftPointer>root._Mypair._Myval2->left._Mypair._Myval2</LeftPointer>)"
        LR"(<RightPointer>root._Mypair._Myval2->right._Mypair._Myval2</RightPointer>)"
        LR"(<ValueNode >root->value</ValueNode>)"
        LR"(</TreeItems>-->)"
        LR"(</Expand>)"
        LR"(</Type>)"
        LR"(</AutoVisualizer>)"};

	TEST_CLASS(UTXML)
	{
	public:
		
        TEST_METHOD(eat_whitespace)
        {
            {
                auto toparse = WS::iterator_access(" ");
                Assert::IsTrue(WS::XML::eat_whitespace(toparse));
                Assert::IsTrue(toparse.empty()==true);
                Assert::IsTrue(WS::XML::eat_whitespace(WS::iterator_access("\t")));
                Assert::IsTrue(WS::XML::eat_whitespace(WS::iterator_access("\x0a")));
                Assert::IsTrue(WS::XML::eat_whitespace(WS::iterator_access("\x0d")));

                Assert::IsFalse(WS::XML::eat_whitespace(WS::iterator_access("a")));
                Assert::IsFalse(WS::XML::eat_whitespace(WS::iterator_access("\b")));
            }
            {
                auto toparse = WS::iterator_access(" \t   x");
                auto eaten = WS::XML::eat_whitespaces(toparse);
                Assert::IsTrue( *toparse.begin() == 'x' );
                Assert::IsTrue( eaten.len() == 5 );
            }
        }
        TEST_METHOD(eat_name)
        {
            {
                auto toparse = WS::iterator_access(L" hallo welt");
                Assert::IsFalse(WS::XML::eat_name(toparse));
                Assert::IsTrue(WS::XML::eat_whitespace(toparse));
                {
                    auto eaten = WS::XML::eat_name(toparse);
                    Assert::IsTrue(eaten);
                    Assert::IsTrue(eaten==WS::iterator_access(L"hallo"));
                }
                {
                    Assert::IsFalse( WS::XML::eat_name(WS::iterator_access(L"1hallo")) );
                }
                {
                    auto name=L"_name1";
                    auto toparse2 = WS::iterator_access(name);
                    auto eaten = WS::XML::eat_name(toparse2);
                    Assert::IsTrue(eaten);
                    Assert::IsTrue(eaten==WS::iterator_access(name));
                }
            }
        }
        TEST_METHOD(eat_comment)
		{
			{
				auto erg = WS::XML::eat_comment( WS::iterator_access("hallo") );
				Assert::IsFalse( erg );
				Assert::IsFalse( erg.error() );
			}
			{
				auto toparse = WS::iterator_access("<!-- hallo Kommentar-->hier geht es weiter");
				auto erg = WS::XML::eat_comment( toparse );
				Assert::IsTrue( erg );
				Assert::IsFalse( erg.error() );
				Assert::IsTrue( erg.content==WS::iterator_access(" hallo Kommentar") );
				Assert::IsTrue( toparse==WS::iterator_access("hier geht es weiter") );
			}
			{
				auto toparse = WS::iterator_access("<!-- fehlerhafter -- Kommentar-->hier geht es weiter");
				auto erg = WS::XML::eat_comment( toparse );
				Assert::IsFalse( erg );
				Assert::IsTrue( erg.error() );
			}
            {
                auto toparse = WS::iterator_access("<!-- fehlerhafter - - Kommentar -- minus-minus nicht erlaubt  -->hier geht es weiter");
                auto erg = WS::XML::eat_comment( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.errorcode == decltype(erg)::enumError::invalidminusminus );
                Assert::IsTrue( erg.error() );
            }
            {
                auto toparse = WS::iterator_access("<!-- fehlerhafter Kommentar --->hier geht es weiter");
                auto erg = WS::XML::eat_comment( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.error() );
            }
            {
                auto toparse = WS::iterator_access("<!-- fehlerhafter Kommentar im kommentar <!-- inner comment --> -->hier geht es weiter");
                auto erg = WS::XML::eat_comment( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.error() );
            }
            {
                auto toparse = WS::iterator_access("<!-- >");
                auto erg = WS::XML::eat_comment( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.error() );
            }
        }
		TEST_METHOD(eat_prolog)
		{

            auto toparse = WS::iterator_access(xml.c_str());
            //auto toparse = WS::iterator_access(xml);
            auto erg = WS::XML::eat_prolog( toparse );
            Assert::IsTrue( erg );
            Assert::IsFalse( erg.error() );

		}
        TEST_METHOD(eat_attributvalue)
        {
            {
                auto erg = WS::XML::eat_attributvalue( WS::iterator_access("hallo") );
                Assert::IsFalse( erg );
            }
            {
                auto value = R"#(Hallo Welt)#";
                
                {
                    auto toparse = WS::iterator_access( std::string{'"'} + value + '"' );
                    auto erg = WS::XML::eat_attributvalue( toparse );
                    Assert::IsTrue( erg );
                    Assert::IsTrue( erg.value == WS::iterator_access(value) );
                }
                {
                    auto toparse = WS::iterator_access( std::string{'\''} + value + '\'' );
                    auto erg = WS::XML::eat_attributvalue( toparse );
                    Assert::IsTrue( erg );
                    Assert::IsTrue( erg.value == WS::iterator_access(value) );
                }
            }
        }
        TEST_METHOD(eat_attributvalue_withescape)
        {
            auto value = R"#(0 &lt; 1 ist gleich 1 &gt; 0)#";

            {
                auto toparse = WS::iterator_access( std::string{'"'} + value + '"' );
                auto erg = WS::XML::eat_attributvalue( toparse );
                Assert::IsTrue( erg );
                Assert::IsTrue( erg.value == WS::iterator_access(R"#(0 < 1 ist gleich 1 > 0)#") );
            }
        }
        TEST_METHOD(eat_attributvalue_withescape_and_closer)
        {
            auto value = R"#(0 &lt; 1 ist gleich 1 > 0)#";

            {
                auto toparse = WS::iterator_access( std::string{'"'} + value + '"' );
                auto erg = WS::XML::eat_attributvalue( toparse );
                Assert::IsTrue( erg );
                //Assert::IsTrue( erg == WS::iterator_access(R"#(0 < 1 ist gleich 1 > 0)#") );
            }
        }
        TEST_METHOD(appender_without_copy)
        {
            auto value = WS::iterator_access("<hallo welt>");
            auto value_org = value;
            decltype(value) value_erg;
            {
                Assert::IsTrue(WS::eat(value, '<'));
                auto new_value = WS::appender<decltype(value)>( WS::eat(value, WS::iterator_access("hallo")).eaten );
                new_value.append( WS::eat_oneof(value, ' ') );
                new_value.append( WS::eat(value, WS::iterator_access("welt")).eaten );
                value_erg = new_value.move();
                auto secondcallvalueisempty = new_value.move();
                Assert::IsTrue(value_erg==secondcallvalueisempty);//ohne umkopieren bleibt ergebnis nach move erhalten
            }

            Assert::IsTrue( value_erg == WS::iterator_access("hallo welt") );
        }
        TEST_METHOD(appender_with_copy)
        {
            auto value = WS::iterator_access("<hallo welt>");
            auto value_org = value;
            decltype(value) value_erg;
            {
                Assert::IsTrue(WS::eat(value, '<'));
                auto new_value = WS::appender<decltype(value)>( WS::eat(value, WS::iterator_access("hallo")).eaten );
                WS::eat_oneof(value, ' ');
                new_value.append( WS::eat(value, WS::iterator_access("welt")).eaten );
                value_erg = new_value.move();
                auto secondcallvalueisempty = new_value.move();
                Assert::IsTrue(secondcallvalueisempty.empty());
            }

            Assert::IsTrue( value_erg == WS::iterator_access("hallowelt") );
        }
        TEST_METHOD(eat_attribut)
        {
            
            auto toparse = WS::iterator_access(LR"#(name = "0 &lt; 1 ist gleich 1 > 0")#");

            {
                auto erg = WS::XML::eat_attribut( toparse );
                Assert::IsTrue( erg );
                Assert::IsTrue( erg.name == WS::iterator_access(LR"#(name)#") );
                Assert::IsTrue( erg.value == WS::iterator_access(LR"#(0 < 1 ist gleich 1 > 0)#") );
            }

        }
        TEST_METHOD(eat_attribut_with_error)
        {
            {
                auto toparse = WS::iterator_access(LR"#(name="0 &lt; 1 ist gleich 1 > 0)#");
                auto erg = WS::XML::eat_attribut( toparse );
                Assert::IsFalse( erg );
            }
            {
                auto toparse = WS::iterator_access(LR"#(  )#");
                auto erg = WS::XML::eat_attribut( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.errorcode== decltype(erg)::enumError::name_missing );
            }
            {
                auto toparse = WS::iterator_access(LR"#(name "xx")#");
                auto erg = WS::XML::eat_attribut( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.errorcode== decltype(erg)::enumError::assign_missing );
            }
            {
                auto toparse = WS::iterator_access(LR"#(name=)#");
                auto erg = WS::XML::eat_attribut( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.errorcode== decltype(erg)::enumError::value_missing );
            }
            {
                auto toparse = WS::iterator_access(LR"#(name="offenes ende)#");
                auto erg = WS::XML::eat_attribut( toparse );
                Assert::IsFalse( erg );
                Assert::IsTrue( erg.errorcode== decltype(erg)::enumError::value_parseerror );
            }
        }
        TEST_METHOD(eat_STag)
        {
            {
                auto value = WS::iterator_access(LR"#(<name>)#");
                auto toparse = value;
                auto erg = WS::XML::eat_STag( toparse );
                Assert::IsTrue( erg );
                Assert::IsTrue( erg.name == WS::iterator_access(LR"#(name)#") );
                Assert::IsTrue( erg.attribute.size() == 0 );
            }
            {
                auto value = WS::iterator_access(LR"#(<name attr1="hallo">)#");
                auto toparse = value;
                auto erg = WS::XML::eat_STag( toparse );
                Assert::IsTrue( erg );
                Assert::IsTrue( erg.name == WS::iterator_access(LR"#(name)#") );
                Assert::IsTrue( erg.attribute.size() == 1 );
            }
            {
                auto value = WS::iterator_access(LR"#(<name attr1="hallo" attr2 = "welt" >)#");
                auto toparse = value;
                auto erg = WS::XML::eat_STag( toparse );
                Assert::IsTrue( erg );
                Assert::IsTrue( erg.name == WS::iterator_access(LR"#(name)#") );
                Assert::IsTrue( erg.attribute.size() == 2 );
                Assert::IsTrue( erg.attribute[1].name == WS::iterator_access(LR"#(attr2)#") );
                Assert::IsTrue( erg.attribute[1].value == WS::iterator_access(LR"#(welt)#") );
            }
        }
    };
    TEST_CLASS(UT_XML_Basics)
    {
        TEST_METHOD(UT_basechar_vs_namechar)
        {
            WCout2Output<> umleiten;
            std::wcout << std::boolalpha;

            for( auto ch=L' '; ch < L'\xffff'; ++ch )
            {
                auto e1 = WS::XML::_is_name_firstchar(ch);
                auto e2 = WS::XML::_NameStartChar(ch);

                if( e1 != e2 )
                {
                    std::wcout << std::setw(4) << std::setbase(16) << static_cast<unsigned int>(ch) << "'" << ch << L"' _is_name_firstchar=" << e1 << L" _NameStartChar=" << e2;
                    std::wcout << L" NOT equal" << std::endl;
                }
                else
                {
                   // std::wcout << L" equal" << std::endl;
                }

            }
        }
    };
}
