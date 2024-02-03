#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define RAII_malloca_SEH
#include "..\..\headeronly\RAII_malloca.h"
#include "..\..\headeronly\cout_umleitung.h"

#include <excpt.h>
//#include <winnt.h>
#include <windows.h>

#include <signal.h>
namespace WS
{
	void AccessVialotion_throw(int code)
	{
		if( code == 0xc0000005)
			throw "AccessViolation";
	}

	class SignaledAccessVialotion
	{
		using singnal_handler_t = void (*)(int);
		singnal_handler_t old{};
	public:

		SignaledAccessVialotion(SignaledAccessVialotion const & ) = delete;
		SignaledAccessVialotion& operator=(SignaledAccessVialotion const & ) = delete;

		SignaledAccessVialotion()
		{
			this->old = signal(SIGSEGV , AccessVialotion_throw);
		}
		~SignaledAccessVialotion()
		{
			signal(SIGSEGV , this->old);
		}
	};
}

namespace UT_RAIImalloc
{
	TEST_CLASS(UTRAIImalloc)
	{
	public:
		
		TEST_METHOD(UT_ctor_memset_dtor)
		{
			WS::RAII_malloca data{100};
			memset(data.ptr,0,data.bytelen);
		}
		TEST_METHOD(UT_fill)
		{
			WS::ptr_array<void*> data;
			WS::RAII_malloca stackmemory{10};
			data = stackmemory;//auto cast auf WS::ptr_array<void*>

			*data.useas<int>()++ = 5;
			*data.useas<int>()++ = 6;
			*data.useas<bool>()++ = true;
			try
			{
				*data.useas<int>()++ = 7;//passt nicht in memory
				Assert::Fail(L"exception erwartet");
			}
			catch( [[maybe_unused]]std::exception & e )
			{
				Logger::WriteMessage(e.what());
			}

			auto reader = data.reset();
			Assert::IsTrue( *reader.useas<int>()++ == 5 );
			Assert::IsTrue( *reader.useas<int>()++ == 6 );
			Assert::IsTrue( *reader.useas<bool>()++ == true );
			try
			{
				Assert::IsTrue( *reader.useas<int>()++ == 7 );
				Assert::Fail(L"exception erwartet");
			}
			catch( [[maybe_unused]]std::exception & e )
			{
				Logger::WriteMessage(e.what());
			}
		}
		TEST_METHOD(UT_reuse)
		{
			//auto data = WS::RAII_malloca(10);//error C2280: 'WS::RAII_malloca::RAII_malloca(WS::RAII_malloca &&)': attempting to reference a deleted function
			//data = WS::RAII_malloca(50);//error C2280: 'WS::RAII_malloca &WS::RAII_malloca::operator =(WS::RAII_malloca &&)': attempting to reference a deleted function

			//auto get_stack_memory = []()->WS::RAII_malloca
			//{
			//	//return WS::RAII_malloca{100};//error C2280: 'WS::RAII_malloca::RAII_malloca(WS::RAII_malloca &&)': attempting to reference a deleted function
			//};//error C2280: 'WS::RAII_malloca::RAII_malloca(WS::RAII_malloca &&)': attempting to reference a deleted function
			//auto x = get_stack_memory();
		}
		TEST_METHOD(UT_SEH)
		{
			//was soll das mit SEH. MS ist doof
			auto Triggering_SEH_exception = []()
			{
				volatile int *pInt = nullptr;
				*pInt = 20;
			};
			auto Triggering_catch = [&]()
			{
				WS::SignaledAccessVialotion sh{};
				try
				{
					Triggering_SEH_exception();
					return 0;
				}
				catch(std::exception & e )
				{
					Logger::WriteMessage( (std::string{__FUNCTION__ "std::exception:"} + e.what()).c_str() );
					return 1;
				}
				catch(char const * e)
				{
					Logger::WriteMessage( (std::string{__FUNCTION__ "char const *:"} + e).c_str() );
				}
				catch(...)
				{
					Logger::WriteMessage( __FUNCTION__ " catch(...)" );

					return 99;
				}
			};
			auto except_filter= [](unsigned int code, [[maybe_unused]]_EXCEPTION_POINTERS *ep)
			{
				Cout2Output umleitung{};
				std::cout << __FUNCTION__ " code:0x" << std::hex << code;

				if (code == STATUS_ACCESS_VIOLATION)
				{
					std::cout << " caught access violation as expected." << std::endl;
					return EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					std::cout << " ???" << std::endl;
					return EXCEPTION_CONTINUE_SEARCH;
				}
			};
			auto Triggering_except = [&]()
			{
				__try
				{
					Triggering_SEH_exception();
					return 0;
				}
				__except(except_filter(GetExceptionCode(), GetExceptionInformation()))
				{
					return 1;
				}
			};

			Triggering_except();
			Triggering_catch();// mit /EHa wird die SE gefangen
		}
	};
}
