#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <sstream>
#include <initializer_list>
#include <memory>

#include <vector>

#include "..\..\headeronly\Auto_Ptr.h"

enum Enum { v1, v2, v3 };

constexpr std::initializer_list<Enum> GetList() { return { v2,v1 }; }


namespace autoptr
{
	TEST_CLASS(autoptr)
	{
	public:
		
		TEST_METHOD(TestMethod_initializer_list)
		{
			std::stringstream cout;
			cout << "expect  " << Enum::v2 << ' ' << Enum::v1 << std::endl;
			cout << "getting ";
			auto x = GetList();
			for( auto iter = x.begin(); iter!=x.end(); ++iter )
				cout << *iter << ' ';
			for (auto e : x)
				cout << e << ' ';
			cout << std::endl;
			Logger::WriteMessage( cout.str().c_str() );
		}
		TEST_METHOD(mem_leak)
		{
			new int{5};
			new std::vector<int>{1,2,3};
		}
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
		TEST_METHOD(auto_ptr__copy_ownertranssfer)
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
				//virtual ~B() {}
				int foo() { return 2; }
			};
			struct AB : A , B
			{
				int foo() { return 3; }
			};
			WP::auto_ptr<AB> ab = std::make_unique<AB>();
			WP::auto_ptr<A> a = ab;
			WP::auto_ptr<B> b = ab;
			Assert::IsTrue(ab->foo()== 3);
			Assert::IsTrue(a->foo() == 1);
			Assert::IsTrue(b->foo() == 2);

			auto pa = a.get();
			auto pb = b.get();
			Assert::IsTrue( (void const *)pa != (void const *)pb );
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
