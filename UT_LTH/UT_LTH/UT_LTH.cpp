#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\WP_lessVariadic.h"
#include "..\..\headeronly\SignatureTest.h"

#include <vector>
#include <type_traits>

#include <windows.h>//au weia für HKEY
//#include <winreg.h>

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

	template<typename value_t,size_t size> 
	int cmparray(value_t const (&l)[size],value_t const (&r)[size])
	{
		auto iter_l = &l[0];
		auto iter_r = &r[0];
		for(auto counter=size; counter --> 0; )
		{
			if( *iter_l<*iter_r)
				return -1;
			if( *iter_r++<*iter_l++)
				return 1;
		}
		return 0;
	};

}

namespace UT_LTH
{
	TEST_CLASS(UT_is_callable)
	{
	public:

		TEST_METHOD(is_callable_test1)
		{
			auto i = 0;
			auto fn1 = [](int,int){return false;};
			auto fn2 = [](int const &,int const &,bool){return false;};
			struct op
			{
				bool operator()( char const *, char const *){return false;}
			};
			auto fn3 = [flag=true](int,int){return false;};

			auto gebunden = std::bind(fn2,std::placeholders::_1,std::placeholders::_2, true);
			auto arraycmp = [](char const (&l)[20],char const (&r)[20])
			{
				return 0;
			};

			using namespace WS::LTH_Helper;
			Assert::IsTrue ( is_callable<decltype(fn1),bool(int,int)>::value );								
			Assert::IsTrue ( is_callable<decltype(fn1),bool(int const &, int const &)>::value );
			Assert::IsTrue ( is_callable<decltype(fn1),bool(unsigned short, unsigned short)>::value );
			Assert::IsFalse( is_callable<decltype(fn1),bool(char const *,char const *)>::value );
			{
				bool erg  = fn1(1,2);
			}
			Assert::IsTrue ( is_callable<decltype(fn3),bool(int,int)>::value );								
			Assert::IsTrue ( is_callable<decltype(fn3),bool(int const &, int const &)>::value );
			Assert::IsTrue ( is_callable<decltype(fn3),bool(unsigned short, unsigned short)>::value );
			Assert::IsFalse( is_callable<decltype(fn3),bool(char const *,char const *)>::value );
			{
				bool erg  = fn3(1,2);
			}

			Assert::IsTrue ( is_callable<decltype(gebunden),bool(int,int)>::value );
			Assert::IsTrue ( is_callable<decltype(gebunden),bool(int const &, int const &)>::value );
			Assert::IsTrue ( is_callable<decltype(gebunden),bool(unsigned short, unsigned short)>::value );
			Assert::IsFalse( is_callable<decltype(gebunden),bool(char const *,char const *)>::value );
			{
				bool erg  = gebunden(1,2);
			}

			Assert::IsTrue ( is_callable<decltype(test::less_fn<int>),bool(int,int)>::value );
			Assert::IsTrue ( is_callable<decltype(test::less_fn<int>),bool(long,long)>::value );
			Assert::IsTrue ( is_callable<decltype(test::less_fn<int>),bool(int const &, int const &)>::value );
			Assert::IsTrue ( is_callable<decltype(test::less_fn<int>),bool(unsigned short, unsigned short)>::value );
			Assert::IsFalse( is_callable<decltype(test::less_fn<int>),bool(char const *,char const *)>::value );
			{
				bool erg  = test::less_fn(1,2);
			}

			Assert::IsFalse( is_callable<op,bool(int,int)>::value );
			Assert::IsFalse( is_callable<op,bool(int const &, int const &)>::value );
			Assert::IsFalse( is_callable<op,bool(unsigned short, unsigned short)>::value );
			Assert::IsTrue ( is_callable<op,bool(char const *,char const *)>::value );
			{
				bool erg  = op{}("hallo","welt");
			}
			Assert::IsFalse( is_callable<decltype(test::less_fn<char const *>),bool(int,int)>::value );
			Assert::IsFalse( is_callable<decltype(test::less_fn<char const *>),bool(int const &, int const &)>::value );
			Assert::IsFalse( is_callable<decltype(test::less_fn<char const *>),bool(unsigned short, unsigned short)>::value );
			Assert::IsTrue ( is_callable<decltype(test::less_fn<char const *>),bool(char const *,char const *)>::value );
			{
				bool erg  = test::less_fn<char const *>("hallo","welt");//pointervergleich, egal
			}

			Assert::IsFalse( is_callable<decltype(arraycmp),bool(int,int)>::value );
			Assert::IsTrue( is_callable<decltype(arraycmp),int(char const(&)[20],char const(&)[20])>::value );

			Assert::IsFalse( is_callable<decltype(i),bool(int,int)>::value );
		}
	};
}

namespace test
{
	template<typename value_t
		, typename less_t, typename std::enable_if<WS::canCall<less_t,bool(value_t,value_t)>::value,int>::type = 5
		, typename ... args_t> 
		bool LTH( value_t const & l, value_t const & r, less_t less, args_t && ... args  )
	{
		if( less( l, r ) )
			return true;
		if( less( r, l ) )
			return false;

		return WS::LTH( std::forward<args_t>(args) ... );
	}
	template<typename value_t
		, typename ... args_t> 
		bool LTH( value_t const & l, value_t const & r, args_t && ... args )
	{
		auto erg = WS::LTHCompare( l, r );
		if( erg.valid() )
			return erg;

		return WS::LTH( std::forward<args_t>(args) ... );
	}

}

namespace UT_LTH
{
	TEST_CLASS(UT_LTH)
	{
	public:
		TEST_METHOD(lth_verschiedene_typen)
		{
			Assert::IsTrue( WS::LTH(int{0},int{1}) );
			Assert::IsTrue( WS::LTH(std::string{"10"},std::string{"2"}) );
			Assert::IsFalse( WS::LTH(HKEY{0},HKEY{0}) );
			Assert::IsTrue( HKEY_CURRENT_USER < HKEY_LOCAL_MACHINE );
			Assert::IsTrue( WS::LTH(HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE) );

			{
				auto v1 = std::make_unique<int>(1);
				Assert::IsTrue( WS::LTH<int const*>(nullptr,v1.get()));
				Assert::IsFalse( WS::LTH<int const*>(v1.get(),nullptr));
				auto v2 = std::make_unique<int>(1);

				Assert::IsFalse( WS::LTH(v1.get(),v2.get()) );//-> *l < *r
				Assert::IsFalse( WS::LTH(v2.get(),v1.get()) );//-> *l < *r

				Assert::IsTrue( WS::LTH((void*)v1.get(),(void*)v2.get()) ==(v1.get()<v2.get()) );//-> l < r
				Assert::IsTrue( WS::LTH((void*)v2.get(),(void*)v1.get()) ==(v2.get()<v1.get()) );//-> l < r

				//Assert::IsFalse( WS::LTH(v1,v2) );//pointer-vergleich?? oder inhalt
				//Assert::IsFalse( WS::LTH(v2,v1) );//pointer-vergleich?? oder inhalt
			}

		}

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

			char s1[10] = "hallo";
			char s2[10] = "hallo";
			char s3[10] = "welt";

			bool erg;
			erg = LTH_Helper::is_callable<decltype(test::cmparray<char,10>),int(char const(&)[10],char const(&)[10])>::value ;
			//static_assert(std::is_same<decltype(s1),char(&)[10]>::value);
			erg = LTH_Helper::is_callable<decltype(test::cmparray(s1,s2)),int(const decltype(s1) &,const decltype(s2) &)>::value ;
			erg = LTH_Helper::is_callable<decltype(test::cmparray(s1,s2)),int(const decltype(s1) ,const decltype(s2) )>::value ;
			erg = LTH_Helper::is_callable<decltype(test::cmparray<char,10>),int(decltype(s1),decltype(s2) )>::value ;
			erg = LTH_Helper::is_callable<decltype(test::cmparray<char,10>),int(decltype(s1) const, decltype(s2) const)>::value ;
			erg = LTH_Helper::is_callable<decltype(test::cmparray<char,10>),int(decltype(s1) const & ,decltype(s2) const &)>::value ;

			Assert::IsTrue( LTH(s1,s2) == LTH(s2,s1) );//alle array-elemente werden verglichen (nicht 0-terminiert). die arrays müssen natürlich vom gleichen datentype sein z.b. char[10] oder int[100], 
			Assert::IsFalse( LTH(s1,s2,test::cmparray<char,10>) );
			Assert::IsFalse( LTH(s2,s1,test::cmparray<char,10>) );
			Assert::IsFalse( LTH(s3,s1,test::cmparray<char,10>) );
			Assert::IsTrue( LTH(s1,s3,test::cmparray<char,10>) );

			Assert::IsFalse( LTH(s1,s2) );//array-vergleich geht nun auch per default. nicht 0-terminiert
			Assert::IsFalse( LTH(s2,s1) );
			Assert::IsFalse( LTH(s3,s1) );
			Assert::IsTrue( LTH(s1,s3) );

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

			auto gth_stdfn = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};//geht auch als std::function
			auto gth = [](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};};//oder als lambda (auch mit capture)
			struct gth_functor
			{
				bool operator()(string const & l, string const & r)
				{
					return r < l;
				}
			};

			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}				) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}		).equal() == true );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}, gth_stdfn	) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}, gth			) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}, gth_functor{}) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}				) == true );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}, gth_stdfn		) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}, gth			) == false );
			Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}, gth_functor{}	) == false );
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
				operator char const *() const{return value.c_str();}
			};
			struct gth 
			{
				bool operator()(string const & l, string const & r ) const
				{
					return r<l;
				}
			};
			auto gth_fn = std::function<WS::tribool(string const&,string const &)>{[](string const &l, string const &r)->WS::tribool{if(r<l) return true;if(l<r) return false;return {};}};
			auto gth_lambda = [&](string const&l,string const &r)->bool{return r<l;};

			if( auto erg = WS::LTH( string{"hallo"}, string{"hallo"} )  )
			{
				Assert::Fail();
			}
			else if( erg.equal() )
			{
			}
			else
			{
				Assert::Fail();
			}

			{
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"}		) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"}		) == true );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	gth{}	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},	gth{}	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	gth_fn	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},	gth_fn	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},	gth_lambda	) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"hallo"},	&string::gth) == false );
				Assert::IsTrue( WS::LTH( string{"hallo"}, string{"welt"},	&string::gth) == false );
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

			Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<int>, string{"hallo"}, string{"welt"},	WS::LTHCompare<char const *>	) );
			//Assert::IsTrue( WS::LTH( 2,2, test::gth_fn<short>, string{"hallo"}, string{"welt"},	WS::LTHCompare<char const *>	) );//compile-error
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
				bool operator<( data const & r) const
				{
					return WS::LTH_Member( *this, r			//so
										  , &data::i
										  , &data::p
								    );
					return WS::LTH(   this->i, r.i			//oder so
									, this->p, r.p
									);

				}
			};
			auto data1=data{1,"hallo"};
			auto data2=data{1,"welt"};
			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,&data::p) );
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,&data::p) );
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,&data::p) );

			Assert::IsFalse( data1 < data1 );
			Assert::IsFalse( data2 < data1 );
			Assert::IsTrue(  data1 < data2 );

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
			

			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,test::less_fn<int>,&data::p,test::less_fn<char const *> ));
			//pointervergleich nicht unbedingt immer die selben ergebnisse
			auto erg1 = WS::LTH_Member(data2,data1,&data::i,test::less_fn<int>,&data::p,test::less_fn<char const *> );
			auto erg2 = WS::LTH_Member(data1,data2,&data::i,test::less_fn<int>,&data::p,test::less_fn<char const *> );
			Assert::IsTrue( erg1 != erg2 );

			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,&data::p,WS::LTHCompare<char const *> ));//stringvergleich
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,&data::p,WS::LTHCompare<char const *> ));
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,&data::p,WS::LTHCompare<char const *> ));

			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,&data::p,[gth=false](char const * l, char const * r){if(gth)return stringcmp(r,l);return stringcmp(l,r);} ) );//capture-lambda
			Assert::IsFalse( WS::LTH_Member(data2,data1,&data::i,&data::p,[gth=false](char const * l, char const * r){if(gth)return stringcmp(r,l);return stringcmp(l,r);} ) );
			Assert::IsTrue(  WS::LTH_Member(data1,data2,&data::i,&data::p,[gth=false](char const * l, char const * r){if(gth)return stringcmp(r,l);return stringcmp(l,r);} ) );
			Assert::IsFalse( WS::LTH_Member(data1,data1,&data::i,&data::p,[gth=true ](char const * l, char const * r){if(gth)return stringcmp(r,l);return stringcmp(l,r);} ) );//capture-lambda
			Assert::IsTrue(  WS::LTH_Member(data2,data1,&data::i,&data::p,[gth=true ](char const * l, char const * r){if(gth)return stringcmp(r,l);return stringcmp(l,r);} ) );
			Assert::IsFalse( WS::LTH_Member(data1,data2,&data::i,&data::p,[gth=true ](char const * l, char const * r){if(gth)return stringcmp(r,l);return stringcmp(l,r);} ) );

			auto data3=data{1,"Hallo"};
			Assert::IsFalse( WS::LTH_Member(data1,data3,&data::p,[](char const * l, char const * r){if(l && !r)return 1;if(!l && r)return -1;if(!l && !r)return 0;  return stringicmp(r,l);} ) );//stringvergleich case insesitive
			Assert::IsFalse( WS::LTH_Member(data3,data1,&data::p,[](char const * l, char const * r){if(l && !r)return 1;if(!l && r)return -1;if(!l && !r)return 0;  return stringicmp(r,l);} ) );//stringvergleich case insesitive
			auto data4=data{1,nullptr};
			Assert::IsFalse( WS::LTH_Member(data1,data4,&data::p,[](char const * l, char const * r){if(l && !r)return 1;if(!l && r)return -1;if(!l && !r)return 0;  return stringicmp(r,l);} ) );//stringvergleich case insesitive
			Assert::IsTrue ( WS::LTH_Member(data4,data1,&data::p,[](char const * l, char const * r){if(l && !r)return 1;if(!l && r)return -1;if(!l && !r)return 0;  return stringicmp(r,l);} ) );//stringvergleich case insesitive
		}
	};
}
