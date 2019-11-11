#pragma once

#include <stdexcept>
// WS::tribool kapselt die stati false, true und invalid
// invalid ist ein wert, wenn er mit dem default-ctor erzeugt wurde, bzw durch zuweisung eines invalid tribool.
// operator bool() und ähnliche werfen std::exception, wenn wert invalid ist. vorher also ggf. mit valid() pruefen
// vergleichsoperatoren vergleichen alle drei moeglichen werte, also keine excpetion wenn ein oder beide werte invalid sind (invalid==invlid)==(false==false)==(true==true)==true
//
//siehe auch UT_tribool.cpp

namespace WS
{
	class tribool//boost tribool gefaellt mir nicht
	{
		unsigned __int8 value=0xff;
	public:
		tribool() = default;
		tribool(bool value) : value(value){}
		tribool( tribool const & r ) : value(r.value){}
		tribool& operator=( tribool const & ) = default;
#		pragma warning(suppress:4458)
		tribool& operator=( bool value )
		{
			this->value = value?1:0;
			return *this;
		}

		bool valid()const
		{
			return this->value==0 || this->value==1;
		}
		bool operator==( bool r )const //(invalid==invalid)==true
		{
			return *this==tribool(r);
		}
		bool operator!=( bool r )const //(invalid==invalid)==true
		{
			return *this!=tribool(r);
		}
		bool operator==( tribool const & r )const //(invalid==invalid)==true
		{
			return this->value==r.value;
		}
		bool operator!=( tribool const & r )const //(invalid==invalid)==true
		{
			return this->value!=r.value;
		}
		operator bool()const
		{
			if( valid()==false )
				throw std::runtime_error( "tribool::operator bool, invalid" );
			return this->value ? true : false;
		}
		bool operator !()const
		{
			return ! this->operator bool();
		}
	};
}
