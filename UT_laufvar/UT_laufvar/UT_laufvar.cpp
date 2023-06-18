#include "pch.h"


#include "CppUnitTest.h"


#include "..\..\headeronly\loopvalue.h"
#include "..\..\headeronly\iterator_access.h"

#include <iostream>
#include <cassert>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#pragma region Cout2Output
#include <mutex>
#include <map>
#include <iostream>
namespace
{
    //cout umleitung////////////////////////////////////
    class LoggerStreambuf : public std::streambuf
    {
    public:
        virtual int_type overflow( int_type c = EOF ) {
            static std::mutex mutex{};
            auto locked = std::lock_guard(mutex);
            static std::map<std::thread::id,std::string> buf;
            if( c != EOF )
            {
                if( c != '\n' )
                    buf[std::this_thread::get_id()] += static_cast<char>(c);
                else
                {
                    Logger::WriteMessage( buf[std::this_thread::get_id()].c_str() );
                    buf.erase(std::this_thread::get_id());
                }
            }
            return c;
        }
    };
    template<typename streambuf_t=LoggerStreambuf>
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
    //cout umleitung////////////////////////////////////
}
#pragma endregion


namespace UT_loopvalue
{
    TEST_CLASS(UT_loopvalue)
    {
    public:
        TEST_METHOD(example1)
        {
            int v = 7;
            for( auto value : WS::loopvalue{7,5} )
            { 
                Assert::IsTrue( value == v-- );
            }
            Assert::IsTrue( v == 4 );
        }
        TEST_METHOD(example2)
        {
            unsigned __int8 v = 255;
            for( WS::loopvalue value{unsigned __int8(255),unsigned __int8(0)}; value.isvalid(); ++value )
            { 
                Assert::IsTrue( value == v-- );
            }
            Assert::IsTrue( v == 255 );//overflow
        }
        TEST_METHOD(example3)
        {
            unsigned __int8 v = 255;
            using defaultdirection=bool;
            for( WS::loopvalue<unsigned __int8> value{255,0,defaultdirection{false}}; value.isvalid(); ++value )
            { 
                Assert::IsTrue( value == v++ );
            }
            Assert::IsTrue( v == 1 );
        }
        TEST_METHOD(iterator_comp_exception)
        {
            auto lc1 = WS::loopvalue{7,5};
            auto lc2 = WS::loopvalue{7,6};

            try
            {
                (void)(lc1.begin() == lc2.begin());//throw exception
                Assert::Fail(L"exception erwartet");
            }
            catch(...){}

        }
        TEST_METHOD(range_based_for)
        {
            auto v = 2;
            for( auto value : WS::loopvalue{v,5} )
            {
                Assert::IsTrue(value==v++);
            }
            Assert::IsTrue(v==6);
            for( auto value : WS::loopvalue{v,5} )
            {
                Assert::IsTrue(value==v--);
            }
            Assert::IsTrue(v==4);
        }
        TEST_METHOD(iterator)
        {
            auto b = unsigned __int8(0);
            auto v = unsigned __int8(3);
            auto loopvalue = WS::loopvalue{v,b};

            //assert(0);
            auto iter = loopvalue.begin();
            Assert::IsTrue( iter != loopvalue.end() );
            Assert::IsTrue( *iter == 3 );
            Assert::IsTrue( iter != loopvalue.end() );
            Assert::IsTrue( *++iter == 2 );
            Assert::IsTrue( iter != loopvalue.end() );
            Assert::IsTrue( *(++iter).operator->() == 1 );
            Assert::IsTrue( iter != loopvalue.end() );
            Assert::IsTrue( *++iter == 0 );
            Assert::IsTrue( iter++ != loopvalue.end() );
            Assert::IsTrue( iter == loopvalue.end() );
            try
            {
                iter.operator->();
                Assert::Fail(L"exception erwartet");
            }
            catch(...){}
        }
        TEST_METHOD(gesamter_wertebereich)
        {
            Cout2Output umleiten;
            std::cout << __FUNCTION__ << "\r\n";
            unsigned char v=0;
            for( auto i=WS::loopvalue<unsigned char>(0,255); i.isvalid(); ++i )
            {
                std::cout << (unsigned int)i << ' ';
                Assert::IsTrue( i == v++ );
            }
            std::cout << std::endl;
        }
        TEST_METHOD(gesamter_wertebereich_high_to_low)
        {
            Cout2Output umleiten;
            std::cout << __FUNCTION__ << "\r\n";
            for( auto i=WS::loopvalue<unsigned char>(255,0); i.isvalid(); ++i )
            {
                std::cout << (unsigned int)i << ' ';
            }
            std::cout << std::endl;
        }
        TEST_METHOD(digits)
        {
            Cout2Output umleiten;
            std::cout << __FUNCTION__ << "\r\n";
            for( auto i=WS::loopvalue('0','9'); i.isvalid(); ++i )
            {
                std::cout << i << ' ';
            }
            std::cout << "\r\n";
            for( auto i=WS::loopvalue('9','0'); i.isvalid(); ++i )
            {
                std::cout << i << ' ';
            }
            std::cout << std::endl;
        }
        TEST_METHOD(falsche_richtung)
        {
            Cout2Output umleiten;
            std::cout << __FUNCTION__ << "\r\n";
            auto values = WS::iterator_access(std::initializer_list<unsigned __int8>{3,2,1,0,255,254});
            auto valueIter = values.begin();
            for( auto i=WS::loopvalue<unsigned __int8>( 3, 254, false ); i.isvalid(); ++i )
            {
                std::cout << (int)i << ' ';
                Assert::IsTrue( i == *valueIter++ );
            }
            std::cout << std::endl;
        }
        TEST_METHOD(uint_0_bis)
        {
            auto i=WS::loopvalue(0u,2u);
            Assert::IsTrue( i.isvalid() );
            decltype(i)::loopvalue_t v = i;//mit startwert belegen
            Assert::IsTrue( i++ == v++ );
            Assert::IsTrue( i == v );
            Assert::IsTrue( ++i == ++v );
            ++i;
            Assert::IsFalse( i.isvalid() );
            Assert::IsTrue( i.isinvalid() );
            try
            {
                v = i;
                Assert::Fail(L"exception erwartet");
            }
            catch(...){}
        }
        TEST_METHOD(uint_bis_0)
        {
            auto i=WS::loopvalue(2u,0u);
            Assert::IsTrue( i.isvalid() );
            decltype(i)::loopvalue_t v = i;//mit startwert belegen
            Assert::IsTrue( i++ == v-- );
            Assert::IsTrue( i == v );
            Assert::IsTrue( ++i == --v );
            ++i;
            Assert::IsFalse( i.isvalid() );
            try
            {
                v = i;
                Assert::Fail(L"exception erwartet");
            }
            catch(...){}
        }
        TEST_METHOD(int_1_bis_1)
        {
            auto i=WS::loopvalue(1,1);
            Assert::IsTrue( i.isvalid() );
            decltype(i)::loopvalue_t v = i;//mit startwert belegen

            Assert::IsTrue( i++ == v-- );
            Assert::IsTrue( i.isvalid()==false );
            Assert::IsTrue( i.isinvalid() );
            try
            {
                v = i;
                Assert::Fail(L"exception erwartet");
            }
            catch(...){}
        }
        TEST_METHOD(int_0_bis_0)
        {
            auto i=WS::loopvalue{0,0};
            Assert::IsTrue( i.isvalid() );
            decltype(i)::loopvalue_t v = i;//mit startwert belegen

            Assert::IsTrue( i++ == v-- );
            Assert::IsTrue( i.isvalid()==false );
            Assert::IsTrue( i.isinvalid() );
            try
            {
                v = i;
                Assert::Fail(L"exception erwartet");
            }
            catch(...){}
        }
        TEST_METHOD(unsigend_short_max_bis_max)
        {
            //auto i=WS::loopvalue<unsigned short>{-1,-1};//error C2398: Element '1': conversion from 'int' to 'unsigned short' requires a narrowing conversion
            auto i=WS::loopvalue{unsigned short(-1),(unsigned short)-1};
            Assert::IsTrue( i.isvalid() );
            decltype(i)::loopvalue_t v = i;//mit startwert belegen

            Assert::IsTrue( i++ == v-- );
            Assert::IsTrue( i.isvalid()==false );
            Assert::IsTrue( i.isinvalid() );
            try
            {
                v = i;
                Assert::Fail(L"exception erwartet");
            }
            catch(...){}
        }
    };
}
