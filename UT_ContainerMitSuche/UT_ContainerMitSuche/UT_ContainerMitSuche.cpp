#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#include <atlstr.h>


//ASSERT ohne dialog
//VERIFY ohne dialog
namespace
{
    //ASSERT/VERIFY ohne dialog
    void AssertMsg( char const * file, int line, char const * text)
    {          
        CStringA str;
        str.Format( "%s(%d) ASSERT: %s", file, line, text );
        Logger::WriteMessage( str.GetString() );
    }
    void AssertMsg( char const * file, int line, wchar_t const * text)
    {          
        CStringW str;
        str.Format( L"%S(%d) ASSERT: %s", file, line, text );
        Logger::WriteMessage( str.GetString() );
    }
}
#define STRINGIFYhelp(x) L#x
#define STRINGIFY(x) STRINGIFYhelp(x)
#undef ASSERT
#ifdef _DEBUG
#define ASSERT(x) {if(!(x))AssertMsg( __FILE__, __LINE__, STRINGIFY(x) ); }
#else
#define ASSERT(x)
#endif
//ASSERT ohne dialog
//VERIFY ohne dialog

#include "..\..\headeronly\containerFastFind.h"
#include "..\..\headeronly\pipe.h"



namespace UTContainerMitSuche
{
    TEST_CLASS(UT_ContainerMitSuche)
    {
        TEST_METHOD(UT_InsertEasy)
        {
            struct Data
            {
                int i;
                std::string s;

                struct GetKey{ auto const & operator()( Data const & data ){ return data.i; } };//return-wert als referenz ist hier unsinn, ich wollte nur prüfen, ob es geht
            };
            //container mit Funktion-objekttype Data::GetKey anlegen
            WS::ContainerMitSuche<std::deque<Data>,Data::GetKey> container;

            //ASSERT(5==6);//schreibt ASSERT ins Test-Fenster
            container.push_back( Data{5,"5"});//index 0
            container.push_back( Data{3,"3"});//index 1
            container.push_back( Data{7,"7"});//index 2

            if( auto index = container.find(4); not index ){}
            else
            {
                Assert::Fail(L"sollte invalid sein");
            }
            if( auto index = container.find(4) )
            {
                Assert::Fail(L"sollte invalid sein");
            }
            if( auto index = container.find(5) )
            {
                Assert::IsTrue( container[index].i==5 );
            }
            else
            {
                Assert::Fail(L"sollte valid sein");
            }

            Assert::IsTrue( container.valid_index( container.find(5) ) );
            Assert::IsFalse( (bool)container.find(4) );
            Assert::IsTrue( (bool)container.find(5) );
            Assert::IsTrue( container.find(5) == 0 );
            Assert::IsTrue( container.valid_index( container.find(7) ) );
            Assert::IsTrue( container.find(7) == 2 );
            Assert::IsTrue( container.valid_index( container.find(3) ) );
            Assert::IsTrue( container.find(3) == 1 );

            Assert::IsFalse( container.valid_index( container.find(4) ) );
            Assert::IsTrue( container.findall(2) == nullptr );
            Assert::IsTrue( container.findall(3) != nullptr );
            Assert::IsTrue( container.findall(3)->size()==1 );
            Assert::IsTrue( container[(container.findall(3)->at(0))].i==3 );
            Assert::IsTrue( container.findall(4) == nullptr );
            Assert::IsTrue( container.findall(5) != nullptr );
            Assert::IsTrue( container.findall(6) == nullptr );
            Assert::IsTrue( container.findall(7) != nullptr );
            Assert::IsTrue( container.findall(8) == nullptr );
            try
            {
                Assert::IsTrue( container.at(100).i == 0 );//liefert value_t{};
                Assert::Fail( L"exception erwartet");
            }
            catch(...)
            {}
            //try
            //{
            //    Assert::IsTrue( container[100].i == 0 );//out of bound im container
            //    Assert::Fail( L"exception erwartet");
            //}
            //catch(...)
            //{}
        }
        TEST_METHOD(UT_InsertEasy_GetKey_Function)
        {
            struct Data
            {
                int i;
                std::string s;
            };
            auto fnGetKey = []( Data const & data ) -> auto //const & //man könnte auch eine referenz auf den key-member liefern, bei int ist das sicher nicht nötig
            {
                return data.i;
            };

            //container mit GetKey-Funktion anlegen
            auto container = WS::ContainerMitSuche<std::vector<Data>,decltype(fnGetKey),WS::null_mutex>{fnGetKey};

            container.push_back( Data{5,"5"} );//index 0
            container.push_back( Data{3,"3"} );//index 1
            container.push_back( Data{7,"7"} );//index 2


            Assert::IsTrue( container.valid_index( container.find(5) ) );
            Assert::IsTrue( container.find(5) == 0 );
            Assert::IsTrue( container.valid_index( container.find(7) ) );
            Assert::IsTrue( container.find(7) == 2 );
            Assert::IsTrue( container.valid_index( container.find(3) ) );
            Assert::IsTrue( container.find(3) == 1 );
        }
        TEST_METHOD(UT_InsertEasy_Data_Ohne_Key)
        {
            struct Data
            {
                int i;

                constexpr bool operator<(Data const& r) const{ return this->i < r.i; }
                //bool operator<(Data const&) const = default;//so compile-error
            };

            Assert::IsTrue( Data{1} < Data{2} );
            //container mit GetKey-Funktion anlegen
            auto container = WS::ContainerMitSuche<std::vector<Data>>{};

            container.push_back( Data{5} );//index 0
            container.push_back( Data{3} );//index 1
            container.push_back( Data{7} );//index 2


            Assert::IsTrue( container.valid_index( container.find(Data{5}) ) );
            Assert::IsTrue( container.find(Data{5}) == 0 );
            Assert::IsTrue( container.valid_index( container.find(Data{7}) ) );
            Assert::IsTrue( container.find(Data{7}) == 2 );
            Assert::IsTrue( container.valid_index( container.find(Data{3}) ) );
            Assert::IsTrue( container.find(Data{3}) == 1 );
        }
        TEST_METHOD(UT_EraseData)
        {
            struct Data
            {
                int i;

                constexpr bool operator<(Data const& r) const{ return this->i < r.i; }
                //bool operator<(Data const&) const = default;//so compile-error
            };

            //container mit GetKey-Funktion anlegen
            auto container = WS::ContainerMitSuche<std::vector<Data>>{};

            container.push_back( Data{5} );//index 0
            container.push_back( Data{3} );//index 1
            container.push_back( Data{7} );//index 2

            auto index = container.find(Data{3});
            Assert::IsTrue((bool)index);
            container.erase_at(index);
            index = container.find(Data{3});
            Assert::IsFalse((bool)index);

            Assert::IsTrue( container.find(Data{5}) == 0 );
            Assert::IsTrue( container.find(Data{7}) == 1 );
        }
        TEST_METHOD(UT_InsertAsyncExternalPipe)
        {
            struct Data
            {
                int i;

                constexpr bool operator<(Data const& r) const{ return this->i < r.i; }
                //bool operator<(Data const&) const = default;//so compile-error
            };

            //container mit GetKey-Funktion anlegen
            auto container = WS::ContainerMitSuche<std::vector<Data>>{};
            auto worker= [&]( Data && v )
            {
                container.push_back( std::move(v) );
            };
            auto pipe = WS::make_pipe<Data>( worker );

            for(int i=0;  i<2500; ++i )
            {
                pipe.AddData(Data{i});//trägt die werte asyncron in den container ei, dauert länger
                //container.push_back( Data{i} );
            }

            while( pipe.pending() )
                Sleep(2);//ohne dauert es deutlich länger
        }
        TEST_METHOD(UT_InsertAsyncInternalPipe)
        {
            struct Data
            {
                int i;

                constexpr bool operator<(Data const& r) const{ return this->i < r.i; }
                //bool operator<(Data const&) const = default;//so compile-error
            };

            //container mit GetKey-Funktion anlegen
            auto container = WS::ContainerMitSucheAsync<std::vector<Data>>{};

            for(int i=0;  i<2500; ++i )
            {
                container.push_back( Data{i} );
            }

            while( container.pending() )
                Sleep(2);//ohne dauert es deutlich länger
        }

    };
}
