#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <initializer_list>
#include <memory>

#include <vector>
#include <deque>
#include <list>
#include <forward_list>

#include "..\..\headeronly\Auto_Ptr.h"

#include <basetsd.h>	//INT_PTR

//#include <afx.h>
//#define new DEBUG_NEW

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

struct ctor_counter
{
	static int counter;
	ctor_counter(){++counter;}
	~ctor_counter(){--counter;}
};
int ctor_counter::counter = 0;

namespace _Immer_Rot
{
	TEST_CLASS( Immer_Rot )
	{
	public:
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
		TEST_METHOD( mem_leak )
		{
			new int{5};
			new std::vector<int>{1,2,3};
		}
		//template<typename T> struct test_extent { int constexpr value=std::extent<T>::value; };
		TEST_METHOD( std__extent )
		{
			Assert::IsTrue( std::is_array<int[3]>::value );
			Assert::IsTrue( std::extent<int[3]>::value == 3 );
			Assert::IsTrue( std::is_array<int[3][4]>::value );
			Assert::IsTrue( std::extent<int[3][4]>::value == 3 );
			Assert::IsFalse( std::extent<int[3][4]>::value == 4 );
			Assert::IsTrue( std::extent<int[3][4], 0>::value == 3 );
			Assert::IsTrue( std::extent<int[3][4], 1>::value == 4 );
			Assert::IsTrue( std::extent<int[3][4], 3>::value == 0 );
			Assert::IsFalse( std::is_array<int>::value );
			Assert::IsTrue( std::extent<int>::value == 0 );
			Assert::IsTrue( std::extent<int, 3>::value == 0 );
			Assert::IsTrue( std::extent<int[]>::value == 0 );
			Assert::IsTrue( std::is_array<int[]>::value );

		}
	};
}

namespace autoptr
{
	TEST_CLASS(UT_managed_auto_ptr)
	{
		TEST_METHOD(managed_auto_ptr_nullptr)
		{
			WS::managed_auto_ptr<int> ptr;
			ptr = WS::managed_auto_ptr<int>{nullptr};
			ptr = WS::auto_ptr<int>{};
			ptr = WS::auto_ptr<int>{nullptr};
			ptr = nullptr;
		}
		TEST_METHOD(managed_auto_ptr_owner)
		{
			WS::managed_auto_ptr<int> ptr;
			ptr = std::make_unique<int>(5);
			Assert::IsTrue(ptr);
			ptr = ptr;
			Assert::IsTrue(ptr);
			auto autoptr = WS::auto_ptr<int>{std::make_unique<int>(6)};
			ptr = autoptr;
			try
			{	
				int i=7;
				//ptr = &i;//error C2280: 'WS::managed_auto_ptr<int>::managed_auto_ptr(int *)': attempting to reference a deleted function
				ptr = WS::auto_ptr<int>{&i};
				Assert::Fail(L"exception erwartet");
			}
			catch(...)
			{}
		}
		TEST_METHOD(managed_auto_ptr_auto_ptr_from_this)
		{
			class A : public WS::enable_auto_ptr_from_this<A>
			{};
			class B : public A
			{};
			class AA : public WS::enable_auto_ptr_from_this<AA>{};

			WS::managed_auto_ptr<A> Aptr;
			WS::managed_auto_ptr<B> Bptr;

			{
				A a;
				B b;

				Aptr = a;
				Assert::IsTrue(Aptr);
				Aptr = nullptr;
				Assert::IsFalse(Aptr);
				Aptr = &a;
				Assert::IsTrue(Aptr);
				Aptr = (A*)nullptr;
				Assert::IsFalse(Aptr);

				Aptr = b;
				Assert::IsTrue(Aptr);
				Aptr = nullptr;
				Assert::IsFalse(Aptr);
				Aptr = &b;
				Assert::IsTrue(Aptr);

				Bptr = a;
				Assert::IsFalse(Bptr);
				Bptr = &a;
				Assert::IsFalse(Bptr);

				Bptr = b;
				Assert::IsTrue(Bptr);
				Bptr = nullptr;
				Assert::IsFalse(Bptr);
				Bptr = &b;
				Assert::IsTrue(Bptr);
				Assert::IsTrue(Aptr);

				AA aa;
				WS::managed_auto_ptr<AA> AAptr;
				//AAptr = a;//sollte compilefehler geben
				//AAptr = b;//sollte compilefehler geben
				//Aptr = aa;//sollte compilefehler geben
				//Bptr = aa;//sollte compilefehler geben
			}

			Assert::IsFalse(Aptr);
			Assert::IsFalse(Bptr);
		}
	};
	TEST_CLASS(autoptr)
	{
	public:
		TEST_METHOD(auto_ptr__compare)
		{
			WS::auto_ptr<int> ptr;
			Assert::IsFalse( ptr );
			Assert::IsTrue( !ptr );
			Assert::IsTrue( ptr==ptr  );
			Assert::IsTrue( ptr==NULL  );
			Assert::IsFalse( ptr!=NULL  );
			Assert::IsTrue( ptr==nullptr  );
			Assert::IsFalse( ptr!=nullptr  );
			Assert::IsFalse( NULL!=ptr  );
			Assert::IsTrue( NULL==ptr  );
			Assert::IsTrue( nullptr==ptr );
			Assert::IsFalse( nullptr!=ptr  );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsFalse( !ptr );
			Assert::IsTrue( ptr==ptr  );
			Assert::IsTrue( ptr!=NULL  );
			Assert::IsFalse( ptr==NULL  );
			Assert::IsTrue( ptr!=nullptr  );
			Assert::IsFalse( ptr==nullptr  );
			Assert::IsTrue( NULL!=ptr  );
			Assert::IsFalse( NULL==ptr  );
			Assert::IsTrue( nullptr!=ptr );
			Assert::IsFalse( nullptr==ptr  );
		}
		TEST_METHOD(auto_ptr__inherit)
		{
			struct A{virtual ~A(){}};
			struct B  : A {virtual ~B(){}};

			WS::auto_ptr<A> pa = std::make_unique<A>();
			pa = std::make_unique<B>();

		}
		TEST_METHOD(auto_ptr__compare_inherit)
		{
			struct A{virtual ~A(){}};
			struct B{virtual ~B(){}};
			struct AB : A, B{};

			WS::auto_ptr<A> pa;
			WS::auto_ptr<B> pb;
			WS::auto_ptr<AB> pab;

			pa = WS::auto_ptr<AB>{new AB{}};
			pab = pa;
			pb = pab;

			Assert::IsTrue( static_cast<void*>(pa.get())!=static_cast<void*>(pb.get()) );
			Assert::IsTrue( pab==pa );
			Assert::IsTrue( pab==pb );


			Assert::IsTrue( pab.get()==pa.get() );
			Assert::IsTrue( pab.get()==pb.get() );

			//??
			auto pa1 = pa.get(); pa1;
			auto pb1 = pb.get();
			auto pab1 = pab.get();
			Assert::IsTrue( pab1==pb1 );
			Assert::IsTrue( (void*)pab1!=(void*)pb1 );
			{
				AB ab;
				auto pab2 = &ab;
				A* pa2 = &ab;
				B* pb2 = &ab;
				Assert::IsTrue( pab2==pb2 );
				Assert::IsTrue( (void*)pab2!=(void*)pb2 );

				Assert::IsTrue( pab!=pb2 );
				Assert::IsTrue( (void*)pa2!=(void*)pb2 );
			}
			//??

			pa = WS::auto_ptr<A>{new A{}};
			pab = pa;
			Assert::IsFalse(pab);
		}
		TEST_METHOD(auto_ptr__copy_leer)
		{
			WS::auto_ptr<int> x;
			auto xx = x;
		}
		TEST_METHOD(auto_ptr__copy_ownerless)
		{
			int i=5;
			WS::auto_ptr<int> x{&i};
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
			WS::auto_ptr<int> x{ &i };
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
			WS::auto_ptr<int> ptr;
			Assert::IsFalse(ptr);
			Assert::IsFalse(ptr!=nullptr);
			Assert::IsTrue(ptr==nullptr);
			
			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr==nullptr );
			Assert::IsTrue( ptr!=nullptr );
		}
		TEST_METHOD(auto_ptr__as_parameter_rvalueref_release)
		{
			auto fn = [](WS::auto_ptr<int>&& ptr)
			{
				Assert::IsTrue( ptr.owner() );
				(void)ptr.transfer();//wenn owner, und das ist er hier, wird das objekt freigegeben
				Assert::IsFalse(ptr.owner());
				Assert::IsFalse(ptr);
			};

			auto ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );
			fn( std::move( ptr ) );
			Assert::IsFalse( ptr );
		}
		TEST_METHOD(auto_ptr__as_parameter_lvalueref)
		{
			auto fn = [](WS::auto_ptr<int>& ptr)
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

			auto ptr= WS::auto_ptr<int>( new int( 5 ), true );
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
			WS::auto_ptr<int>  ptr_owner;
			auto donothing = [](WS::auto_ptr<int> && ptr)
			{
				ptr;
			};
			auto takeownership = [&](WS::auto_ptr<int> && ptr)
			{
				ptr_owner = ptr.transfer();//transfer ownership
			};
			auto moveit = [&](WS::auto_ptr<int> && ptr)
			{
				ptr_owner = std::move(ptr);//move pointer ptr wird nullptr
			};
			auto assignit = [&](WS::auto_ptr<int> && ptr)//dont do this, its senseless
			{
				ptr_owner = ptr;//ptr bleibt owner
			};
			auto releaseptr = [&]( WS::auto_ptr<int> && ptr )
			{
				(void)ptr.transfer();
			};

			auto ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			donothing( std::move( ptr ) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			takeownership( std::move( ptr ) );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr.owner() );
			Assert::IsTrue( ptr_owner.get() == ptr.get() );
			Assert::IsTrue( ptr_owner == ptr );
			Assert::IsTrue( ptr_owner == ptr.get() );
			Assert::IsTrue( ptr_owner.get() == ptr );

			releaseptr( std::move( ptr_owner ) );
			Assert::IsFalse( ptr_owner );
			Assert::IsFalse( ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			moveit( std::move(ptr) );
			Assert::IsFalse( ptr );
			Assert::IsTrue( ptr_owner );
			Assert::IsTrue( ptr_owner.owner() );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			assignit( std::move(ptr) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );
			Assert::IsTrue( ptr_owner );
			Assert::IsTrue( ptr_owner == ptr );
		}
		TEST_METHOD(auto_ptr__as_parameter_copy)
		{
			WS::auto_ptr<int>  ptr_owner;
			auto donothing = [](WS::auto_ptr<int> ptr)
			{
			};
			auto takeownership = [&](WS::auto_ptr<int> ptr)
			{
				ptr_owner = ptr.transfer();//transfer ownership
			};
			auto releaseptr = [&]( WS::auto_ptr<int> ptr )
			{
				(void)ptr.transfer();
			};

			auto ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			donothing( ptr );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );

			donothing( ptr.transfer() );
			Assert::IsFalse( ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			donothing( std::move(ptr) );
			Assert::IsFalse( ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );

			takeownership( ptr  );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.owner() );
			Assert::IsTrue( ptr_owner == ptr );
			takeownership( ptr.transfer() );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr.owner() );
			Assert::IsTrue( ptr_owner == ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			takeownership( std::move( ptr ) );
			Assert::IsFalse( ptr );
			Assert::IsTrue( ptr_owner.owner() );
		}
		TEST_METHOD(auto_ptr_assign)
		{
			{	//1 zuweisung x bleibt owner
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = x;
				Assert::IsTrue( x );
				Assert::IsTrue( x.owner() );
				Assert::IsTrue( x2 );
				Assert::IsFalse( x2.owner() );
				Assert::IsTrue( x2==x );
			}
			{	//2 same as 1
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = x.ownerless();
				Assert::IsTrue( x );
				Assert::IsTrue( x.owner() );
				Assert::IsTrue( x2 );
				Assert::IsFalse( x2.owner() );
				Assert::IsTrue( x2==x );
			}
			{	//3 move x wird nullptr x2 wird owner
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = std::move(x);
				Assert::IsFalse( x );
				Assert::IsTrue( x2 );
				Assert::IsTrue( x2.owner() );
			}
			{	//4 transfer x behält pointer x2 wird owner
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = x.transfer();
				Assert::IsTrue( x );
				Assert::IsFalse( x.owner() );
				Assert::IsTrue( x2 );
				Assert::IsTrue( x2.owner() );
				Assert::IsTrue( x2==x );
			}
		}
		TEST_METHOD(auto_ptr__ownership_transfer)
		{
			WS::auto_ptr<int> x{ std::make_unique<int>(5) };
			Assert::IsTrue(x.owner());
			auto xx = x.transfer();
			Assert::IsFalse(x.owner());
			Assert::IsTrue(xx.owner());
			auto xxx = x.transfer();
			Assert::IsFalse(x.owner());
			Assert::IsTrue(xx.owner());
			Assert::IsFalse(xxx.owner());
			Assert::IsTrue(xx == x);
			Assert::IsTrue(xx == xxx);
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
			WS::auto_ptr<AB> ab = std::make_unique<AB>();
			WS::auto_ptr<A> a = ab;
			WS::auto_ptr<B> b = ab;

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
			struct X : WS::enable_auto_ptr_from_this<X>{ int value; X(){} X(int v):value(v){}};

			WS::auto_ptr<X> ptr;
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
		TEST_METHOD(auto_ptr_from_this_von_moved_objekt)
		{
			struct Data : WS::enable_auto_ptr_from_this<Data>
			{
				int value = 0;
				Data(int value):value(value){}
				Data(){}
				Data( Data const & r ) : value(r.value){};
				Data(Data&&r) : Data(){swap(r);}
				void swap(Data&r){std::swap( this->value, r.value );}
			};


			Data d1{1};
			auto p1=d1.auto_ptr_from_this();
			Assert::IsTrue( p1->value == d1.value );

			auto d2{d1};
			auto p2=d2.auto_ptr_from_this();
			Assert::IsTrue( p1->value == d1.value );
			Assert::IsTrue( p2->value == d1.value );
			++d2.value;
			++p2->value;
			Assert::IsTrue( p2->value != d1.value );
			Assert::IsTrue( p2->value == d2.value );

			auto d3{std::move( d1 )};
			auto p3=d3.auto_ptr_from_this();
			Assert::IsTrue( p1->value == d1.value );
			Assert::IsTrue( p1->value == Data{}.value );
			Assert::IsTrue( p3->value == d3.value );
			Assert::IsTrue( p3->value != d1.value );
		}
		TEST_METHOD(auto_ptr_from_this_von_objekt_im_vector)
		{	//per auto_ptr_from_this gemerkte pointer auf std::vector-elemente werden durch reallok im vector zu nullptr, obwohl die ojekte im vector am index noch vorhanden sind, aber halt an anderer adresse
			//deshalb nicht auf dynamische container anwenden, zu gefährlich für den programmablauf
			struct Data : WS::enable_auto_ptr_from_this<Data>
			{
				Data(int value):value(value){}
				int value;
			};
			std::vector<Data> container;
			//container.reserve( 3 );
			std::vector<WS::auto_ptr<Data>> ptrs;

			int value = 0;
			container.emplace_back( value );
			ptrs.emplace_back( container[0].auto_ptr_from_this() );
			Data const * p1 = &container[0];

			for( value=1;value < 50; ++value )
			{
				container.emplace_back( value );
				ptrs.emplace_back( container.back().auto_ptr_from_this() );
				Data const * p2 = &container[0];
				if( p1 != p2 )
				{	//sollte der container umkopiert worden sein, sind die gemerkten auto_pointer null. ist halt so
					p1 = p2;
					for( int index=0; index<value; ++index )
					{
						auto x1 = container[index].auto_ptr_from_this().get(); x1;
						auto x2 = ptrs[index].get();
						Assert::IsTrue( x2==nullptr );
						ptrs[index] = container[index].auto_ptr_from_this();
					}
				}
				else
				{	//solange nicht umkopiert wurde sind die pointer gültig
					for( int index=0; index<=value; ++index )
						Assert::IsTrue( container[index].auto_ptr_from_this()==ptrs[index] );
				}
			}
		}
		TEST_METHOD(auto_ptr_als_owner_im_vector)
		{	//vector mit auto_ptr funktioniert besser als enable_auto_ptr_from_this. auch ein realloc macht nichts kaputt
			struct Data : WS::enable_auto_ptr_from_this<Data>
			{
				Data(int value):value(value){}
				int value;
			};
			std::vector<WS::auto_ptr<Data>> container;
			//container.reserve( 3 );
			std::vector<WS::auto_ptr<Data>> ptrs;

			int value = 0;
			container.emplace_back( WS::auto_ptr_owner_parameter<Data>{ new Data{value} } );
			ptrs.emplace_back( container[0] );
			auto * p1 = &container[0];

			for( value=1;value < 50; ++value )
			{
				container.emplace_back( WS::auto_ptr_owner_parameter<Data>{ new Data{value} } );
				ptrs.emplace_back( container.back() );
				WS::auto_ptr<Data> * p2 = &container[0];
				if( p1 != p2 )
				{	//auch wenn die objekte im container umkopiert worden sind, sind die gemerkten auto_pointer noch gültig
					p1 = p2;
					for( int index=0; index<value; ++index )
					{
						Assert::IsTrue( container[index]==ptrs[index] );
						Assert::IsTrue( container[index]->value==index );
						Assert::IsTrue( ptrs[index]->value==index );
					}
				}
				else
				{	//solange nicht umkopiert wurde sind die pointer gültig
					for( int index=0; index<=value; ++index )
					{
						Assert::IsTrue( container[index]==ptrs[index] );
						Assert::IsTrue( container[index]->value==index );
						Assert::IsTrue( ptrs[index]->value==index );
					}
				}
			}

			container.clear();
			//nach clear alle gemerkten nicht owner pointer null
			for( auto & ptr : ptrs )
				Assert::IsFalse( ptr );
		}
		TEST_METHOD(shared_weak_ptr)
		{
			WS::auto_ptr<int> p{ std::make_shared<int>(5) };
			Assert::IsNotNull(p.get());
			auto p1 = p;
			Assert::IsNotNull(p.get());
			Assert::IsNotNull(p1.get());
			auto p2 = p.weak();
			Assert::IsNotNull(p2.get());
			p= WS::auto_ptr<int>{};
			Assert::IsNull(p.get());
			Assert::IsNotNull(p1.get());
			Assert::IsNotNull(p2.get());
			p1 = WS::auto_ptr<int>{};
			Assert::IsNull(p1.get());
			Assert::IsNull(p2.get());
		}
		TEST_METHOD(auto_ptr_owner_parameter)
		{
			int _startwert = 5;
			struct Int
			{
				int & startwert;
				int value;
				Int(int value, int & startwert):value(value),startwert(startwert){}
				~Int(){
					Assert::IsTrue( startwert++ == value );
				}
			};
			auto foo = []( WS::auto_ptr_owner_parameter<Int> ){};

			Int I{11,_startwert};
			//foo( &I );//dont do something like this. you cant manage a stack-objekt

			foo( (Int*)nullptr );
			foo( new Int{5,_startwert} );
			foo( std::make_unique<Int>(6,_startwert) );
			foo( std::unique_ptr<Int>{} );
			foo( std::make_shared<Int>(7,_startwert) );
			foo( std::shared_ptr<Int>{} );
			foo( WS::auto_ptr<Int>{} );
			foo( WS::auto_ptr<Int>(new Int{8,_startwert},true) ); 
			foo( WS::auto_ptr<Int>(std::make_shared<Int>(9,_startwert)) ); 

			auto ptr = new Int{10,_startwert};
			try
			{
				foo( WS::auto_ptr<Int>(ptr,false) ); 
				Assert::Fail( L"exception erwartet, weil foo keinen owner als parameter erhaelt" );
			}
			catch(...){}
			Assert::IsTrue( _startwert==10 );
			delete ptr;
			Assert::IsTrue( _startwert==11 );
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_shared_ptr)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )->WS::auto_ptr<int>//auto_ptr_owner_parameter wirft exception, wenn ptr nicht owner ist
			{
				return ptr;
			};

			WS::auto_ptr<int> ptr { std::shared_ptr<int>( new int( 5 ) ) };
			Assert::IsFalse( ptr.owner() );
			Assert::IsTrue( ptr.is_shared_ptr() );
			auto fnTest =[&ptr]( WS::auto_ptr<int> && ptr_shared )
			{
				Assert::IsFalse( ptr.is_owner() );
				Assert::IsTrue( ptr.is_shared_ptr() );
				Assert::IsFalse( ptr_shared.is_owner() );
				Assert::IsTrue( ptr_shared.is_shared_ptr() );
				Assert::IsTrue( ptr_shared==ptr );
				(void)ptr_shared.transfer();//jetzt passiert nicht, weil shared_ptr
				Assert::IsTrue( ptr != nullptr );
				Assert::IsTrue( ptr_shared != nullptr );
				Assert::IsTrue( ptr_shared == ptr );
				ptr_shared = nullptr;
				Assert::IsTrue( ptr_shared == nullptr );
				Assert::IsTrue( ptr != nullptr );
			};
			fnTest( fnTakeOverOwnershipAndReturnOwner( ptr.transfer() ) );//egal ob transfer oder direkt uebergeben
			fnTest( fnTakeOverOwnershipAndReturnOwner( ptr ) );
			ptr = std::shared_ptr<int>( new int{6} );
			fnTest( fnTakeOverOwnershipAndReturnOwner( ptr ) );
			{
				ptr = fnTakeOverOwnershipAndReturnOwner( std::shared_ptr<int>( new int{7} ) );
				Assert::IsTrue( *ptr==7 );
			}
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_transfer)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )->WS::auto_ptr<int>//auto_ptr_owner_parameter wirft exception, wenn ptr nicht owner ist
			{
				return ptr;
			};
			WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
			Assert::IsTrue( ptr.owner() );
			auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( ptr.transfer() );//egal ob transfer oder direkt uebergeben
			Assert::IsFalse( ptr.owner() );
			Assert::IsTrue( ptr_owner.owner() );
			Assert::IsTrue( *ptr == 5 );
			Assert::IsTrue( *ptr_owner == 5 );
			Assert::IsTrue( ptr_owner==ptr );
			(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerstört
			Assert::IsTrue( ptr == nullptr );
			Assert::IsTrue( ptr_owner == nullptr );
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_referenz)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
				Assert::IsTrue( ptr.owner() );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( ptr );//egal ob transfer oder direkt uebergeben. ptr ist nicht mehr owner. waere er es bei aufruf nicht, flöge eine exception
				Assert::IsFalse( ptr.owner() );
				Assert::IsTrue( ptr_owner.owner() );
				Assert::IsTrue( *ptr == 5 );
				Assert::IsTrue( *ptr_owner == 5 );
				Assert::IsTrue( ptr_owner == ptr );
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerstört
				Assert::IsTrue( ptr == nullptr );
				Assert::IsTrue( ptr_owner == nullptr );
			}
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_move)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
				Assert::IsTrue( ptr.owner() );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( std::move(ptr) );//std::move führt hier zu ptr==nullptr, evtl besser transfer()
				Assert::IsFalse( ptr.owner() );
				Assert::IsTrue( ptr_owner.owner() );
				Assert::IsTrue( ptr==nullptr );//std::move setzt ptr auf nullptr. wenn pointer stehen bleiben soll transfer() benutzen, s.o.
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerstört
				Assert::IsTrue( ptr == nullptr );
				Assert::IsTrue( ptr_owner == nullptr );
			}
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_unique_ptr)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				auto ptr = std::unique_ptr<int>( new int( 5 ) );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( std::move(ptr) );//std::move führt hier zu ptr==nullptr
				Assert::IsTrue( ptr == nullptr );
				Assert::IsTrue( ptr_owner != nullptr );
				Assert::IsTrue( *ptr_owner == 5 );
				Assert::IsTrue( ptr_owner.owner() );
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerstört
			}
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_unmanged_ptr)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				auto unmanged_ptr = new int( 5 );
				Assert::IsTrue( *unmanged_ptr==5 );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( unmanged_ptr );
				Assert::IsTrue( ptr_owner.owner() );
				Assert::IsTrue( *unmanged_ptr==5 );
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerstört
				Assert::IsTrue( ptr_owner == nullptr );
				try
				{
					Assert::IsTrue( *unmanged_ptr!=5 );//greift auf freigegeben speicher zu
				}catch(...){}
			}
		}
		TEST_METHOD(uebergebe_ptr_als_ownerless_exception)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )
			{
				return WS::auto_ptr<int>(ptr);
			};
			int i=5;
			WS::auto_ptr<int> ptr( &i );
			Assert::IsFalse( ptr.owner() );
			try
			{
				//fnTakeOverOwnershipAndReturnOwner wirft exception, wenn parameter nicht owner ist
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( ptr );
				Assert::Fail( L"exception erwartet" );
			}
			catch(std::exception &  )
			{}
		}
		TEST_METHOD(auto_ptr_owner_parameter_zugriff)
		{
			auto ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
			WS::auto_ptr<int>{ptr};//cast nach auto_ptr genau einmal möglich, danach ist ptr empty
			Assert::IsTrue( static_cast<WS::auto_ptr<int>>(ptr) == nullptr );
			Assert::IsTrue( static_cast<WS::auto_ptr<int>>(ptr) == NULL );

			ptr = WS::auto_ptr_owner_parameter<int>( new int( 6 ) );
			int *				p1 = nullptr;
			WS::auto_ptr<int>	p2;
			{
				WS::auto_ptr<int> owner = ptr;//übertragung des objekt-pointers auf owner, ptr wird empty
				Assert::IsTrue( static_cast<WS::auto_ptr<int>>(ptr) == nullptr );
				p1 = owner;
				p2 = owner;
				Assert::IsTrue( owner.owner() );
				Assert::IsTrue( *owner == 6 );
				Assert::IsFalse( p2.owner() );
				Assert::IsTrue( *p2 == 6 );
				Assert::IsTrue( *p1 == 6 );
			}
			//Assert::IsTrue( *p1 != 6 );//unmanaged pointer auf ungültigen speicher
			Assert::IsTrue( p2 == nullptr );//managed pointer wird zu nullptr
		}
		TEST_METHOD(release_as_unique_ptr)
		{	//release sind schlechte funktionen, weil die pointer im zugriff bleiben, die zerstörung der objekt aber nicht bemerkt wird
			WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
			Assert::IsTrue( ptr.owner() );
			{
				auto p1 = ptr.release_as_unique_ptr();
				Assert::IsTrue( p1.get()==ptr.get() );
				Assert::IsTrue( *p1==5 );
				Assert::IsTrue( *ptr==5 );
			}
			//Assert::IsFalse( *ptr==5 );//vorsicht ptr liefert noch den bereits freigegebenen speicher

			{
				auto ptr1 = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
				ptr = ptr1.move();
				Assert::IsTrue( static_cast<WS::auto_ptr<int>>(ptr1)==nullptr );
				Assert::IsTrue( ptr.owner() );
				{
					auto p1 = ptr.release_as_unique_ptr();
					Assert::IsTrue( p1.get() );
				}
				//Assert::IsTrue( *ptr==5 );
			}

			{
				WS::auto_ptr<int> ptr1 = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
				ptr = ptr1;
				Assert::IsFalse( ptr.owner() );
				{
					auto p1 = ptr.release_as_unique_ptr();
					Assert::IsFalse( p1.get() );
				}
				Assert::IsTrue( *ptr==5 );
			}
		}
		TEST_METHOD(natvis_test)
		{
			{
				int i=5;
				WS::auto_ptr<int> ptr1;//erwarte anzeige ptr1 nullptr
				ptr1 = WS::auto_ptr<int>( &i );//erwarte anzeige ptr1 unmanged: 0x... {5} refs=1
				Assert::IsFalse(ptr1.is_managed());
				auto x = ptr1.release();//erwarte anzeige ptr1 unmanged: 0x... {5} refs=1
				x;
			}
			{
				WS::auto_ptr<int> ptr1;
				ptr1 = WS::auto_ptr<int>( new int{5}, true );//erwarte anzeige ptr1 is_owner: 0x...{5} refs=1 
				Assert::IsTrue(ptr1.is_managed());
				auto x = ptr1.release_as_unique_ptr();//erwarte anzeige ptr1 unmanged: 0x... {5} refs=1
				Assert::IsFalse(ptr1.is_managed());
				Assert::IsTrue( x!=nullptr );
			}
			{
				WS::auto_ptr<int> ptr1;
				ptr1 = WS::auto_ptr<int>( std::shared_ptr<int>( new int{5} ) );//erwarte anzeige ptr1 is_shared: 0x...{5} refs=1
				Assert::IsTrue(ptr1.is_managed());
				auto x = ptr1.release_as_unique_ptr();//erwarte anzeige ptr1 is_shared: 0x...{5} refs=1
				Assert::IsTrue(ptr1.is_managed());
				Assert::IsTrue( x==nullptr );
				auto x2 = ptr1.release();//erwarte anzeige ptr1 is_shared: 0x...{5} refs=1
				Assert::IsTrue(ptr1.is_managed());
				Assert::IsTrue( x2!=nullptr );
				auto ptr2 = ptr1;//erwarte anzeige ptr1 is_shared: 0x...{5} refs=2
				ptr2 = nullptr;
			}
			{
				std::vector<std::string> vs{"Hallo","Welt"};
				WS::auto_ptr<std::vector<std::string>> ptr1;
				ptr1 = &vs;//erwarte anzeige ptr1 unmanged: 0x...{size=2} refs=1
				auto x = ptr1.release_as_unique_ptr();//erwarte anzeige ptr1 unmanaged: 0x...{size=2} refs=1
				Assert::IsFalse(ptr1.is_managed());
				Assert::IsTrue( x==nullptr );
			}
			{
				struct A : WS::enable_auto_ptr_from_this<A>{};
				WS::auto_ptr<A> ptr1;
				{
					A a;
					ptr1 = a.auto_ptr_from_this();//erwarte anzeige ptr1 manged: 0x...{...} refs=2 
					Assert::IsTrue( ptr1==&a );
					Assert::IsTrue(ptr1.is_managed());
				}
				Assert::IsTrue( ptr1==nullptr );
			}
			{
				WS::auto_ptr<int> ptr1;
				ptr1 = WS::auto_ptr<int>( new int{5}, true );//erwarte anzeige ptr1 is_owner: 0x...{5} refs=1 
				Assert::IsTrue(ptr1.is_managed());
				auto x = ptr1.release_as_unique_ptr();//erwarte anzeige ptr1 unmanaged: 0x...{5} refs=1
				Assert::IsFalse(ptr1.is_managed());
			}

		}
		TEST_METHOD(dtor_counter)
		{
			struct A
			{
				static int XRef( int add=0 ) { static int counter=0; return counter += add; }
				A() { (void)XRef( 1 ); }
				~A(){ (void)XRef( -1 ); }
			};
			{
				WS::auto_ptr<A> ptr;
				{
					A a;
					Assert::IsTrue( A::XRef()==1 );
					ptr = &a;
					Assert::IsTrue( A::XRef()==1 );
					Assert::IsTrue( ptr==&a );
				}
				Assert::IsTrue( A::XRef()==0 );
				Assert::IsTrue( ptr );//unmanaged, pointer kaputt

				ptr =  std::make_unique<A>();
				Assert::IsTrue( A::XRef()==1 );
				Assert::IsTrue( ptr );
				auto ptr1 = ptr;
				Assert::IsTrue( A::XRef()==1 );
				Assert::IsTrue( ptr );
				ptr =  std::make_unique<A>();
				Assert::IsTrue( A::XRef()==1 );
				Assert::IsTrue( ptr );
				Assert::IsTrue( !ptr1 );
			}
			Assert::IsTrue( A::XRef()==0 );
		}
		TEST_METHOD(dtor_counter_shared)
		{
			struct A
			{
				static int XRef( int add=0 ) { static int counter=0; return counter += add; }
				A() { (void)XRef( 1 ); }
				~A(){ (void)XRef( -1 ); }
			};
			{
				WS::auto_ptr<A> ptr;
				Assert::IsTrue( A::XRef()==0 );

				ptr =  std::make_shared<A>();
				Assert::IsTrue( A::XRef()==1 );
				Assert::IsTrue( ptr );
				auto ptr1 = ptr;
				auto ptr2 = ptr;
				Assert::IsTrue( A::XRef()==1 );
				Assert::IsTrue( ptr );
				ptr =  std::make_shared<A>();
				Assert::IsTrue( A::XRef()==2 );
				Assert::IsTrue( ptr );
				Assert::IsTrue( ptr1 );
			}
			Assert::IsTrue( A::XRef()==0 );
		}

	};
	TEST_CLASS( UT_auto_ptr_vw )
	{
		TEST_METHOD(add_values)
		{

			{
				WS::auto_ptr_vw<int> vw;
				{
					auto  neu{vw.push_back( new int{1} )};
					Assert::IsFalse( neu.is_owner());
					Assert::IsTrue( *neu==1 );
				}
				{
					auto  & neu{vw.push_back( new int{2} )};
					Assert::IsTrue( neu.is_owner());
					Assert::IsTrue( *neu==2 );
				}
				{
					auto  && neu{vw.push_back( new int{3} )};
					Assert::IsTrue( neu.is_owner());
					Assert::IsTrue( *neu==3 );
				}
			}

			WS::auto_ptr_vw<int> vw;
			{
				auto && neu{vw.push_back( new int{1} )};
				Assert::IsTrue( neu.is_owner());
				Assert::IsTrue( *neu==1 );
			}
			(void)vw.push_back( std::unique_ptr<int>(new int{2}) );
			(void)vw.push_back( std::shared_ptr<int>(new int{3}) );

			int value = 0;
			std::vector<WS::auto_ptr<int>> ptrs;
			for(auto & ptr : vw ) //als const &
			{
				Assert::IsTrue( ptr.is_manager() );
			}
			for(auto ptr : vw ) //als kopie
			{
				Assert::IsFalse( ptr.is_owner() );
			}

			for(auto & ptr : vw )
			{
				Assert::IsTrue( ptr.is_manager() );
				Assert::IsTrue( *ptr==++value );
				ptrs.push_back( ptr );
			}

			value = 0;
			for(auto & ptr : ptrs)
			{
				Assert::IsFalse( ptr.is_owner() );
				Assert::IsTrue( *ptr==++value );
			}

			for(auto ptr : vw ) //als kopie
			{
				Assert::IsFalse( ptr.is_owner() );
				*ptr = ++value;
			}
			value = vw.size();
			for(auto ptr : vw ) //als kopie
			{
				Assert::IsTrue(*ptr == ++value);
			}


			vw.clear();

			for(auto & ptr : ptrs)
			{
				Assert::IsTrue( ptr==nullptr || ptr.is_shared_ptr() );
			}

		}
		TEST_METHOD(erase_value)
		{
			int value = 0;
			WS::auto_ptr<int> ptr1 { new int{++value}, true};
			WS::auto_ptr<int> ptr2 { std::make_unique<int>( ++value ) };
			WS::auto_ptr<int> ptr3 { std::make_shared<int>( ++value ) };
			auto Ptr2 = ptr2.get();
			auto Ptr3 = ptr3.get();
			auto ptr4 = ptr1;


			WS::auto_ptr_vw<int> vw;
			(void)vw.push_back( ptr1 );
			(void)vw.push_back( ptr2 );
			(void)vw.push_back( ptr3 ); ptr3 = nullptr;

			{
				auto rm{ vw.erase( Ptr2 ) };
				Assert::IsTrue( rm.is_owner_or_shared() );
				Assert::IsTrue( *rm==2 );
			}
			{
				auto rm { vw.erase( Ptr2 ) };
				Assert::IsTrue( rm==nullptr );
			}
			{
				WS::auto_ptr<int> rm3;
				{
					auto rm{ vw.erase( Ptr3 ) };
					auto rm2 = rm;
					rm3 = rm.weak();
					Assert::IsTrue( rm.is_owner_or_shared() );
					Assert::IsTrue( *rm==3 );
				}
				Assert::IsTrue( rm3==nullptr );
			}
			{
				auto rm = vw.erase( Ptr3 );
				Assert::IsTrue( rm==nullptr );
			}
			{
				auto rm{ vw.erase( ptr1 ) };
				Assert::IsTrue( rm.is_owner_or_shared() );
				Assert::IsTrue( *rm==1 );
			}
			{
				auto rm = vw.erase( ptr1 );
				Assert::IsTrue( rm==nullptr );
			}
		}
		TEST_METHOD(replace_value)
		{
			int value = 0;
			WS::auto_ptr<int> ptr1 { new int{++value}, true};
			WS::auto_ptr<int> ptr2 { std::make_unique<int>( ++value ) };
			WS::auto_ptr<int> ptr3 { std::make_shared<int>( ++value ) };
			auto Ptr2 = ptr2.get();
			auto Ptr3 = ptr3.get(); 


			WS::auto_ptr_vw<int> vw;
			auto p1 = vw.push_back( ptr1 );
			auto p2 = vw.push_back( ptr2 );
			auto p3 = vw.push_back( ptr3 ); ptr3 = nullptr;
			p1; p2; p3;

			{
				auto rm{ vw.replace( Ptr2, new int{++value} ) };
				Assert::IsTrue( rm.is_owner_or_shared() );
				Assert::IsTrue( *rm==2 );
			}
			{
				auto rm{ vw.replace( Ptr3, std::make_unique<int>( ++value ) ) };
				Assert::IsTrue( rm.is_owner_or_shared() );
				Assert::IsTrue( *rm==3 );
			}
			{
				auto rm{ vw.replace( ptr1, std::make_unique<int>( ++value ) ) };
				Assert::IsTrue( rm.is_owner_or_shared() );
				Assert::IsTrue( *rm==1 );
			}
			Assert::IsTrue( *vw[0]==6 );
			Assert::IsTrue( *vw[1]==4 );
			Assert::IsTrue( *vw[2]==5 );
		}
		TEST_METHOD(auto_ptr_vw__container)
		{
			{
				auto init_counter = ctor_counter::counter;
				{

					WS::auto_ptr_vw<ctor_counter,std::deque<WS::auto_ptr<ctor_counter>>> vw;
					(void)vw.push_back( new ctor_counter{} );
					Assert::IsTrue(init_counter+1==ctor_counter::counter);
					(void)vw.erase( nullptr );
					(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
					(void)vw[0];
				}
				Assert::IsTrue(init_counter==ctor_counter::counter, L"memory leak detected");
			}		
			{
				WS::auto_ptr_vw<int,std::deque<WS::auto_ptr<int>>> vw;
				(void)vw.push_back( new int{5} );
				(void)vw.erase( nullptr );
				(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
				(void)vw[0];
			}
			{
				WS::auto_ptr_vw<int,std::list<WS::auto_ptr<int>>> vw;//list bedingt brauchbar
				(void)vw.push_back( new int{5} );
				(void)vw.erase( nullptr );
				(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
				//(void)vw[0];//list unterstützt operator[] nicht
			}
			{
				WS::auto_ptr_vw<int,std::forward_list<WS::auto_ptr<int>>> vw;//forward_list unbrauchbar
				//(void)vw.push_back( new int{5} );//forward_list unterstütz emplace_back nicht
				//(void)vw.erase( nullptr );
				(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
				//(void)vw[0];
			}
		}
	};
}

