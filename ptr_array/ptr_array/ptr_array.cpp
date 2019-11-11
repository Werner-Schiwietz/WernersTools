#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../../headeronly/Ptr_Array.h"
#include "../../headeronly/UniqueArray.h"

namespace ptrarray
{
	TEST_CLASS(ptrarray)
	{
	public:
		
		TEST_METHOD(charpointer)
		{
			WS::ptr_array<char const *> ptr( "hallo", 5 );
			WS::ptr_array<unsigned __int8 const*> ptr2( ptr );

			Assert::IsTrue( ptr.ElementCount()==ptr2.ElementCount());
			Assert::IsTrue(ptr.ByteLen() == ptr2.ByteLen());

			while(ptr.ElementCount())
				Assert::IsTrue(static_cast<unsigned __int8>(*ptr++)==*ptr2++);

			Assert::IsTrue(ptr.ElementCount() == ptr2.ElementCount());
			Assert::IsTrue(ptr.ByteLen() == ptr2.ByteLen());
			Assert::IsTrue(ptr.ElementCount() == 0);
			Assert::IsTrue(ptr.ByteLen() == 0);
		}
		TEST_METHOD(range_Exception)
		{
			WS::ptr_array<char *> ptr;
			try
			{
				++ptr;
				Assert::Fail(L"exception erwartet");
			}
			catch(...)
			{ }
			try
			{
				ptr++;
				Assert::Fail(L"exception erwartet");
			}
			catch (...)
			{
			}
			try
			{
				--ptr;
				Assert::Fail(L"exception erwartet");
			}
			catch (...)
			{
			}
			try
			{
				ptr--;
				Assert::Fail(L"exception erwartet");
			}
			catch (...)
			{
			}
			auto ptr1 = WS::ptr_array<char *>( new char[6], 6 );
			ptr = ptr1;
			*ptr='a';
			for( int i=5; i --> 0; )
				*++ptr = *ptr + 1;
			++ptr;

			try
			{
				++ptr;
				Assert::Fail(L"exception erwartet");
			}
			catch (...)
			{
			}
			for (char i = 6; i-- > 0; )
				Assert::IsTrue( 'a'+i == * --ptr );

			delete [] ptr1;
		}

		TEST_METHOD(uniqueArray)
		{
			WS::ptr_array<int*> ptr;
			{
				using element_count = size_t;
				auto memptr2 = WS::UniqueArray<int>( element_count(4) );
				Assert::IsTrue(memptr2.elemente()==4);
				//auto memptr = memptr2;//error C2280: attempting to reference a deleted function
				auto memptr = std::move(memptr2);
				Assert::IsTrue(memptr2.elemente() == 0);
				ptr = memptr.get();
				Assert::IsTrue(ptr.ElementCount()==4);
				Assert::IsTrue(ptr.ByteLen() == ptr.ElementCount()*sizeof(int));


				while(ptr.ElementCount())
					*ptr++ = ptr.ElementCount();
				ptr = memptr.get();
				

				for(WS::ptr_array<int const *> cptr = memptr.get(); cptr.ElementCount(); ++cptr)
					Assert::IsTrue( *cptr == cptr.ElementCount() );

				{
					auto const & memptr3 = memptr;
					for (size_t i = 0; i < memptr3.elemente(); ++i)
						Assert::IsTrue(static_cast<size_t>(memptr3[i]) == memptr3.elemente()-i );
					//for (size_t i = 0; i < memptr3.elemente(); ++i)
					//	memptr3[i] = memptr3.elemente() - i - 1;//error C3892:  'memptr3': you cannot assign to a variable that is const
				}
				{
					auto & memptr3 = memptr;
					for (size_t i = 0; i < memptr3.elemente(); ++i)
						Assert::IsTrue(static_cast<size_t>(memptr3[i]) == memptr3.elemente() - i);
					for (size_t i = 0; i < memptr3.elemente(); ++i)
						memptr3[i] = i ;
				}
			}
		}
	};
}
