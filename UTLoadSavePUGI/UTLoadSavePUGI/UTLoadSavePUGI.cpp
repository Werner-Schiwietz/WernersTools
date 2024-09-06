#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "..\..\headeronly\cout_umleitung.h"
#include "..\..\headeronly\char_helper.h"
#include "..\..\headeronly\to_underlying.h"
#include "..\..\headeronly\rotate.h"
#include "..\..\headeronly\pugi_loadsave.h"
#include "..\..\headeronly\cout_umleitung.h"

#include <tuple>
#include <functional>
#include <vector>
#include <map>


struct Data
{
	int								Integer{};
	unsigned short					Ushort{};
	pugi::string_t					String{};
	bool							Bool{};
	enum enum_t{_0,_1,_2,_4=4}		Enum{},
		Enum2{enum_t(41)};
	enum enum3_t : short {}			Enum3{1};
	double							Double{3.1415926535897932384626433832795028841971};
	std::vector<__int8>				Array{1,3,2,4,3,5};
	std::pair<int,pugi::string_t>	Pair{1,_T("2")};
	std::tuple<int,pugi::string_t,unsigned short>	Tuple{1,_T("2"),3};

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
		//return std::tie( this->Integer
		//				, this->Ushort
		//				, this->String
		//				, this->Bool
		//				, this->Enum
		//				, this->Enum2
		//				, this->Enum3
		//				, this->Double
		//				, this->Array
		//				, this->Pair
		//				, this->Tuple
		//);
		return std::make_tuple( std::ref(  this->Integer)
								, std::ref(this->Ushort)
								, std::ref(this->String)
								, std::ref(this->Bool)
								, std::ref(this->Enum)
								, std::ref(this->Enum2)  
								, std::ref(this->Enum3)  
								, std::ref(this->Double) 
								, std::ref(this->Array)
								, std::ref(this->Pair)
								, std::ref(this->Tuple)
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
								, std::ref(this->Array)
								, std::ref(this->Pair)
								, std::ref(this->Tuple)
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
			(void)WS::from_node( nodedata, NAME_AND_STR( Array ) );
			(void)WS::from_node( nodedata, NAME_AND_STR( Pair ) );
			(void)WS::from_node( nodedata, NAME_AND_STR( Tuple ) );

			static_assert( WS::_node::IsContainer_v<const std::vector<__int8> & > );
			static_assert( WS::_node::IsContainer_v<int> == false );
			static_assert( WS::_node::IsContainer_v<pugi::string_t> == false );
			static_assert( WS::_node::IsStdPair_v<pugi::string_t> == false );
			static_assert( WS::_node::IsStdPair_v<std::pair<int,int>> == true );
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
			
	bool ret_v = true;
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Integer ));//same as ret_v &= to_node(mynode,Integer,"Integer");
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Ushort ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( String ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Bool ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Enum ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Enum2 ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Enum3 ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Double ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Array ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Pair ));
	ret_v &= WS::to_node(mynode,NAME_AND_STR( Tuple ));
	return ret_v;
}



namespace UTLoadSavePUGI
{
	TEST_CLASS(UTLoadSavePUGI)
	{
	public:
		TEST_METHOD(zerlege_tuple)
		{
			auto tuple = std::make_tuple(5,"hallo",short{7});
			{
				auto tuple1 = WS::get_rest(tuple);
				auto tuple2 = WS::get_rest(tuple1);
				auto tuple3 = WS::get_rest(tuple2);//tuple3 ist leer
				//auto tuple4 = WS::get_rest(tuple3);//error C2672: 'WS::get_rest': no matching overloaded function found

				std::get<0>(tuple1) = "welt";
			}
			{
				auto tuple1 = WS::get_rest(const_cast<decltype(tuple) const&>(tuple));
				auto tuple2 = WS::get_rest(tuple1);
				auto tuple3 = WS::get_rest(tuple2);//tuple3 ist leer
				//auto tuple4 = WS::get_rest(tuple3);//error C2672: 'WS::get_rest': no matching overloaded function found

				//std::get<0>(tuple1) = "welt";//error C3892: 'std::get': you cannot assign to a variable that is const
			}
		}
		TEST_METHOD(Data_to_from_xml_1)
		{
			WCout2Output wcout_umleiten;
			std::wcout << __FUNCTION__ << std::endl;
			auto data = Data{};
			data.Integer = -1;
			data.Ushort = 2;
			data.String=PUGIXML_TEXT("hallo");
			data.Bool=true;
			data.Enum=Data::enum_t::_4;

			pugi::xml_document doc;
			auto nodedoc = doc.append_child(PUGIXML_TEXT("Data_to_from_xml_1"));//node als rahmen für die testdaten
			//schreiben der ersten struktur
			data.save( nodedoc, _T("testData") );
			//schreiben der zweiten, identische struktur und inhalt mit anderer funktion
			WS::to_node( nodedoc, data, _T("testData") );

			//xml-text besorgen, zum anschauen
			std::stringstream ss;
			doc.save(ss);
			[[maybe_unused]]auto xml = ss.str();
			std::wcout << xml.c_str() << std::endl;


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
			std::wcout << std::endl;
		}
		TEST_METHOD(Data_to_from_xml_2)
		{
			WCout2Output wcout_umleiten;
			std::wcout << _T(__FUNCTION__) << std::endl;

			auto data = Data{};
			data.Integer = -2;
			data.Ushort = 3;
			data.String=PUGIXML_TEXT("welt");
			data.Bool=false;
			data.Enum=Data::enum_t::_2;


			pugi::xml_document doc;
			auto nodedoc = doc.append_child(PUGIXML_TEXT("Data_to_from_xml_2"));//node als rahmen für die testdaten

			WS::to_node( nodedoc, data, _T("testData") );
			//WS::to_node( nodedoc, data.getTuple(), _T("testData") );//klappt so nicht, die membername als tag wären weg und durch "value" ersetzt

			//xml-text besorgen, zum anschauen
			std::stringstream ss;
			doc.save(ss);
			[[maybe_unused]]auto xml = ss.str();
			std::wcout << xml.c_str() << std::endl;

			Data data2;
			WS::from_node( nodedoc, data2, _T("testData") );
			Assert::IsTrue( data == data2 );

			//Data data3;
			//auto t = data3.getTuple();t;
			//WS::from_node( nodedoc, t, _T("testData") );//wird nicht kompiliert, aber egal, save über tuple klappt ja auch nicht
			//Assert::IsTrue( data3 == data2 );

			std::wcout << std::endl;
		}
		TEST_METHOD(map_to_from_xml)
		{
			WCout2Output wcout_umleiten;
			std::wcout << _T(__FUNCTION__) << std::endl;

			std::map<unsigned short,std::wstring> Map{{2,_T("welt")},{1,_T("hallo")}};


			static_assert(WS::_node::IsMap_v<decltype(Map)>);
			static_assert(WS::_node::IsMap_v<std::string> == false);


			pugi::xml_document doc;
			auto nodedoc = doc.append_child(PUGIXML_TEXT("map_to_from_xml"));//node als rahmen für die testdaten

			WS::to_node( nodedoc, NAME_AND_STR(Map) );
			//WS::to_node( nodedoc, data.getTuple(), _T("testData") );//klappt so nicht, die membername als tag wären weg und durch "value" ersetzt

			//xml-text besorgen, zum anschauen
			std::stringstream ss;
			doc.save(ss);
			[[maybe_unused]]auto xml = ss.str();
			std::wcout << xml.c_str() << std::endl;

			std::map<unsigned short,std::wstring> Map2;
			WS::from_node( nodedoc, Map2, _T("Map") );
			Assert::IsTrue( Map == Map2);


			std::wcout << std::endl;
		}

	};
}
