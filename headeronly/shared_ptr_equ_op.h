#pragma once

#include <memory>

template<typename T> inline bool operator==( std::shared_ptr<T> const & l, std::shared_ptr<T> const & r )
{
	if( (l==nullptr) != (r==nullptr) )
		return false;
	if( l==nullptr ) 
		return true;//beide nullptr
	return *l == *r;
}
template<typename T> inline bool operator!=( std::shared_ptr<T> const & l, std::shared_ptr<T> const & r )
{
	return !(l==r);
}