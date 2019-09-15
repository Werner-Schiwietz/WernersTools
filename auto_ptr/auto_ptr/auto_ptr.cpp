#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <initializer_list>
#include <memory>

#include <vector>

#include "..\..\headeronly\Auto_Ptr.h"

#include <basetsd.h>	//INT_PTR

enum Enum { v1, v2, v3 };

constexpr std::initializer_list<Enum> GetList() { return { v2,v1 }; }

extern "C" int __cdecl _purecall(); //https://docs.microsoft.com/de-de/cpp/c-runtime-library/reference/purecall?view=vs-2019

namespace
{
	bool find_purecall( INT_PTR*** vtable )
	{
		auto purecall = _purecall;
		for( size_t index=0; vtable[0][index]; ++index )
			if( (void*)vtable[0][index]==(void*)purecall )
				return true;
		return false;
	}

	struct virtualA
	{
		virtualA()
		{
			Assert::IsTrue( find_purecall( (INT_PTR***)this ) );
			//(void)fA();//Link-Fehler in Version 16.2.0
			usingPureVirtual();
		}
		virtual int fA() = 0;
		void usingPureVirtual() 
		{ 
			if( find_purecall( (INT_PTR***)this )==false )
				fA();
		}
	};
	struct virtualAA : virtualA
	{
		virtualAA() : virtualA()
		{
			Assert::IsTrue( find_purecall( (INT_PTR***)this ) );
			(void)fA();//
			(void)fAA();//ist zwar als pure declariert, hat aber implementierung
			usingPureVirtual();
		}
		void usingPureVirtual() 
		{ 
			(void)fA(); 
			if(find_purecall( (INT_PTR***)this )==false)
				(void)fAA();//?? aus ctor abort?? 
		}
		virtual int fA() override {return 1;}
		virtual int fAA()  = 0 { return 1; }
	};
	struct virtualAAA : virtualAA
	{
		virtualAAA() : virtualAA()
		{
			Assert::IsFalse( find_purecall( (INT_PTR***)this ) );
			(void)fA();
			(void)fAA();
		}
		virtual int fAA()  override { return 2; }
	};
}

namespace Allerei
{
	TEST_CLASS( Allerei )
	{
	public:
		TEST_METHOD( findiung_pure_virtual_function )
		{
			virtualAAA aaa;
			aaa.usingPureVirtual();
			aaa.virtualAA::usingPureVirtual();
			aaa.virtualA::usingPureVirtual();
		}
		TEST_METHOD( always_red_TestMethod_initializer_list )
		{
			std::stringstream cout;
			cout << "expect  " << Enum::v2 << ' ' << Enum::v1 << std::endl;
			cout << "getting ";
			auto x = GetList();
			std::vector<Enum> vec{GetList()};
			std::vector<Enum> vec2{Enum::v2,Enum::v1};
			for( auto iter = x.begin(); iter!=x.end(); ++iter )
				cout << *iter << ' ';
			for( auto e : x )
				cout << e << ' ';
			cout << std::endl;
			Logger::WriteMessage( cout.str().c_str() );

			auto & container = x;//red
			//auto & container = vec2;//green
			auto i = container.begin();
			Assert::IsTrue( i != container.end() );
			Assert::IsTrue( *i == Enum::v2 );
			Assert::IsTrue( ++i != container.end() );
			Assert::IsTrue( *i == Enum::v1 );
			Assert::IsTrue( ++i == container.end() );
		}
		TEST_METHOD( mem_leak )
		{
			new int{5};
			new std::vector<int>{1,2,3};
		}
	};
}

namespace autoptr
{
	TEST_CLASS(autoptr)
	{
	public:
		TEST_METHOD(auto_ptr__copy_leer)
		{
			WP::auto_ptr<int> x;
			auto xx = x;
		}
		TEST_METHOD(auto_ptr__copy_ownerless)
		{
			int i=5;
			WP::auto_ptr<int> x{&i};
			Assert::IsFalse( x.owner() );
			Assert::IsTrue(x==&i);
			auto xx = x;
			Assert::IsFalse(x.owner());
			Assert::IsTrue(x == &i);
			Assert::IsFalse(xx.owner());
			Assert::IsTrue(xx == &i);
		}
		TEST_METHOD(auto_ptr__copy_ownerless2)
		{
			int i = 5;
			WP::auto_ptr<int> x{ &i };
			Assert::IsFalse(x.owner());
			Assert::IsTrue(x == &i);
			auto xx = x;
			Assert::IsFalse(x.owner());
			Assert::IsTrue(x == &i);
			Assert::IsFalse(xx.owner());
			Assert::IsTrue(xx == &i);
		}
		TEST_METHOD(auto_ptr__comp_nullptr)
		{
			WP::auto_ptr<int> ptr;
			Assert::IsFalse(ptr);
			Assert::IsFalse(ptr!=nullptr);
			Assert::IsTrue(ptr==nullptr);
			
			ptr= WP::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr==nullptr );
			Assert::IsTrue( ptr!=nullptr );
		}
		TEST_METHOD(auto_ptr__as_parameter_rvalueref_release)
		{
			auto fn = [](WP::auto_ptr<int>&& ptr)
			{
				Assert::IsTrue( ptr.owner() );
				(void)ptr.transfer();//wenn owner, und das ist er hier, wird das objekt freigegeben
				Assert::IsFalse(ptr.owner());
				Assert::IsFalse(ptr);
			};

			auto ptr= WP::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );
			fn( std::move( ptr ) );
			Assert::IsFalse( ptr );
		}
		TEST_METHOD(auto_ptr__as_parameter_lvalueref)
		{
			auto fn = [](WP::auto_ptr<int>& ptr)
			{
				Assert::IsTrue( ptr );
				if( ptr.owner() )
				{
					(void)ptr.transfer();//wenn owner, wird das objekt freigegeben
					Assert::IsFalse(ptr.owner());
					Assert::IsFalse(ptr);
				}
				else
				{
					(void)ptr.transfer();//wenn nicht owner, wird das objekt nicht freigegeben
					Assert::IsFalse(ptr.owner());
					Assert::IsTrue(ptr);
				}
			};

			auto ptr= WP::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			auto ptr1 = ptr;
			Assert::IsTrue( ptr1 );
			Assert::IsFalse( ptr1.owner() );

			fn( ptr1 );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );
			fn( ptr );
			Assert::IsFalse( ptr );
			Assert::IsFalse( ptr1 );
		}
		TEST_METHOD(auto_ptr__as_parameter_rvalueref)
		{
			WP::auto_ptr<int>  ptr_owner;
			auto donothing = [](WP::auto_ptr<int> && ptr)
			{
			};
			auto takeownership = [&](WP::auto_ptr<int> && ptr)
			{
				ptr_owner = ptr.transfer();//transfer ownership
			};
			auto moveit = [&](WP::auto_ptr<int> && ptr)
			{
				ptr_owner = std::move(ptr);//move pointer ptr wird nullptr
			};
			auto assignit = [&](WP::auto_ptr<int> && ptr)//dont do this, its senseless
			{
				ptr_owner = ptr;//ptr bleibt owner
			};
			auto releaseptr = [&]( WP::auto_ptr<int> && ptr )
			{
				(void)ptr.transfer();
			};

			auto ptr= WP::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			donothing( std::move( ptr ) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			takeownership( std::move( ptr ) );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr.owner() );
			Assert::IsTrue( ptr_owner.get() == ptr.get() );

			releaseptr( std::move( ptr_owner ) );
			Assert::IsFalse( ptr_owner );
			Assert::IsFalse( ptr );

			ptr= WP::auto_ptr<int>( new int( 5 ), true );
			moveit( std::move(ptr) );
			Assert::IsFalse( ptr );
			Assert::IsTrue( ptr_owner );
			Assert::IsTrue( ptr_owner.owner() );

			ptr= WP::auto_ptr<int>( new int( 5 ), true );
			assignit( std::move(ptr) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );
			Assert::IsTrue( ptr_owner );
			Assert::IsTrue( ptr_owner.get() == ptr.get() );
		}
		TEST_METHOD(auto_ptr__as_parameter_copy)
		{
			WP::auto_ptr<int>  ptr_owner;
			auto donothing = [](WP::auto_ptr<int> ptr)
			{
			};
			auto takeownership = [&](WP::auto_ptr<int> ptr)
			{
				ptr_owner = ptr.transfer();//transfer ownership
			};
			auto releaseptr = [&]( WP::auto_ptr<int> ptr )
			{
				(void)ptr.transfer();
			};

			auto ptr= WP::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			donothing( ptr );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			donothing( ptr.transfer() );
			Assert::IsFalse( ptr );

			ptr= WP::auto_ptr<int>( new int( 5 ), true );
			donothing( std::move(ptr) );
			Assert::IsFalse( ptr );

			ptr= WP::auto_ptr<int>( new int( 5 ), true );

			takeownership( ptr  );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );
			Assert::IsTrue( ptr_owner.get() == ptr.get() );
			takeownership( ptr.transfer() );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr.owner() );
			Assert::IsTrue( ptr_owner.get() == ptr.get() );

			ptr= WP::auto_ptr<int>( new int( 5 ), true );
			takeownership( std::move( ptr ) );
			Assert::IsFalse( ptr );
			Assert::IsTrue( ptr_owner.owner() );
		}
		TEST_METHOD(auto_ptr_assign)
		{
			{	//1 zuweisung x bleibt owner
				WP::auto_ptr<int> x{ std::make_unique<int>(5) };
				WP::auto_ptr<int> x2;
				x2 = x;
				Assert::IsTrue( x );
				Assert::IsTrue( x.owner() );
				Assert::IsTrue( x2 );
				Assert::IsFalse( x2.owner() );
				Assert::IsTrue( x2.get()==x.get() );
			}
			{	//2 same as 1
				WP::auto_ptr<int> x{ std::make_unique<int>(5) };
				WP::auto_ptr<int> x2;
				x2 = x.ownerless();
				Assert::IsTrue( x );
				Assert::IsTrue( x.owner() );
				Assert::IsTrue( x2 );
				Assert::IsFalse( x2.owner() );
				Assert::IsTrue( x2.get()==x.get() );
			}
			{	//3 move x wird nullptr x2 wird owner
				WP::auto_ptr<int> x{ std::make_unique<int>(5) };
				WP::auto_ptr<int> x2;
				x2 = std::move(x);
				Assert::IsFalse( x );
				Assert::IsTrue( x2 );
				Assert::IsTrue( x2.owner() );
			}
			{	//4 transfer x behält pointer x2 wird owner
				WP::auto_ptr<int> x{ std::make_unique<int>(5) };
				WP::auto_ptr<int> x2;
				x2 = x.transfer();
				Assert::IsTrue( x );
				Assert::IsFalse( x.owner() );
				Assert::IsTrue( x2 );
				Assert::IsTrue( x2.owner() );
				Assert::IsTrue( x2.get()==x.get() );
			}
		}
		TEST_METHOD(auto_ptr__)
		{
			WP::auto_ptr<int> x{ std::make_unique<int>(5) };
			Assert::IsTrue(x.owner());
			auto xx = x.transfer();
			Assert::IsFalse(x.owner());
			Assert::IsTrue(xx.owner());
			auto xxx = x.transfer();
			Assert::IsFalse(x.owner());
			Assert::IsTrue(xx.owner());
			Assert::IsFalse(xxx.owner());
			Assert::IsTrue(xx.get() == x.get());
			Assert::IsTrue(xx.get() == xxx.get());
		}

		TEST_METHOD(cast_multiple_inheritance)
		{
			struct A
			{
				//virtual ~A(){}
				int foo(){return 1;}
			};
			struct B
			{
				virtual ~B() {}//ohne virtual klappt der transfer nicht
				int foo() { return 2; }
			};
			struct AB : A , B
			{
				int foo() { return 3; }
			};
			WP::auto_ptr<AB> ab = std::make_unique<AB>();
			WP::auto_ptr<A> a = ab;
			WP::auto_ptr<B> b = ab;

			Assert::IsTrue( ab.owner() );
			Assert::IsTrue(ab->foo()== 3);
			Assert::IsTrue(a->foo() == 1);
			Assert::IsTrue(b->foo() == 2);

			b = ab.transfer();//ups
			Assert::IsTrue( b.owner() );
			Assert::IsTrue(ab->foo()== 3);
			Assert::IsTrue(a->foo() == 1);
			Assert::IsTrue(b->foo() == 2);

			auto pab = ab.get();
			auto pa = a.get();
			auto pb = b.get();
			Assert::IsTrue( (void const *)pa != (void const *)pb );
			Assert::IsTrue( (void const *)pa == (void const *)pab || (void const *)pb == (void const *)pab );

		}
		TEST_METHOD(auto_ptr_from_this)
		{
			struct X : WP::enable_auto_ptr_from_this<X>{ int value; X(){} X(int v):value(v){}};

			WP::auto_ptr<X> ptr;
			Assert::IsNull( ptr.get() );
			{
				X x{5};
				ptr = x.auto_ptr_from_this();
				Assert::IsNotNull(ptr.get());
				Assert::IsFalse(ptr.owner());
				Assert::IsTrue(ptr->value == x.value );
			}
			Assert::IsNull(ptr.get());
		}
		TEST_METHOD(shared_weak_ptr)
		{
			WP::auto_ptr<int> p{ std::make_shared<int>(5) };
			Assert::IsNotNull(p.get());
			auto p1 = p;
			Assert::IsNotNull(p.get());
			Assert::IsNotNull(p1.get());
			auto p2 = p.weak();
			Assert::IsNotNull(p2.get());
			p= WP::auto_ptr<int>{};
			Assert::IsNull(p.get());
			Assert::IsNotNull(p1.get());
			Assert::IsNotNull(p2.get());
			p1 = WP::auto_ptr<int>{};
			Assert::IsNull(p1.get());
			Assert::IsNull(p2.get());
		}
	};
}
