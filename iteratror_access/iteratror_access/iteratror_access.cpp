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
			auto beg_end = WP::iterator_access(L"hallo welt");

			auto ss = std::wstringstream{};
			for( auto const & ch : beg_end )
				ss << ch;

			//beg_end.GetNextEntry();//deprecated
			
			Assert::IsTrue(stringcmp(beg_end.begin(), ss.str().c_str())==0 );
		}
		TEST_METHOD(from_vector_with_life_time_extender)
		{
			using my_type = int;
			auto beg_end1 = WP::iterator_access(L"hallo welt");
			auto beg_end = WP::iterator_access(std::vector<my_type>{1,2,3,4});
			my_type v = 1;
			for (auto const& value : beg_end)
				Assert::IsTrue( value == v++ );
		}
	};
}
