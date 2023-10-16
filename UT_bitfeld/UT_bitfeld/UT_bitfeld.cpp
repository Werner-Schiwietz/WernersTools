#include "pch.h"
#include "CppUnitTest.h"

#pragma warning(push,4)
#include "..\..\headeronly\bitfeld.h"
#include "..\..\headeronly\noimplicitcast.h"

#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace
{
	enum class bits : _int64
	{
		kein	= 0x00000000000000000000,
		b1		= 0x1,
		b2		= b1<<1,
		b3		= b2<<1,
		b4		= b3<<1,
		b5		= b4<<1,
		b6		= b5<<1,
		b7		= b6<<1,
		b33		= 0x1'0000'0000
	};
	bits operator| ( bits l, bits r)
	{
		return bits{ WS::to_underlying(l) | WS::to_underlying(r) };
	}
	bits operator& ( bits l, bits r)
	{
		return bits{ WS::to_underlying(l) & WS::to_underlying(r) };
	}

	enum class egal : unsigned _int16 {};
	inline egal operator|( egal l, egal r )
	{
		return egal( WS::to_underlying(l) | WS::to_underlying(r) );
	}

	inline egal operator&( egal l, egal r )
	{
		return egal( WS::to_underlying(l) & WS::to_underlying(r) );
	}

	inline egal operator~( egal v )
	{
		return egal( ~WS::to_underlying(v) );
	}
}


namespace bitfeld
{
	TEST_CLASS(bitfeld)
	{
	public:
		TEST_METHOD(begin_end)
		{
			auto x = WS::make_bitfeld((unsigned long)(2+8+16+0x8000'0000));
			auto iter = x.begin();
			auto xe = x.end();

			Assert::IsTrue(2 == *iter++);
			Assert::IsTrue(8 == *iter++);
			Assert::IsTrue(16 == *iter++);
			Assert::IsTrue(0x8000'0000 == *iter++);
			Assert::IsTrue(iter == xe);


			int i=0;
			for( decltype(auto) v : WS::bitfeld<int>(1+4+16) )
			{
				Assert::IsTrue(i!=0 || v==1);
				Assert::IsTrue(i!=1 || v==4);
				Assert::IsTrue(i!=2 || v==16);
				Assert::IsTrue(i<3);
				++i;
			}
		}
		TEST_METHOD(multibit)
		{
			Assert::IsFalse( WS::bitfeld<int>{}.is_multibit() );
			Assert::IsFalse( WS::bitfeld<int>{4}.is_multibit() );
			Assert::IsTrue( WS::bitfeld<int>{4|32}.is_multibit() );

			auto bits = WS::bitfeld<int>(1+4+16);
			for( auto iter = bits.begin(); iter!=bits.end(); ++iter )
			{
				Assert::IsTrue((*iter)==16 || iter.is_multibit());
				Assert::IsTrue((*iter)!=16 || iter.is_multibit()==false);
			}
		}
		TEST_METHOD(falsche_iteration)
		{
			unsigned long arr[] = {2,8,16,0x8000'0000,0};
			unsigned long * parr = arr;

			for( auto bit = WS::make_bitfeld((unsigned long)(2+8+16+0x8000'0000)).begin();
				 bit;
				 ++bit,++parr)
			{
				//bit(auto cast nach bitfeld_t) bit.getFirstBit() und *bit machen das selbe
				Assert::IsTrue( *parr == bit );
				Assert::IsTrue( *parr == bit.getFirstBit() );
				Assert::IsTrue( *parr == *bit );
			}
			Assert::IsTrue( *parr == 0 );
		}
		TEST_METHOD(range_based_for)
		{
			unsigned long arr[] = {2,8,16,0x8000'0000};
			unsigned long * parr = arr;

			for( auto bit :  WS::make_bitfeld((unsigned long)(2+8+16+0x8000'0000)) )
			{
				Assert::IsTrue( *parr++ == bit );
			}
		}
		TEST_METHOD(split_bits)
		{
			using bitfeld_t = char;
			bitfeld_t bits{1|4|8|'\x80'};
			auto splited = WS::split_bits(bits);

			Assert::IsTrue( splited.getValue() == bits );
			Assert::IsTrue( splited.getFirstBit() == 1 );
			Assert::IsTrue( splited.getRest().getValue() == bitfeld_t{4|8|'\x80'} );

			splited = splited.getRest();
			Assert::IsTrue( splited.getFirstBit() == 4 );
			Assert::IsTrue( splited.getRest().getValue() == bitfeld_t{8|'\x80'} );

			splited = splited.getRest();
			Assert::IsTrue( splited.getFirstBit() == 8 );
			Assert::IsTrue( splited.getRest().getValue() == bitfeld_t{'\x80'} );

			splited = splited.getRest();
			Assert::IsTrue( splited.getFirstBit() == bitfeld_t{'\x80'} );
			Assert::IsFalse( splited.getFirstBit() == 0x80 ); //getFirstBit liefert -128 0x80 ist (int)128

			Assert::IsTrue( splited.getRest().getValue() == bitfeld_t{0} );
		}
		TEST_METHOD(bitfeld_mit_enum)
		{
			std::vector<bits> vec{bits::b1 , bits::b3 , bits::b33};
			auto ebits = bits::b1 | bits::b3 | bits::b33;

			auto iter = vec.begin();
			auto splited = WS::split_bits(ebits);
			Assert::IsTrue(splited.first_bit == (*iter++) );
			splited = splited.rest;
			Assert::IsTrue(splited.first_bit == (*iter++) );
			splited = splited.rest;
			Assert::IsTrue(splited.first_bit == (*iter++) );
			splited = splited.rest;
			Assert::IsTrue( splited.first_bit == bits::kein );
			Assert::IsTrue( iter == vec.end() );

			iter = vec.begin();
			for( auto bit : WS::make_bitfeld(ebits) )
			{
				Assert::IsTrue(bit == (*iter++) );
			}
			Assert::IsTrue( iter == vec.end() );
		}
		TEST_METHOD(bitfeld_mit_unnamed_enum)
		{
			enum {} unnamed_standard{};
			std::vector<decltype(unnamed_standard)> vec{  decltype(unnamed_standard)(1)
				, decltype(unnamed_standard)(2)
				, decltype(unnamed_standard)(8)
				, decltype(unnamed_standard)(32) };

			for( auto v : vec )
				unnamed_standard = decltype(unnamed_standard)(unnamed_standard | v);

			auto iter = vec.begin();
			for( auto v : WS::make_bitfeld(unnamed_standard) )
			{
				Assert::IsTrue( v == *iter++ );
			}
			Assert::IsTrue( iter==vec.end() );

		}
		TEST_METHOD(bitfeld_mit_unsigned_short_class_enum)
		{
			egal named_short{};
			std::vector<decltype(named_short)> vec{   decltype(named_short)(1)
				, decltype(named_short)(2)
				, decltype(named_short)(8)
				, decltype(named_short)(32) };

			for( auto v : vec )
				named_short = decltype(named_short)(named_short | v);

			auto iter = vec.begin();
			for( auto v : WS::make_bitfeld(named_short) )
			{
				Assert::IsTrue( v == *iter++ );
			}
			Assert::IsTrue( iter==vec.end() );
		}
		TEST_METHOD(bitfeld_mit_strongtype)
		{
			TYPEDEF(MyTyp,int) values{1|4|16};//typ MyTyp definieren, underlying-typ ist int
			int index = 0;
			for( decltype(auto) bit : WS::bitfeld{values} )
			{
				switch(index++)
				{
				case 0:
					Assert::IsTrue(bit.to_underlying()==1);
					break;
				case 1:
					Assert::IsTrue(bit.to_underlying()==4);
					break;
				case 2:
					Assert::IsTrue(bit.to_underlying()==16);
					break;
				default:
					Assert::Fail();
					break;
				}
			}
		}
		TEST_METHOD(union_bitfelder)
		{
			TYPEDEF(X1,short);

			Assert::IsTrue( WS::bits::Union(X1{5},X1{12}) == X1{13} );
			Assert::IsTrue(	WS::bits::Union( 1|8, 2|4 ) == (1|2|4|8) );
			Assert::IsTrue(	WS::bits::Union( 1, 0 ) == 1);
		}
		TEST_METHOD(intersect_bitfelder)
		{
			TYPEDEF(X1,short);
			Assert::IsTrue(	WS::bits::Intersect( X1{5}, X1{12} ) == X1{4});
			Assert::IsTrue(	WS::bits::Intersect( 1|8, 2|4|8 ) == (8) );
			Assert::IsTrue(	WS::bits::Intersect( 1|2|4, 8 ) == 0);
		}
		TEST_METHOD(invert_bitfeld)
		{
			TYPEDEF(X1,short);
			Assert::IsTrue(	WS::bits::Invert( X1{5} ) == X1{~5});
			Assert::IsTrue(	WS::bits::Invert( 1|8 ) == ~(1|8) );
		}
		TEST_METHOD(cutoff_bitfeld)
		{
			TYPEDEF(X1,short);
			Assert::IsTrue(	WS::bits::Without( X1{0xffffi16}, X1{0xc0} ) == X1{0xff3fi16});
		}
	};
}
