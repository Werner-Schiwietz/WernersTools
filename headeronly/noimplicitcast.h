#pragma once
//Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

//#include <utility>

//hier gibt es zwei implementierungen fuer das Problem int birne;int apfel = birne;
//neu per #define TYPEDEF(name,typ[,typ_trait]) //ist ein einzeiler define, also ehr c++
//alt per #define CreateNICType(name,typ)		//wird alles per verschachtelten defines implementiert
//ich bevorzuge TYPEDEF

//problem
//{	//keinerlei typsicherheit
//	typedef int apfel;
//	typedef int birne;
//
//	apfel Apfel=5;
//	birne Birne=7;
//
//	Apfel=Birne;
//}

//Beispiel ohne ableitung per typedef
//typedef noimplicitcast<int, 5/*willkuerliche, aber eindeutige id*/> apfel;
//typedef noimplicitcast<int, 7/*willkuerliche, aber eindeutige id*/> birne;
//
////apfel Apfel=5;//error C2440: 'initializing' : cannot convert from 'int' to 'noimplicitcast<int,5>'
////birne Birne=7;//error C2440: 'initializing' : cannot convert from 'int' to 'noimplicitcast<int,7>'
//apfel Apfel=apfel(5);//ok
//birne Birne=birne(7);//ok
//Apfel=Birne;//error C2679: binary '=' : no operator found which takes a right-hand operand of type 'birne' (or there is no acceptable conversion)

//WOBEI apfel Apfel=apfel(5)*apfel(5);// imMom funktioniert der ddatentyp aber apfel² sein muesste
//UND  apfel Apfel=apfel(5)/apfel(5);// imMom funktioniert der ddatentyp aber apfel::value_type sein muesste

template <typename T, int typeidentifier=0> class noimplicitcast
{
public:
	using value_type = T;
	value_type value;

	noimplicitcast(){}

	noimplicitcast( const noimplicitcast& rhs ){
		this->value=rhs.value;}

	noimplicitcast( noimplicitcast&& rhs ){
		this->value=std::move(rhs.value);}

	explicit noimplicitcast( const value_type& value ){
		this->value=value;}

	explicit noimplicitcast( value_type&& value ){
		this->value=std::move(value);}

	//seit vc14 zuweisungs-op nötig
	noimplicitcast& operator=( noimplicitcast const & r )
	{
		this->value = r.value;
		return *this;
	}
	noimplicitcast& operator=( noimplicitcast && r )
	{
		this->value = std::move(r.value);
		return *this;
	}

	explicit operator value_type const &()const{
		return this->value;}
	explicit operator value_type &(){
		return this->value;}
	value_type const & toValueType() const
	{
		return this->value;
	}
	value_type & toValueType() 
	{
		return this->value;
	}

	bool operator==( const noimplicitcast& rhs )const
	{
		return this->value==rhs.value;
	}
	bool operator!=( const noimplicitcast& rhs )const
	{
		return this->value!=rhs.value;
	}
	bool operator<( const noimplicitcast& rhs )const
	{
		return this->value < rhs.value;
	}
	bool operator>( const noimplicitcast& rhs )const
	{
		return this->value > rhs.value;
	}
	bool operator<=( const noimplicitcast& rhs )const
	{
		return this->value <= rhs.value;
	}
	bool operator>=( const noimplicitcast& rhs )const
	{
		return this->value >= rhs.value;
	}

	noimplicitcast operator+( noimplicitcast const &r ){ return noimplicitcast (this->value+r.value);}
	noimplicitcast operator-( noimplicitcast const &r ){ return noimplicitcast (this->value-r.value);}
	noimplicitcast operator*( noimplicitcast const &r ){ return noimplicitcast (this->value*r.value);}
	noimplicitcast operator/( noimplicitcast const &r ){ return noimplicitcast (this->value/r.value);}
	noimplicitcast& operator+=( noimplicitcast const &r ){ this->value+=r.value;return *this;}
	noimplicitcast& operator-=( noimplicitcast const &r ){ this->value-=r.value;return *this;}
	noimplicitcast& operator*=( noimplicitcast const &r ){ this->value*=r.value;return *this;}
	noimplicitcast& operator/=( noimplicitcast const &r ){ this->value/=r.value;return *this;}
};

template<typename value_t> struct True_tester
{
	constexpr bool operator()( value_t const & ){return true;}
};

#define CreateNICTypeTestedBegin( classname, type, tester_obj )\
	class classname : public noimplicitcast<type>{\
public:\
	classname() = default;\
	explicit classname(type value,bool testit=true) : noimplicitcast(value) {if(testit)tester_obj(value);}\
	classname(classname const &) = default;\
	classname(classname && r) : noimplicitcast( std::move(r.value) ){}\
	classname& operator=( classname const & ) & = default;\
	classname operator=( classname &&  r){ classname{std::move(r.value)}.swap(*this); return std::move(*this);}\
	void swap(classname & r){std::swap(this->value, r.value);}\
	bool	TestValue(type val){return tester_obj(val);}\
	bool	TestValue(){return tester_obj(this->value);}
#define CreateNICTypeBegin( classname, type ) CreateNICTypeTestedBegin( classname, type, True_tester<type>() )
#define CreateNICTypeEnd }
#define NICPlusPlusOperatoren( classname, type ) \
	classname& operator++( )&{ TestValue(++this->value);return *this;}\
	classname  operator++(int){ classname retvalue=*this;operator++();return retvalue;}
#define NICPlusOperatoren( classname, type ) \
	classname  operator+ ( classname const &r )const{ return classname (this->value+r.value);}\
	classname& operator+=( classname const &r )&{ this->value+=r.value;TestValue();return *this;}
#define NICMinusMinusOperatoren( classname, type ) \
	classname& operator--( )&{ TestValue(--this->value);return *this;}\
	classname  operator--(int){ classname retvalue=*this;operator--();return retvalue;}
#define NICMinusOperatoren( classname, type ) \
	classname  operator- ( classname const &r )const{ return classname (this->value-r.value);}\
	classname& operator-=( classname const &r )&{ this->value-=r.value;TestValue();return *this;}
#define NICMalOperatoren( classname, type ) \
	classname  operator* ( classname const &r )const{ return classname (this->value*r.value);}\
	classname& operator*=( classname const &r )&{ this->value*=r.value;return *this;}
#define NICMalOperatorenType( classname, type ) \
	template<typename T>classname  operator* ( T const &r )const{ return classname (this->value*r);}\
	template<typename T>classname& operator*=( T const &r )&{ this->value*=r;TestValue();return *this;}
#define NICGeteiltOperatoren( classname, type ) \
	classname  operator/ ( classname const &r )const{ return classname (this->value/r.value);}\
	classname& operator/=( classname const &r )&{ this->value/=r.value;TestValue();return *this;}
#define NICGeteiltOperatorenType( classname, type ) \
	classname::value_type  operator/ ( classname const &r )const{ return this->value/r.value;}\
	template<typename T>classname operator/ ( T const &r )const{ return classname(this->value/r);}\
	template<typename T>classname& operator/=( T const &r )&{ this->value/=r;TestValue();return *this;}
#define NICRechenOperatorenNormal( classname, type )\
	NICPlusPlusOperatoren( classname, type ) \
	NICMinusMinusOperatoren( classname, type ) \
	NICPlusOperatoren( classname, type ) \
	NICMinusOperatoren( classname, type ) \
	NICMalOperatoren( classname, type ) \
	NICGeteiltOperatoren( classname, type )
#define NICRechenOperatorenType( classname, type )\
	NICPlusPlusOperatoren( classname, type ) \
	NICMinusMinusOperatoren( classname, type ) \
	NICPlusOperatoren( classname, type ) \
	NICMinusOperatoren( classname, type ) \
	NICMalOperatorenType( classname, type ) \
	NICGeteiltOperatorenType( classname, type )

#define CreateNICType( classname, type ) CreateNICTypeBegin(classname,type) CreateNICTypeEnd
#define CreateNICPlusType( classname, type ) CreateNICTypeBegin(classname,type) NICPlusOperatoren(classname,type) CreateNICTypeEnd
#define CreateNICCalcType( classname, type ) CreateNICTypeBegin(classname,type) NICRechenOperatorenType(classname,type) CreateNICTypeEnd
#define CreateNICCalcNormalType( classname, type ) CreateNICTypeBegin(classname,type) NICRechenOperatorenNormal(classname,type) CreateNICTypeEnd


class CFile;

namespace WS
{
	//Explicit_Type gibt einer fachlichkeit mit wenig aufwand einen eigenen namen, um programme lesbarer zu machen, und zuweisungen zu inhaltlich falschen werten zu verhindern
	//z.b. 
	//	TYPEDEF( x_koordiante, int ) x(5);
	//	TYPEDEF( y_koordiante, int ) y(1);
	//  y = x führt zu compilefehler
	//und statt void SetPixel( int y, int x ) mit aufruf SetPixel( 1, 2 );
	//ist void SetPixel( y_koordiante y, x_koordiante x ) eindeutig. der aufruf muss wie folgt aussehen
	//SetPixel( y_koordiante(3), x_koordiante(2) ); man erkennt sofort die bedeutung der parameter 
	//TYPEDEF leitet von Explicit_Type_Trait_all und implementiert damit fast alle operatoren
	//ggf kann TYPEDEF2 verwendet werden um andere operatoren verfuegbar zu machen. 
	//TYPEDEF2(name,type,OP_TYPE[...,OP_TYPE])
	//TYPEDEF_BEGIN wird eine offene struct angelegt die mit TYPEDEF2_END bzw '}' geschlossen werden muss, dazwischen aber platz fuer erweiterungen laest
	//erweitern mit eigener value-test-methode
	//TYPEDEF_BEGIN(int0_bis_5_t,int)
	//	static void test_valid(int const & v) {if(v<0||v>5)throw std::invalid_argument( "wert ist für den datentyp ungültig" );}
	//TYPEDEF_END;


	template<typename type_t,typename value_t> struct Explicit_Type_TraitBase 
	{
		typedef type_t type_t;
		typedef value_t value_t;
		using value_type = value_t;
		value_type value;

		explicit 
		Explicit_Type_TraitBase( value_type value, bool test_value=true ) : value(value){if(test_value)static_cast<type_t&>(*this).test_valid(value);}
		Explicit_Type_TraitBase(Explicit_Type_TraitBase<type_t,value_t> const &r) : value(r.value){}
		Explicit_Type_TraitBase(type_t const &r) : value(r.value){}
		Explicit_Type_TraitBase(){};

		static void test_valid(value_t const & ) 
		{
		}
		void test_valid() const
		{
			test_valid( this->value );
		}

		explicit operator value_type const &() const
		{
			return value;
		}
		explicit operator value_type &() 
		{
			return value;
		}
		value_type const & toValueType() const &
		{
			return value;
		}
		value_type & toValueType() &
		{
			return value;
		}
		value_type toValueType() &&
		{
			return value;
		}
		value_type const & to_underlying() const &
		{
			return value;
		}
		value_type & to_underlying() &
		{
			return value;
		}
		value_type to_underlying() &&
		{
			return value;
		}
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_nop{};//OP der nichts implementiert, weil fuer Explicit_Type mindestens einer angegeben werden muss
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_equ
	{
		friend bool operator==( type_t const & l, type_t const & r )
		{
			return l.toValueType() == r.toValueType();
		}
		friend bool operator!=( type_t const & l, type_t const & r )
		{
			return l.toValueType() != r.toValueType();
		}
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_lth
	{
		friend bool operator<( type_t const & l, type_t const & r ) 
		{
			return l.toValueType() < r.toValueType();
		}
	};	
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_equ_less : Explicit_Type_Trait_equ<type_t,value_t>, Explicit_Type_Trait_lth<type_t,value_t>{};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_compare : Explicit_Type_Trait_equ<type_t,value_t>, Explicit_Type_Trait_lth<type_t,value_t>
	{
		friend bool operator<=( type_t const & l, type_t const & r ) 
		{
			return l.toValueType() <= r.toValueType();
		}
		friend bool operator>( type_t const & l, type_t const & r ) 
		{
			return l.toValueType() > r.toValueType();
		}
		friend bool operator>=( type_t const & l, type_t const & r ) 
		{
			return l.toValueType() >= r.toValueType();
		}
	};	

	template<typename type_t,typename value_t> struct Explicit_Type_Trait_incpre
	{
		friend type_t const & operator++( type_t & l)
		{
			l.test_valid( ++l.toValueType() );
			return l;
		}
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_inc : Explicit_Type_Trait_incpre<type_t,value_t>
	{
		friend type_t operator++( type_t & l, int)
		{
			auto retvalue = l;
			l.test_valid( ++l.toValueType() );
			return retvalue ;
		}
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_decpre
	{
		friend type_t const & operator--( type_t & l)
		{
			l.test_valid( --l.toValueType() );
			return l;
		}
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_dec : Explicit_Type_Trait_decpre<type_t,value_t>
	{
		friend type_t operator--( type_t & l, int)
		{
			auto retvalue = l;
			l.test_valid( --l.toValueType() );
			return retvalue ;
		}
	};

	template<typename type_t,typename value_t> struct Explicit_Type_Trait_add
	{
		friend type_t   operator+( type_t const & l, type_t const &r ){ return type_t(l.toValueType()+r.toValueType()); }
		friend type_t & operator+=( type_t & l, type_t const &r ){ l.value+=r.value;l.test_valid();return l; }
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_sub
	{
		friend type_t   operator-( type_t const & l, type_t const &r ){ return type_t(l.toValueType()-r.toValueType()); }
		friend type_t & operator-=( type_t & l, type_t const &r ){ l.value-=r.value;l.test_valid();return l; }
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_mul
	{
		friend type_t   operator*( type_t const & l, value_t const &r ){ return type_t(l.toValueType()*r); }
		friend type_t   operator*( value_t const & l, type_t const &r ){ return type_t(l*r.toValueType()); }
		friend type_t & operator*=( type_t & l, value_t const &r ){ l.value*=r;l.test_valid();return l; }
	};
	template<typename type_t,typename value_t> struct Explicit_Type_Trait_div
	{
		friend value_t  operator/( type_t const & l, type_t const &r ){ return l.toValueType()/r.toValueType(); }
		friend type_t   operator/( type_t const & l, value_t const &r ){ return type_t(l.toValueType()/r); }
		friend type_t & operator/=( type_t & l, value_t const &r ){ l.value/=r;l.test_valid();return l; }
	};

	template<typename type_t,typename value_t> struct Explicit_Type_Trait_loadsafe
	{
		friend void ReadData( CFile *pFile, type_t& Object )
		{
			ReadData( pFile, Object.toValueType() );
		}
		friend void WriteData( CFile *pFile, type_t const & Object )
		{
			WriteData( pFile, Object.toValueType() );
		}
	};

	template<typename type_t, typename value_t> struct Explicit_Type_Trait_deref//kann fuer pointer_typen sinn machen
	{
		using deref_t = decltype(*std::declval<value_t>());
		//friend value_t & operator->( type_t & l )//error C2801: 'WS::operator ->' must be a non-static member
		//{
		//	return l.toValueType();
		//}
		value_t & operator->( )
		{
			//der cast gefaellt mir nicht, ich schaffe es aber anders nicht
			return static_cast<type_t&>(*this).toValueType();
		}
		value_t const & operator->( ) const
		{
			return static_cast<type_t const &>(*this).toValueType();
		}
		deref_t & operator *( )
		{
			return *static_cast<type_t&>(*this).toValueType();
		}
		deref_t const & operator *( ) const
		{
			return *static_cast<type_t const &>(*this).toValueType();
		}
	};
	template<typename type_t, typename value_t> struct Explicit_Type_Trait_pointeradd//kann fuer pointer_typen sinn machen
	{
		friend type_t operator +( type_t const & l, size_t r )
		{
			return type_t( l.toValueType() + r);
		}
		friend type_t & operator +=( type_t & l, size_t r )
		{
			l.toValueType() += r;
			l.ValueTester();
			return l;
		}
	};


	template<typename type_t,typename value_t> struct Explicit_Type_Trait_all 
		: Explicit_Type_Trait_compare<type_t,value_t>
		, Explicit_Type_Trait_inc<type_t,value_t>
		, Explicit_Type_Trait_dec<type_t,value_t>
		, Explicit_Type_Trait_add<type_t,value_t>
		, Explicit_Type_Trait_sub<type_t,value_t>
		, Explicit_Type_Trait_mul<type_t,value_t>
		, Explicit_Type_Trait_div<type_t,value_t>
		, Explicit_Type_Trait_loadsafe<type_t,value_t>{};

	template<typename type_type,typename value_type
		,template<typename,typename> class Trait=Explicit_Type_Trait_all
		,template<typename,typename> class ... OtherTraits>
	struct Explicit_Type 
		: Explicit_Type_TraitBase<type_type,value_type>
		, Trait<type_type,value_type>
		, OtherTraits<type_type,value_type> ...
	{
		using type_t = type_type;
		using value_t = value_type;
		using this_t = Explicit_Type<type_t,value_t,Trait,OtherTraits...>;
		using Explicit_Type_TraitBase<type_t,value_t>::Explicit_Type_TraitBase;//ctor
	};
}


#define TYPEDEF2_BEGIN(type_name,value_type,...) struct type_name : WS::Explicit_Type<type_name,value_type,__VA_ARGS__>{typedef value_type value_t;using Explicit_Type::Explicit_Type;
#define TYPEDEF2_END }
#define TYPEDEF2(type_name,value_type,...) TYPEDEF2_BEGIN(type_name,value_type,##__VA_ARGS__) TYPEDEF2_END

#define TYPEDEF_END }
#if _MSC_VER >= 1921 //Visual Studio 2019 version 16.1 	
#	define TYPEDEF_BEGIN(type_name, value_type,...) TYPEDEF2_BEGIN(type_name, value_type,##__VA_ARGS__)
#	define TYPEDEF(type_name,value_type,...) TYPEDEF_BEGIN(type_name,value_type,##__VA_ARGS__) TYPEDEF_END
#else
//TYPEDEF2 nur nötig, weil VC den default-template Explicit_Type_Trait_all von Explicit_Type_Trait_all nicht korrekt auswertet
#	define TYPEDEF_BEGIN(type_name,value_type) TYPEDEF2_BEGIN(type_name,value_type,WS::Explicit_Type_Trait_all)
#	define TYPEDEF(type_name,value_type) TYPEDEF_BEGIN(type_name,value_type) TYPEDEF_END
#endif

