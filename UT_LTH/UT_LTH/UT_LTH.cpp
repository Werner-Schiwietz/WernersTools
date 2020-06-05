#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\WP_lessVariadic.h"
#include "..\..\headeronly\SignatureTest.h"

#include <vector>
#include <type_traits>

namespace WS
{
	template<typename value_t, typename op_t, typename ret_t=decltype( std::declval<op_t>()(std::declval<value_t>(),std::declval<value_t>()) )> struct op_func
	{
		op_t const & op;
		template<typename op_t> op_func( op_t const & op ) : op(op) {}
		auto operator() (value_t const & l, value_t const & r){ return op(l,r);}
	};

}

namespace test
{
	inline bool compare(  )
	{
		return false;
	}

	template<typename value_t, typename ... Args> bool compare( value_t const & l, value_t const & r, std::function<bool( value_t const&,value_t const&)> const & less, Args const & ... args )
	{
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;

		return compare( args ... );
	}

	template<typename value_t, typename ... Args> bool compare( value_t const & l, value_t const & r, Args const & ... args )
	{
		if( l < r )
			return true;
		if( r < l  )
			return false;

		return compare( args ... );
	}


	template<typename value_t> bool less_fn(value_t const & l, value_t const & r )
	{
		return l < r;
	}
	template<typename value_t> struct less_func
	{
		bool operator()(value_t const & l, value_t const & r )
		{
			return l < r;
		}
	};
	template<typename value_t> int comp_less_fn(value_t const & l, value_t const & r )
	{
		if( l < r )
			return -1;
		if( r < l  )
			return 1;
		return 0;
	}
	template<typename value_t> bool gth_fn(value_t const & l, value_t const & r )
	{
		return   r < l;
	}
}

namespace
{
	template<typename op, typename signature> struct op_sig;
	template<typename op, typename ret_t, typename ... args_t> struct op_sig<op, ret_t(args_t...)> 
	{
		using op_t = op;
		using op_soll_t = decltype( std::declval<op_t>()( std::declval<args_t>()...) );
		static_assert( std::is_same<op_soll_t,ret_t>::value );
		op_sig(){}
		op_sig(op_t){}
	};


	bool testit(  )
	{
		return false;
	}
	template<typename value_t, typename less_t, template<typename less_t,bool(value_t,value_t)> class op_sig,typename ... args_t> bool testit( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		static_assert(  WS::canCall<less_t,bool(value_t,value_t)>::value );
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;
		return false;
	}
	template<typename value_t,typename ... args_t> bool testit( value_t const & l, value_t const & r, args_t && ... args )
	{
		if( l < r ) 
			return true;
		if( r < l  )
			return false;
		return testit( std::forward<args_t>(args)... );
	}

	template<typename value_t, typename less_t, typename x=op_sig<less_t,bool(value_t,value_t)>::op_soll_t, typename ... args_t> bool testit( value_t const & l, value_t const & r, less_t less, args_t && ... args )
	{
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;
		return testit( std::forward<args_t>(args)... );
	}

	bool testit2(  )
	{
		return false;
	}
	template<typename value_t, typename less_t, typename f_t=std::enable_if_t<WS::canCall<less_t,bool(value_t,value_t)>::value,less_t>,typename ... args_t> bool testit2( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;
		return testit2( std::forward<args_t>(args)... );
	}
	template<typename value_t, typename ... args_t> bool testit2( value_t const & l, value_t const & r, args_t && ... args  )
	{
		if(  l < r ) 
			return true;
		if( r < l )
			return false;
		return testit2( std::forward<args_t>(args)... );
	}



	template<typename value_t, typename less_t> bool testitY( value_t const & l, value_t const & r, less_t less )
	{
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;
		return false;
	}

}
//namespace test
//{
//	inline bool LTH(  )
//	{
//		return false;
//	}
//
//	template<typename value_t, typename less_t, typename std::enable_if_t<WS::canCall<less_t,bool(value_t,value_t)>::value,int>			= 5,typename ... args_t> bool LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
//	{
//		if( less( l, r ) )
//			return true;
//		if( less( r, l ) )
//			return false;
//
//		return test::LTH( std::forward<args_t>(args) ... );
//	}
//	template<typename value_t, typename less_t, typename std::enable_if_t<WS::canCall<less_t,WS::tribool(value_t,value_t)>::value,int>	= 4,typename ... args_t> bool LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
//	{
//		auto erg = less( l, r );
//		if( erg.valid() )
//			return erg;
//
//		return test::LTH( std::forward<args_t>(args) ... );
//	}
//	template<typename value_t, typename less_t, typename std::enable_if_t<WS::canCall<less_t,int(value_t,value_t)>::value,int>			= 3,typename ... args_t> bool LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
//	{
//		auto erg = less( l, r );
//		if(erg < 0 )
//			return true;
//		if(erg > 0 )
//			return false;
//
//		return test::LTH( std::forward<args_t>(args) ... );
//	}
//
//	template<typename value_t, typename ... args_t> bool LTH( value_t const & l, value_t const & r, args_t && ... args )
//	{
//		auto erg = WS::LTHCompare( l, r );
//		if( erg.valid() )
//			return erg;
//
//		return test::LTH( std::forward<args_t>(args) ... );
//	}
//}


namespace UT_LTH
{
	TEST_CLASS(UT_LTH)
	{
	public:
		
		TEST_METHOD(lth_ohne_op)
		{
			using namespace WS;
			struct integer
			{
				using value_t = int;
				value_t value;
				bool operator<(integer const & r ) const 
				{
					return this->value < r.value;
				}
			};
			struct string
			{
				using value_t = std::string;
				value_t value;
				bool operator<(string const & r ) const 
				{
					return this->value < r.value;
				}
			};
			struct element
			{
				integer			v1;
				string			v2;
				bool operator<(element const & r) const
				{
					return LTH(   this->v1	, r.v1
								, this->v2	, r.v2 );
				}
			};
			struct data_t
			{
				element e1;
				element e2;
				bool	e1_less_e2;
			};
			std::vector<data_t> data 
			{
				  {{1,{"hallo"}},{2,{"hallo"}},true}
				, {{2,{"hallo"}},{2,{"welt"}},true}
				, {{2,{"hallo"}},{1,{"welt"}},false}
			};

			for( auto const & d : data)
			{
				Assert::IsTrue( (d.e1 < d.e2) == d.e1_less_e2 );
				Assert::IsTrue( (d.e2 < d.e1) != d.e1_less_e2 );
			}

			Assert::IsFalse( LTH((char const *)"hallo",(char const *)"hallo") );
			Assert::IsTrue( LTH((char const *)"hallo",(char const *)"welt", 1,1 ) );
			Assert::IsFalse( LTH(1,1,(char const *)"hallo",(char const *)"hallo") );
			Assert::IsTrue(  LTH(1,1,(char const *)"hallo",(char const *)"welt") );
			//Assert::IsTrue(  LTH(1,1,"hallo","welt") );//error C2660: 'WS::LTH': function does not take 2 arguments. weil "hallo" typ char[6] und "welt" typ char[5] ist
			Assert::IsFalse( LTH(1,1,(char const *)"welt",(char const *)"hallo") );

		}
		TEST_METHOD(lth_mit_op)
		{
			struct string
			{
				using value_t = std::string;
				value_t value;
				bool operator<(string const & r ) const 
				{
					return this->value < r.value;
				}
			};
			auto gth = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};

			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}		) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}, gth	) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}		) == true );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}, gth	) == false );
		}
		TEST_METHOD(lth_mit_op2)
		{
			struct string
			{
				using value_t = std::string;
				value_t value;
				bool operator<(string const & r ) const 
				{
					return this->value < r.value;
				}
			};

			//auto gth = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};
			//auto gth = [](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};};
			auto gth = [&](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};};

			using namespace WS;

			Assert::IsTrue( LTH( string{"hallo"}, string{"hallo"}		) == false );
			Assert::IsTrue( LTH( string{"hallo"}, string{"hallo"}, gth	) == false );
			Assert::IsTrue( LTH( string{"hallo"}, string{"welt"}		) == true );
			Assert::IsTrue( LTH( string{"hallo"}, string{"welt"}, gth	) == false );
			Assert::IsTrue( LTH( string{"hallo"}, string{"hallo"}		, string{"hallo"}, string{"hallo"}		) == false );
			Assert::IsTrue( LTH( string{"hallo"}, string{"hallo"}, gth	, string{"hallo"}, string{"hallo"}, gth	) == false );
			Assert::IsTrue( LTH( string{"hallo"}, string{"hallo"}		, string{"hallo"}, string{"welt"}		) == true );
			Assert::IsTrue( LTH( string{"hallo"}, string{"hallo"}, gth	, string{"hallo"}, string{"welt"}, gth	) == false );
		}
		//TEST_METHOD(lth_mit_fn_obj_test)
		//{
		//	struct string
		//	{
		//		using value_t = std::string;
		//		value_t value;
		//		bool operator<(string const & r ) const 
		//		{
		//			return this->value < r.value;
		//		}
		//		static bool gth(string const & l, string const & r){return r<l;}
		//	};
		//	struct gth 
		//	{
		//		bool operator()(string const & l, string const & r ) const
		//		{
		//			return r<l;
		//		}
		//	};
		//	auto gth_fn = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};
		//	auto gth_lambda = [](string const&l,string const &r)->bool{return r<l;};

		//	{
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"hallo"}		) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"welt"}		) == true );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"hallo"},	gth{}	) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"welt"},		gth{}	) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"hallo"},	gth_fn	) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"welt"},		gth_fn	) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"hallo"},	gth_lambda	) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"welt"},		gth_lambda	) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"hallo"},	&string::gth) == false );
		//		Assert::IsTrue( test::LTH( string{"hallo"}, string{"welt"},		&string::gth) == false );
		//	}
		//	{
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"hallo"}	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"welt"}		) == true );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"hallo"},	gth{}	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"welt"},	gth{}	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"hallo"},	gth_fn	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"welt"},	gth_fn	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"hallo"},	gth_lambda	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"welt"},	gth_lambda	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"hallo"},	&string::gth) == false );
		//		Assert::IsTrue( test::LTH( 2,2, string{"hallo"}, string{"welt"},	&string::gth) == false );
		//	}
		//	{
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"}	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"}		) == true );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	gth{}	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	gth{}	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	gth_fn	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	gth_fn	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	gth_lambda	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	gth_lambda	) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	&string::gth) == false );
		//		Assert::IsTrue( test::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	&string::gth) == false );
		//	}
		//}
		TEST_METHOD(lth_mit_fn_obj)
		{
			struct string
			{
				using value_t = std::string;
				value_t value;
				bool operator<(string const & r ) const 
				{
					return this->value < r.value;
				}
				static bool gth(string const & l, string const & r){return r<l;}
			};
			struct gth 
			{
				bool operator()(string const & l, string const & r ) const
				{
					return r<l;
				}
			};
			auto gth_fn = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};
			auto gth_lambda = [](string const&l,string const &r)->bool{return r<l;};

			{
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}		) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}		) == true );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	gth{}	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},		gth{}	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	gth_fn	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},		gth_fn	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},		gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	&string::gth) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},		&string::gth) == false );
			}
			{
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"hallo"}	) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"welt"}		) == true );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"hallo"},	gth{}	) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"welt"},	gth{}	) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"hallo"},	gth_fn	) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"welt"},	gth_fn	) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"hallo"},	gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"welt"},	gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"hallo"},	&string::gth) == false );
				Assert::IsTrue( WS::LTH( 2,2, string{"hallo"}, string{"welt"},	&string::gth) == false );
			}
			{
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"}	) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"}		) == true );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	gth{}	) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	gth{}	) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	gth_fn	) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	gth_fn	) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"hallo"},	&string::gth) == false );
				Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	&string::gth) == false );
			}
		}
		TEST_METHOD(UT_op_func)
		{
			struct string
			{
				using value_t = std::string;
				value_t value;
				bool operator<(string const & r ) const 
				{
					return this->value < r.value;
				}
				static bool gth(string const & l, string const & r){return r<l;}
			};
			struct gth 
			{
				bool operator()(string const & l, string const & r ) const
				{
					return r<l;
				}
			};
			struct gth_param
			{
				bool exor;
				gth_param(bool exor):exor(exor){}
				bool operator()(string const & l, string const & r ) const
				{
					return (r<l)^this->exor;
				}
			};

			auto gth_fn = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};
			auto gth_lambda = [](string const&l,string const &r)->bool{return r<l;};

			{
				WS::op_func<string,gth> op1(gth{});
				auto erg1 = op1(string{"hallo"},string{"welt"});

				WS::op_func<string,decltype(gth_lambda)> op2{gth_lambda};
				auto erg2 = op2(string{"hallo"},string{"welt"});

				WS::op_func<string,decltype(gth_fn)> op3{gth_fn};
				auto erg3 = op2(string{"hallo"},string{"welt"});
			}
			{
				auto erg1 = test::compare(string{"h"},string{"h"},string{"h"},string{"w"});
				auto erg2 = test::compare(string{"h"},string{"h"},string{"w"},string{"h"});
				auto erg3 = test::compare(string{"h"},string{"h"},string{"h"},string{"h"});
			}
			{
				auto erg1 = test::compare(string{"h"},string{"h"},std::function<bool(string const &,string const &)>{gth{}},string{"h"},string{"w"});
				auto erg2 = test::compare(string{"h"},string{"h"},std::function<bool(string const &,string const &)>{gth{}},string{"w"},string{"h"});
				auto erg3 = test::compare(string{"h"},string{"h"},std::function<bool(string const &,string const &)>{gth{}},string{"h"},string{"h"});
			}
			{
				auto erg1 = test::compare(string{"h"},string{"h"},string{"h"},string{"w"},std::function<bool(string const &,string const &)>{gth{}});
				auto erg2 = test::compare(string{"h"},string{"h"},string{"w"},string{"h"},std::function<bool(string const &,string const &)>{gth{}});
				auto erg3 = test::compare(string{"h"},string{"h"},string{"h"},string{"h"},std::function<bool(string const &,string const &)>{gth{}});
			}
			{

				//auto erg1 = test::compare(string{"h"},string{"h"},gth_lambda,string{"h"},string{"w"});
				//auto erg2 = test::compare(string{"h"},string{"h"},gth_lambda,string{"w"},string{"h"});
				//auto erg3 = test::compare(string{"h"},string{"h"},gth_lambda,string{"h"},string{"h"});
			}
			//{
			//	auto erg1 = test::compare(string{"h"},string{"h"},string{"h"},string{"w"},gth_lambda);
			//	auto erg2 = test::compare(string{"h"},string{"h"},string{"w"},string{"h"},gth_lambda);
			//	auto erg3 = test::compare(string{"h"},string{"h"},string{"h"},string{"h"},gth_lambda);
			//}
			//{
			//	auto erg1 = test::compare(string{"h"},string{"h"},gth_param{true},string{"h"},string{"w"});
			//	auto erg2 = test::compare(string{"h"},string{"h"},gth_param{true},string{"w"},string{"h"});
			//	auto erg3 = test::compare(string{"h"},string{"h"},gth_param{true},string{"h"},string{"h"});
			//}
			//{
			//	auto erg1 = test::compare(string{"h"},string{"h"},string{"h"},string{"w"},gth_param{true});
			//	auto erg2 = test::compare(string{"h"},string{"h"},string{"w"},string{"h"},gth_param{true});
			//	auto erg3 = test::compare(string{"h"},string{"h"},string{"h"},string{"h"},gth_param{true});
			//}
			//{
			//	auto erg1 = test::compare(string{"h"},string{"h"},gth_param{false},string{"h"},string{"w"});
			//	auto erg2 = test::compare(string{"h"},string{"h"},gth_param{false},string{"w"},string{"h"});
			//	auto erg3 = test::compare(string{"h"},string{"h"},gth_param{false},string{"h"},string{"h"});
			//}
			//{
			//	auto erg1 = test::compare(string{"h"},string{"h"},string{"h"},string{"w"},gth_param{false});
			//	auto erg2 = test::compare(string{"h"},string{"h"},string{"w"},string{"h"},gth_param{false});
			//	auto erg3 = test::compare(string{"h"},string{"h"},string{"h"},string{"h"},gth_param{false});
			//}

		}
		TEST_METHOD(UT_testit)
		{
			struct integer
			{
				using value_t = int;
				value_t value;
				bool operator<(integer const & r ) const 
				{
					return this->value < r.value;
				}
			};
			struct string
			{
				using value_t = std::string;
				value_t value;
				bool operator<(string const & r ) const 
				{
					return this->value < r.value;
				}
			};
			struct gth 
			{
				bool operator()(string const & l, string const & r ) const
				{
					return r<l;
				}
			};
			struct gth_param
			{
				bool exor;
				gth_param(bool exor):exor(exor){}
				bool operator()(string const & l, string const & r ) const
				{
					return (r<l)^this->exor;
				}
			};

			auto gth_fn = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};
			auto gth_lambda = [](string const&l,string const &r)->bool{return r<l;};


			Assert::IsTrue( testitY(string{"hallo"},string{"welt"},test::less_fn<string>) );
			Assert::IsFalse( testitY(string{"welt"},string{"hallo"},test::less_fn<string>) );
			auto XX = op_sig<decltype(&test::less_fn<string>),bool(string,string)>{test::less_fn<string>};
			Assert::IsTrue( testit(string{"hallo"},string{"welt"},test::less_fn<string>, 1, 2 ));
			Assert::IsFalse( testit(string{"welt"},string{"hallo"},test::less_fn<string>, 2,2 ) );

			//op_sig<bool,bool(string,string)> x;

			testit(string{"hallo"},string{"welt"});
			testit(string{"hallo"},string{"welt"}, test::less_fn<string>);
			testit(string{"hallo"},string{"welt"}, gth_lambda);
			testit(string{"hallo"},string{"welt"}, gth{});
			testit(string{"hallo"},string{"welt"}, gth_param{true});
			//testit(string{"hallo"},string{"welt"}, gth_fn);//imMom compilefehler
			//testit(string{"hallo"},string{"welt"}, test::less_fn<int>);//compilefehler, ber diese zeile ist kaum zu identifizieren

			//testit(string{"hallo"},string{"welt"}, 1, 2);
			//testit(string{"hallo"},string{"welt"}, test::less_fn<string>, 1, 2);
			//testit(string{"hallo"},string{"welt"}, gth_lambda, 1, 2);
			//testit(string{"hallo"},string{"welt"}, gth{}, 1, 2);
			//testit(string{"hallo"},string{"welt"}, gth_param{true}, 1, 2);
			//testit(string{"hallo"},string{"welt"}, gth_fn, 1, 2);
			//testit(string{"hallo"},string{"welt"}, test::less_fn<int>, 1, 2);


			struct xxx
			{
				bool operator()(int,int){return false;}
			};
			op_sig<xxx,bool(int,int)> x1{xxx{}};
			//op_sig<xxx,int(int,int)> x2{xxx{}};

			op_sig<decltype(gth_lambda),bool(string,string)> x5{gth_lambda};
			op_sig<decltype(gth_lambda),bool(string const &,string const &)> x6{gth_lambda};
			op_sig<decltype(gth_lambda),bool(string &,string &)> x7{gth_lambda};
			//op_sig<decltype(gth_lambda),bool(int,int)> x8{gth_lambda};

			op_sig<gth,bool(string const&,string const &)> y1{gth{}};
			//op_sig<gth,int(string,string)> y2{gth{}}
			using fn_t = decltype(&test::less_fn<string>);
			fn_t  fkt = test::less_fn<string>;
			op_sig<fn_t,bool(string,string)>{test::less_fn<string>};
		}
		TEST_METHOD(UT_enable_if)
		{
			auto lesscharptr = [](char const * l,char const * r)->bool{return strcmp(l,r)<0; };
			auto lessint = [&](int l,int r)->bool{return l < r; };
			std::enable_if_t<true,int> i1 = 5;
			//std::enable_if_t<WS::canCall<decltype(lessint),bool(int,int)>::value,int> i = 5;
			std::enable_if_t<WS::canCall<decltype(lessint),bool(int,int)>::value,int> i2 = 5;
			//std::enable_if_t<false,int> i2 = 5;
			//std::enable_if_t<WS::canCall<decltype(lesscharptr ),bool(int,int)>::value,int> i2 = 5;

			testit2(2,2);
			testit2(2,2,lessint);
			testit2(2,2,3,4);
			testit2(2,2,lessint,3,4);
			testit2(2,2,lessint,3,4,lessint);
			testit2(2,2,3,4,lessint);
			testit2(2,2,"x","y",lesscharptr);
			testit2(2,2,lessint,"x","y",lesscharptr);
			//testit2(1,2,"x","y",lessint);

			//testit2(2,2,test::less_func<int>{});
			testit2(2,2,test::less_fn<int>);
			testit2(2,2,test::less_fn<int>,"x","y",test::less_fn<char const *>);
			//testit2(2,2,test::less_func<int>{},"x","y",test::less_func<char const *>{});
		}
	};
	TEST_CLASS(UT_LTH_MEMBER)
	{
		TEST_METHOD(UT_ohne_op)
		{
			struct data
			{
				int i;
				char const * p ;
			};
			auto data1=data{1,"hallo"};
			auto data2=data{1,"welt"};
			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,&data::p) );
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,&data::p) );
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,&data::p) );
		}
		TEST_METHOD(UT_mit_op)
		{
			struct data
			{
				int i;
				char const * p ;
			};
			auto data1=data{1,"hallo"};
			auto data2=data{1,"welt"};
			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,test::less_fn<int>,&data::p) );
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,test::less_fn<int>,&data::p) );
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,test::less_fn<int>,&data::p) );

			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,test::less_func<int>{},&data::p) );
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,test::less_func<int>{},&data::p) );
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,test::less_func<int>{},&data::p) );

			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,test::comp_less_fn<int>,&data::p) );
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,test::comp_less_fn<int>,&data::p) );
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,test::comp_less_fn<int>,&data::p) );
			

			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,test::less_fn<int>,&data::p,test::less_fn<char const *> ));//pointervergleich
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,test::less_fn<int>,&data::p,test::less_fn<char const *> ));
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,test::less_fn<int>,&data::p,test::less_fn<char const *> ));

			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,&data::p,WS::LTHCompare<char const *> ));
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,&data::p,WS::LTHCompare<char const *> ));
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,&data::p,WS::LTHCompare<char const *> ));
		}
	};
}
