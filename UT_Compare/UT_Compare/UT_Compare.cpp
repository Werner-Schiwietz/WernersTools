#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\compare.h"

#include <vector>
#include <algorithm>

#pragma warning(push,4)

namespace BasisUnitTests
{
    //testet die windows-api Base64 Funktionen gegen eigene
    //hintergrund: windows hatte iregendwann einmal die funktionsweise geaendert. unser programm hatte sich danach anders verhalten
    TEST_CLASS(UT_Compare)
    {
    public:
        TEST_METHOD(Compare_Double_WithoutNaN_Works)
        {
            Assert::AreEqual(-1, WS::compare(1.0, 2.0));
            Assert::AreEqual(1, WS::compare(2.0, 1.0));
            Assert::AreEqual(0, WS::compare(2.0, 2.0));
        }

        TEST_METHOD(Compare_Double_PositiveAndNegativeZero_AreEqual)
        {
            Assert::AreEqual(0, WS::compare(-0.0, 0.0));
            Assert::AreEqual(0, WS::compare(0.0, -0.0));
        }

        TEST_METHOD(Compare_Double_NaN_IsGreaterThanNormalValue)
        {
            constexpr double nan = std::numeric_limits<double>::quiet_NaN();

            Assert::AreEqual(1, WS::compare(nan, 1.0));
            Assert::AreEqual(-1, WS::compare(1.0, nan));
        }

        TEST_METHOD(Compare_Double_TwoNaNs_AreEqual)
        {
            constexpr double nan1 = std::numeric_limits<double>::quiet_NaN();
            constexpr double nan2 = std::numeric_limits<double>::quiet_NaN();

            Assert::AreEqual(0, WS::compare(nan1, nan2));
        }

        TEST_METHOD(Less_Double_WithNaN_Works)
        {
            constexpr double nan = std::numeric_limits<double>::quiet_NaN();

            Assert::IsTrue(WS::less(1.0, nan));
            Assert::IsFalse(WS::less(nan, 1.0));
            Assert::IsFalse(WS::less(nan, nan));
        }

        TEST_METHOD(Sort_Double_WithNaN_PutsNaNAtEnd)
        {
            constexpr double nan = std::numeric_limits<double>::quiet_NaN();

            std::vector<double> values{ 3.0, nan, 1.0, 2.0, nan };
            std::sort(values.begin(), values.end(), WS::less);

            Assert::AreEqual(1.0, values[0]);
            Assert::AreEqual(2.0, values[1]);
            Assert::AreEqual(3.0, values[2]);
            Assert::IsTrue(std::isnan(values[3]));
            Assert::IsTrue(std::isnan(values[4]));
        }

        TEST_METHOD(Sort_Double_ZeroesRemainEquivalent)
        {
            std::vector<double> values{ 1.0, -0.0, 0.0, -1.0 };
            std::sort(values.begin(), values.end(), WS::less);

            Assert::AreEqual(-1.0, values[0]);
            Assert::AreEqual(0.0, values[1]);
            Assert::AreEqual(0.0, values[2]);
            Assert::AreEqual(1.0, values[3]);
        }
        TEST_METHOD(Compare_Int_Works)
        {
            Assert::AreEqual(-1, WS::compare(1, 2));
            Assert::AreEqual(1, WS::compare(2, 1));
            Assert::AreEqual(0, WS::compare(2, 2));
        }
        TEST_METHOD(Compare_String_Works)
        {
            Assert::AreEqual(-1, WS::compare(std::string("abc"), std::string("abd")));
            Assert::AreEqual(1, WS::compare(std::string("abd"), std::string("abc")));
            Assert::AreEqual(0, WS::compare(std::string("abc"), std::string("abc")));
        }
        TEST_METHOD(Less_Int_Works)
        {
            Assert::IsTrue(WS::less(1, 2));
            Assert::IsFalse(WS::less(2, 1));
            Assert::IsFalse(WS::less(2, 2));
        }
        TEST_METHOD(Less_String_Works)
        {
            Assert::IsTrue(WS::less(std::string("abc"), std::string("abd")));
            Assert::IsFalse(WS::less(std::string("abd"), std::string("abc")));
            Assert::IsFalse(WS::less(std::string("abc"), std::string("abc")));
        }

    };
}