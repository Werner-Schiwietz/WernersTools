
#include <memory>
#include <algorithm>
#pragma push_macro("max")
#undef max
#pragma push_macro("min")
#undef min

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
namespace WS
{
	template<typename value_type, typename compare_type = std::less<value_type>> class tree_t;

	namespace Tree_Helper
	{
		template<typename value_type,typename compare_type = std::less<value_type> > void trace(  tree_t<value_type,compare_type> const & tree );
	}
}

#include <vector>
namespace WS
{
	template<typename value_type, typename compare_type /*= std::less<value_type>*/> class tree_t
	{
	public:
		using value_t=value_type;
		using compare_t=compare_type;
		class node_t
		{
		public:
			value_t value;
			enum class branch_id {left=1,right=2};

			using parent_t = node_t *;
			parent_t parent = nullptr;
			std::unique_ptr<node_t> left;
			std::unique_ptr<node_t> right;

			node_t( value_t value ) : value(value){}
			node_t( value_t value,parent_t parent ) : value(value), parent(parent){}

			template<branch_id> std::unique_ptr<node_t> &	_branch() = delete;
			template<> std::unique_ptr<node_t> &			_branch<branch_id::left>() {return this->left;}
			template<> std::unique_ptr<node_t> &			_branch<branch_id::right>() {return this->right;}

			template<branch_id> static constexpr branch_id	_other();
			template<> static constexpr branch_id			_other<branch_id::left>(){return branch_id::right;}
			template<> static constexpr branch_id			_other<branch_id::right>(){return branch_id::left;}

			struct _node_branchcount_t
			{
				node_t * node;
				size_t branchcount = 0;
				_node_branchcount_t( node_t * node) : node(node){}
				_node_branchcount_t& operator=( node_t * _node ) & { this->node=_node;++branchcount;return *this;}

				node_t * operator->() const { return this->node;}
				operator node_t *() const { return this->node;}
			};
			template<branch_id leftright> _node_branchcount_t _most()
			{
				_node_branchcount_t ret_value{ this };
				if( ret_value )
					while( auto & node = ret_value->_branch<leftright>() )
						ret_value = node.get();
				return ret_value;
			}
			template<branch_id leftright> _node_branchcount_t  _up()
			{
				_node_branchcount_t ret_value = this;
				if( ret_value )
				{
					while( ret_value->parent && ret_value->parent->_branch<leftright>().get() == ret_value )
						ret_value = ret_value->parent;
					ret_value = ret_value->parent;
				}
				return ret_value;
			}

			template<branch_id leftright> auto _set_child( std::unique_ptr<node_t> && node )
			{
				auto ret_value = std::move(_branch<leftright>());
				if( ret_value ) ret_value->parent = nullptr;
				if( node ) node->parent = this;
				_branch<leftright>() = std::move(node);
				return ret_value;
			}

			template<branch_id leftright> static std::unique_ptr<node_t> & _rotatefrom_( std::unique_ptr<node_t> & node_ptr )
			{
				ASSERT( node_ptr );
				auto node = node_ptr.get();
				auto branch = node_ptr->_branch<leftright>().get();
				ASSERT(branch);
				auto _parent = node->parent;
				auto _1 = branch->_set_child<_other<leftright>()>(std::move(node_ptr));
				node_ptr = node->_set_child<leftright>(std::move(_1));//node_ptr ist jetzt branch, also neue astgabel
				node_ptr->parent = _parent;

				return node_ptr;
			}

			static node_t&	_add( value_t newvalue, std::unique_ptr<node_t>& sheet, parent_t parent, compare_t const & comparator )
			{
				if( sheet ) 
					return sheet->add( newvalue, comparator );//recursiv is bad here
				else 
					return *(sheet = std::unique_ptr<node_t>{ new node_t( newvalue, parent ) });
			}
			node_t&			add( value_t v, compare_t const & comparator  )
			{
				if( comparator( v, this->value ) )
					return _add( v, this->left, parent_t(this), comparator );
				
				if( comparator( this->value, v ) )
					return _add( v, this->right, parent_t(this), comparator );

				this->value = v;
				return *this;
			}
			node_t *		find( value_t v ) const
			{
				if( this->value < v )
					return this->right==nullptr ? nullptr : this->right->find( v );

				if( v < this->value )
					return this->left==nullptr ? nullptr : this->left->find( v );

				return const_cast<node_t*>(this);
			}

			struct deepweight_t
			{
				size_t deep=0;
				size_t weight=0;
				deepweight_t operator*( deepweight_t const & r ) const {return {std::max(this->deep,r.deep),this->weight+r.weight};}
				deepweight_t operator+( deepweight_t const & r ) const {return {this->deep+r.deep,this->weight+r.weight};}
			};
			deepweight_t	deepweight() const { return this ? deepweight_t{1,1} + left->deepweight() * right->deepweight() : deepweight_t{0,0}; }
			size_t			weight() const { return this ? (1 + left->weight() + right->weight()) : 0; }
			size_t			deep() const { return this ? (1 + std::max(left->deep(), right->deep())) : 0; }

			static bool _is_balanced_avl( deepweight_t const & ldw, deepweight_t const & rdw  )
			{
				return abs(static_cast<long>(ldw.deep) - static_cast<long>(rdw.deep)) < 2;
			}
			static bool _is_balanced_absolut( deepweight_t const & ldw, deepweight_t const & rdw  )
			{
				deepweight_t dw{ deepweight_t {1,1} +  ldw * rdw };
				if( dw.deep == 0 ) 
					return true;

				return (size_t{1} << (dw.deep-1)) <= dw.weight;
			}
			static bool _is_balanced( deepweight_t const & ldw, deepweight_t const & rdw  )
			{
				return _is_balanced_avl(ldw,rdw);
				return _is_balanced_absolut(ldw,rdw);
			}
			bool		is_balanced() const 
			{
				return this ? _is_balanced(left->deepweight(),right->deepweight()) : true;
			}
		};

		std::unique_ptr<node_t> root;
		compare_t				comparator;

		template<typename node_t::branch_id leftright>  struct _iterator
		{
			node_t * node = nullptr;
			~_iterator() = default;
			_iterator() noexcept = default;
			_iterator(_iterator const &) noexcept = default;
			_iterator(_iterator &&) noexcept = default;
			_iterator& operator=(_iterator const &) & noexcept = default;
			_iterator& operator=(_iterator &&) & noexcept = default;

			_iterator( tree_t & tree )
			{
				if( tree.root )
					this->node = tree.root->_most<leftright>();
			}

			bool operator==( _iterator const & r )const { return this->node==r.node; }
			bool operator!=( _iterator const & r )const { return !operator==( r ); }

		public:
			value_t const & operator*() const { return this->node->value; }
			value_t const & operator->() const & {return operator*();}
			_iterator& operator++()
			{
				if( auto & next_node = this->node->_branch<node_t::_other<leftright>()>() )
					this->node = next_node->_most<leftright>();
				else
					this->node = this->node->_up<node_t::_other<leftright>()>();
				return *this;
			}
			_iterator operator++( int ) { _iterator retvalue{*this}; (void)operator++(); return retvalue; }
		};
		using iterator =	_iterator<tree_t::node_t::branch_id::left>;
		using riterator =	_iterator<tree_t::node_t::branch_id::right>;

		~tree_t() = default;
		tree_t() = default;
		tree_t(tree_t const &) = delete;//erstmal, wg. unique_ptr
		tree_t(tree_t &&) = default;
		tree_t& operator=(tree_t const &) = delete;
		tree_t& operator=(tree_t &&) = default;

		tree_t( compare_t const & comparator) : comparator(comparator){}
		tree_t( compare_t && comparator) : comparator(std::move(comparator)){}

		iterator begin() { return iterator{*this}; }
		iterator end() { return iterator{}; }
		riterator rbegin() { return riterator{*this}; }
		riterator rend() { return riterator{}; }

		std::unique_ptr<node_t> & node_ptr( node_t const * node )
		{
			if( node )
			{
				if( node->parent )
				{
					if(node->parent->left.get()==node)
						return node->parent->left;
					if(node->parent->right.get()==node)
						return node->parent->right;
					throw std::runtime_error(__FUNCTION__ " parent-verkettung");
				}
				else
					return root;
			}
			throw std::runtime_error(__FUNCTION__ " parent");
		}

		template<typename node_t::branch_id leftright> node_t * _too_heavy( node_t * node )
		{
			auto ow = node->_branch<leftright>()->_branch<node_t::_other<leftright>()>()->weight();
			auto w = node->_branch<leftright>()->_branch<leftright>()->weight();
			if( w < ow )
			{
				node = node_t::_rotatefrom_<node_t::_other<leftright>()>( node->_branch<leftright>() ).get();
				node_t::_rotatefrom_<leftright>( node_ptr( node->parent ) );
			}
			else
				node = node_t::_rotatefrom_<leftright>( node_ptr(node) )->parent;
			return node;
		}
		std::unique_ptr<node_t> _exchange_cut_parent( std::unique_ptr<node_t> & parentnode_ptr, std::unique_ptr<node_t> & terminal_ptr_in )
		{
			ASSERT( parentnode_ptr && terminal_ptr_in );
			ASSERT( terminal_ptr_in->left==nullptr && terminal_ptr_in->right==nullptr );
			
			auto terminal_ptr = std::move(terminal_ptr_in);//aus baum aushaengen

			auto parentnode = parentnode_ptr.get();
			auto terminal = terminal_ptr.get();

			terminal->left = std::move( parentnode->left );
			if( terminal->left ) terminal->left->parent = terminal;

			terminal->right = std::move( parentnode->right );
			if( terminal->right ) terminal->right->parent = terminal;

			std::swap( terminal->parent, parentnode->parent );
			if( parentnode->parent==parentnode )
				parentnode->parent = terminal;

			std::swap( parentnode_ptr, terminal_ptr );//tauschen

			return terminal_ptr;//alten parent ausgehaengt zurück liefern, parent zeigt auf ex-parent von ex-terminal, oder ex-terminal er ansonsten auf sich selbst zeigen würde
		}
		node_t &	_add( value_t const & value )
		{
			if( root )
			{
				auto & ret_value = root->add( value, this->comparator );
				return ret_value;
			}
			
			root = std::unique_ptr<node_t>{ new node_t( value ) };
			return *root;
		}

		bool		is_balanced() const { return root->is_balanced(); }
		void		balance( node_t * node  )
		{
			while( node )
			{
				auto ldw = node->left->deepweight();
				auto rdw = node->right->deepweight();

				if( ldw.deep < rdw.deep && rdw.weight - ldw.weight > 1 )
					node = _too_heavy<node_t::branch_id::right>( node );
				else if( rdw.deep < ldw.deep && ldw.weight - rdw.weight > 1 )
					node = _too_heavy<node_t::branch_id::left>( node );
				else
					node = node->parent;
			}
		}
		node_t &	add( value_t const & value )
		{
			auto & retvalue = _add( value );

			balance( &retvalue );

			return retvalue;
		}
		bool		remove( value_t v )
		{
			auto todelete = root ? root->find( v ) : nullptr;
			if( todelete )
			{
				auto parent = todelete->parent;
				auto ldw = todelete->left->deepweight();
				auto rdw = todelete->right->deepweight();


				auto constexpr left = node_t::branch_id::left;
				auto constexpr right = node_t::branch_id::right;

				std::unique_ptr<node_t> *  ptrtodelete_ptr = &node_ptr(todelete);
				node_t* mit_node_tauschen = nullptr;
				if(  ldw.deep < rdw.deep || (ldw.deep == rdw.deep && ldw.weight < rdw.weight) )
				{
					mit_node_tauschen = todelete->right->_most<node_t::branch_id::left>();
					if( mit_node_tauschen->right )
						mit_node_tauschen = node_t::_rotatefrom_<right>( node_ptr(mit_node_tauschen) )->left.get();
				}
				else if( rdw.weight )
				{
					mit_node_tauschen =todelete->left->_most<right>();
					if( mit_node_tauschen->left )
						mit_node_tauschen = node_t::_rotatefrom_<left>( node_ptr(mit_node_tauschen) )->right.get();
				}

				if( mit_node_tauschen )
				{
					auto to_delete_ptr = _exchange_cut_parent( node_ptr(todelete), node_ptr(mit_node_tauschen) );
					auto balance_node = to_delete_ptr->parent;
					to_delete_ptr = nullptr;

					balance( balance_node );
					return true;
				}
				else
				{
					ASSERT( (*ptrtodelete_ptr)->left==nullptr && (*ptrtodelete_ptr)->right==nullptr );
					if( (*ptrtodelete_ptr)->left==nullptr && (*ptrtodelete_ptr)->right==nullptr )
					{
						parent = (*ptrtodelete_ptr)->parent;
						*ptrtodelete_ptr = nullptr;
						todelete=nullptr;
						balance( parent );
						return true;
					}
				}
			}
			return false;
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
		template<typename value_t,typename compare_t> std::vector<trace_string_t> _tracetree(  tree_t<value_t,compare_t> const & tree )
		{
			using tree_t = tree_t<value_t,compare_t>;

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
			enum class enumFiller{Left=-1,None,Right};
			auto fillchar = [](bool getit)->trace_char_t { return getit ? '-' : ' '; };
			std::function<void(tree_t::node_t const *, size_t ebene, enumFiller filler)> fn_node_out;
			fn_node_out = [&](tree_t::node_t const * node, size_t ebene, enumFiller filler)
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
		template<typename value_t,typename compare_t> void trace(  tree_t<value_t,compare_t> const & tree )
		{
			using tree_t = tree_t<value_t,compare_t>;
			auto lines = _tracetree(tree);
			for( auto & line : lines )
				trace( line );
			if( lines.rbegin() != lines.rend() )
				trace( trace_string_t( stringlen( *lines.rbegin()), L'-') + trace_string_t(tree.is_balanced() ? L" balanced" : L" not balanced" ));
		}
	}
}

#pragma pop_macro("max")
#pragma pop_macro("min")
