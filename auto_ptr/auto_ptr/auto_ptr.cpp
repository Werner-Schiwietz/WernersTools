#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <initializer_list>
#include <memory>

#include <vector>
#include <deque>
#include <set>
#include <list>
#include <forward_list>

#include "..\..\headeronly\Auto_Ptr.h"

#include <basetsd.h>	//INT_PTR

//#include <afx.h>
//#define new DEBUG_NEW

enum Enum { v1, v2, v3 };

//vorsicht std::initializer_list darf man/kann man nicht variablen zuweisen, die daten der liste gehen verloren
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

			auto & container = x;//red eine std::initializer_list darf nicht zwischen gespeichert werden
			//auto & container = vec;//green
			//auto & container = vec2;//green

			for( auto iter = container.begin(); iter!=container.end(); ++iter )
				cout << *iter << ' ';
			for( auto e : container )
				cout << e << ' ';
			cout << std::endl;
			Logger::WriteMessage( cout.str().c_str() );

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

				Aptr = (A*)nullptr;
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
				AA const & constaa = aa;
				AA *paa = &aa;
				AA const * constpaa = &aa;constpaa;
				WS::managed_auto_ptr<AA> AAptr;
				AAptr  = aa;

				auto constAAptr = WS::managed_auto_ptr<AA const>{ aa };
				constAAptr = &aa;
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );
				constAAptr = paa;
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );
				constAAptr = constpaa;
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );
				constAAptr = WS::managed_auto_ptr<AA const>{aa};
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );
				constAAptr = WS::managed_auto_ptr<AA const>{&aa};
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );
				constAAptr = WS::managed_auto_ptr<AA const>{constaa};
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );
				constAAptr = WS::managed_auto_ptr<AA const>{&constaa};
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );
				constAAptr = WS::managed_auto_ptr<AA const>{constpaa};
				Assert::IsTrue( constpaa == constAAptr );
				Assert::IsTrue( constAAptr == constpaa );

				//AAptr = constAAptr;//error C2682: cannot use 'dynamic_cast' to convert from 'const autoptr::UT_managed_auto_ptr::managed_auto_ptr_auto_ptr_from_this::AA *' to 'autoptr::UT_managed_auto_ptr::managed_auto_ptr_auto_ptr_from_this::AA *'
				//AAptr = constpaa;//static_assert "wenn U const ist muss T auch const sein"


				//AAptr = a;//sollte compilefehler geben
				//AAptr = b;//sollte compilefehler geben
				//Aptr = aa;//sollte compilefehler geben
				//Bptr = aa;//sollte compilefehler geben
			}

			Assert::IsFalse(Aptr);
			Assert::IsFalse(Bptr);
		}
		TEST_METHOD(managed_auto_ptr_auto_ptr_from_this_from_unmanaged_auto_ptr)
		{
			class A : public WS::enable_auto_ptr_from_this<A>
			{public:virtual ~A(){}};
			class B : public A
			{public:virtual ~B(){}};
			class C
			{public:virtual ~C(){}};


			WS::managed_auto_ptr<A> Aptr;
			WS::managed_auto_ptr<B> Bptr;
			WS::managed_auto_ptr<C> Cptr;

			A a;
			B b;
			C c;
			Aptr = &a;
			Bptr = &a;
			Bptr = &b;
			//Cptr = &c;//error C2679: binary '=': no operator found which takes a right-hand operand of type 'BasisUnitTests::UT_managed_auto_ptr::managed_auto_ptr_auto_ptr_from_this_from_unmanaged_auto_ptr::C *' (or there is no acceptable conversion)
			Aptr = a;
			Bptr = a;
			Bptr = b;
			//Cptr = c;//error C2679: binary '=': no operator found which takes a right-hand operand of type 'BasisUnitTests::UT_managed_auto_ptr::managed_auto_ptr_auto_ptr_from_this_from_unmanaged_auto_ptr::C' (or there is no acceptable conversion)

			WS::auto_ptr<B> B2ptr;//unmanaged_auto_ptr, die konnte man bis 2022-02-19 einem managed_auto_ptr nicht zuweisen. nun geht es, wenn T von enable_auto_ptr_from_this abgeleitet ist
			WS::auto_ptr<C> C2ptr;//unmanaged_auto_ptr, die konnte man bis 2022-02-19 einem managed_auto_ptr nicht zuweisen. nun geht es, wenn T von enable_auto_ptr_from_this abgeleitet ist
			WS::auto_ptr<A> A2ptr;//unmanaged_auto_ptr, die konnte man bis 2022-02-19 einem managed_auto_ptr nicht zuweisen. nun geht es, wenn T von enable_auto_ptr_from_this abgeleitet ist
			A2ptr = (A*)nullptr;
			A2ptr = a;
			//A2ptr = &c;//error C2679: binary '=': no operator found which takes a right-hand operand of type 'autoptr::UT_managed_auto_ptr::managed_auto_ptr_auto_ptr_from_this_from_unmanaged_auto_ptr::C *' (or there is no acceptable conversion)
			A2ptr = &b;
			A2ptr = &a;
			//B2ptr = &a;//error C2679: binary '=': no operator found which takes a right-hand operand of type 'BasisUnitTests::UT_managed_auto_ptr::managed_auto_ptr_auto_ptr_from_this_from_unmanaged_auto_ptr::A *' (or there is no acceptable conversion)
			B2ptr = &b;
			C2ptr = &c;

			Aptr = A2ptr;
			Assert::IsTrue(Aptr.is_managed());
			Bptr = B2ptr;
			Assert::IsTrue(Aptr.is_managed());
			Bptr = A2ptr;
			Assert::IsTrue(Bptr==nullptr);
			{
				WS::auto_ptr<C> C3ptr;
				Cptr = C3ptr;
				C3ptr = std::make_unique<C>();
				Cptr = C3ptr;
				Assert::IsTrue( Cptr.is_managed() );
				Assert::IsFalse( Cptr.is_owner() );
				try
				{
					Cptr = C2ptr;//von C kann kein managed_auto_ptr erzeugt werden -> exception
					Assert::Fail( L"exception erwartet" );
				}
				catch( ... ) {}
			}

			Aptr = std::make_unique<A>();
			Assert::IsTrue(Aptr.is_managed());
			Assert::IsTrue(Aptr.is_owner());
			Bptr = std::make_unique<A>();//geht seit 2022-04-09, allerdings wird Bptr immer nullptr sein, weil der dynamic_cast<B*>(&a) nullptr liefert
			Assert::IsFalse(Bptr);
			std::unique_ptr<A> AuniquePtr{ std::make_unique<B>() };
			Bptr = std::move(AuniquePtr);
			Assert::IsTrue(Bptr);
			//Bptr = std::make_unique<C>();//kann nicht gehen, da C mit B keine klassenbeziehung hat
			{
				WS::auto_ptr<A> x2(std::make_unique<B>());//geht seit 2022-04-09
				WS::auto_ptr<A> x{WS::auto_ptr<B>{std::make_unique<B>()}};//so ging es schon immer
				x = std::make_unique<B>();
			}

			Aptr = WS::auto_ptr<B>{std::make_unique<B>()};//so ging es schon immer
			Aptr = std::make_unique<B>();//geht seit 2022-04-09
			{
				WS::managed_auto_ptr<A> AxPtr{std::make_unique<B>()};
				Assert::IsTrue(AxPtr);
				AxPtr = std::make_unique<B>();
				Assert::IsTrue(AxPtr);
				
			}
			Assert::IsTrue(Aptr.is_managed());
			Assert::IsTrue(Aptr.is_owner());
			Cptr = std::make_unique<C>();
			Assert::IsTrue(Cptr.is_managed());
			Assert::IsTrue(Cptr.is_owner());
		}
		TEST_METHOD(managed_auto_ptr_sorted_set)
		{
			class A : public WS::enable_auto_ptr_from_this<A>
			{
			public:
				int v=5;
				A(){}
				A(int v):v(v){}
				virtual ~A(){}
			};

			std::set<WS::managed_auto_ptr<A>> set;
			A a1{5},a2{4},a3{6};
			set.insert(a1);
			set.insert(a2);
			set.insert(WS::managed_auto_ptr<A>{a3});//so ohne roten kringel
			WS::managed_auto_ptr<A> middle_ptr = *(++set.begin());//!!nicht sicher ob a1 a2 oder a3 middle werden wird!!
			A & middle = *middle_ptr;
			auto testSortierung = [&]()->bool
			{
				auto v = set.begin();
				for( auto prev=v++;v!=set.end();++v,++prev )
				{
					if( (*prev<*v) == false )
						return false;
				}
				return true;
			};
			Assert::IsTrue(testSortierung());

			Assert::IsTrue(middle_ptr);
			middle.~A();//explicit dtor, without delete. just for testing, never do things like this
			Assert::IsFalse(middle_ptr);
			Assert::IsTrue(testSortierung());//obwohl der mittlere ptr im set nullptr liefert ist die sortierung noch korrekt
			new(&middle) A{2};//palcement new. auf speicher von middle den ctor ausf�hren. dadurch bekommt der managed_auto_ptr KEINE neue speicheradresse
			Assert::IsFalse(middle_ptr);
			Assert::AreEqual(middle.v,2);

		}
	};
	TEST_CLASS(autoptr)
	{
	public:
		TEST_METHOD(auto_ptr_constcast)
		{
			WS::auto_ptr<int> ptr;
			WS::auto_ptr<int> ptr2;
			WS::auto_ptr<int const> cptr;

			ptr = std::unique_ptr<int>{ new int{5} };
			cptr = WS::toconst_cast(ptr);
			ptr2 = WS::notconst_cast(cptr);

			Assert::IsTrue( *cptr == *ptr);++*ptr;
			Assert::IsTrue( *ptr2 == *ptr);++*ptr;

			cptr = WS::toggleconst_cast(ptr);
			ptr2 = WS::toggleconst_cast(cptr);

			cptr = ptr2;

			ptr = nullptr;
			Assert::IsTrue( cptr == nullptr );
			Assert::IsTrue( ptr2 == nullptr );
		}
		TEST_METHOD(auto_ptr_from_const_cast)
		{
			WS::auto_ptr<int> ptr;
			WS::auto_ptr<int> ptr2;
			WS::auto_ptr<int const> cptr;

			ptr = std::unique_ptr<int>{ new int{5} };
			cptr = ptr;
			ptr2 = cptr.notconst();

			Assert::IsTrue( *cptr == *ptr);++*ptr;
			Assert::IsTrue( *ptr2 == *ptr);++*ptr;

			ptr = nullptr;
			Assert::IsTrue( cptr == nullptr );
			Assert::IsTrue( ptr2 == nullptr );
		}
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
			Assert::IsFalse( x.is_owner() );
			Assert::IsTrue(x==&i);
			auto xx = x;
			Assert::IsFalse(x.is_owner());
			Assert::IsTrue(x == &i);
			Assert::IsFalse(xx.is_owner());
			Assert::IsTrue(xx == &i);
		}
		TEST_METHOD(auto_ptr__copy_ownerless2)
		{
			int i = 5;
			WS::auto_ptr<int> x{ &i };
			Assert::IsFalse(x.is_owner());
			Assert::IsTrue(x == &i);
			auto xx = x;
			Assert::IsFalse(x.is_owner());
			Assert::IsTrue(x == &i);
			Assert::IsFalse(xx.is_owner());
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
				Assert::IsTrue( ptr.is_owner() );
				(void)ptr.transfer();//wenn is_owner, und das ist er hier, wird das objekt freigegeben
				Assert::IsFalse(ptr.is_owner());
				Assert::IsFalse(ptr);
			};

			auto ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );
			fn( std::move( ptr ) );
			Assert::IsFalse( ptr );
		}
		TEST_METHOD(auto_ptr__as_parameter_lvalueref)
		{
			auto fn = [](WS::auto_ptr<int>& ptr)
			{
				Assert::IsTrue( ptr );
				if( ptr.is_owner() )
				{
					(void)ptr.transfer();//wenn is_owner, wird das objekt freigegeben
					Assert::IsFalse(ptr.is_owner());
					Assert::IsFalse(ptr);
				}
				else
				{
					(void)ptr.transfer();//wenn nicht is_owner, wird das objekt nicht freigegeben
					Assert::IsFalse(ptr.is_owner());
					Assert::IsTrue(ptr);
				}
			};

			auto ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );

			auto ptr1 = ptr;
			Assert::IsTrue( ptr1 );
			Assert::IsFalse( ptr1.is_owner() );

			fn( ptr1 );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );
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
				ptr_owner = ptr;//ptr bleibt is_owner
			};
			auto releaseptr = [&]( WS::auto_ptr<int> && ptr )
			{
				(void)ptr.transfer();
			};

			auto ptr= WS::auto_ptr<int>( new int( 5 ), true );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );

			donothing( std::move( ptr ) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );

			takeownership( std::move( ptr ) );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr.is_owner() );
			Assert::IsTrue( ptr_owner.get() == ptr.get() );
			Assert::IsTrue( ptr_owner == ptr );
			Assert::IsTrue( ptr_owner == ptr.get() );
			Assert::IsTrue( ptr_owner.get() == ptr );

			releaseptr( std::move( ptr_owner ) );
			Assert::IsFalse( ptr_owner );
			Assert::IsFalse( ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			moveit( std::move(ptr) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr_owner );
			Assert::IsTrue( ptr_owner.is_owner() );
			Assert::IsTrue( ptr_owner==ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			assignit( std::move(ptr) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );
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
			Assert::IsTrue( ptr.is_owner() );

			donothing( ptr );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );

			donothing( ptr.transfer() );
			Assert::IsFalse( ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			donothing( std::move(ptr) );
			Assert::IsFalse( ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );

			takeownership( ptr  );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr.is_owner() );
			Assert::IsTrue( ptr_owner == ptr );
			takeownership( ptr.transfer() );
			Assert::IsTrue( ptr );
			Assert::IsFalse( ptr.is_owner() );
			Assert::IsTrue( ptr_owner == ptr );

			ptr= WS::auto_ptr<int>( new int( 5 ), true );
			takeownership( std::move( ptr ) );
			Assert::IsTrue( ptr );
			Assert::IsTrue( ptr_owner.is_owner() );
		}
		TEST_METHOD(auto_ptr_assign)
		{
			{	//1 zuweisung x bleibt is_owner
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = x;
				Assert::IsTrue( x );
				Assert::IsTrue( x.is_owner() );
				Assert::IsTrue( x2 );
				Assert::IsFalse( x2.is_owner() );
				Assert::IsTrue( x2==x );
			}
			{	//2 same as 1
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = x.ownerless();
				Assert::IsTrue( x );
				Assert::IsTrue( x.is_owner() );
				Assert::IsTrue( x2 );
				Assert::IsFalse( x2.is_owner() );
				Assert::IsTrue( x2==x );
			}
			{	//3 move x wird nullptr x2 wird is_owner
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = std::move(x);
				Assert::IsTrue( x );
				Assert::IsTrue( x2 );
				Assert::IsTrue( x2.is_owner() );
				Assert::IsFalse( x.is_owner() );
				Assert::IsTrue( x2 == x );
			}
			{	//4 transfer x beh�lt pointer x2 wird is_owner
				WS::auto_ptr<int> x{ std::make_unique<int>(5) };
				WS::auto_ptr<int> x2;
				x2 = x.transfer();
				Assert::IsTrue( x );
				Assert::IsFalse( x.is_owner() );
				Assert::IsTrue( x2 );
				Assert::IsTrue( x2.is_owner() );
				Assert::IsTrue( x2==x );
			}
		}
		TEST_METHOD(auto_ptr__ownership_transfer)
		{
			WS::auto_ptr<int> x{ std::make_unique<int>(5) };
			Assert::IsTrue(x.is_owner());
			auto xx = x.transfer();
			Assert::IsFalse(x.is_owner());
			Assert::IsTrue(xx.is_owner());
			auto xxx = x.transfer();
			Assert::IsFalse(x.is_owner());
			Assert::IsTrue(xx.is_owner());
			Assert::IsFalse(xxx.is_owner());
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

			Assert::IsTrue( ab.is_owner() );
			Assert::IsTrue(ab->foo()== 3);
			Assert::IsTrue(a->foo() == 1);
			Assert::IsTrue(b->foo() == 2);

			b = ab.transfer();//ups
			Assert::IsTrue( b.is_owner() );
			Assert::IsTrue(ab->foo()== 3);
			Assert::IsTrue(a->foo() == 1);
			Assert::IsTrue(b->foo() == 2);

			auto pab = ab.get();
			auto pa = a.get();
			auto pb = b.get();
			Assert::IsTrue( (void const *)pa != (void const *)pb );
			Assert::IsTrue( (void const *)pa == (void const *)pab || (void const *)pb == (void const *)pab );

		}
		TEST_METHOD(auto_ptr_dangling_pointer)
		{
			int acounter=0;
			int ccounter=0;
			struct A : WS::enable_auto_ptr_from_this<A>{int& counter;virtual ~A(){--counter;}A(int& counter):counter(++counter){}};
			struct B : A{B(int& counter):A(counter){}};
			struct C{int& counter;virtual ~C(){--counter;}C(int& counter):counter(++counter){}};
			struct D : C{D(int& counter):C(counter){}};

			{
				{
					WS::auto_ptr<A> aPtr;
					aPtr = (A*)nullptr;
					//aPtr = A{acounter};
					A a{acounter};
					aPtr = a;
					aPtr = &a;
					WS::auto_ptr<A> aPtr2 = aPtr;

				}
				WS::auto_ptr<A> aPtr;
				WS::auto_ptr<C> cPtr;
				{
					Assert::IsFalse(aPtr);
					Assert::IsFalse(cPtr);
					Assert::AreEqual(acounter,0);
					Assert::AreEqual(ccounter,0);
					A a{acounter};
					C c{ccounter};
					Assert::AreEqual(acounter,1);
					Assert::AreEqual(ccounter,1);
					aPtr = &a;
					cPtr = &c;
					Assert::IsTrue(aPtr);
					Assert::IsTrue(cPtr);
					Assert::AreEqual(acounter,1);
					Assert::AreEqual(ccounter,1);
				}
				//a und c sind zerst�rt
				Assert::AreEqual(acounter,0);
				Assert::AreEqual(ccounter,0);

				//aPtr ist nullptr, aber cPtr ist nicht NULL
				Assert::IsFalse(aPtr);//nullptr, weil A von enable_auto_ptr_from_this abgeleitet ist
				Assert::IsTrue(cPtr);//tja, dangling pointer
			}
		}
		TEST_METHOD(auto_ptr_from_this_delete)
		{
			int acounter=0;
			int ccounter=0;
			struct A : WS::enable_auto_ptr_from_this<A>{int& counter;virtual ~A(){--counter;}A(int& counter):counter(++counter){}};
			struct B : A{B(int& counter):A(counter){}};
			struct C{int& counter;virtual ~C(){--counter;}C(int& counter):counter(++counter){}};
			struct D : C{D(int& counter):C(counter){}};

			{
				Assert::AreEqual(ccounter,0);
				WS::auto_ptr<C> Ptr = std::make_unique<C>(ccounter);
				Assert::AreEqual(ccounter,1);
				delete Ptr;
				Assert::AreEqual(ccounter,0);
				Assert::IsTrue( Ptr );	//dangling pointer
				Ptr.release();			//prevent double free
				Assert::AreEqual(ccounter,0);
			}

			{
				Assert::AreEqual(acounter,0);
				WS::auto_ptr<A> Ptr = std::make_unique<A>(acounter);
				Assert::AreEqual(acounter,1);
				Assert::IsTrue(Ptr.is_owner());
				auto Ptr2 = Ptr;
				Assert::IsFalse(Ptr2.is_owner());
				WS::auto_ptr<A> aPtr1 = Ptr->auto_ptr_from_this();
				WS::managed_auto_ptr<A> aPtr2 = *Ptr;
				Assert::IsFalse(aPtr1.is_owner());
				Assert::IsFalse(aPtr2.is_owner());
				Assert::AreEqual(acounter,1);
				delete Ptr;//since 2020-04-16 no problem when its base-class is enable_auto_ptr_from_this
				Assert::AreEqual(acounter,0);
				Ptr = nullptr;
				Assert::AreEqual(acounter,0);
				Assert::IsFalse( Ptr );
				Assert::IsFalse( aPtr1 );
				Assert::IsFalse( aPtr2 );

				Ptr = std::make_unique<A>(acounter);
				Assert::AreEqual(acounter,1);
				Ptr2 = Ptr;
				aPtr1 = Ptr->auto_ptr_from_this();
				aPtr2 = *Ptr;
				Ptr = nullptr;//speicher freigeben und alle pointer auf null
				Assert::AreEqual(acounter,0);
				Assert::IsFalse( Ptr );
				Assert::IsFalse( aPtr1 );
				Assert::IsFalse( aPtr2 );

				Ptr = std::make_unique<B>(acounter);
				Assert::AreEqual(acounter,1);
				Ptr2 = Ptr;
				aPtr1 = Ptr->auto_ptr_from_this();
				aPtr2 = *Ptr;
				delete Ptr;
				Assert::AreEqual(acounter,0);
				Assert::IsFalse( Ptr );
				Assert::IsFalse( aPtr1 );
				Assert::IsFalse( aPtr2 );

				Ptr = std::make_unique<B>(acounter);
				Assert::AreEqual(acounter,1);
				Ptr2 = Ptr;
				aPtr1 = Ptr->auto_ptr_from_this();
				aPtr2 = *Ptr;
				Ptr = nullptr;//speicher freigeben und alle pointer auf null
				Assert::AreEqual(acounter,0);
				Assert::IsFalse( Ptr );
				Assert::IsFalse( aPtr1 );
				Assert::IsFalse( aPtr2 );

				{
					WS::auto_ptr<B> bPtr = std::make_unique<A>(acounter);
					Assert::AreEqual(acounter,0);
					Assert::IsFalse(bPtr);
				}
				{
					auto vecPtr = std::make_unique<std::vector<A>>();
					WS::auto_ptr<std::vector<A>> vec = std::move(vecPtr);
				}
				{
					WS::auto_ptr<C> cPtr1 = std::unique_ptr<C>{};
					WS::auto_ptr<C> cPtr2 = std::unique_ptr<D>{};
					WS::auto_ptr<D> dPtr1 = std::unique_ptr<C>{};
					WS::auto_ptr<D> dPtr2 = std::unique_ptr<D>{};

					Assert::IsFalse(cPtr1);
					Assert::IsFalse(cPtr2);
					Assert::IsFalse(dPtr1);
					Assert::IsFalse(dPtr2);
				}
				{
					WS::auto_ptr<C> cPtr1 = std::make_unique<C>(ccounter);
					WS::auto_ptr<C> cPtr2 = std::make_unique<D>(ccounter);
					WS::auto_ptr<D> dPtr1 = std::make_unique<C>(ccounter);
					WS::auto_ptr<D> dPtr2 = std::make_unique<D>(ccounter);
					Assert::AreEqual(ccounter,3);

					Assert::IsTrue(cPtr1);
					Assert::IsTrue(cPtr2);
					Assert::IsFalse(dPtr1);
					Assert::IsTrue(dPtr2);
				}
				Assert::AreEqual(ccounter,0);
			}
			Assert::AreEqual(acounter,0);
			Assert::AreEqual(ccounter,0);
		}
		TEST_METHOD(auto_ptr_from_this)
		{
			struct X : WS::enable_auto_ptr_from_this<X>{ int value; X(){} X(int v):value(v){}};

			WS::auto_ptr<X> ptr;
			Assert::IsNull( ptr.get() );
			{
				X x{5};
				X x1 = x;
				ptr = x.auto_ptr_from_this();
				Assert::IsNotNull(ptr.get());
				Assert::IsFalse(ptr.is_owner());
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
		TEST_METHOD(auto_ptr_from_this_moved)
		{
			struct A : WS::enable_auto_ptr_from_this<A>
			{
				int v{};
				~A(){}
				A() noexcept {}
				A(int v) noexcept :v(v){}
				A(A const & r) noexcept : v(r.v){}
				A(A && r) noexcept : enable_auto_ptr_from_this(std::move(r)){swap(r);}
				A& operator=(A const & r) & noexcept {this->v=r.v;return *this;}
				A& operator=(A && r) & noexcept {A{std::move(r)}.swap(*this);return *this;}
				void swap( A & r )
				{
					std::swap(this->v,r.v);
				}
			};

			A a1{1};
			A a2 = std::move(a1);

			auto a1_ptr = a1.auto_ptr_from_this();
			a1_ptr->v=2;
			auto a2_ptr = a2.auto_ptr_from_this();
			auto v1 = a1_ptr->v;
			auto v2 = a2_ptr->v;
			Assert::IsTrue(v1==2);
			Assert::IsTrue(v2==1);
			auto pa1 = a1_ptr.get();pa1;
			auto pa2 = a2_ptr.get();pa2;
			a1 = std::move(a2);
			auto v1_2 = a1_ptr->v;
			Assert::IsTrue(v1_2==1);
			Assert::IsTrue(a2_ptr->v==int{});
			Assert::IsTrue( a1_ptr.get()==pa1 );
			Assert::IsTrue( a2_ptr.get()==pa2 );
		}
		TEST_METHOD(auto_ptr_from_this_von_objekt_im_vector)
		{	//per auto_ptr_from_this gemerkte pointer auf std::vector-elemente werden durch realloc im vector zu nullptr, obwohl die ojekte im vector am index noch vorhanden sind, aber halt an anderer adresse
			//deshalb nicht auf dynamische container anwenden, zu gef�hrlich f�r den programmablauf
			struct Data : WS::enable_auto_ptr_from_this<Data>
			{
				Data(int value):value(value){}
				Data(Data const &) noexcept = default;
				Data(Data &&) noexcept = default;//ohne noexcept nutzt vector beim umkopieren den ctor, mit noexcept nutzt vector beim umkopieren diesen mtor
				Data& operator=(Data const &) noexcept = default;
				Data& operator=(Data &&) noexcept = default;
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
				{	//solange nicht umkopiert wurde sind die pointer g�ltig
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
				{	//auch wenn die objekte im container umkopiert worden sind, sind die gemerkten auto_pointer noch g�ltig
					p1 = p2;
					for( int index=0; index<value; ++index )
					{
						Assert::IsTrue( container[index]==ptrs[index] );
						Assert::IsTrue( container[index]->value==index );
						Assert::IsTrue( ptrs[index]->value==index );
					}
				}
				else
				{	//solange nicht umkopiert wurde sind die pointer g�ltig
					for( int index=0; index<=value; ++index )
					{
						Assert::IsTrue( container[index]==ptrs[index] );
						Assert::IsTrue( container[index]->value==index );
						Assert::IsTrue( ptrs[index]->value==index );
					}
				}
			}

			container.clear();
			//nach clear alle gemerkten nicht is_owner pointer null
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
				Assert::Fail( L"exception erwartet, weil foo keinen is_owner als parameter erhaelt" );
			}
			catch(...){}
			Assert::IsTrue( _startwert==10 );
			delete ptr;
			Assert::IsTrue( _startwert==11 );
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_shared_ptr)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )->WS::auto_ptr<int>//auto_ptr_owner_parameter wirft exception, wenn ptr nicht is_owner ist
			{
				return ptr;
			};

			WS::auto_ptr<int> ptr { std::shared_ptr<int>( new int( 5 ) ) };
			Assert::IsFalse( ptr.is_owner() );
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
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )->WS::auto_ptr<int>//auto_ptr_owner_parameter wirft exception, wenn ptr nicht is_owner ist
			{
				return ptr;
			};
			WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
			Assert::IsTrue( ptr.is_owner() );
			auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( ptr.transfer() );//egal ob transfer oder direkt uebergeben
			Assert::IsFalse( ptr.is_owner() );
			Assert::IsTrue( ptr_owner.is_owner() );
			Assert::IsTrue( *ptr == 5 );
			Assert::IsTrue( *ptr_owner == 5 );
			Assert::IsTrue( ptr_owner==ptr );
			(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerst�rt
			Assert::IsTrue( ptr == nullptr );
			Assert::IsTrue( ptr_owner == nullptr );
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_referenz)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht is_owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
				Assert::IsTrue( ptr.is_owner() );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( ptr );//egal ob transfer oder direkt uebergeben. ptr ist nicht mehr is_owner. waere er es bei aufruf nicht, fl�ge eine exception
				Assert::IsFalse( ptr.is_owner() );
				Assert::IsTrue( ptr_owner.is_owner() );
				Assert::IsTrue( *ptr == 5 );
				Assert::IsTrue( *ptr_owner == 5 );
				Assert::IsTrue( ptr_owner == ptr );
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerst�rt
				Assert::IsTrue( ptr == nullptr );
				Assert::IsTrue( ptr_owner == nullptr );
			}
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_move)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht is_owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
				Assert::IsTrue( ptr.is_owner() );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( std::move(ptr) );//std::move f�hrt hier zu ptr==nullptr, evtl besser transfer()
				Assert::IsFalse( ptr.is_owner() );
				Assert::IsTrue( ptr_owner.is_owner() );
				Assert::IsTrue( ptr == ptr_owner );
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerst�rt
				Assert::IsTrue( ptr == nullptr );
				Assert::IsTrue( ptr_owner == nullptr );
			}
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_unique_ptr)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht is_owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				auto ptr = std::unique_ptr<int>( new int( 5 ) );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( std::move(ptr) );//std::move f�hrt hier zu ptr==nullptr
				Assert::IsTrue( ptr == nullptr );
				Assert::IsTrue( ptr_owner != nullptr );
				Assert::IsTrue( *ptr_owner == 5 );
				Assert::IsTrue( ptr_owner.is_owner() );
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerst�rt
			}
		}
		TEST_METHOD(uebergebe_ptr_als_owner_per_unmanged_ptr)
		{
			auto fnTakeOverOwnershipAndReturnOwner=[]( WS::auto_ptr_owner_parameter<int> ptr )//auto_ptr_owner_parameter wirft exception, wenn ptr nicht is_owner ist
			{
				return WS::auto_ptr<int>(ptr);
			};
			{
				auto unmanged_ptr = new int( 5 );
				Assert::IsTrue( *unmanged_ptr==5 );
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( unmanged_ptr );
				Assert::IsTrue( ptr_owner.is_owner() );
				Assert::IsTrue( *unmanged_ptr==5 );
				(void)ptr_owner.transfer();//liefert owner_ptr als returnwert. da dieses nicht verwendet wird, wird verwaltetes objekt zerst�rt
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
			Assert::IsFalse( ptr.is_owner() );
			try
			{
				//fnTakeOverOwnershipAndReturnOwner wirft exception, wenn parameter nicht is_owner ist
				auto ptr_owner = fnTakeOverOwnershipAndReturnOwner( ptr );
				Assert::Fail( L"exception erwartet" );
			}
			catch(std::exception &  )
			{}
		}
		TEST_METHOD(auto_ptr_owner_parameter_zugriff)
		{
			auto ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
			WS::auto_ptr<int>{ptr};//cast nach auto_ptr genau einmal m�glich, danach ist ptr empty
			Assert::IsTrue( static_cast<WS::auto_ptr<int>>(ptr) == nullptr );
			Assert::IsTrue( static_cast<WS::auto_ptr<int>>(ptr) == NULL );

			ptr = WS::auto_ptr_owner_parameter<int>( new int( 6 ) );
			int *				p1 = nullptr;
			WS::auto_ptr<int>	p2;
			{
				WS::auto_ptr<int> is_owner = ptr;//�bertragung des objekt-pointers auf is_owner, ptr wird empty
				Assert::IsTrue( static_cast<WS::auto_ptr<int>>(ptr) == nullptr );
				p1 = is_owner;
				p2 = is_owner;
				Assert::IsTrue( is_owner.is_owner() );
				Assert::IsTrue( *is_owner == 6 );
				Assert::IsFalse( p2.is_owner() );
				Assert::IsTrue( *p2 == 6 );
				Assert::IsTrue( *p1 == 6 );
			}
			//Assert::IsTrue( *p1 != 6 );//unmanaged pointer auf ung�ltigen speicher
			Assert::IsTrue( p2 == nullptr );//managed pointer wird zu nullptr
		}
		TEST_METHOD(release_as_unique_ptr)
		{	//release sind schlechte funktionen, weil die pointer im zugriff bleiben, die zerst�rung der objekt aber nicht bemerkt wird
			WS::auto_ptr<int> ptr = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
			Assert::IsTrue( ptr.is_owner() );
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
				Assert::IsTrue( ptr.is_owner() );
				{
					auto p1 = ptr.release_as_unique_ptr();
					Assert::IsTrue( p1.get() );
				}
				//Assert::IsTrue( *ptr==5 );
			}

			{
				WS::auto_ptr<int> ptr1 = WS::auto_ptr_owner_parameter<int>( new int( 5 ) );
				ptr = ptr1;
				Assert::IsFalse( ptr.is_owner() );
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
		TEST_METHOD(dtor_cast_inherit)
		{
			struct A
			{
				static int XRef( int add=0 ) { static int counter=0; return counter += add; }
				A() { (void)XRef( 1 ); }
				virtual ~A(){ (void)XRef( -1 ); }
			};
			struct B : A
			{
			};

			{
				Assert::IsTrue( A::XRef()==0 );
				A a;
				B b;
				Assert::IsTrue( A::XRef()==2 );

				WS::auto_ptr<A> ptrA;
				WS::auto_ptr<B> ptrB;

				ptrA = &a;
				Assert::IsTrue( ptrA );
				ptrA = &b;
				Assert::IsTrue( ptrA );

				ptrA = &b;
				Assert::IsTrue( ptrA );
				ptrB = &b;
				Assert::IsTrue( ptrB );
				//ptrB = &a;// error C2679: binary '=': no operator found which takes a right-hand operand of type 'autoptr::autoptr::dtor_cast::A *' (or there is no acceptable conversion)
				ptrA = &a;
				ptrB = ptrA;
				Assert::IsTrue( ptrB==nullptr );
			}
			Assert::IsTrue( A::XRef()==0 );
		}
		TEST_METHOD(dtor_cast_const)
		{
			auto ptr = std::unique_ptr<char[]>{ new char[]{"hallo"}};
			WS::auto_ptr<char[]> ptrA{ptr.get()};
			Assert::IsTrue(ptrA);
			WS::auto_ptr<char const[]> ptrB;

			ptrB = ptrA;
			Assert::IsTrue(ptrA);
			Assert::IsTrue(ptrB);
			ptrB = std::move(ptrA);//w�re A owner gewesen, w�rde er es jetzt nicht mehr owner sein. der pointer bleibt aber immer gesetzt
			#pragma warning(suppress:26800)//ptrA sei per move ung�ltig, ist aber nicht so
			Assert::IsTrue(ptrA);
			Assert::IsTrue(ptrB);

			//ptrA = ptrB;//error C2338: static_assert failed: 'wenn U const ist muss T auch const sein'
			//ptrA = std::move(ptrB);//error C2338: static_assert failed: 'wenn U const ist muss T auch const sein'
			//Assert::IsTrue(ptrB);
			//Assert::IsFalse(ptrA);//von char const[] kann nicht auf char[] gecastet werden
		}
		TEST_METHOD(UT_auto_ptr_owner_cast)
		{
			struct B
			{
				virtual ~B(){}
			};
			struct D : B{};
			struct C {};

			WS::auto_ptr<B> Bptr{ WS::make_auto_ptr<D>() };
			Assert::IsTrue(Bptr.is_owner());
			WS::auto_ptr<D> Dptr = std::move(Bptr);//Bptr ist angelegt als D, kann also dem Dptr zugewiesen werden. Dptr wird owner
			#pragma warning(suppress:26800)//using a moved object
			Assert::IsFalse(Bptr.is_owner());
			Assert::IsTrue(Dptr.is_owner());
			Assert::IsTrue( Dptr == Bptr );

			WS::auto_ptr<C> Cptr{ WS::make_auto_ptr<C>() };
			//Dptr = std::move(Cptr);//error C2338: WS::auto_ptr<struct `public: void __thiscall BasisUnitTests::UT_auto_ptr::UT_auto_ptr_owner_cast(void)'::`2'::D>::operator = pointer sind nicht zuweisbar
			//WS::auto_ptr<D> Dptr3 = std::move(Cptr);//error C2338: static_assert failed: 'WS::auto_ptr<struct `public: void __thiscall autoptr::autoptr::UT_auto_ptr_owner_cast(void)'::`2'::D>::auto_ptr pointer sind nicht zuweisbar'


			Bptr = WS::make_auto_ptr<B>() ;
			Assert::IsTrue(Bptr.is_owner());
			WS::auto_ptr<D> Dptr2 = std::move(Bptr);//Bptr ist B nicht D, kann also nicht dem Dptr zugewiesen werden. Bptr bleibt owner
			Assert::IsTrue(Bptr.is_owner());
			Assert::IsTrue(Dptr2==nullptr);

			Bptr = WS::make_auto_ptr<B>() ;
			Assert::IsTrue(Bptr.is_owner());
			Dptr = std::move(Bptr);
			Assert::IsTrue(Bptr.is_owner());
			Assert::IsTrue(Dptr2==nullptr);

			auto Bptr2 = std::move(Bptr);
			Assert::IsFalse(Bptr.is_owner());
			Assert::IsTrue(Bptr2.is_owner());

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
				//{
				//	auto  & neu{vw.push_back( new int{2} )};//error C2440: 'initializing': cannot convert from '_Ty' to '_Ty &'
				//	Assert::IsTrue( neu.is_owner()==false);
				//	Assert::IsTrue( *neu==2 );
				//}
				{
					auto  && neu{vw.push_back( new int{3} )};
					Assert::IsFalse( neu.is_owner());
					Assert::IsTrue( *neu==3 );
				}
			}

			WS::auto_ptr_vw<int> vw;
			{
				auto && neu{vw.push_back( new int{1} )};
				Assert::IsFalse( neu.is_owner());
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

					WS::auto_ptr_vw<ctor_counter,std::deque> vw;
					(void)vw.push_back( new ctor_counter{} );
					Assert::IsTrue(init_counter+1==ctor_counter::counter);
					(void)vw.erase( nullptr );
					(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
					(void)vw[0];
				}
				Assert::IsTrue(init_counter==ctor_counter::counter, L"memory leak detected");
			}		
			{
				WS::auto_ptr_vw<int,std::deque> vw;

				(void)vw.push_back( new int{5} );
				(void)vw.erase( nullptr );
				(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
				(void)vw[0];
			}
			{
				WS::auto_ptr_vw<int,std::list> vw;//list bedingt brauchbar
				(void)vw.push_back( new int{5} );
				(void)vw.erase( nullptr );
				(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
				//(void)vw[0];//list unterst�tzt operator[] nicht
			}
			{
				WS::auto_ptr_vw<int,std::forward_list> vw;//forward_list unbrauchbar
				//(void)vw.push_back( new int{5} );//forward_list unterst�tz emplace_back nicht, nicht einmal push_back
				//(void)vw.erase( nullptr );
				(void)vw.replace( nullptr, (decltype(vw)::pointer_t)nullptr );
				//(void)vw[0];
			}
		}
	};
}

