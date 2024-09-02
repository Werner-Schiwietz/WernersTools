#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\cout_umleitung.h"
#include "..\..\headeronly\char_helper.h"
#include "..\..\headeronly\to_underlying.h"
#include "..\..\headeronly\rotate.h"
#include "..\..\headeronly\pugi_loadsave.h"

#include <tuple>
#include <functional>


struct Data
{
	int							Integer{};
	unsigned short				Ushort{};
	pugi::string_t				String{};
	bool						Bool{};
	enum enum_t{_0,_1,_2,_4=4}	Enum{},
		Enum2{enum_t(41)};
	enum enum3_t : short {}		Enum3{1};
	double						Double{3.1415926535897932384626433832795028841971};

	//using tuple_type = std::tuple<
	//	  decltype(Data::Integer) &
	//	, decltype(Data::Ushort) &
	//	, decltype(Data::String) &
	//	, decltype(Data::Enum) &
	//	, decltype(Data::Enum2) &
	//>;
	//using ctuple_type = std::tuple<
	//	decltype(Data::Integer) const &
	//	, decltype(Data::Ushort) const &
	//	, decltype(Data::String) const &
	//	, decltype(Data::Enum) const &
	//	, decltype(Data::Enum2) const &
	//>;
	constexpr auto getTuple() // liefert tuple_type
	{
		return std::make_tuple( std::ref(this->Integer)
								, std::ref(this->Ushort)
								, std::ref(this->String)
								, std::ref(this->Bool)
								, std::ref(this->Enum)
								, std::ref(this->Enum2)  
								, std::ref(this->Enum3)  
								, std::ref(this->Double) 
		);
	}
	constexpr auto getTuple() const // liefert ctuple_type
	{
		return std::make_tuple( std::ref(this->Integer)
								, std::ref(this->Ushort)
								, std::ref(this->String)
								, std::ref(this->Bool)
								, std::ref(this->Enum)
								, std::ref(this->Enum2)  
								, std::ref(this->Enum3)  
								, std::ref(this->Double) 
		);
	}
	//void setTuple( ctuple_type const & r)
	//{
	//	getTuple() = r;
	//}


	Data() noexcept {};
	Data(Data const &r) noexcept : Data()
	{
		getTuple() = r.getTuple();
	}
	Data(Data && r) noexcept : Data()
	{
		swap( r);
	}
	Data&  operator=( Data const & r ) 
	{
		this->getTuple() = r.getTuple();
		return *this;
	}
	Data&  operator=( Data && r ) noexcept
	{
		Data{std::move(r)}.swap(*this);
		return *this;
	}
	void swap( Data & r ) noexcept
	{
		auto lt = getTuple();
		auto rt = r.getTuple();
		std::swap( lt,rt );
	}

	bool operator==( Data const & r ) const
	{
		return this->getTuple() == r.getTuple();
	}
#if _HAS_CXX20
#else
	bool operator!=( Data const & r ) const
	{
		return this->getTuple() != r.getTuple();
	}
#endif
private:
public:
	template<typename T> friend auto WS::_node::getter(pugi::xml_node const & );//friend hat keine wirkung auf _node::Has_Load_ctor_v
	Data( pugi::xml_node const & node_with_member, PUGIXML_CHAR const * node_name );
	bool load( pugi::xml_node const & node_with_member, PUGIXML_CHAR const * node_name );
	bool save( pugi::xml_node parent, PUGIXML_CHAR const * node_name ) const;
};

// spezialisirung zu enum_helper.h
template<> inline WS::auto_ptr<pugi::char_t const[]> tostring<pugi::char_t,10,Data::enum_t>(Data::enum_t value )
{
	//das enum-innere per find/replace in return-switch wandeln
	//regexp: (,(?([^\r\n])\s)*)?(\b\w+)\s*(=\s*[^,])?(,?(?([^\r\n])\s)*)(//.*)?
	//		: case enumT1::$2:\treturn _T("$2");
	switch(value)
	{
	case Data::enum_t::_0:	return PUGIXML_TEXT("_0");
	case Data::enum_t::_1:	return PUGIXML_TEXT("_1");
	case Data::enum_t::_2:	return PUGIXML_TEXT("_2");
	case Data::enum_t::_4:	return PUGIXML_TEXT("_4");
	}

	constexpr int radix = 10;
	size_t chars = 20;
	auto buf = std::unique_ptr<pugi::char_t[]>{ new pugi::char_t[chars]{} };//buffer ohne const anlegen
	tostring(WS::to_underlying(value), buf.get(), chars, radix );

	return buf;// zu buffer const casten und als auto_ptr zurückgeben
}
template<> Data::enum_t stringto<Data::enum_t,pugi::char_t>(pugi::char_t const * psz )
{
	//das enum-innere per find/replace in return-switch wandeln
	//regexp: (,(?([^\r\n])\s)*)?(\b\w+)\s*(=\s*[^,])?(,?(?([^\r\n])\s)*)(//.*)?
	//		: if( stringcmp(psz,PUGIXML_TEXT("$2"))==0 )return Data::enum_t::$2;

	if( stringcmp(psz,PUGIXML_TEXT("_0"))==0 )return Data::enum_t::_0;
	if( stringcmp(psz,PUGIXML_TEXT("_1"))==0 )return Data::enum_t::_1;
	if( stringcmp(psz,PUGIXML_TEXT("_2"))==0 )return Data::enum_t::_2;
	if( stringcmp(psz,PUGIXML_TEXT("_4"))==0 )return Data::enum_t::_4;

	return static_cast<Data::enum_t>(::stringto<std::underlying_type_t<Data::enum_t>>(psz));
}

inline Data::Data( pugi::xml_node const & container, PUGIXML_CHAR const * node_name ) : Data()
{

	{
		//ist das schlau?? so kann ein einzelchild und eine liste von children verarbeitet werden
		//allerdings geht so ein member Data in Data nicht. geht aber eh nicht, ctor würde ja nie ein ende finden würde
		pugi::xml_node nodedata;
		if( stringcmp(container.name(),node_name)==0 )
			nodedata = container;
		else 
			nodedata = container.child(node_name);

		if(true)
		{

			if( nodedata )
			{
				(void)WS::from_node( nodedata, NAME_AND_STR( Integer ) );
				(void)WS::from_node( nodedata, NAME_AND_STR( Ushort ) );
				(void)WS::from_node( nodedata, NAME_AND_STR( String ) );
				(void)WS::from_node( nodedata, NAME_AND_STR( Bool ) );
				(void)WS::from_node( nodedata, NAME_AND_STR( Enum ) );
				(void)WS::from_node( nodedata, NAME_AND_STR( Enum2 ) );
				(void)WS::from_node( nodedata, NAME_AND_STR( Enum3 ) );
				(void)WS::from_node( nodedata, NAME_AND_STR( Double ) );
			}
		}
		else if(true)
		{
			if( nodedata )
			{
				(void)WS::from_node( nodedata, NAME_AND_STR( Integer ), [&]( pugi::xml_node const & node ) { return node.text().as_int(); } );
				(void)WS::from_node( nodedata, NAME_AND_STR( Ushort ), [&]( pugi::xml_node const & node ) { return static_cast<unsigned short>(node.text().as_uint()); } );
				(void)WS::from_node( nodedata, NAME_AND_STR( String ), [&]( pugi::xml_node const & node ) { return node.text().as_string(); } );
				(void)WS::from_node( nodedata, NAME_AND_STR( Bool ), [&]( pugi::xml_node const & node ) { return node.text().as_bool(); } );
				(void)WS::from_node( nodedata, NAME_AND_STR( Enum ), [&]( pugi::xml_node const & node ) { return stringto<Data::enum_t>( node.text().as_string() ); } );
				(void)WS::from_node( nodedata, NAME_AND_STR( Enum2 ), [&]( pugi::xml_node const & node ) { return stringto<Data::enum_t>( node.text().as_string() ); } );
				(void)WS::from_node( nodedata, NAME_AND_STR( Enum3 ), [&]( pugi::xml_node const & node ) { return stringto<Data::enum3_t>( node.text().as_string() ); } );
				(void)WS::from_node( nodedata, NAME_AND_STR( Double ), [&]( pugi::xml_node const & node ) { return node.text().as_double(); } );
			}
		}
		else if(true)
		{
			if( nodedata )
			{
				if( auto node = nodedata.child( PUGIXML_TEXT( "Integer" ) ) )
					this->Integer = node.text().as_int();
				if( auto node = nodedata.child( PUGIXML_TEXT( "Ushort" ) ) )
					this->Ushort = static_cast<unsigned short>(node.text().as_uint());
				if( auto node = nodedata.child( PUGIXML_TEXT( "String" ) ) )
					this->String = node.text().as_string();
				if( auto node = nodedata.child( PUGIXML_TEXT( "Bool" ) ) )
					this->Bool = node.text().as_bool();
				if( auto node = nodedata.child( PUGIXML_TEXT( "Enum" ) ) )
					this->Enum = stringto<Data::enum_t>( node.text().as_string() );
				if( auto node = nodedata.child( PUGIXML_TEXT( "Enum2" ) ) )
					this->Enum2 = stringto<Data::enum_t>( node.text().as_string() );
				if( auto node = nodedata.child( PUGIXML_TEXT( "Enum3" ) ) )
					this->Enum3 = stringto<Data::enum3_t>( node.text().as_string() );
				if( auto node = nodedata.child( PUGIXML_TEXT( "Double" ) ) )
					this->Double = node.text().as_double();
			}
		}
	}
}
inline bool Data::load( pugi::xml_node const & node_with_member, PUGIXML_CHAR const * node_name )
{
	*this = Data{node_with_member,node_name};
	return true;
}
inline bool Data::save( pugi::xml_node parent, PUGIXML_CHAR const * node_name ) const
{
	auto mynode = parent.append_child( node_name );

	(void)WS::to_node(mynode,	_T("Data"),	_T("_type_"));
	(void)WS::to_node(mynode,	1,			_T("_version_"));
			
	if(true)
	{
		bool ret_v = true;
		ret_v &= WS::to_node(mynode,NAME_AND_STR( Integer ));//same as ret_v &= to_node(mynode,Integer,"Integer");
		ret_v &= WS::to_node(mynode,NAME_AND_STR( Ushort ));
		ret_v &= WS::to_node(mynode,NAME_AND_STR( String ));
		ret_v &= WS::to_node(mynode,NAME_AND_STR( Bool ));
		ret_v &= WS::to_node(mynode,NAME_AND_STR( Enum ));
		ret_v &= WS::to_node(mynode,NAME_AND_STR( Enum2 ));
		ret_v &= WS::to_node(mynode,NAME_AND_STR( Enum3 ));
		ret_v &= WS::to_node(mynode,NAME_AND_STR( Double ));
		return ret_v;
	}
	else if(true)//
	{
		bool ret_v = true;
		if( auto node = mynode.append_child( PUGIXML_TEXT("Integer") ) )
			ret_v &= node.text().set( this->Integer );
		if( auto node = mynode.append_child( PUGIXML_TEXT("Ushort") ) )
			ret_v &= node.text().set( this->Ushort );
		if( auto node = mynode.append_child( PUGIXML_TEXT("String") ) )
			ret_v &= node.text().set( this->String.c_str() );
		if( auto node = mynode.append_child( PUGIXML_TEXT("Bool") ) )
			ret_v &= node.text().set( this->Bool );
		if( auto node = mynode.append_child( PUGIXML_TEXT("Enum") ) )
			ret_v &= node.text().set( tostring<TCHAR>(this->Enum).get() );
		if( auto node = mynode.append_child( PUGIXML_TEXT("Enum2") ) )
			ret_v &= node.text().set( tostring<TCHAR>(this->Enum2).get() );
		if( auto node = mynode.append_child( PUGIXML_TEXT("Enum3") ) )
			ret_v &= node.text().set( tostring<TCHAR>(this->Enum3).get() );
		if( auto node = mynode.append_child( PUGIXML_TEXT("Double") ) )
			ret_v &= node.text().set( this->Double, 20 );
		return ret_v;
	}
}


namespace UTLoadSavePUGI
{
	TEST_CLASS(UTLoadSavePUGI)
	{
	public:

		TEST_METHOD(TestMethod1)
		{
			auto data = Data{};
			data.Integer = -1;
			data.Ushort = 2;
			data.String=PUGIXML_TEXT("hallo");
			data.Bool=true;
			data.Enum=Data::enum_t::_4;

			pugi::xml_document doc;
			auto nodedoc = doc.append_child(PUGIXML_TEXT("test"));//node als rahmen für die testdaten
			//schreiben der ersten struktur
			data.save( nodedoc, _T("testData") );
			//schreiben der zweiten, identische struktur und inhalt mit anderer funktion
			WS::to_node( nodedoc, data, _T("testData") );

			//xml-text besorgen, zum anschauen
			std::stringstream ss;
			doc.save(ss);
			[[maybe_unused]]auto xml = ss.str();


			auto data2_1 = Data{ nodedoc, _T("testData") };//lesen des ersten Datensatz Data
			Data data2_2;
			Data data2;

			for( auto datanode : nodedoc.children(_T("testData")) )//alle Data der reihe nach lesen
			{
				data2_2 = Data{ datanode, _T("testData") };						//per ctor von Data
				Assert::IsTrue( WS::from_node( datanode, data2, _T("testData") ) );	//per from_node
																				//es ist nur ein aufruf nötig
			}

			Assert::IsTrue( data==data2);
			Assert::IsTrue( data2==data2_1);
			auto data3 = data2;
			Assert::IsTrue( data3==data2);
			auto data4 = std::move(data3);
			Assert::IsTrue( data4==data2);
		#pragma warning(suppress:26800)//Use of a moved from object
			Assert::IsTrue( data3!=data2);
		}
	};
}
