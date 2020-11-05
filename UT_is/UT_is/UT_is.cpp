#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#include "..\..\headeronly\is_.h"
#include <memory>
#include <string>
#include <vector>
#include <set>

namespace UTis
{
	TEST_CLASS(UTis)
	{
	public:
		TEST_METHOD(UT_is_shared_ptr_const)
		{
			std::shared_ptr<int> p4 {new int{4}};
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(p4)> );
			std::shared_ptr<int> & rp4 = p4;
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(rp4)> );
			std::shared_ptr<int> const cp4 = p4;
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(cp4)> );
			std::shared_ptr<int> const &  crp4 = cp4;
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(crp4)> );

			auto ptr = std::shared_ptr<int> {new int{5}};
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(std::move(ptr))> );
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(std::shared_ptr<int>{})> );
			auto const constptr = std::shared_ptr<int> {new int{6}};
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(std::move(constptr))> );
		}
		TEST_METHOD(UT_is_shared_ptr_volatile)
		{
			std::shared_ptr<int> volatile p4  {new int{4}};
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(p4)> );
			auto & rp4 = p4;
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(rp4)> );

			auto volatile ptr = std::shared_ptr<int> {new int{5}};
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(std::move(ptr))> );
		}
		TEST_METHOD(UT_is_shared_ptr_const_volatile)
		{
			std::shared_ptr<int> const volatile p4  {new int{4}};
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(p4)> );
			auto & rp4 = p4;
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(rp4)> );

			auto const volatile ptr = std::shared_ptr<int> {new int{5}};
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(std::move(ptr))> );
		}
		TEST_METHOD(UT_is_shared_ptr)
		{
			auto deleter = [](int*& p)
			{
				delete p;
				p=nullptr;
			};
			std::shared_ptr<int> p4 {new int{4}};
			std::shared_ptr<int> p5 {new int{5}, deleter, std::allocator<int>()};
			std::unique_ptr<int,decltype(deleter)> p6 {new int{6}, deleter};
			//std::unique_ptr<int,decltype(deleter)> p7 {new int{7}};//ist in C:\Users\SchiwietzW\Source\Repos\Werner-Schiwietz\Experimental\UT_Allerei\UT_Allerei\UT_is_shared_ptr.cpp übersetzbar??


			Assert::IsFalse( WS::is_shared_ptr<std::string>::value );
			Assert::IsFalse( WS::is_shared_ptr<std::unique_ptr<int>>::value );
			Assert::IsFalse( WS::is_shared_ptr<int>::value );
			Assert::IsFalse( WS::is_shared_ptr_v<decltype(p6)> );
			//Assert::IsFalse( WS::is_shared_ptr<decltype(p7)>::value );

			Assert::IsTrue( WS::is_shared_ptr<std::shared_ptr<int>>::value );
			Assert::IsTrue( WS::is_shared_ptr<decltype(p5)>::value );
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(p5)> );
			Assert::IsTrue( WS::is_shared_ptr_v<decltype(p4)> );
		}
		TEST_METHOD(UT_is_vector)
		{
			Assert::IsFalse( WS::is_std_vector_v<std::set<std::shared_ptr<std::string>>> );
			

			Assert::IsTrue( WS::is_std_vector_v<std::vector<std::shared_ptr<std::string>>> );
			auto v1 = std::vector<int>{};
			auto const v2 = v1;
			auto volatile v3 = v1;
			auto const volatile v4 = v1;
			auto const & v5 = v1;
			auto volatile & v6 = v1;
			auto const volatile & v7 = v1;
			Assert::IsTrue( WS::is_std_vector_v<decltype(v1)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(v2)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(v3)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(v4)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(v5)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(v6)> );
			Assert::IsTrue( WS::is_std_vector_v<decltype(v7)> );
		}
		TEST_METHOD(UT_is_set)
		{
			Assert::IsFalse( WS::is_std_set_v<std::vector<std::shared_ptr<std::string>>> );

			struct less
			{
				bool operator()(std::shared_ptr<std::string> const & l, std::shared_ptr<std::string> const & r) const {return l<r;}
			};
			Assert::IsTrue( WS::is_std_set_v<std::set<std::shared_ptr<std::string>,less>> );
			Assert::IsTrue( WS::is_std_set_v<std::set<std::shared_ptr<std::string>>> );
		}
	};
}
