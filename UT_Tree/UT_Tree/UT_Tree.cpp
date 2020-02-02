#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;



#include "..\..\headeronly\SyncReadWrite.h"
#include <set>

#include <algorithm>
#pragma push_macro("max")
#undef max
#pragma push_macro("min")
#undef min

#include <vector>
#include <functional>

namespace WS
{
	template<typename value_type> class tree_t
	{
	public:
		using value_t=value_type;
		class node_t
		{
		public:
			value_t value;

			using parent_t = node_t *;
			parent_t parent = nullptr;
			std::unique_ptr<node_t> left;
			std::unique_ptr<node_t> right;

			node_t( value_t value ) : value(value){}
			node_t( value_t value,parent_t parent ) : value(value), parent(parent){}

			node_t * _most_left() 
			{
				node_t * ret_value = this;
				if( ret_value )
					while( ret_value->left )
						ret_value = ret_value->left.get();
				return ret_value;
			}
			node_t * _right_up()
			{
				node_t * ret_value = this;
				if( ret_value )
				{
					while( ret_value->parent && ret_value->parent->right.get() == ret_value )
						ret_value = ret_value->parent;
					ret_value = ret_value->parent;
				}
				return ret_value;
			}

			auto & _get_child( node_t const * node )
			{
				if(node==nullptr) throw std::invalid_argument( __FUNCTION__ " nullptr not valid");
				if( left.get() == node )return left;
				if( right.get() == node )return right;
				throw std::invalid_argument( __FUNCTION__ " not a child");
			}
			auto _set_child( std::unique_ptr<node_t> && node, std::unique_ptr<node_t> & child )
			{
				auto ret_value = std::move(child);
				if( ret_value ) ret_value->parent = nullptr;
				if( node ) node->parent = this;
				child = std::move(node);
				return ret_value;
			}
			auto _set_left( std::unique_ptr<node_t> && node ){ return _set_child(std::move(node), left); }
			auto  _set_right( std::unique_ptr<node_t> && node ){ return _set_child(std::move(node), right); }

			using _fn_set_t = std::unique_ptr<node_t> (node_t::*)( std::unique_ptr<node_t> && );
			static void	_exchangewith_( std::unique_ptr<node_t> & node_ptr, std::unique_ptr<node_t> & branch, _fn_set_t _set_branch1, _fn_set_t _set_branch2 )
			{
				ASSERT( node_ptr );
				ASSERT( node_ptr->left(branch) || node_ptr->right(branch) );
				auto node = node_ptr.get();
				auto _parent = node->parent;
				auto _1 = (*branch.*_set_branch1)(std::move(node_ptr));
				node_ptr = (*node.*_set_branch2)(std::move(_1));
				node_ptr->parent = _parent;
			}
			static void	_exchangewith_right( std::unique_ptr<node_t> & node_ptr )
			{
				_exchangewith_( node_ptr, node_ptr->right, &_set_left, &_set_right );
			}
			static void	_exchangewith_left( std::unique_ptr<node_t> & node_ptr )
			{
				_exchangewith_( node_ptr, node_ptr->left, &_set_right, &_set_left );
			}

			static node_t& _add( value_t newvalue, std::unique_ptr<node_t>& sheet, parent_t parent )
			{
				if( sheet ) 
					return sheet->add( newvalue );//recursiv is bad here
				else 
					return *(sheet = std::unique_ptr<node_t>{ new node_t( newvalue, parent ) });
			}
			node_t& add( value_t v )
			{
				if( v < this->value )
					return _add( v, this->left, parent_t(this) );
				
				if( this->value < v )
					return _add( v, this->right, parent_t(this) );

				this->value = v;
				return *this;
			}
			node_t * find( value_t v ) const
			{
				if( this->value < v )
					return this->right==nullptr ? nullptr : this->right->find( v );

				if( v < this->value )
					return this->left==nullptr ? nullptr : this->left->find( v );

				return const_cast<node_t*>(this);
			}
			//bool remove( value_t v )
			//{
			//	auto todelete = find( v );
			//	if( todelete )
			//	{
			//		if( this->left==nullptr && this->right==nullptr )
			//		{
			//		}
			//		auto lw = this->left->weight();
			//		auto rw = this->right->weight();
			//
			//		auto lw = this->left->weight();
			//		auto rw = this->right->weight();
			//		if(  lw < rw )
			//		{
			//			;
			//		}
			//	}
			//		return remove( v, this->left );
			//
			//	if( v < this->value )
			//		return remove( v, this->right);
			//}


			size_t weight() const { return this ? (1 + left->weight() + right->weight()) : 0; }
			size_t deep() const { return this ? (1 + std::max(left->deep(), right->deep())) : 0; }
		};
		std::unique_ptr<node_t> root;

		//TODO
		struct iterator
		{
			node_t * node = nullptr;
			~iterator() = default;
			iterator() noexcept = default;
			iterator(iterator const &) noexcept = default;
			iterator(iterator &&) noexcept = default;
			iterator& operator=(iterator const &) & noexcept = default;
			iterator& operator=(iterator &&) & noexcept = default;

			iterator( tree_t & tree )
			{
				if( tree.root )
					this->node = tree.root->_most_left();
			}

			bool operator==( iterator const & r )const { return this->node==r.node; }
			bool operator!=( iterator const & r )const { return !operator==( r ); }

		public:
			value_t const & operator*() const { return this->node->value; }
			value_t const & operator->() const & {return operator*();}
			iterator& operator++()
			{
				if( this->node->right )
					this->node = this->node->right->_most_left();
				else
					this->node = this->node->_right_up();
				return *this;
			}
			iterator operator++( int ) { iterator retvalue{*this}; (void)operator++(); return retvalue; }
		};
		iterator begin() { return iterator{*this}; }
		iterator end() { return iterator{}; }

		node_t &  add( value_t value )
		{
			if( root )
			{
				auto & ret_value = root->add( value );
				//auto wl = root->left->weight();
				//auto wr = root->right->weight();
				//auto dl = root->left->deep();
				//auto dr = root->right->deep();
				return ret_value;
			}
			
			root = std::unique_ptr<node_t>{ new node_t( value ) };
			return *root;
		}
	};
	
	namespace Tree_Helper
	{
		using trace_string_t = std::wstring;
		using trace_char_t = wchar_t;

		template<typename string_t> size_t stringlen( string_t const & );
		template<typename char_t> size_t stringlen( std::basic_string<char_t> string )
		{
			return string.length();
		}

		template<typename string_t, typename char_t> char_t const * c_str( string_t const & );
		template<typename char_t> char_t const * c_str( std::basic_string<char_t> const & string )
		{
			return string.c_str();
		}

		template <typename string_t, typename value_t> string_t tostring( value_t const * value, trace_char_t leftfiller, trace_char_t rightfiller )
		{
			auto ret_value = std::wstring{leftfiller} + std::wstring{L"["};
			if( value ) 
				ret_value += std::to_wstring( *value );

			return ret_value += std::wstring{L"]"} + std::wstring{rightfiller};
		}
		void trace(trace_string_t const & str)
		{
			Logger::WriteMessage(c_str(str));
		}
		template<typename value_t> std::vector<trace_string_t> _tracetree(  tree_t<value_t> const & tree )
		{

			auto & node = tree.root;
			size_t ebene = 0;

			std::vector<trace_string_t> lines;
			auto max_deep = node->deep()-1;
			lines.resize( max_deep+1 );

			
			auto fill_string = []( int chars, trace_char_t ch=' ' )
			{
				if(chars<0)chars=0; 
				//lol C2397
				//return std::wstring{static_cast<size_t>(chars),L' '};//lol C2397
				return std::wstring(static_cast<size_t>(chars),ch);
			};
			auto fill_lines = [&]( size_t bisebene )
			{
				size_t maxlen = 0;
				for( auto index = lines.size(); index --> bisebene; )
				{
					maxlen = std::max( maxlen, stringlen(lines[index]) );
				}

				for( auto index = lines.size(); index --> bisebene; )
				{
					size_t fill_chars = maxlen - stringlen(lines[index]);
					lines[index] += fill_string( fill_chars );
				}
			};
			enum enumFiller{Left=-1,None,Right};
			auto fillchar = [](bool getit)->trace_char_t { return getit ? '-' : ' '; };
			std::function<void(tree_t<value_t>::node_t const *, size_t ebene, enumFiller filler)> fn_node_out;
			fn_node_out = [&](tree_t<value_t>::node_t const * node, size_t ebene, enumFiller filler)
			{
				if( ebene < lines.size() )
				//if( node || 1 )
				{
					fn_node_out( node ? node->left.get() : nullptr, ebene + 1, enumFiller::Right );
					fn_node_out( node ? node->right.get() : nullptr, ebene + 1, enumFiller::Left );
					fill_lines( ebene + 1 );

					auto strvalue = tostring<trace_string_t>( node ? &node->value : nullptr, fillchar(filler==enumFiller::Left), fillchar(filler==enumFiller::Right) );
					int strlen = static_cast<int>(stringlen(strvalue));
					int fillleft = 0;
					int fillright = 0;
					if( ebene < max_deep )
					{
						auto chars = static_cast<int>(stringlen(lines[ebene+1]) - stringlen(lines[ebene]));

						fillleft = (chars - strlen) / 2;
						fillright = chars - strlen - fillleft;
					}
					lines[ebene] += fill_string(fillleft, fillchar(filler==enumFiller::Left)) + strvalue + fill_string(fillright, fillchar(filler==enumFiller::Right) ) ;
				}
				else if( ebene < lines.size() )
				{
					switch(filler)
					{
					case enumFiller::Left:
						lines[ebene] += L"-[] ";
						break;
					case enumFiller::None:
						lines[ebene] += L" [] ";
						break;
					case enumFiller::Right:
						lines[ebene] += L" []-";
						break;
					}
				}
			};

			fn_node_out(node.get(), ebene, enumFiller::None );

			return lines;
		}
		template<typename value_t> void trace(  tree_t<value_t> const & tree )
		{
			auto lines = _tracetree(tree);
			for( auto & line : lines )
				trace( line );
		}
	}
}
#pragma pop_macro("max")
#pragma pop_macro("min")

namespace UTTree
{
	TEST_CLASS(UTTree)
	{
	public:
		
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

			for( auto & v : {5,3,10,3,7,8,9,11,12,13,14,15} )
			{
				add( v );
				Assert::IsTrue( check() );
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

			auto add =[&]( int v )->void{ tree.add( v ); set.insert( v ); };
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

			WS::tree_t<int>::node_t::_exchangewith_right(tree.root);
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_exchangewith_left(tree.root);
			Assert::IsTrue( check() );

			add( 15 );
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_exchangewith_right(tree.root);
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_exchangewith_left(tree.root);
			Assert::IsTrue( check() );

			WS::tree_t<int>::node_t::_exchangewith_left(tree.root->right);
			Assert::IsTrue( check() );
			WS::tree_t<int>::node_t::_exchangewith_right(tree.root->right);
			Assert::IsTrue( check() );
		}

		TEST_METHOD(trace_tree)
		{
			WS::tree_t<int> tree;

			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(5);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(10);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(3);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(7);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(8);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(9);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(15);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});

			tree.add(20);
			WS::Tree_Helper::trace(tree);
			WS::Tree_Helper::trace(WS::Tree_Helper::trace_string_t{L" ende "});
		}
	};
}
