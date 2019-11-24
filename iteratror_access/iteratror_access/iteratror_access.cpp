#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../../headeronly/iterator_access.h"


#include <sstream>

namespace iteratroraccess
{
	TEST_CLASS(iteratroraccess)
	{
	public:
		
		TEST_METHOD(natvis)
		{
			auto beg_end = WS::iterator_access(L"hallo welt");

			auto ss = std::wstringstream{};
			for( auto const & ch : beg_end )
				ss << ch;

			//beg_end.GetNextEntry();//deprecated
			
			Assert::IsTrue(stringcmp(beg_end.begin(), ss.str().c_str())==0 );
		}
		TEST_METHOD(from_vector_with_life_time_extender)
		{
			using my_type = int;
			wchar_t const * pb=nullptr;
			wchar_t const * pe=nullptr;

			{
				decltype(WS::iterator_access( std::wstring(L"hallo welt") )) life_time_test;
				{
					auto beg_end1 = WS::iterator_access(std::wstring(L"hallo welt"));
					pb = &*beg_end1.begin();
					Assert::IsTrue( beg_end1 == WS::iterator_access( L"hallo welt" ) );
					life_time_test = beg_end1;
					Assert::IsTrue( life_time_test.begin() == beg_end1.begin() );
					Assert::IsTrue( &*life_time_test.begin() == &*beg_end1.begin() );

					Assert::IsTrue( beg_end1 == WS::iterator_access( L"hallo welt" ) );
					Assert::IsTrue( life_time_test == WS::iterator_access( L"hallo welt" ) );
				}
				Assert::IsTrue( life_time_test == WS::iterator_access( L"hallo welt" ) );
				Assert::IsTrue( &*life_time_test.begin() == pb );
			}
			{
				decltype(WS::iterator_access( std::wstring(L"hallo welt") )) life_time_test;
				{
					auto str = std::wstring( L"hallo welt" );
					auto beg_end1 = WS::iterator_access(str);
					pb = &*beg_end1.begin();
					Assert::IsTrue( beg_end1 == WS::iterator_access( L"hallo welt" ) );
					life_time_test = beg_end1;
					Assert::IsTrue( life_time_test.begin() == beg_end1.begin() );
					Assert::IsTrue( &*life_time_test.begin() == &*beg_end1.begin() );

					Assert::IsTrue( beg_end1 == WS::iterator_access( L"hallo welt" ) );
					Assert::IsTrue( life_time_test == WS::iterator_access( L"hallo welt" ) );
				}
				//laufzeitfehler, str ist zerstrört, ohne lifetime_extender ...
				//Assert::IsTrue( life_time_test == WS::iterator_access( L"hallo welt" ) );
				//Assert::IsTrue( &*life_time_test.begin() == pb );
			}

			auto beg_end = WS::iterator_access(std::vector<my_type>{1,2,3,4});
			my_type v = 1;
			for (auto const& value : beg_end)
				Assert::IsTrue( value == v++ );
		}
	};
}
