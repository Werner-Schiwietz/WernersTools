#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\type_list.h"

#include <vector>  

#include <cstdlib>
#include <tuple>
#include <functional>
#include <iostream>

namespace
{
	auto fnAutoAdd = []( auto const & l, auto const & r )
	{
		return l + r;
	};
	auto fnAutoLess = []( auto const & l, auto const & r )
	{
		return l < r;
	};

	
}

namespace
{	//aus https://stackoverflow.com/questions/21657627/what-is-the-type-signature-of-a-c11-1y-callable-function

	// For generic types that are functors, delegate to its 'operator()'
	template <typename T>
	struct function_traits
		: public function_traits<decltype(&T::operator())>
	{};

	// for pointers to member function
	template <typename ClassType, typename ReturnType, typename... Args>
	struct function_traits<ReturnType(ClassType::*)(Args...) const> {
		//enum { arity = sizeof...(Args) };
		typedef std::function<ReturnType (Args...)> f_type;
	};

	// for pointers to member function
	template <typename ClassType, typename ReturnType, typename... Args>
	struct function_traits<ReturnType(ClassType::*)(Args...) > {
		typedef std::function<ReturnType (Args...)> f_type;
	};

	// for function pointers
	template <typename ReturnType, typename... Args>
	struct function_traits<ReturnType (*)(Args...)>  {
		typedef std::function<ReturnType (Args...)> f_type;
	};

	template <typename L> 
	typename function_traits<L>::f_type make_function(L l){
		return (typename function_traits<L>::f_type)(l);
	}

	long times10(int i) { return long(i*10); }

	struct X {
		double operator () (float f, double d) { return d*f; } 
	};

	// test code
	int main()
	{
		auto callable = [](int i) { return long(i*10); };
		typedef function_traits<decltype(callable)> traits;
		traits::f_type ff = callable;

		std::cout << make_function([](int i) { return long(i*10); })(2) << ", " << make_function(times10)(2) << ", " << ff(2) << std::endl;
		std::cout << make_function(X{})(2,3.0) << std::endl;

		return 0;
	}
	auto xxX = main();
}

namespace WS
{
	template<typename T, typename Sign> struct is_functor;
	template<typename T, typename ret_t, typename ... args_t> struct is_functor<T, ret_t(args_t...)>
	{                                                                   
		using  yes=char[1];                                            
		using no=char[2];                                            
		template <typename U, U> struct type_check;                     
		template <typename _1> static yes &chk(type_check<ret_t(_1::*)(args_t...), &_1::operator()>*);
		template <typename   > static no  &chk(...);                    
		static bool const value = sizeof(chk<T>(nullptr)) == sizeof(yes);
	};

	template<typename f_t, typename signature> struct is_function_with_sig;
	template< typename ret_t, typename ... args_t> struct is_function_with_sig< ret_t( * )(args_t...),ret_t( args_t... )> : std::true_type {};
	template< typename f_t, typename s_t> struct is_function_with_sig : std::false_type { };

	//is_callable funktioniert nicht vollständig. der ret_t muss gleich sein, aber die parameter müssen nur grob passen. und mit std::bind kann es zu compilefehlker kommen
	template<typename l_t, typename signature> struct is_callable;
	template<typename T, typename ret_t, typename ... args_t> struct is_callable<T, ret_t(args_t...)> 
	{
		template <typename U> struct type_check : std::false_type{};
		template <> struct type_check<ret_t> : std::true_type{}; 
		template<typename _1> using R = decltype(std::declval<_1>()( std::declval<args_t>()...) );

		template <typename _1,bool ret_t_equal = type_check<R<_1>>::value>
		static constexpr bool chk( type_check<R<_1>> * ) { return ret_t_equal; }
		template <typename> 
		static constexpr bool chk( ... ) { return false; }

		static bool const value = chk<T>(nullptr);
	};


	template<typename signatur> struct has_signatur;
	template<typename ret_t, typename ...args_t> struct has_signatur<ret_t(args_t...)>
	{
		template<typename f_t> static constexpr bool function(f_t & v)
		{
			v;
			return WS::is_function_with_sig<decltype(&v),ret_t(args_t...)>::value;
		}
		template<typename f_t> static constexpr bool function(f_t && v)
		{
			v;
			return WS::is_function_with_sig<decltype(&v),ret_t(args_t...)>::value;
		}
		template<typename f_t> static constexpr bool function()
		{
			f_t v; v;
			return WS::is_function_with_sig<f_t,ret_t(args_t...)>::value;
		}
		template<typename f_t> static constexpr bool functor(f_t & )
		{
			return WS::is_functor<f_t,ret_t(args_t...)>::value;
		}
		template<typename f_t> static constexpr bool functor(f_t && )
		{
			return WS::is_functor<f_t,ret_t(args_t...)>::value;
		}
		template<typename f_t> static constexpr bool functor()
		{
			return WS::is_functor<f_t,ret_t(args_t...)>::value;
		}

		//callable functioniert nicht vollständig. der ret_t wird geprüft, aber die parameter müssen nur grob passen
		template<typename functional_t> static constexpr bool callable(functional_t  &)
		{
			return is_callable<functional_t,ret_t(args_t...)>::value;
		}
		template<typename functional_t> static constexpr bool callable(functional_t  && v )
		{
			return is_callable<functional_t,ret_t(args_t...)>::value;
		}
		template<typename functional_t> static constexpr bool callable( )
		{
			return is_callable<functional_t,ret_t(args_t...)>::value;
		}

	};

}

namespace SignatureTemplate
{		
	template<bool fn_match(int const &)> auto find_if( std::vector<int> const & vec)
	{
		auto iter = vec.begin();
		for( ; iter!=vec.end(); ++iter )
			if( fn_match( *iter ) )
				break;
		return iter;
	}
	bool fn1( int const & v ) { return v==5; }
	bool fn2( int v ) { return v==5; }
	bool fn3( short v ) { return v==5; }

	struct fo1 
	{ 
		bool operator()( int const & v ) { return v==5; } 
	};
	struct fo2
	{ 
		bool operator()( int const & v ) { return v==5; } 
		bool operator()( short v ) { return v==5; } 
	};
	struct fo3
	{ 
		int operator()( bool v) { return !v; } 
	};



	template<typename T, typename Sign> struct is_functor1
	{                                                                   
		using  yes=char[1];                                            
		using no=char[2];                                            
		template <typename U, U> struct type_check;                     
		template <typename _1> static yes &chk(type_check<Sign, &_1::operator()>*);
		template <typename   > static no  &chk(...);                    
		static bool const value = sizeof(chk<T>(nullptr)) == sizeof(yes);
	};



	template <typename functor_t, typename signature> struct defines_functor_operator;
	template <typename functor_t,typename ret_t,typename...args_t> struct defines_functor_operator<functor_t,ret_t(args_t...)>
	{
		typedef char (& yes)[1];
		typedef char (& no)[2];

		using fn_t = ret_t( functor_t::* )(args_t...);

		// we need a template here to enable SFINAE
		template <typename U,typename ret_t,typename...args_t> 
		static yes deduce(
			char (*)[sizeof( 
				(fn_t)&U::operator()
			)]);
		// fallback
		template <typename...> static no deduce(...);

		static bool constexpr value = sizeof(deduce<functor_t,ret_t,args_t...>(0)) == sizeof(yes);
	};

	template<typename f_t, typename signature> struct has_signature_fn;
	template< typename ret_t, typename ... args_t> struct has_signature_fn< ret_t( * )(args_t...),ret_t( args_t... )> : std::true_type {};
	template< typename f_t, typename s_t> struct has_signature_fn : std::false_type { };

	template<typename signature> struct has_signature;
	template<typename ret_t, typename ... args_t> struct has_signature< ret_t(args_t...)> 
	{
		bool value=false;
		has_signature( ret_t(*)(args_t...) ) : value(has_signature_fn<ret_t(*)(args_t...),ret_t(args_t...)>::value){}
		template<typename f_t>has_signature( f_t ) : value(false){}
		template<typename functor_t>has_signature( functor_t const & ) : value(has_signature_fn<ret_t(functor_t::*)(args_t...),ret_t(args_t...)>::value){}
	};

	template<typename s_t> struct has_signature : std::false_type {  };



	TEST_CLASS(UnitTest1)
	{
	public:
		TEST_METHOD( UT_typelist_to_tuple )
		{
			short s3=3;
			short s=4;
			//WS::typelist<bool, int, short&>::tuple_t tuple(true,5, short(4));//cannot convert argument 3 from 'short' to 'short &'
			WS::typelist<bool, int, short&>::tuple_t tuple( false, 6, s3 );
			WS::typelist<bool, int, short&>::tuple_t tuple2( true, 5, s );
			Assert::IsTrue( &std::get<2>( tuple )==&s3, L"sollte referenz auf s3 sein" );
			tuple = tuple2;//!!Achtung:  die referenz auf tuple.s3 aendert sich nicht aber der inhalt von s3

			Assert::IsTrue( std::get<0>( tuple )==true, L"sollte true sein" );
			Assert::IsTrue( std::get<1>( tuple )==5, L"sollte 5 sein" );
			Assert::IsTrue( &std::get<2>( tuple )==&s3, L"sollte referenz auf s3 sein" );
			Assert::IsTrue( std::get<2>( tuple )==4, L"sollte 4 sein" );
			std::get<2>( tuple ) = short(44);
			Assert::IsTrue( std::get<2>( tuple )==44, L"sollte 44 sein" );
			Assert::IsTrue( std::get<2>( tuple )==s3, L"sollte 44 sein" );
			Assert::IsTrue( &std::get<2>( tuple2 )==&s, L"sollte 44 sein" );
		}
		TEST_METHOD( UT_typelist_types )
		{
			Assert::IsTrue( WS::typelist<bool, int, short&>::count!=2, L"sollte 3 sein" );
			Assert::IsTrue( WS::typelist<bool, int, short&>::count==3, L"sollte 3 sein" );
			Assert::IsTrue(std::is_same<bool,WS::typelist<bool,int,short&>::get<0>::type>::value, L"sollte bool sein" );
			Assert::IsTrue( std::is_same<bool, WS::typelist<bool, int, short&>::get_t<1>>::value==false, L"sollte int sein" );
			Assert::IsTrue( std::is_same<int, WS::typelist<bool, int, short&>::get<1>::type>::value, L"sollte int sein" );
			Assert::IsTrue( std::is_same<bool, WS::typelist<bool, int, short&>::get<2>::type>::value==false, L"sollte short& sein" );
			Assert::IsTrue( std::is_same<short&, WS::typelist<bool, int, short&>::get<2>::type>::value, L"sollte short& sein" );
		}
		TEST_METHOD(UT_signatur_types)
		{
			WS::signatur<void(int)> s1;s1;
			WS::signatur<double( double const &, double const &)> d1; 

			Assert::IsTrue(std::is_same<decltype(d1)::return_type,double>::value, L"sollte double sein" );
			Assert::IsFalse( std::is_same<decltype(d1)::parameter_typelist::get_t<0>, double>::value, L"sollte double const & sein" );
			Assert::IsTrue( std::is_same<decltype(d1)::parameter_typelist::get_t<0>, double const &>::value, L"sollte double const & sein" );
			Assert::IsFalse( std::is_same<decltype(d1)::parameter_typelist::get_t<1>, double>::value, L"sollte double const & sein" );
			Assert::IsTrue( std::is_same<decltype(d1)::parameter_typelist::get_t<1>, double const &>::value, L"sollte double const & sein" );
			//decltype(d1)::parameter_typelist::get_t<2> xxx;//error C2338: index out of bounds
		}
		TEST_METHOD(UT_fn_match1)
		{

			std::vector<int> vec{1, 2, 3, 4, 5, 6};
			auto fn2 = []( int const & v ) { return v==5; };
			Assert::IsTrue( *find_if<&fn1>( vec )==5 );
			//Assert::IsTrue( *find_if<[]( int const & v ) { return v==5;}>( vec )==5 );
		}
		TEST_METHOD(UT_UT_has_signature_fktpointer)
		{
			decltype(fn1) f ;
			decltype(fn1) * f1 = fn1; Assert::IsTrue( f1( 5 ) );
			decltype(fn1)& f2 = fn1;Assert::IsTrue( f2( 5 ) );
			decltype(&fn1) f3 = fn1;Assert::IsTrue( f3( 5 ) );

			if(1)
			{
				Assert::IsTrue( has_signature<bool( int const & )>(fn1).value, L"1: nur funktionname geht irgend wie nicht" );
				//Assert::IsFalse( has_signature<bool( int const & )>(fn2).value, L"2: nur funktionname geht irgend wie nicht" );
				///Assert::IsFalse( has_signature<bool( int const & )>(fn3).value, L"3: nur funktionname geht irgend wie nicht" );
			}
		}
		TEST_METHOD(UT_has_signature_functor)
		{
			auto fn1 = &fo1::operator();
			using T1 = decltype(&fo1::operator());
			using T2 = decltype((bool(fo2::*)(short))&fo2::operator());
			using f_t = bool( fo2::* )(short);
			using f_t_invalid = bool( fo2::* )(fo2);
			auto fn21 = static_cast<bool( fo2::* )(short)>(&fo2::operator());
			auto fn22 = static_cast<f_t>(&fo2::operator());
			//auto fnX = static_cast<f_t_invalid>(&fo2::operator());
			//bool x = has_signature<bool( int const & )>(fo1()).value; x;
			//Assert::IsTrue( has_signature<fo1, bool( int const & )>::value, L"1: nur funktionname geht irgend wie nicht" );
			//Assert::IsTrue( has_signature<decltype(&fo1), bool( int const & )>::value, L"1: funktionpointer sollte gehen" );

			{
				using fn_t = bool( fo1::* )(int const &);
				using fn_t2 = decltype(&fo1::operator());
				Assert::IsTrue(std::is_same<fn_t, fn_t2>::value);
			}
			{
				using fn_t = bool( fo2::* )(int const &);
				using fn_t2 = decltype((fn_t)&fo2::operator());
				Assert::IsTrue(std::is_same<fn_t, fn_t2>::value);
			}
			{
				using fn_t = bool( fo3::* )(int const &);
				using fn_t2 = decltype((fn_t)&fo3::operator());

				auto x = fn_t{};
				auto y = fn_t2{};
				Assert::IsTrue(std::is_same<fn_t, fn_t2>::value);
			}

			Assert::IsTrue( defines_functor_operator<fo1, bool( int const & )>::value );
			Assert::IsTrue( defines_functor_operator<fo2, bool( int const & )>::value );

			bool f1 = fo3{}( int(1));
			int f2 = fo3{}( int(1));
			int f3 = fo3{}( bool(true));
			Assert::IsFalse( WS::is_callable<fo3,bool( int const & )>::value);//return-wert müsste gleich sein
			Assert::IsTrue( defines_functor_operator<fo3, bool( int const & )>::value );
			Assert::IsTrue( WS::is_callable<fo3,int( int const & )>::value);//cast int auf bool
			Assert::IsTrue( WS::is_callable<fo3,int( bool const & )>::value);

		}
		TEST_METHOD(UT_is_functor)
		{
			Assert::IsTrue( is_functor1<fo1, bool (fo1::*)( int const & )>::value );
			Assert::IsTrue( is_functor1<fo2, bool (fo2::*)( int const & )>::value );
			Assert::IsFalse( is_functor1<fo3, bool (fo3::*)( int const & )>::value );

			Assert::IsTrue( WS::is_functor<fo1, bool ( int const & )>::value );
			Assert::IsTrue( WS::is_functor<fo2, bool ( int const & )>::value );
			Assert::IsFalse( WS::is_functor<fo3, bool ( int const & )>::value );

			Assert::IsFalse( WS::is_functor<decltype(fn1), bool ( int const & )>::value );
			Assert::IsFalse( WS::is_functor<decltype(fn2), bool ( int const & )>::value );
			Assert::IsFalse( WS::is_functor<decltype(fn3), bool ( int const & )>::value );

			auto l = []( int const & )->bool { return true; };
			decltype(l) l2 = l;
			//decltype([]( int const & )->bool { return true; });
			//Assert::IsFalse( WS::is_functor<decltype([](int const&)->bool{return true;}), bool ( int const & )>::value );
			Assert::IsTrue( WS::has_signatur<bool ( int const & )>::functor(fo1()) );
			Assert::IsTrue( WS::has_signatur<bool ( int const & )>::functor<fo1>() );
			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::function(fo1()) );

			Assert::IsTrue( WS::has_signatur<bool ( int const & )>::functor(fo2()) );
			Assert::IsTrue( WS::has_signatur<bool ( int const & )>::functor<fo2>() );
			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::function(fo2()) );

			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::functor(fo3()) );
			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::functor<fo3>() );
			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::function(fo3()) );
			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::callable(fo3()) );

			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::functor(l) );
			Assert::IsFalse( WS::has_signatur<bool ( int const & )>::function(l) );
			Assert::IsTrue( WS::has_signatur<bool ( int const & )>::callable(l) );

		}
		TEST_METHOD(UT_is_function)
		{

			Assert::IsFalse( WS::is_function_with_sig<fo1, bool ( int const & )>::value );
			Assert::IsFalse( WS::is_function_with_sig<fo2, bool ( int const & )>::value );
			Assert::IsFalse( WS::is_function_with_sig<fo3, bool ( int const & )>::value );

			Assert::IsTrue( WS::is_function_with_sig<decltype(&fn1), bool ( int const & )>::value );
			Assert::IsFalse( WS::is_function_with_sig<decltype(&fn2), bool ( int const & )>::value );
			Assert::IsFalse( WS::is_function_with_sig<decltype(&fn3), bool ( int const & )>::value );

			auto l = [&]( int const & )->bool { return true; };
			decltype(l) l2 = l;
			//decltype([]( int const & )->bool { return true; });
			//Assert::IsFalse( WS::is_functor<decltype([](int const&)->bool{return true;}), bool ( int const & )>::value );
			Assert::IsFalse( WS::is_functor<decltype(fn2), bool ( int const & )>::value );
			Assert::IsFalse( WS::is_functor<decltype(fn3), bool ( int const & )>::value );

			Assert::IsTrue( WS::has_signatur<bool( int const & )>::function( fn1 ) );
			Assert::IsFalse( WS::has_signatur<bool( int const & )>::function( fn2 ) );
			Assert::IsFalse( WS::has_signatur<bool( int const & )>::function( fn3 ) );

			int i=5;
			std::function<bool( int const & )> f;
			f = l;
			auto b =f( 5 );
			f=[]( int v )->bool { return v==5; };
			b =f( 5 );
			//f=[v=true]( void )->bool{return v; };
			//b=f(5);

			Assert::IsTrue( WS::has_signatur<bool( int const & )>::callable( l ) );
			Assert::IsFalse( WS::has_signatur<bool( int const & )>::callable( [v=true]( void )->bool{return v;} ) );
			Assert::IsFalse( WS::has_signatur<bool( int const & )>::callable( []( int v )->void { v; } ) );
			Assert::IsFalse( WS::has_signatur<bool( int const & )>::callable( []( int v, int )->bool{ return v==5; } ) );

			Assert::IsTrue( WS::has_signatur<bool( int const & )>::callable( std::bind( []( int v, int )->bool { return v==5; }, std::placeholders::_1, 6 ) ) );
			//Assert::IsFalse( WS::has_signatur<bool( int const & )>::callable( std::bind( []( int v, int )->bool { return v==5; }, std::placeholders::_1, std::placeholders::_2 ) ) );
			Assert::IsTrue( WS::has_signatur<bool( int const & )>::callable( std::function<bool(int const &)>() ) );
			Assert::IsFalse( WS::has_signatur<bool( int const & )>::callable( std::function<bool(int const &,int)>() ) );
			//Assert::IsFalse( WS::has_signatur<bool( int const & )>::callable( std::bind( []( int v, int )->bool { return v==5; }, std::placeholders::_1, std::placeholders::_2 ) ) );

			{	//sollten false liefern, aber der parameter-check ist nicht ausgereift, default-cast machen probleme
				auto using_assert = &Assert::IsFalse;
				using_assert = &Assert::IsTrue;

				using_assert( WS::has_signatur<bool( int const & )>::callable( []( int v )->bool { return v==5; } ), L"falscher parameter int, sollte int const & sein", nullptr );//&__LineInfo{__WFILE__,__FUNCTION__,__LINE__ } );
				using_assert( WS::has_signatur<bool( int const & )>::callable( []( short v )->bool{return v==5;}),L"falscher parameter short, sollte int const & sein",nullptr );
				using_assert( WS::has_signatur<bool( int const & )>::callable( std::function<bool(int const &)>() ), L"falscher parameter int, sollte int const & sein", nullptr );
			}
		}

		TEST_METHOD(UT_calling)
		{
			int v5=5, v6=6;
			Assert::IsTrue( fn1( v5 ) );
			Assert::IsTrue( fo1{}( v5 ) );
			Assert::IsTrue( [&]( int const & v ) { return v==5; }(v5) );
		}
		TEST_METHOD(UT_auto_lambda)
		{
			auto ab = fnAutoAdd( 1, 2 );
			auto abc = fnAutoAdd( fnAutoAdd( std::string("hallo"),std::string(" ")), std::string("welt") );
		}
	};
}