#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\DelayDelete.h"
#include "..\..\headeronly\cout_umleitung.h"

namespace UTDelayDelete
{
    TEST_CLASS(UT_DelayDelete)
    {
        TEST_CLASS_INITIALIZE(Init)
        {
        }
        TEST_CLASS_CLEANUP(Cleanup)
        {
        }

    public:
        TEST_METHOD(DelayDelete2)
        {
            Cout2Output<> coutumleiten{};
            {
                WP::DelayDelete<30,std::chrono::seconds> delaydelete;
                struct myint
                {
                    int v;
                    ~myint()
                    {
                        std::cout << __FUNCTION__ << " " << v << std::endl;
                    }
                };
                struct mystring
                {
                    std::string v;
                    ~mystring()
                    {
                        std::cout << __FUNCTION__ << " " << v << std::endl;
                    }
                };

                int v=10;
                delaydelete.Add( new myint{++v} );
                delaydelete.Add( new myint{++v} );
                delaydelete.Add( new myint{++v} );
                delaydelete.Add( new myint{++v} );
                delaydelete.Add( new mystring{"welt"} );

                delaydelete.CleanAll();
            }
        }
        TEST_METHOD(DelayDelete1)
        {
            Cout2Output<> coutumleiten{};
            {
                WP::DelayDelete<30,std::chrono::milliseconds> delaydelete;
                struct myint
                {
                    int v;
                    ~myint()
                    {
                        std::cout << __FUNCTION__ << " " << v << std::endl;
                    }
                };
                struct mystring
                {
                    std::string v;
                    ~mystring()
                    {
                        std::cout << __FUNCTION__ << " " << v << std::endl;
                    }
                };

                int v=0;
                delaydelete.Add( new myint{++v}, std::chrono::seconds{10} );
                delaydelete.Add( new myint{++v} );
                delaydelete.Add( new myint{++v}, std::chrono::seconds{50} );//das müsste der letzte sein. er wird beim verlassen der funktion zerstört
                delaydelete.Add( new myint{++v}, std::chrono::milliseconds{5} );
                delaydelete.Add( new mystring{"hallo"} );

                while( delaydelete.Clean() > 1 )//
                    ;
            }
        }
    };
}
