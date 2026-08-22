#include "pch.h"
#include "CppUnitTest.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import UTF;


namespace UTUTF
{
	TEST_CLASS( UTFTests )
	{
	public:

		TEST_METHOD( TestBasicASCIIConversion )
		{
			char8_t const* input = u8"A";

			UTF32Result res = UTF8toUTF32( input );

			Assert::IsTrue( res.codepoint == U'A' );
			Assert::IsTrue( res.units_consumed == 1 );
			Assert::IsTrue( res.error == UTFError::None );
			Assert::IsTrue( *input == u8'\0' );
		}

		TEST_METHOD( TestTwoByteConversion )
		{
			char8_t const* input = u8"ä";

			UTF32Result res = UTF8toUTF32( input );

			Assert::IsTrue( res.codepoint == U'ä' );
			Assert::IsTrue( res.units_consumed == 2 );
			Assert::IsTrue( res.error == UTFError::None );
			Assert::IsTrue( *input == u8'\0' );
		}

		TEST_METHOD( TestThreeByteConversion ) 
		{
			char8_t const *input = u8"€";

			UTF32Result res = UTF8toUTF32( input );

			Assert::IsTrue( res.codepoint == U'€' );
			Assert::IsTrue( res.units_consumed == 3 );
			Assert::IsTrue( res.error == UTFError::None );
			Assert::IsTrue( *input == u8'\0' );
		}

		TEST_METHOD( TestInvalidSequence ) 
		{
			char8_t bad[] = { (char8_t)0xC0, (char8_t)0xAF, (char8_t)0x00 };
			char8_t const *input = bad;

			UTF32Result res = UTF8toUTF32( input );

			Assert::IsTrue( res.error == UTFError::OverlongEncoding );
			Assert::IsTrue( res.units_consumed == 0 );
			Assert::IsTrue( *input == (char8_t)0xC0 );
			Assert::IsTrue( input == bad );
		}

		TEST_METHOD( TestUTF16_BMP )
		{
			char16_t const *input = u"A";

			UTF32Result res = UTF16toUTF32( input );

			Assert::IsTrue( res.codepoint == U'A' );
			Assert::IsTrue( res.units_consumed == 1 );
			Assert::IsTrue( res.error == UTFError::None );
			Assert::IsTrue( *input == u'\0' );
		}

		TEST_METHOD( TestUTF16_SurrogatePair )
		{
			char16_t s[] = { (char16_t)0xD83D, (char16_t)0xDE00, (char16_t)0x0000 };
			char16_t const *input = s;

			UTF32Result res = UTF16toUTF32( input );

			Assert::IsTrue( res.codepoint == 0x1F600 );
			Assert::IsTrue( res.units_consumed == 2 );
			Assert::IsTrue( res.error == UTFError::None );
			Assert::IsTrue( *input == u'\0' );
		}

		TEST_METHOD( TestUTF16_UnpairedLow )
		{
			char16_t bad[] = { (char16_t)0xDC00, (char16_t)0x0000 };
			char16_t const *input = bad;

			UTF32Result res = UTF16toUTF32( input );

			Assert::IsTrue( res.error == UTFError::InvalidSequence );
			Assert::IsTrue( res.units_consumed == 0 );
			Assert::IsTrue( input == bad );
		}

		TEST_METHOD( TestConvert_Copy_Char8 )
		{
			char8_t const* input = u8"CopyTest";

			auto out = Convert<char8_t>( input );

			Assert::IsTrue( out == std::basic_string<char8_t>{ u8"CopyTest" } );
		}

		TEST_METHOD( TestConvert_Copy_Char16 )
		{
			char16_t const* input = u"Copy16";

			auto out = Convert<char16_t>( input );

			Assert::IsTrue( out == std::basic_string<char16_t>{ u"Copy16" } );
		}

		TEST_METHOD( TestConvert_Copy_Char32 )
		{
			char32_t const* input = U"Copy32";

			auto out = Convert<char32_t>( input );

			Assert::IsTrue( out == std::basic_string<char32_t>{ U"Copy32" } );
		}

		TEST_METHOD( TestConvert_Copy_WChar )
		{
			wchar_t const* input = L"CopyW";

			auto out = Convert<wchar_t>( input );

			Assert::IsTrue( out == std::basic_string<wchar_t>{ L"CopyW" } );
		}

		TEST_METHOD( TestConvert_Char16ToChar8 )
		{
			char16_t const* input = u"ä";

			auto out = Convert<char8_t>( input );

			Assert::IsTrue( out == std::basic_string<char8_t>{ u8"ä" } );
		}

	};
}
