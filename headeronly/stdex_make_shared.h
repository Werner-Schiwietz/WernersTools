#pragma once

#include <memory>

#include "is_.h"

namespace stdex//brauchbar
{
	//usage stdex::make_shared( datentyp{...} ); also man uebergibt ein refernz oder rv-referenz und spart sich den template-parameter
	template<class _Ty> auto make_shared( _Ty const & v)
	{
		using pointer_element_t = std::remove_reference_t<_Ty>;
		return std::make_shared<pointer_element_t, _Ty const &>( v );
	}
	template<class _Ty> auto make_shared( _Ty && v)
	{
		using pointer_element_t = std::remove_reference_t<_Ty>;
		return std::make_shared<pointer_element_t, _Ty &&>( std::forward<_Ty>(v) );
	}
}