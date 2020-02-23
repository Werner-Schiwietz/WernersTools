#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


#include "..\..\experimental\headeronly\tree.h"
#include <set>

#include <algorithm>
#pragma push_macro("max")
#undef max
#pragma push_macro("min")
#undef min

#include <vector>
#include <functional>
#include <cstdlib>

#include <cassert>
#include <crtdbg.h>

#pragma warning(push,4)

#ifndef ASSERT
#	if !defined(_ASSERT) 
#		define ASSERT(x) assert(x)
#	else
#		define ASSERT(x) _ASSERT(x)
#	endif
#endif

//auto x(int v)
//{
//	return gsl::narrow<short>(v);
//}

namespace UTTree
{
	TEST_CLASS(UTTree)
	{
		//TEST_CLASS_ATTRIBUTE(oldwarn,0)
		//TEST_CLASS_ATTRIBUTE(oldassert,0);
		//TEST_CLASS_ATTRIBUTE(olderror,0);
	public:
		TEST_CLASS_INITIALIZE(init)
		{
			//_ASSERT ohne UI, macht MS-Testumgebung bei run automatisch. aber nur mit _ASSERT nicht bei assert
			//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
			//_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
			//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);

			//andere einstellungen
			//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
			//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
			//_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
			//_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
			//_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
			//_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
		}
		TEST_CLASS_CLEANUP(clean)
		{
			//_CrtSetReportMode(_CRT_WARN, oldwarn );
			//_CrtSetReportMode(_CRT_ERROR, olderror );
			//_CrtSetReportMode(_CRT_ASSERT, oldassert );
		}
		TEST_METHOD(add_some)
		{
			WS::tree_t<int> tree;
			std::set<int> set;

			auto add =[&]( int v )->void{ tree.add( v ); set.insert( v ); };
			auto check =[&]()->bool
			{
				auto iter_set = set.begin();
				//for( auto & v : tree ) {
				for( auto i=tree.begin(); i!=tree.end(); ++i) 
				{ 
					auto & v = *i;
					if( *iter_set++ != v )
						return false;
				}
				return iter_set==set.end();
			};

			for( auto & v : {50,30,100,30,70,80,90,110,120,130,140,150,73,76,78,20,10,15} )
			{
				add( v );
				Assert::IsTrue( check() );
				WS::Tree_Helper::trace(tree);
				ASSERT(tree.is_balanced());
				Assert::IsTrue(  tree.is_balanced(), L"not balanced" );
			}
			set.clear();
			tree.root.reset();
			for( auto & v : {50,30,100,30,70,80,90,110,120,130,140,150,73,76,78,20,10,15} )
			{
				add( v );
				Assert::IsTrue( check() );
				WS::Tree_Helper::trace(tree);
				ASSERT(tree.is_balanced());
				Assert::IsTrue(  tree.is_balanced(), L"not balanced" );
			}
		}
		TEST_METHOD(iterator)
		{
			struct compare_gth
			{ 
				constexpr bool operator()(int const & l, int const & r) const 
				{
					return (r < l);
				} 
			};

			WS::tree_t<int> tree;
			//WS::tree_t<int, compare_gth> rtree{};
			WS::tree_t<int, compare_gth> rtree{compare_gth{}};
			Assert::IsTrue( tree.comparator( 1, 2) );
			Assert::IsFalse( rtree.comparator( 1, 2) );


			std::vector<int> vec = {50,30,100,30,70,80,90,110,120,130,140,150,73,76,78,20,10,15};

			std::set<int> set( vec.begin(), vec.end() ); 
			for( auto & v : vec )
			{
				tree.add( v );
				rtree.add( v );
			}

			auto iter = tree.begin();
			auto riter = rtree.rbegin();

			for( ;iter != tree.end(); ++iter, ++riter)
			{
				Assert::IsTrue( *iter == *riter );
			}
		}
		TEST_METHOD(riterator)
		{
			WS::tree_t<int> tree;
			std::vector<int> vec = {50,30,100,30,70,80,90,110,120,130,140,150,73,76,78,20,10,15};
			std::set<int> set( vec.begin(), vec.end() ); 
			for( auto & v : vec )
				tree.add( v );

			int last_value = *tree.rbegin() + 1;
			for( auto riter = tree.rbegin(); riter != tree.rend(); ++riter )
			{
				Assert::IsTrue( *riter < last_value );
				last_value = *riter;
			}
		}

		template<typename node_t> bool check_node ( std::unique_ptr<node_t> const & node )
		{
			if( node )
			{
				if( node->left )
				{
					if( node->left->parent != node.get() )
						return false;
					if( check_node(node->left)==false )
						return false;
				}
				if( node->right )
				{
					if( node->right->parent != node.get() )
						return false;
					if( check_node(node->right)==false )
						return false;
				}
			}
			return true;
		};
		bool check_struktur( WS::tree_t<int> const & tree )
		{
			if( tree.root->parent ) 
				return false;

			return check_node( tree.root );
		}
		TEST_METHOD(move_rightleft)
		{
			WS::tree_t<int> tree;
			std::set<int> set;

			auto constexpr left = WS::tree_t<int>::node_t::branch_id::left;
			auto constexpr right = WS::tree_t<int>::node_t::branch_id::right;


			auto add =[&]( int v )->void{ tree._add( v ); set.insert( v ); };
			auto check =[&]()->bool
			{
				if( check_struktur(tree)==false )
					return false;

				auto iter_set = set.begin();
				//for( auto & v : tree ) {
				for( auto i=tree.begin(); i!=tree.end(); ++i) 
				{ 
					auto & v = *i;
					if( *iter_set++ != v )
						return false;
				}
				return iter_set==set.end();
			};


			add( 10 );
			add( 20 );

			WS::tree_t<int>::node_t::_rotatefrom_<right>(tree.root);
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_rotatefrom_<left>(tree.root);
			Assert::IsTrue( check() );

			add( 15 );
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_rotatefrom_<right>(tree.root);
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_rotatefrom_<left>(tree.root);
			Assert::IsTrue( check() );

			WS::tree_t<int>::node_t::_rotatefrom_<left>(tree.root->right);
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_rotatefrom_<right>(tree.root->right);
			Assert::IsTrue( check() );
		}

		TEST_METHOD(trace_tree)
		{
			WS::tree_t<int> tree;

			WS::Tree_Helper::trace(tree);

			tree.add(5);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			tree.add(10);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			tree.add(3);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			tree.add(7);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			tree._add(8);
			WS::Tree_Helper::trace(tree);
			tree.add(8);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			tree._add(9);
			WS::Tree_Helper::trace(tree);
			tree.add(9);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			tree._add(15);
			WS::Tree_Helper::trace(tree);
			tree.add(15);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			tree._add(20);
			WS::Tree_Helper::trace(tree);
			tree.add(20);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());
			
		}
		TEST_METHOD(trace_tree2)
		{
			WS::tree_t<int> tree;

			WS::Tree_Helper::trace(tree);

			tree.add(1);
			tree.add(2);
			Assert::IsTrue(tree.is_balanced());
			tree._add(3);
			WS::Tree_Helper::trace(tree);
			tree.add(3);
			Assert::IsTrue(tree.is_balanced());
			WS::Tree_Helper::trace(tree);
			tree.add(4);
			Assert::IsTrue(tree.is_balanced());
			WS::Tree_Helper::trace(tree);
			tree.add(5);
			Assert::IsTrue(tree.is_balanced());
			WS::Tree_Helper::trace(tree);
			tree._add(6);
			WS::Tree_Helper::trace(tree);
			tree.add(6);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());
		}

		TEST_METHOD(remove_tree_1)
		{
			WS::tree_t<int> tree;

			Assert::IsFalse( tree.remove( 5 ) );
			tree.add(5);
			Assert::IsTrue( tree.remove( 5 ) );

			tree.add(4);
			tree.add(5);
			Assert::IsTrue( tree.remove( 5 ) );
			Assert::IsTrue( tree.remove( 4 ) );
			tree.add(4);
			tree.add(5);
			Assert::IsTrue( tree.remove( 4 ) );
			Assert::IsTrue(tree.is_balanced());
			Assert::IsTrue( tree.remove( 5 ) );

		}
		TEST_METHOD(remove_tree_2)
		{
			WS::tree_t<int> tree;

			tree.add(1);
			tree.add(2);
			tree.add(3);
			tree.add(4);
			tree.add(5);
			tree.add(6);
			tree.add(7);
			tree.add(8);
			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());

			while(tree.root)
			{
				tree.remove(tree.root->value);
				WS::Tree_Helper::trace(tree);
				Assert::IsTrue(tree.is_balanced());
			}
		}
		TEST_METHOD(remove_tree_3)
		{
			WS::tree_t<int> tree;

			std::vector<int> values={1,2,3,4,5,6,7,8}; 
			for( auto const & v : values )
				tree.add(v);

			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());
			for( auto const & v : values )
			{
				Assert::IsTrue( tree.remove(v) );
				WS::Tree_Helper::trace(tree);
				Assert::IsTrue(tree.is_balanced());
			}

			WS::Tree_Helper::trace(tree);
			Assert::IsTrue(tree.is_balanced());
		}
	};
}
