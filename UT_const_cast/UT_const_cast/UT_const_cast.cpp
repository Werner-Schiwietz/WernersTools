#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#pragma warning(push,4)
#include "..\..\headeronly\const_cast.h"

#include <memory>


namespace UT_allgemein
{
	TEST_CLASS(UT_const_cast)
	{
	public:
		TEST_METHOD(const_ref_zu_ref)
		{
			int i=5;
			auto const & cri = i;
			static_assert(std::is_reference_v<decltype(cri)>);
			static_assert(std::is_const_v<std::remove_reference_t<decltype(cri)>>);

			{
				decltype(auto) casted = WS::notconst_cast(cri);
				static_assert(std::is_reference_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_reference_t<decltype(casted)>> == false);

				casted = 6;
				Assert::IsTrue(i==6);
			}
			{
				decltype(auto) casted = WS::toggleconst_cast(cri);
				static_assert(std::is_reference_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_reference_t<decltype(casted)>> == false);

				casted = 7;
				Assert::IsTrue(i==7);
			}
		}
		TEST_METHOD(ref_zu_const_ref)
		{
			int i=5;
			{
				decltype(auto) casted = WS::toconst_cast(i);
				static_assert(std::is_reference_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_reference_t<decltype(casted)>> == true);

				Assert::IsTrue( casted == i );
				i = 6;
				Assert::IsTrue( casted == i );
			}
			{
				decltype(auto) casted = WS::toggleconst_cast(i);
				static_assert(std::is_reference_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_reference_t<decltype(casted)>> == true);

				Assert::IsTrue( casted == i );
				i = 7;
				Assert::IsTrue( casted == i );
			}
		}
		TEST_METHOD(const_pointer_zu_pointer)
		{
			int i=5;
			auto const * cpi = &i;
			static_assert(std::is_pointer_v<decltype(cpi)>);
			static_assert(std::is_const_v<std::remove_pointer_t<decltype(cpi)>>);

			{
				decltype(auto) casted = WS::notconst_cast(cpi);
				static_assert(std::is_pointer_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_pointer_t<decltype(casted)>> == false);

				*casted = 6;
				Assert::IsTrue(i==6);
			}
			{
				decltype(auto) casted = WS::toggleconst_cast(cpi);
				static_assert(std::is_pointer_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_pointer_t<decltype(casted)>> == false);

				*casted = 7;
				Assert::IsTrue(i==7);
			}
		}
		TEST_METHOD(pointer_zu_const_pointer)
		{
			int i=5;
			auto * pi = &i;
			static_assert(std::is_pointer_v<decltype(pi)>);
			static_assert(std::is_const_v<std::remove_pointer_t<decltype(pi)>> == false );

			{
				decltype(auto) casted = WS::toconst_cast(pi);
				//decltype(auto) casted2 = WS::notconst_cast(pi);//error C2280
				//decltype(auto) casted2 = WS::toconst_cast(casted);//error C2280
				static_assert(std::is_pointer_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_pointer_t<decltype(casted)>> == true);

				i = 6;
				Assert::IsTrue(*pi==6);
			}
			{
				decltype(auto) casted = WS::toggleconst_cast(pi);
				static_assert(std::is_pointer_v<decltype(casted)>);
				static_assert(std::is_const_v<std::remove_pointer_t<decltype(casted)>> == true);

				i = 7;
				Assert::IsTrue(*pi==7);
			}
		}

		TEST_METHOD( this_cast )
		{
			struct A
			{
				int v=5;

				int const & GetValue() const &
				{
					return v;
				}
				int & GetValue() &
				{
					//cast mit richtungsangabe. gibt compilefehler, wenn keine veränderung nötig ist
					return WS::notconst_cast(WS::toconst_cast(*this).GetValue());
				}
			};
			struct B
			{
				int v=5;

				int const & GetValue() const &
				{
					return v;
				}
				int & GetValue() &
				{
					return WS::toggleconst_cast(WS::toggleconst_cast(this)->GetValue());
				}
			};
			{
				{
					A v;
					Assert::IsTrue( v.GetValue() == 5);
					Assert::IsTrue( WS::toggleconst_cast(v).GetValue() == 5);

					v.GetValue() = 6;
					Assert::IsTrue( v.GetValue() == 6);
					Assert::IsTrue( WS::toggleconst_cast(v).GetValue() == 6);
				}
				{
					A const v;
					Assert::IsTrue( v.GetValue() == 5);
					Assert::IsTrue( WS::toggleconst_cast(v).GetValue() == 5);

					WS::toggleconst_cast(v).GetValue() = 6;
					Assert::IsTrue( v.GetValue() == 6);
					Assert::IsTrue( WS::toggleconst_cast(v).GetValue() == 6);
				}
			}
			{
				{
					B v;
					Assert::IsTrue( v.GetValue() == 5);
					Assert::IsTrue( WS::toconst_cast(v).GetValue() == 5);

					v.GetValue() = 6;
					Assert::IsTrue( v.GetValue() == 6);
					Assert::IsTrue( WS::toconst_cast(v).GetValue() == 6);
				}
				{
					B const v;
					Assert::IsTrue( v.GetValue() == 5);
					Assert::IsTrue( WS::notconst_cast(v).GetValue() == 5);

					WS::notconst_cast(v).GetValue() = 6;
					Assert::IsTrue( v.GetValue() == 6);
					Assert::IsTrue( WS::notconst_cast(v).GetValue() == 6);
				}
			}

		}
		TEST_METHOD( rvalue_cast )
		{
			struct X{int v=5;};
			[[maybe_unused]]decltype(auto) x1 = X{};
			Assert::IsTrue( x1.v == 5 );
			{
				[[maybe_unused]]decltype(auto) x2 = WS::toconst_cast(X{});
				Assert::IsTrue( x2.v == 5 );//könnte evtl. schon kaputt sein, ist es aber nicht
			}
		}

		TEST_METHOD(shared_ptr)
		{
			struct A
			{
				~A(){}
				int value{};

				bool operator==( A const & r ) const{ return this->value==r.value;}
			};



			std::shared_ptr<A> APtr{ new A{5}};

			decltype(auto)  Ptr1 = WS::toconst_cast( APtr );Ptr1;
			Assert::IsTrue( *APtr == *Ptr1 );
			//decltype(auto)  PtrX1 = WS::toconst_cast( Ptr1 );PtrX1; //error C2280: attempting to reference a deleted function
			decltype(auto)  Ptr2 = WS::notconst_cast( Ptr1 );Ptr2;
			//decltype(auto)  PtrX2 = WS::notconst_cast( Ptr2 );PtrX2; //error C2280: attempting to reference a deleted function
			Assert::IsTrue( *APtr == *Ptr2 );
			Assert::IsTrue( *Ptr1 == *Ptr2 );

			decltype(auto)  Ptr3 = WS::toggleconst_cast( Ptr2 );Ptr3;
			decltype(auto)  Ptr4 = WS::toggleconst_cast( Ptr3 );Ptr4;

			static_assert( std::is_same_v<decltype(Ptr3),decltype(Ptr1)>);
			static_assert( std::is_same_v<decltype(Ptr4),decltype(Ptr2)>);
			static_assert( std::is_same_v<decltype(Ptr4),decltype(Ptr1)> == false);
			static_assert( std::is_same_v<decltype(Ptr3),decltype(Ptr2)> == false);

		}
	};
}
