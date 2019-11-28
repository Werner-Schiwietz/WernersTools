//#pragma once
#ifndef __WP_ITERATOR_ACCESS__
#define __WP_ITERATOR_ACCESS__

//WS::iterator_access haelt zwei iteratoren und kann z.b. direkt fuer range based for benutzt werden
//konsturiert aus allem was begin(T) und end(T) kann
//zusaetzlich nullterminierte char bzw wchar_t felder
//auch char* kann ein iterator sein
//beispiel:
//auto ia = iterator_access(std::vector const &);			//der vector muss länger leben als das erzeugt _iterator_access
//auto ia = iterator_access(std::string && );				//die lebenszeit eines rvalue wird verlängert, sonst würde mit kaputten itrerartoren gearbeitet
//auto ia = iterator_access(T *begin, T * end);				//iteratoren
//auto ia = iterator_access(char const *begin, size_t len);	//pointer  und länge des arrays
//auto ia = iterator_access( coantainer_type const & );		//viele conatiner, die begin() und end() haben e.g. std::basis_string
//auto ia = iterator_access( CString );//fuer die nutzung der MFC klasse CString muss altstr.h vor diesem header in der includereihenfolge stehen

#include "char_helper.h"

#include <memory>
#include <functional> //lambda als any missbrauchen

#pragma push_macro("max")
#undef max
#pragma push_macro("min")
#undef min

#if _HAS_CXX17
#	define _WS_DEPECATED_(message) [[deprecated( #message )]]
#else
#	define _WS_DEPECATED_(message)
#endif
/*//nur um breakpoints an auf shared_ptr machen zu koennen
namespace WS
{
	template<class _Ty> class debugshared_ptr//fuer breakpoint in dtor, zum testen
		: public std::shared_ptr<_Ty>
	{	
	public:
		using base_t = std::shared_ptr<_Ty>;
		using base_t::base_t;

		debugshared_ptr( debugshared_ptr const & ) = default;
		debugshared_ptr( debugshared_ptr && ) = default;
		debugshared_ptr & operator=( debugshared_ptr const & ) = default;
		debugshared_ptr & operator=( debugshared_ptr && ) = default;

		~debugshared_ptr()
		{
			if(this->unique() )
				int i=5;
		}
	};
}
*/
namespace WS
{
	using std::begin;
	using std::end;


	template<typename iterator_type> struct _iterator_access
	{
		using iterator_t = iterator_type;
		using value_t = std::decay_t<decltype(*std::declval<iterator_t>())>;
		iterator_t	first;
		iterator_t	last;//eigentlich last+1, also end nach iteratorlogik
		std::function<void(void)> rvalue_lifetime_extender;//statt std::any. die function wird nie aufgerufen. das shared-ptr objekt als capture parameter ist das wichtige
		~_iterator_access(){};
		_iterator_access(){ init_member(); }
		_iterator_access( _iterator_access const & ) = default; 
		_iterator_access( iterator_t first, iterator_t last )				: first( first ), last( last ){}
		_iterator_access( iterator_t first, iterator_t last, std::function<void(void)> rvalue_lifetime_extender ) : first( first ), last( last ), rvalue_lifetime_extender(rvalue_lifetime_extender){}
		_iterator_access( iterator_t first, size_t items )					: first( first ), last( first+items ){}
		_iterator_access( char * first) : _iterator_access<iterator_t>( first, stringlen(first)){}
		_iterator_access( wchar_t * first) :_iterator_access<iterator_t>( first, stringlen(first)){}
		_iterator_access( char const * first) : _iterator_access<iterator_t>( first, stringlen(first)){}
		_iterator_access( wchar_t const * first) : _iterator_access<iterator_t>( first, stringlen(first)){}

		_iterator_access& operator+=(_iterator_access const & r)
		{
			if( this->empty() )
				*this = r;
			else if( r.empty() == false )
			{
				if( this->end() != r.begin() )
					throw std::invalid_argument( __FUNCTION__ " r.begin() != end()" );
				else
					this->end() = r.end();
			}
		}
		_iterator_access operator+(_iterator_access const & r)
		{
			return _iterator_access{*this} += r;
		}
		_iterator_access& append(_iterator_access const & r)
		{
			return operator+=( r );
		}

		operator _iterator_access<iterator_t const>() const
		{
			return _iterator_access<iterator_t const>( *this );
		}
		auto const &	begin()const{return first;}
		auto const &	end()const{return last;}
		auto &			begin(){return first;}
		auto &			end(){return last;}
		_WS_DEPECATED_(deprecated: dont use this. use begin() end()) auto			GetNextEntry()//dont use this, its historic
		{
			auto retvalue = begin();
			if( begin()!=end())
				++begin();
			return retvalue;
		}

		size_t	len() const{return last-first;}
		bool	empty() const{return first==last;}
		operator bool() const {return empty()==false;}
		bool operator !() const {return empty();}
		operator iterator_t() const{ return begin();}

		template<typename other_iterator_t> int cmp( _iterator_access<other_iterator_t> const & r ) const
		{
			auto il = this->begin();
			auto ir = r.begin();

			for(;;)
			{
				if( il==this->end() && ir==r.end() )
					break;//do gleich
				if( il==this->end() )
					return -1;
				if( ir==r.end() )
					return 1;

				if( *il < *ir )
					return -1;
				if( *ir < *il )
					return 1;
				++il;++ir;
			}
			return 0;
		}
		template<typename other_iterator_t> bool operator==( other_iterator_t const & r ) const {return cmp(r)==0;}
		template<typename other_iterator_t> bool operator!=( other_iterator_t const & r ) const {return cmp(r)!=0;}
		template<typename other_iterator_t> bool operator<( other_iterator_t const & r ) const {return cmp(r)<0;}
		template<typename other_iterator_t> bool operator>( other_iterator_t const & r ) const {return cmp(r)>0;}
		template<typename other_iterator_t> bool operator<=( other_iterator_t const & r ) const {return !(cmp(r)>0);}
		template<typename other_iterator_t> bool operator>=( other_iterator_t const & r ) const {return !(cmp(r)<0);}

		auto left( size_t chars ) const
		{
			chars = std::min(chars,len());//include <alogrithm>
			return _iterator_access( begin(), begin()+chars );
		}
		auto right( size_t chars ) const
		{
			chars = std::min(chars,len());//include <alogrithm>
			return _iterator_access( end()-chars, end() );
		}
		auto find( value_t const & value )
		{
			auto iter = begin();
			while( iter!=end() && *iter != value )
				++iter;
			return iter;
		}
		auto find( value_t const & value ) const
		{
			auto iter = begin();
			while( iter!=end() && *iter != value )
				++iter;
			return iter;
		}
		template<typename other_itherator_t>auto findoneof( _iterator_access<other_itherator_t> const & elemente ) 
		{
			auto iter = begin();
			while( iter!=end() && elemente.find(*iter )==elemente.end() )
				++iter;
			return iter;
		}
		template<typename other_itherator_t>auto findoneof( _iterator_access<other_itherator_t> const & elemente ) const
		{
			auto iter = begin();
			while( iter!=end() && elemente.find(*iter )==elemente.end() )
				++iter;
			return iter;
		}
		auto reversefind( value_t const & value )
		{
			auto iter = end();
			while( iter--!=begin()() && *iter != value ){}
			return iter;
		}
		auto reversefind( value_t const & value ) const
		{
			_reverse_iterator_acces()
			auto iter = end();
			while( iter--!=begin() && *iter != value ){}
			return iter;
		}
		template<typename other_itherator_t>auto reversefindoneof( _iterator_access<other_itherator_t> const & elemente ) 
		{
			auto iter = end();
			while( iter--!=begin() && elemente.find(*iter )==elemente.end() ){}
			return iter;
		}
		template<typename other_itherator_t>auto reversefindoneof( _iterator_access<other_itherator_t> const & elemente ) const
		{
			auto iter = end();
			while( iter--!=begin() && elemente.find(*iter )==elemente.end() ){}
			return iter;
		}

		auto reverse_iterator_acces( )
		{
			return _iterator_access<std::reverse_iterator<iterator_t>>( std::reverse_iterator<iterator_t>(end()), std::reverse_iterator<iterator_t>(begin()) );
		}

	private:
		template<bool ispointer = std::is_pointer_v<iterator_t>> void init_member()
		{
			first = nullptr;
			last = nullptr;
		}
		template<> void init_member<false>()
		{
			last = first;
		}
	};


	template<typename iterator_t, typename container_t> inline auto iterator_access( iterator_t first, iterator_t last, std::shared_ptr<container_t> && container ) { return _iterator_access<iterator_t>( first, last, std::move(container) ); }
	template<typename iterator_t> inline auto iterator_access( iterator_t first, iterator_t last){ return _iterator_access<iterator_t>( first, last ); }
	template<typename iterator_t> inline auto iterator_access( iterator_t first, iterator_t last, std::function<void(void)> rvalue_lifetime_extender){ return _iterator_access<iterator_t>( first, last, rvalue_lifetime_extender ); }
	template<typename iterator_t> inline auto iterator_access( iterator_t first, size_t len){ return _iterator_access<iterator_t>( first, len ); }
	//komfort fuer nullterminierte zeichenketten
	inline auto iterator_access( char * first){ return _iterator_access<char *>( first, stringlen(first)); }
	inline auto iterator_access( wchar_t * first){ return _iterator_access<wchar_t *>( first, stringlen(first)); }
	inline auto iterator_access( char const * first){ return _iterator_access<char const *>( first, stringlen(first)); }
	inline auto iterator_access( wchar_t const * first){ return _iterator_access<wchar_t const *>( first, stringlen(first)); }
	//komfort fuer klassen die begin() unterstützen
	template<class container_t> inline auto iterator_access( container_t const & r )
	{
		return iterator_access( begin(r), end(r) );
	}
	template<class container_t> inline auto iterator_access( container_t & r )
	{
		return iterator_access( begin( r ), end( r ) );
	}

	template<class container_t> auto iterator_access( container_t && r )//lebensverlängerung fuer rvalues, damit die iteratoren nicht ins leere laufen. erster versuch war mit (std/boost)::any. waere besser, aber sind nicht immer verfügbar C++17
	{
		auto as_shared_ptr = std::make_shared<container_t>( std::move( r ) );
		return iterator_access( begin( *as_shared_ptr ), end( *as_shared_ptr ), std::function<void(void)>([as_shared_ptr](){}) );
	}

	 template<typename char_t, size_t size> inline auto array_iterator_access( char_t (& Array)[size] ){ return _iterator_access<char_t*>( Array, size ); }
}

class TraceClass;
template<typename char_t> void tracevalue( TraceClass const &t, std::remove_const_t<char_t> const & value );
//void tracevalue( TraceClass const &t, wchar_t const & value );
template<typename char_t> void tracevalue( TraceClass const &t, WS::_iterator_access<char_t> const & value )
{
	for( auto & ch : value )
		tracevalue( t, ch );
}

template<typename char_t> bool is_white_space( char_t ch );
template<> inline bool is_white_space<char>( char ch )
{
	//return isblank( unsigned char(ch) );
	return isspace(unsigned char(ch));
}
template<> inline bool is_white_space<wchar_t>( wchar_t ch )
{
	//return iswblank( ch );
	return iswspace(ch);
}
template<typename iterator_t> WS::_iterator_access<iterator_t> trimleft( WS::_iterator_access<iterator_t> const & value, bool (*eater)( typename WS::_iterator_access<iterator_t>::value_t ) = is_white_space<WS::_iterator_access<iterator_t>::value_t> )
{
	auto retvalue = value;
	while( retvalue.begin()!=retvalue.end() && eater(*retvalue.begin()) )
		++retvalue.begin();
	return retvalue;
}
template<typename iterator_t> WS::_iterator_access<iterator_t> trimright( WS::_iterator_access<iterator_t> const & value, bool (*eater)( typename WS::_iterator_access<iterator_t>::value_t ) = is_white_space<WS::_iterator_access<iterator_t>::value_t> )
{	
	auto retvalue = value;
	while( retvalue.begin()!=retvalue.end() && eater(*(retvalue.end()-1)) )
		--retvalue.end();
	return retvalue;
}
template<typename iterator_t> WS::_iterator_access<iterator_t> trim( WS::_iterator_access<iterator_t> const & value, bool (*eater)( typename WS::_iterator_access<iterator_t>::value_t ) = is_white_space<WS::_iterator_access<iterator_t>::value_t> )
{
	return trimright( trimleft(value,eater), eater );
}

#pragma pop_macro("max")
#pragma pop_macro("min")
#endif //__WP_ITERATOR_ACCESS__