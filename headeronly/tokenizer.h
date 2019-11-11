//#pragma once
#define __WP_TOKENIZER__


#include "iterator_access.h"

//tokenize und tokenizer gibt ueber WS::_iterator_access zugriff auf token, ohne die daten (i.d.r. texte) umkopieren zu müssen, oder ueberhaupt ein new zu machen. 
//ueberspringt leere tokens. der char_type des delimiter muss zum char_type der tokens und token passen. also nicht char und wchar_t mischen
//einfachste methode ueber tokenizer
//	for( auto & token : WS::tokenizer(L"   hallo,dritte |, welt ", L", ;|") )
//		traceln( token );
//gibt 'hallo\ndritte\nwelt' aus
//
//beispiel fuer tokenize
//	auto tokens="hallo,dritte  ,;, ,      welt";
//	for( auto token=WS::tokenize(tokens,", ;"); token; token=WS::tokenize(tokens,", ;", token) )
//		traceln( token );
//oder
//	WS::_iterator_access<wchar_t const> token;
//	while(token=WS::tokenize( L"   hallo,dritte |, welt ", L", ;|", token ) )
//		traceln( token );
namespace WS
{
	template<typename iterator_t, typename delimiter_iterator_t> _iterator_access<iterator_t> tokenize( 
		_iterator_access<iterator_t> const & tokens, 
		_iterator_access<delimiter_iterator_t> const & delimiterlist, 
		_iterator_access<iterator_t> const & lasttoken=_iterator_access<iterator_t>() )
	{
		using value_t = _iterator_access<iterator_t>::value_t;
		using delimiter_value_t = _iterator_access<delimiter_iterator_t>::value_t;
		static_assert( std::is_same<std::remove_const_t<value_t>, std::remove_const_t<delimiter_value_t>>::value, "*iterator_t muss gleich sein, nur const ist egal" );
		//if( lasttoken.end() && (lasttoken.end()<tokens.begin()||tokens.end()<lasttoken.end()) )
		//   throw std::invalid_argument (__FUNCTION__);

		auto fnDelimiter = [&](iterator_t const & iter)
		{
			return std::find(delimiterlist.begin(), delimiterlist.end(), *iter) != delimiterlist.end();
		};

		//wo mit suche beginnen?
		auto iter = tokens.begin();
		if( lasttoken.begin()!=lasttoken.end() )
			//hier geht es weiter, sonst am anfang
			iter = lasttoken.end();

		_iterator_access<iterator_t> retvalue( iter, iter );
		//skip delimiter
		while( iter!=tokens.end() && fnDelimiter(iter) )
			retvalue.last = retvalue.first = ++iter;
		//next token
		while( iter!=tokens.end() && fnDelimiter(iter)==false )
			retvalue.last = ++iter;

		return retvalue;
	}

	template<typename tokens_t, typename delimiter_t, typename lasttoken_t>
	lasttoken_t tokenize( tokens_t tokens
				  , delimiter_t delimiter
				  , lasttoken_t lasttoken )
	{
		using token_iterator_t = lasttoken_t::iterator_t;

		return tokenize(
				_iterator_access<token_iterator_t>(tokens)
				, iterator_access(delimiter)
				, lasttoken );
	}
	template<typename tokens_t, typename delimiter_t>
	auto tokenize( tokens_t tokens, delimiter_t delimiter )
	{
		using iterator_t = decltype(iterator_access(tokens))::iterator_t;
		return tokenize(
				iterator_access(tokens)
				, iterator_access(delimiter)
				, _iterator_access<iterator_t>() );
	}

	template<typename tokens_t, typename delimiter_t>
	struct _tokenizer
	{
		using iterator_t = typename decltype(iterator_access( std::declval<tokens_t>() ))::iterator_t;
		using delimiter_iterator_t = typename decltype(iterator_access(std::declval<delimiter_t>()))::iterator_t;

		_iterator_access<iterator_t> tokens;
		_iterator_access<delimiter_iterator_t> delimiter;

		_tokenizer( tokens_t && tokens, delimiter_t && delimiter ) 
			: tokens(iterator_access(std::forward<tokens_t>(tokens)))
			, delimiter(iterator_access(std::forward<delimiter_t>(delimiter)))
		{
			//auto x = iterator_access( std::forward<tokens_t>( tokens ) );
			//tokens = std::move(x);
			//delimiter = iterator_access(std::forward<delimiter_t>(delimiter));
		}

		//VORSICHT ++end() == begin()
		struct iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::remove_cv_t<tokens_t>;
			using pointer = std::remove_reference_t<tokens_t> *;
			using reference = tokens_t &;
			using difference_type = void;//macht doch nur bei random sinn, oder?? aber ohne funktioniert std::iterator_traits nicht. dafuer muessen diese 5 vorhanden sein

			_iterator_access<iterator_t> lasttoken;
			_tokenizer * tokenizer = nullptr;
			iterator(){}
			iterator( _tokenizer & tokenizer ) : tokenizer(&tokenizer){}

			iterator& operator++()
			{
				this->lasttoken = tokenize( tokenizer->tokens, tokenizer->delimiter, this->lasttoken );
				if( this->lasttoken.len()==0 )
					this->lasttoken = _iterator_access<iterator_t>();
				
				return *this;
			}
			iterator operator++(int)
			{
				auto retvalue = *this;
				operator++();
				return retvalue;
			}
			bool operator==( iterator const & r )
			{
				return this->tokenizer == r.tokenizer 
						&& ( this->tokenizer==nullptr || this->lasttoken == r.lasttoken );
			}
			bool operator!=( iterator const & r )
			{
				return !operator==(r);
			}

			_iterator_access<iterator_t> & operator*()
			{
				return lasttoken;
			}
			_iterator_access<iterator_t> * operator->()
			{
				return &lasttoken;
			}
		};
		iterator begin()
		{
			iterator retvalue( *this );
			return ++retvalue;
		}
		iterator end()
		{
			return iterator( *this );
		}
	};

	template<typename tokens_t, typename delimiter_t>
	//zur benutzung in einer range based for schleife
	auto tokenizer( tokens_t && tokens , delimiter_t && delimiter )
	{
		return _tokenizer<tokens_t, delimiter_t>( std::forward<tokens_t>(tokens), std::forward<delimiter_t>(delimiter) );
	}
}
