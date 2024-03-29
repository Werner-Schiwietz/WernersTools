//#pragma once
//Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

#ifndef __WP_ITERATOR_ACCESS__
#define __WP_ITERATOR_ACCESS__

//WS::iterator_access haelt zwei iteratoren und kann z.b. direkt fuer range based for benutzt werden
//konsturiert aus allem was begin(T) und end(T) kann
//zusaetzlich nullterminierte char bzw wchar_t felder
//auch char* kann ein iterator sein
//beispiel:
//auto ia = iterator_access(std::vector const &);			//der vector muss l�nger leben als das erzeugt _iterator_access
//auto ia = iterator_access(std::string && );				//die lebenszeit eines rvalue wird verl�ngert, sonst w�rde mit kaputten itrerartoren gearbeitet
//auto ia = iterator_access(T *begin, T * end);				//iteratoren
//auto ia = iterator_access(char const *begin, size_t len);	//pointer  und l�nge des arrays
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
#if _HAS_CXX17 
#	include <any>
	using rvalue_lifetime_extender_t = std::any;
	template<typename container_t> rvalue_lifetime_extender_t extend( std::shared_ptr<container_t> ptr ) { return rvalue_lifetime_extender_t( ptr ); }
	bool has_value( rvalue_lifetime_extender_t const & r ) { return r.has_value(); }
#else
#	include <functional> //lambda als any missbrauchen
	using rvalue_lifetime_extender_t = std::function<void(void)>;//die function wird nie aufgerufen. das shared-ptr objekt als capture parameter ist das wichtige
	template<typename container_t> rvalue_lifetime_extender_t extend( std::shared_ptr<container_t> ptr ) { return rvalue_lifetime_extender_t( [ptr]() {} ); }
	bool has_value( rvalue_lifetime_extender_t const & r ) { return (bool)r; }
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
	nullptr_t begin(nullptr_t);//sonst gibt es ohne include <atlstr.h> compilerfehler bei using ::WS::begin;
	nullptr_t end(nullptr_t);
	template<typename iterator_type> struct _iterator_access
	{
		using iterator_t = iterator_type;
		using value_t = std::decay_t<decltype(*std::declval<iterator_t>())>; 
		iterator_t	first;
		iterator_t	last;//eigentlich last+1, also end nach iteratorlogik
		rvalue_lifetime_extender_t rvalue_lifetime_extender;//statt std::any. die function wird nie aufgerufen. das shared-ptr objekt als capture parameter ist das wichtige
		~_iterator_access(){};
		_iterator_access(){ init_member(); }
		_iterator_access( _iterator_access const & ) = default; 
		_iterator_access( iterator_t first, iterator_t last )				: first( first ), last( last ){}
		_iterator_access( iterator_t first, iterator_t last, rvalue_lifetime_extender_t rvalue_lifetime_extender ) : first( first ), last( last ), rvalue_lifetime_extender(rvalue_lifetime_extender){}
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
			return *this;
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
		bool operator==(bool value) const {return operator bool()==value;}
		bool operator!=(bool value) const {return operator bool()!=value;}
		operator iterator_t() const{ return begin();}

		template<typename l_iterator_t,typename r_iterator_t> static bool lth(l_iterator_t const &l, r_iterator_t const & r) 
		{
			return *l < *r;
		}
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

				if( lth(il,ir) )
					return -1;
				if( lth(ir,il) )
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
			return _iterator_access{ begin(), begin()+chars, this->rvalue_lifetime_extender };
		}
		auto right( size_t chars ) const
		{
			chars = std::min(chars,len());//include <alogrithm>
			return _iterator_access{ end()-chars, end(), this->rvalue_lifetime_extender };
		}
		auto mid( size_t skipped_chars ) const
		{
			return right( len()-skipped_chars );
		}
		auto mid( size_t skipped_chars, size_t chars ) const
		{
			return mid( skipped_chars ).left( chars );
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
			//_reverse_iterator_acces()
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
	template<typename _iterator_access_t,typename buffer_t=std::basic_string<typename _iterator_access_t::value_t>> struct appender
	{
		_iterator_access_t	value;
		buffer_t			buffer;

		appender(){}
		appender(_iterator_access_t value):value(value){}
		void append( _iterator_access_t toadd )//per copy, keine & oder && const &
		{
			if( toadd.empty() )
				return;

			if( this->buffer.empty()==false )
				this->buffer.append( toadd.begin(), toadd.end() );
			else if( this->value.empty() )
				this->value = toadd;
			else if( this->value.end() == toadd.begin() )
				this->value.end() = toadd.end();
			else 
			{	//ver�nderung -> umkopieren
				usebuffer();
				this->buffer.append( toadd.begin(), toadd.end() );
			}
		}
		void append( typename _iterator_access_t::value_t addone )
		{
			usebuffer();
			this->buffer += addone;		
		}
		WS::_iterator_access<typename _iterator_access_t::value_t const*> move();//kann einmal abgeholt werden, danach ist ggf das ergebnis leer. grund: ggf wird der buffer per move ins ergebnis geschoben.

		bool empty() const { return !value && buffer.empty(); }//ist kein text gelesen
		bool allocated() const { return !buffer.empty(); }//wurde text umkopiert 
	protected:
		void usebuffer()
		{
			if( this->buffer.empty() )
				if( this->value )
				{
					this->buffer = buffer_t{this->value.begin(),this->value.end()};
					this->value.end() = this->value.begin();//value leeren
				}
		}
	};	

	template<typename iterator_t, typename container_t> inline auto iterator_access( iterator_t first, iterator_t last, std::shared_ptr<container_t> && container ) { return _iterator_access<iterator_t>( first, last, std::move(container) ); }
	template<typename iterator_t> inline auto iterator_access( iterator_t first, iterator_t last){ return _iterator_access<iterator_t>( first, last ); }
	template<typename iterator_t> inline auto iterator_access( iterator_t first, iterator_t last, rvalue_lifetime_extender_t rvalue_lifetime_extender){ return _iterator_access<iterator_t>( first, last, rvalue_lifetime_extender ); }
	template<typename iterator_t> inline auto iterator_access( iterator_t first, size_t len){ return _iterator_access<iterator_t>( first, len ); }
	//komfort fuer nullterminierte zeichenketten
	inline auto iterator_access( char * first){ return _iterator_access<char *>( first, stringlen(first)); }
	inline auto iterator_access( wchar_t * first){ return _iterator_access<wchar_t *>( first, stringlen(first)); }
	inline auto iterator_access( char const * first){ return _iterator_access<char const *>( first, stringlen(first)); }
	inline auto iterator_access( wchar_t const * first){ return _iterator_access<wchar_t const *>( first, stringlen(first)); }
	//komfort fuer klassen die begin() unterst�tzen
	template<class container_t> inline auto iterator_access( container_t const & r )
	{
		using ::std::begin;
		using ::std::end;
		using ::WS::begin;
		using ::WS::end;

		return iterator_access( begin(r), end(r) );
	}
	template<class container_t> inline auto iterator_access( container_t & r )
	{
		using ::std::begin;
		using ::std::end;
		using ::WS::begin;
		using ::WS::end;

		return iterator_access( begin( r ), end( r ) );
	}

	template<class container_t> auto iterator_access( container_t && r )//lebensverl�ngerung fuer rvalues, damit die iteratoren nicht ins leere laufen. erster versuch war mit (std/boost)::any. waere besser, aber sind nicht immer verf�gbar C++17
	{
		using ::std::begin;
		using ::std::end;
		using ::WS::begin;
		using ::WS::end;

		auto as_shared_ptr = std::make_shared<container_t>( std::move( r ) );
		return iterator_access( begin( *as_shared_ptr ), end( *as_shared_ptr ), extend(as_shared_ptr) );
	}

	template<typename char_t, size_t size> inline auto array_iterator_access( char_t (& Array)[size] ){ return _iterator_access<char_t*>( Array, size ); }

	template<typename _iterator_access_t,typename buffer_t> 
	WS::_iterator_access<typename _iterator_access_t::value_t const*> appender<_iterator_access_t,buffer_t>::move()
	{
		using ret_t = WS::_iterator_access<typename _iterator_access_t::value_t const*>;
		ret_t retvalue;
		if( this->buffer.empty() )
		{
			auto b = &*this->value.begin();
			auto e = b+this->value.len();
			return ret_t{b,e,this->value.rvalue_lifetime_extender};
		}
		{
			auto temp = iterator_access( std::move( this->buffer ) );//rvalue_lifetime_extender anlegen
			auto b = &*temp.begin();
			auto e = b+temp.len();//*temp.end() geht nicht

			return ret_t{b,e,temp.rvalue_lifetime_extender};
		}
	}

}

namespace WS
{
	template<typename iterator_t> struct find_replace_t
	{
		_iterator_access<iterator_t> found;
		_iterator_access<iterator_t> newvalue;

		operator bool() const { return found && newvalue;}
		bool operator !() const { return !operator bool();}
	};
	template<typename iterator_t> struct find_replace_char
	{
		_iterator_access<iterator_t> found;
		typename _iterator_access<iterator_t>::value_t newvalue;

		operator bool() const { return found;}
		bool operator !() const { return !operator bool();}
	};
	template<typename iterator_t, typename fn_ret_t>  _iterator_access<iterator_t> find_replace( _iterator_access<iterator_t> parse, std::function<fn_ret_t(WS::_iterator_access<iterator_t>)> fn_find )
	{
		using retvalue_t = _iterator_access<iterator_t>;
		using value_t = typename _iterator_access<iterator_t>::value_t;
		using buffer_t = std::basic_string<value_t>;//funktioniert nur, wenn

		buffer_t char_buffer;

		_iterator_access<iterator_t> toparse=parse;
		while( auto replace = fn_find( toparse ) )
		{
			char_buffer.append( toparse.begin(), replace.found.begin() );
			if constexpr ( std::is_same<fn_ret_t,find_replace_t<iterator_t>>::value )
				char_buffer.append( replace.newvalue.begin(), replace.newvalue.end() );
			else if constexpr ( std::is_same<fn_ret_t,find_replace_char<iterator_t>>::value )
				char_buffer +=  replace.newvalue;
			else
				static_assert(false, "fn_find liefert falsches ergebnis" );

			toparse.begin() = replace.found.end();
		}


		if( char_buffer.empty() )
			return parse;

		char_buffer.append( toparse.begin(), toparse.end() );

		auto temp = iterator_access( std::move( char_buffer ) );//rvalue_lifetime_extender anlegen
		auto b = &*temp.begin();
		auto e = b+temp.len();//*temp.end() geht nicht
		return retvalue_t{b,e,temp.rvalue_lifetime_extender};
	}
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