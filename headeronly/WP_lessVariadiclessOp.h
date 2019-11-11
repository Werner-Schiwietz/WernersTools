#pragma once

#include "tribool.h"
#include <functional>

//#include <atlstr.h>

#pragma warning(push,4)


namespace WS //vorgefertigte Comparefunktionen fuer LTH-funktionalitaet
{
	namespace //anonymous namespace statt static
	{
		std::function<WS::tribool(CString const &l, CString const &r)> LTH_CStringNoCase = [](CString const &l, CString const &r) -> WS::tribool
		{
			auto erg = l.CompareNoCase( r );
			if( erg < 0 )
				return true;
			else if ( erg > 0 )
				return false;
			return WS::tribool();
		};
	}
}

#pragma warning(pop)