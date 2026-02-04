#include "pch.h"
#include "CppUnitTest.h"

//#include <afx.h> //DEBUG_NEW

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#ifndef ASSERT 
#define ASSERT(x)
#endif

#pragma warning(push,4)

#if defined _DEBUG && defined DEBUG_NEW
#	define new DEBUG_NEW
#endif


#include "..\..\headeronly\array_helper.h"
#include "..\..\headeronly\setnew_getold.h"


using IntArray_t = int[13];
//using pointer_auf_IntArray13_t = IntArray_t *;
using pointer_auf_IntArray13_t = int (*)[13];
struct X
{
	using array_t = int(*)[13];
	union
	{
		array_t pArr{};
		int* ptr;
	};
	//pointer_auf_IntArray13_t pArr{};
};

struct ObjCounter
{
	static int counter;
	ObjCounter() { ++counter; }
	~ObjCounter() { --counter; }
	static int getCounter() { return counter; }
};
int ObjCounter::counter = 0;

namespace UT_arraypointer
{
	TEST_CLASS(UT_arraypointer)
	{
	public:
		BEGIN_TEST_CLASS_ATTRIBUTE()
			END_TEST_CLASS_ATTRIBUTE()

			TEST_METHOD(detectmemleak)
		{
			{
				auto p1 = new int[5] {};
				delete[] p1;
			}
			{
				auto p1 = WS::MakeArray<int, 5>();
				delete[] p1;
			}
			{
#pragma warning(suppress:4156)
				auto p1 = WS::MakeArrayPtr<int, 5>();
			}
		}
		TEST_METHOD(dtor)
		{
			{
				using x_t = ObjCounter(*)[5];

				Assert::IsTrue(ObjCounter::getCounter() == 0);
				x_t x1 = WS::MakeArray<ObjCounter, 5>();
				Assert::IsTrue(ObjCounter::getCounter() == 5);
				x_t x2 = WS::MakeArray<ObjCounter, 5>();
				Assert::IsTrue(ObjCounter::getCounter() == 10);
#pragma warning(suppress:4156)
				delete x1; x1 = nullptr;//warning C4156: deletion of an array expression without using the array form of 'delete'; array form substituted
				Assert::IsTrue(ObjCounter::getCounter() == 5);
				delete[] x2; x2 = nullptr;
				Assert::IsTrue(ObjCounter::getCounter() == 0);

				{
					auto p = new ObjCounter[5]{};
					auto pa = (x_t)(p);
#pragma warning(suppress:4156)
					auto x3 = std::unique_ptr<std::remove_pointer_t<x_t>>(pa);
					Assert::IsTrue(ObjCounter::getCounter() == 5);
				}
				Assert::IsTrue(ObjCounter::getCounter() == 0);
				{
					auto x3 = WS::MakeArrayPtr<ObjCounter, 5>();
					Assert::IsTrue(ObjCounter::getCounter() == 5);
					auto x4 = WS::MakeArrayPtr<ObjCounter, 5>(x3.get());
					Assert::IsTrue(ObjCounter::getCounter() == 10);
				}
				Assert::IsTrue(ObjCounter::getCounter() == 0);
			}

			{
				Assert::IsTrue(ObjCounter::getCounter() == 0);
				auto z1 = WS::MakeArray<ObjCounter, 2>();
				Assert::IsTrue(ObjCounter::getCounter() == 2);
				//delete[] WS::getold{z1} = nullptr;//error C2440: 'delete': cannot convert from 'WS::getold<ObjCounter (*)[2]>' to 'void*'
				delete[](WS::getold{ z1 } = nullptr);
				Assert::IsTrue(ObjCounter::getCounter() == 0);
			}
			{
				int ar[5]{};
				using par_t = std::add_pointer_t<decltype(ar)>;

				par_t par = &ar;

				for (int i = 0; auto& v : *par)
					v = ++i;
			}
		}
		TEST_METHOD(initializer)
		{
			short y1[5] = { 1,2,3,4,5 };
			short y2[5]{ 1,2,3,4,5 };

			auto x1 = WS::MakeArray<int, 5>(1);

			for (int i = 1; decltype(auto) v : *x1)
			{
				Assert::IsTrue(v == i);
				i = 0;
			}
			{
#pragma warning(suppress:4156)
				auto x2 = WS::MakeArrayPtr(x1, 5, 4, 3, 2, 1);
				for (int i = 5; decltype(auto) v : *x2)
				{
					Assert::IsTrue(v == i--);
				}
			}
			auto x2 = WS::MakeArray<short const, 5>(1i16, 2i16, 3i16, 4i16, 5i16);
			for (short i = 1; decltype(auto) v : *x2)
				Assert::IsTrue(v == i++);

			{	//es gibt füe arrays keinen copyctor
				//decltype(y2) y3{ y2 };

				auto const& cy2 = y2;
				{
#pragma warning(suppress:4996)
					auto x3 = WS::MakeArray(cy2, 1i16, 2i16);//kopie von cy2 geht in C++ nicht, template-parameter automatisch aus 1.Parameter bestimmt
					Assert::IsTrue((*x3)[0] == 1);
					Assert::IsTrue((*x3)[1] == 2);
					Assert::IsTrue((*x3)[2] == 0);
					delete[] x3;
				}
#pragma warning(suppress:4996)
				auto x3 = WS::MakeArray(cy2/*,cy2*/);//kopie von cy2 geht in C++ nicht, template-parameter automatisch aus 1.Parameter bestimmt
				Assert::IsTrue(WS::size(x3) == 5);
				for (decltype(auto) v : *x3)
					Assert::IsTrue(v == 0);
				//*x3[1] = 1;//schreibt sonst wo hin, nicht in den index 1
				(*x3)[1] = 1;
				Assert::IsTrue((*x3)[1] == 1);
				//auto x3 = WS::MakeArray(&y1,y1);//kopie von y1, template-parameter automatisch aus 1.Parameter bestimmt
				delete[]x3;
			}

			auto p1 = new int{ 5 };
			//delete WS::getold{p1}=nullptr;//den compileerror verstehe ich nicht
			//delete p1=nullptr;//error C2186: '=': an operand cannot have type 'void'
			delete (WS::getold{ p1 } = nullptr);

			auto px1 = WS::getold{ x1 } = nullptr;
			delete[] px1;
			delete[](WS::getold{ x2 } = nullptr);
		}
		TEST_METHOD(zugriff)
		{
			X x;

			decltype(auto) xx = new int[13];//macht auch nur einen int* 
			delete[] xx;

			//x.pArr = static_cast<int (*)[13]>(new int[13]);//error
			x.pArr = reinterpret_cast<int (*)[13]>(new int[13]);
			delete[] x.pArr; x.pArr = nullptr;
			//x.pArr = (int (*)[13])new int[13]{};//works
			x.pArr = WS::MakeArray<int, 13>();//works
			//static_assert(size(x.pArr)==13);//error
			Assert::IsTrue(WS::size(x.pArr) == 13);

			for (int i = 0; int& v : *x.pArr)
			{
				v = i++;
			}

			int* ptr = *x.pArr;
			Assert::IsTrue((*x.pArr)[3] == 3);
			Assert::IsTrue(ptr[4] == 4);
			//Assert::IsTrue( x.pArr[1]==1 );//error C2446: '==': no conversion from 'int' to 'int [13]'

			int arr[13]{};
			int value = 0x5555'5555; value;
			delete[] x.pArr; x.pArr = nullptr;
			x.pArr = &arr;
			x.ptr = arr;

			//arr[13]=5;//error C4789: in function 'void __cdecl UnitTest1::UnitTest1::TestMethod2(void) __ptr64' buffer 'arr' of size 52 bytes will be overrun; 4 bytes will be written starting at offset 52
			//(*x.pArr)[13] = 5;//speicherüberschreiber :( aber nur warning C6386 im VS2022, kein error, keine warning beim debuggen
		}
		TEST_METHOD(arrayzugriff)
		{
			int (*arr2)[2] = WS::MakeArray<int, 2>();

			(*arr2)[0] = 1;
			(*arr2)[1] = 2;
			//(*arr2)[2] = 3;//speicherüberschreiber, C6386 allerdings keine richtige warning, nur grüner kringel

			delete[] arr2;
		}
	};
}
