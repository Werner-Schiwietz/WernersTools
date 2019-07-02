#pragma once

#include "tribool.h"
#include <functional>

//#include <atlstr.h>

#pragma warning(push,4)


namespace WP //vorgefertigte Comparefunktionen fuer LTH-funktionalitaet
{
	namespace //anonymous namespace statt static
	{
		std::function<WP::tribool(CString const &l, CString const &r)> LTH_CStringNoCase = [](CString const &l, CString const &r) -> WP::tribool
		{
			auto erg = l.CompareNoCase( r );
			if( erg < 0 )
				return true;
			else if ( erg > 0 )
				return false;
			return WP::tribool();
		};
	}
}

#pragma warning(pop)