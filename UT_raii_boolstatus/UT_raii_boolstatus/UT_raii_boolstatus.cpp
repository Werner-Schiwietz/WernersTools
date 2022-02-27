#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <Windows.h>
#include <atlstr.h>

#include "..\..\headeronly\raii_boolstatus.h"


namespace UT_raiiboolstatus
{
	TEST_CLASS(UT_raiiboolstatus)
	{
	public:
		
		TEST_METHOD(remove_dbl_linefeed_from_clipboard)
		{
			auto cb = OpenClipboard(NULL);

            auto hglb = GetClipboardData(CF_UNICODETEXT); 
            if (hglb != NULL) 
            { 
                CStringW str = (LPCWSTR)GlobalLock(hglb); 
                if (str ) 
                { 
                    GlobalUnlock(hglb); 

                    auto len = str.GetLength();
                    str.Replace(L"\r\n\r\n",L"\r\n");
                    len = str.GetLength();

                    EmptyClipboard();

                    auto hglbCopy = GlobalAlloc(GMEM_MOVEABLE, 
                                            (str.GetLength() + 1) * sizeof(wchar_t)); 
                    if (hglbCopy ) 
                    {

                        auto lptstrCopy = (LPWSTR)GlobalLock( hglbCopy );
                        memcpy( lptstrCopy, str.GetString(),
                                (str.GetLength() + 1) * sizeof( wchar_t ) );
                        GlobalUnlock( hglbCopy );

                        // Place the handle on the clipboard. 

                        SetClipboardData( CF_UNICODETEXT, hglbCopy );
                    }
                } 
            } 
            CloseClipboard(); 

		}
        TEST_METHOD( UT_zuweisen )
        {
            WS::raii_boolstatus master;
            Assert::IsTrue( master );
            Assert::IsTrue( master.status() );
            {
                auto raii_setter = master;
                Assert::IsFalse( master );
                Assert::IsFalse( raii_setter );
            }
            Assert::IsTrue( master );
        }
        TEST_METHOD( UT_als_returnvalue_ctor_zuweisung )
        {
            WS::raii_boolstatus master;
            //liefert kopie des masters!! nie als referenz liefern
            auto block = [&]() -> WS::raii_boolstatus {
                return master;
            };

            Assert::IsTrue( master );
            Assert::IsTrue( master.status() );

            {
                auto raii_setter1 = block();
                Assert::IsFalse( master );
                Assert::IsFalse( raii_setter1 );
                {
                    auto raii_setter2 = block();
                    Assert::IsFalse( master );
                    Assert::IsFalse( raii_setter1 );
                    Assert::IsFalse( raii_setter2 );
                }
                Assert::IsFalse( master );
                Assert::IsFalse( raii_setter1 );
            }
            Assert::IsTrue( master );
        }
        TEST_METHOD( UT_als_returnvalue_zuweisung )
        {
            WS::raii_boolstatus master;
            //liefert kopie des masters!! nie als referenz liefern
            auto block = [&]() -> WS::raii_boolstatus {
                return master;
            };

            {
                decltype(master) raii_setter;

                Assert::IsTrue(master);
                Assert::IsTrue(master.status());
                Assert::IsTrue(raii_setter);

                raii_setter = block();

                Assert::IsTrue(master == false);
                Assert::IsTrue(master.status() == false);
                Assert::IsTrue(raii_setter == false);

                raii_setter.reset();

                Assert::IsTrue(master);
                Assert::IsTrue(master.status());
                Assert::IsTrue(raii_setter);
            }
            Assert::IsTrue(master);
            Assert::IsTrue(master.status());
        }
        TEST_METHOD( UT_2_master )
        {
            WS::raii_boolstatus master1,master2;
            Assert::IsTrue(master1);
            Assert::IsTrue(master2);

            auto raii_setter = master1;

            Assert::IsFalse(master1);
            Assert::IsTrue(master2);
            Assert::IsFalse(raii_setter);

            raii_setter = master2;

            Assert::IsTrue(master1);
            Assert::IsFalse(master2);
            Assert::IsFalse(raii_setter);

            raii_setter.reset();
            Assert::IsTrue(raii_setter);

            Assert::IsTrue(master1);
            Assert::IsTrue(master2);
        }
    };
}
