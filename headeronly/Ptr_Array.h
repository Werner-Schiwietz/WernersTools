#pragma once 

#include <stdexcept>
#include <basetsd.h>	//INT_PTR

#ifndef ASSERT
#	define UNDEF_ASSERT
#	define ASSERT(x) ((void)0)
#endif

#include "WP_helper.h"


namespace WS
{
	template<bool isconst,typename T> struct _getpointertype;
	template<typename T> struct _getpointertype<true,T>
	{
		typedef typename WS::puretype<T>::type const * type;
	};
	template<typename T> struct _getpointertype<false,T>
	{
		typedef typename WS::puretype<T>::type * type;
	};

	template<bool isconst,typename T> struct _getreferenztype;
	template<typename T> struct _getreferenztype<true,T>
	{
		typedef typename WS::puretype<T>::type const & type;
	};
	template<typename T> struct _getreferenztype<false,T>
	{
		typedef typename WS::puretype<T>::type & type;
	};
	template<typename T> struct getreferenztype
	{
		typedef typename WS::_getreferenztype<WS::is_const_pointer<T>::value,T>::type type;
	};


		//mit nested iterator bekomme ich std::_Unchecked nicht hin
		template<typename element_t> class _ptrarray_iterator
		{
		public:
			//damit iterator_traits funktioniert brauchen wird folgende typen
			//difference_type  value_type pointer reference iterator_category 
			typedef std::random_access_iterator_tag iterator_category;
			typedef element_t value_type;
			typedef std::ptrdiff_t difference_type;
			typedef difference_type distance_type;	// retained
			typedef element_t* pointer;
			typedef element_t& reference;

			typedef pointer _Unchecked_type;

		private:
			pointer first = nullptr;
			pointer last = nullptr;//end, also eigentlich last+1
			pointer akt = nullptr;
		public:
			_ptrarray_iterator()=default;
			_ptrarray_iterator(  pointer akt, pointer _begin, pointer _end ) : akt(akt), first(_begin), last(_end){}


			pointer operator->() const
			{
				if( this->akt < this->first || this->akt >= this->last )
					throw std::out_of_range( __FUNCTION__ " out of range" );

				return this->akt;
			}
			reference operator*() const
			{
				return *operator->();
			}

			_ptrarray_iterator& operator++()
			{
				//if( _akt==_end )
				//	throw std::out_of_range( __FUNCTION__ " out of range" );
				++this->akt;
				return *this;
			}
			_ptrarray_iterator operator++(int)
			{
				_ptrarray_iterator retvalue = *this;
				++(*this);
				return retvalue ;
			}
			_ptrarray_iterator& operator--()
			{
				--this->akt;
				return *this;
			}
			_ptrarray_iterator operator--(int)
			{
				_ptrarray_iterator retvalue = *this;
				--(*this);
				return retvalue ;
			}

			_ptrarray_iterator& operator-=( int offset )
			{
				this->akt -= offset;
				return *this;
			}
			_ptrarray_iterator operator-( int offset ) const
			{
				_ptrarray_iterator retvalue( *this );
				retvalue -= offset;
				return retvalue ;
			}
			_ptrarray_iterator& operator+=( int offset )
			{
				this->akt += offset;
				return *this;
			}
			_ptrarray_iterator operator+( int offset ) const
			{
				_ptrarray_iterator retvalue( *this );
				retvalue += offset;
				return retvalue ;
			}

			int operator-( _ptrarray_iterator const & r ) const 
			{
				return this->akt - r.akt;
			}

			bool operator<( _ptrarray_iterator const & r ) const
			{
				return this->akt < r.akt;
			}

			bool operator==( _ptrarray_iterator const & r ) const
			{
				return this->akt==r.akt;
			}
			bool operator!=( _ptrarray_iterator const & r ) const
			{
				return !operator==(r);
			}

			_Unchecked_type _Unchecked() const
			{	// make an unchecked iterator
				if( this->akt < this->first || this->akt > this->last )//end liefert zwar keine speicheradresse auf die man zugreifen kann, aber brauchen die adresse als abbruchbedingung
					throw std::out_of_range( __FUNCTION__ " invalid iterator" );

				return this->akt;
			}

		};
}//namespace WS

namespace std
{
	//std-funktionen wie z.b. std::accumulate benutzen ggf unchecked iteratoren und koennen dadurch, je nach iterator, sehr viel schneller sein
	template<typename T> inline auto _Unchecked(typename WS::_ptrarray_iterator<T> _Iter)
	{	//ptr_array<T>::iterator::_Unchecked_type
		return _Iter._Unchecked();
	}
}

namespace WS
{
	//diese klasse allokiert keinen speicher und gibt auch keinen speicher frei
	//sie dient dem kontrollierten, ueberwachten zugriff auf speicherblöcke
	//usage siehe UT_ptr_array
	template <typename pointer_t>class ptr_array
	{
	public:
		typedef pointer_t											pointer_type;
		typedef typename std::remove_pointer_t<pointer_type>		element_type;
		typedef typename element_type &								element_ref;

		static_assert( std::is_pointer<pointer_type>::value, "es muss schon ein pointer sein" );
	private:
		void*	membegin		= nullptr;
		void*	pointer			= nullptr;
		size_t	elementcount	= 0;
		size_t	bytelen			= 0;

	public:
		size_t					ElementCount() const
		{
			return elementcount;
		}
		size_t					ByteLen() const
		{
			return bytelen;
		}
		size_t					UsedByteLen() const
		{
			return (__int8*)pointer - (__int8*)membegin;
		}


		operator	pointer_type()
		{
			//ASSERT(pointer==nullptr || ElementCount() );//dass ist oft gewollt, um y.b. end()-position zu bestimmen
			return (pointer_type)pointer;
		}
		operator	std::add_const_t<pointer_type> () const
		{
			//ASSERT(pointer==nullptr || ElementCount() );//dass ist oft gewollt, um y.b. end()-position zu bestimmen
			return (pointer_type)pointer;
		}
		void* memorystartpos() const
		{
			return membegin;
		}

		pointer_type operator->() 
		{
			if( ElementCount() == 0 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return (pointer_type)pointer;
		}
		pointer_type operator->()  const
		{
			if( ElementCount() == 0 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return (pointer_type)pointer;
		}

		element_ref				operator*() 
		{
			if( ElementCount() == 0 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return *(pointer_type)pointer;
		}
		element_type const &	operator*() const
		{
			if( ElementCount() == 0 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return *(pointer_type)pointer;
		}
		element_ref				operator[]( size_t index )
		{
			if( index >= elementcount )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ((pointer_type)pointer)[index];
		}
		element_type const&		operator[]( size_t index ) const
		{
			if( index >= elementcount )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ((pointer_type)pointer)[index];
		}
		element_ref				operator[]( INT_PTR index )
		{
			if( index>=static_cast<INT_PTR>(elementcount) || static_cast<INT_PTR>(elementcount)<0 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			if( index < 0 && UsedByteLen()/sizeof(element_type) < static_cast<UINT_PTR>(-index) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ((pointer_type)pointer)[index];
		}
		element_type const&		operator[]( INT_PTR index ) const
		{
			if( index>=static_cast<INT_PTR>(elementcount) || static_cast<INT_PTR>(elementcount)<0 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			if( index < 0 && -index >= UsedByteLen()/sizeof(element_type) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ((pointer_type)pointer)[index];
		}
		//element_ref				operator[]( INT_PTR index )//INT_PTR um warnings bei verwender zu vermeiden, dieser operator sollte entfernt werden!
		//{
		//	return operator[]( static_cast<size_t>(index) );
		//}
		//element_type const&		operator[]( INT_PTR index ) const //INT_PTR um warnings bei verwender zu vermeiden, dieser operator sollte entfernt werden!
		//{
		//	return operator[]( static_cast<size_t>(index) );
		//}

		template<size_t ElementCount>ptr_array( element_type (&datenblock)[ElementCount] )
			: pointer( (void*)datenblock )
			, elementcount( ElementCount )
			, bytelen( elementcount * sizeof(element_type) )
			, membegin( (void*)datenblock )
		{
		}
		ptr_array() {}
		ptr_array( pointer_type datenblock, size_t element_count, pointer_type datenblockstartadr=nullptr )
			: pointer( (void*)datenblock )
			, membegin( datenblockstartadr ? (void*)datenblockstartadr : (void*)datenblock )
			, elementcount( element_count )
			, bytelen( elementcount * sizeof(element_type) )
		{
			if( membegin > pointer )//die startposition des speichers darf nicht hinter der lese/schreibposition liegen
				throw std::invalid_argument( __FUNCTION__ " pointer invalid" );
		}
		ptr_array( ptr_array const & ) = default;
		template<typename otherpointer> ptr_array( ptr_array<otherpointer> const & r )
			: pointer( ( const_cast<void*>( static_cast<void const*>( (typename ptr_array<otherpointer>::pointer_type)r ) ) ) ) 
			, bytelen( r.ByteLen() )
			, elementcount( r.ByteLen() / sizeof(element_type) )
			, membegin( const_cast<void*>( r.memorystartpos() ) )
		{

			static_assert( WS::is_const_pointer<otherpointer>::value==false || WS::is_const_pointer<pointer_type>::value, "U const* kann nicht zu T * konvertiert werden" );
		}

		ptr_array	operator+( size_t pointer_um_N_elemente_weiter_setzen ) const
		{
			if( pointer_um_N_elemente_weiter_setzen > elementcount )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ptr_array( ((pointer_type)pointer) + pointer_um_N_elemente_weiter_setzen, elementcount - pointer_um_N_elemente_weiter_setzen, (pointer_type)membegin );
		}
		ptr_array&	operator+=( size_t pointer_um_N_elemente_weiter_setzen ) 
		{
			return *this = operator+( pointer_um_N_elemente_weiter_setzen );
		}
		ptr_array	operator+( INT_PTR pointer_um_N_elemente_weiter_setzen ) const
		{
			if( pointer_um_N_elemente_weiter_setzen >= 0 )
				return operator+( static_cast<size_t>( pointer_um_N_elemente_weiter_setzen ) );
			else
				return operator-( static_cast<size_t>( -pointer_um_N_elemente_weiter_setzen ) );
		}
		ptr_array&	operator+=( INT_PTR pointer_um_N_elemente_weiter_setzen ) 
		{
			if( pointer_um_N_elemente_weiter_setzen >= 0 )
				return *this = operator+( static_cast<size_t>( pointer_um_N_elemente_weiter_setzen ) );
			else
				return *this = operator-( static_cast<size_t>( -pointer_um_N_elemente_weiter_setzen ) );
		}
		ptr_array&	operator+=( ptr_array<std::remove_const_t<element_type> *> const & append ) 
		{
			if( ElementCount() < append.ElementCount() )
				throw std::out_of_range( __FUNCTION__ " ziel hat zu wenig speicher" );
			memcpy( this->pointer, &*append, append.ByteLen() );
			operator+=( append.ElementCount() );
			return *this;
		}
		ptr_array&	operator+=( ptr_array<std::remove_const_t<element_type> const*> const & append ) 
		{
			if( ElementCount() < append.ElementCount() )
				throw std::out_of_range( __FUNCTION__ " ziel hat zu wenig speicher" );
			memcpy( this->pointer, &*append, append.ByteLen() );
			operator+=( append.ElementCount() );
			return *this;
		}

		ptr_array&	operator++()
		{
			if( elementcount == 0 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			++*((pointer_type*)&pointer);
			--elementcount;
			bytelen -= sizeof(element_type);
			return *this;
		}
		ptr_array	operator++(int)
		{
			ptr_array retvalue = *this;
			++*this;
			return retvalue;
		}

		ptr_array	operator-( size_t pointer_um_N_elemente_weiter_setzen ) const
		{
			//ASSERT( (__int8*)pointer >= (__int8*)membegin );//kann nie anders sein, sonst exception
			if( pointer_um_N_elemente_weiter_setzen > size_t((pointer_type)pointer - (pointer_type)membegin) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ptr_array( ((pointer_type)pointer) - pointer_um_N_elemente_weiter_setzen, elementcount + pointer_um_N_elemente_weiter_setzen, (pointer_type)membegin );
		}
		ptr_array&	operator-=( size_t pointer_um_N_elemente_weiter_setzen ) 
		{
			return *this = operator-( pointer_um_N_elemente_weiter_setzen );
		}
		ptr_array	operator-( INT_PTR pointer_um_N_elemente_weiter_setzen ) const
		{
			if( pointer_um_N_elemente_weiter_setzen >= 0 )
				return operator-( static_cast<size_t>( pointer_um_N_elemente_weiter_setzen ) );
			else
				return operator+( static_cast<size_t>( -pointer_um_N_elemente_weiter_setzen ) );
		}
		ptr_array&	operator-=( INT_PTR pointer_um_N_elemente_weiter_setzen ) 
		{
			if( pointer_um_N_elemente_weiter_setzen >= 0 )
				return *this = operator-( static_cast<size_t>( pointer_um_N_elemente_weiter_setzen ) );
			else
				return *this = operator+( static_cast<size_t>( -pointer_um_N_elemente_weiter_setzen ) );
		}

		ptr_array&	operator--()
		{
			if( (pointer_type)pointer < (pointer_type)membegin + 1 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			--*((pointer_type*)&pointer);
			++elementcount;
			bytelen += sizeof(element_type);
			return *this;
		}
		ptr_array	operator--(int)
		{
			ptr_array retvalue = *this;
			--*this;
			return retvalue;
		}

		using iterator = _ptrarray_iterator<std::remove_pointer_t<pointer_t>>;
		using const_iterator = _ptrarray_iterator<std::add_const_t<std::remove_pointer_t<pointer_t>>>;

		auto begin()
		{
			auto a = (pointer_t)pointer;
			auto b = (pointer_t)pointer;
			auto e = ((pointer_t)pointer) + ElementCount();
			return iterator( a,b,e );
		}
		auto begin() const
		{
			auto a = (pointer_t)pointer;
			auto b = (pointer_t)pointer;
			auto e = ((pointer_t)pointer) + ElementCount();
			return const_iterator( a,b,e );
		}
		auto end()
		{
			//auto b = (pointer_t)pointer;
			pointer_t b = (pointer_t)pointer;
			auto a = ((pointer_t)pointer) + ElementCount();
			auto e = ((pointer_t)pointer) + ElementCount();
			return iterator( a,b,e );
		}
		auto end() const
		{
			auto a = ((std::add_const_t<pointer_t>)pointer) + ElementCount();
			auto b = (std::add_const_t<pointer_t>)pointer;
			auto e = ((std::add_const_t<pointer_t>)pointer) + ElementCount();
			return const_iterator( a,b,e );
		}
	};

	//typsicher zugriff auf speicher, welcher als array eines datentyps betrachtet wird.
	//wird nur von Ptr_array<void*> bzw ptr_array<void const*> in der methode useas benutzt
	//veraenderungen auf die leseposition schlagen auf das referenzierte void*-Array durch
	template <typename element_t, typename ptr_void_t> class TRef //
	{
		static_assert( std::is_pointer<element_t>::value==false, "sollte kein pointer sein" );
	public:
		typedef ptr_void_t																					ptr_void_type;
		typedef element_t																					element_type;
		typedef typename WS::_getreferenztype<WS::is_const_pointer<ptr_void_t>::value,element_t>::type 		element_ref_type;
		typedef typename WS::_getpointertype<WS::is_const_pointer<ptr_void_t>::value,element_t>::type 		element_pointer_type;

		ptr_void_type & begin_ref;
		ptr_void_type & ptr_ref;
		size_t	&		bytelen_ref;
		size_t			ElementCount()
		{
			return bytelen_ref / sizeof(element_type);
		}
	public:
		~TRef()
		{
		}
		TRef( ptr_void_type & pvoid, size_t & bytelen, ptr_void_type & begin) : ptr_ref(pvoid), bytelen_ref(bytelen), begin_ref(begin)
		{
			if( begin_ref > ptr_ref )
				throw std::invalid_argument( __FUNCTION__ " pointer invalid" );
			if( bytelen < sizeof(element_type) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
		}
		TRef( TRef const & r ) = default;
		element_ref_type operator*()
		{
			if( bytelen_ref<sizeof( element_type ) || ptr_ref<begin_ref )
				throw std::out_of_range( __FUNCTION__ );

			return *(element_pointer_type)ptr_ref;
		}
		std::add_const_t<element_type> & operator*() const
		{
			if( bytelen_ref<sizeof( element_type ) || ptr_ref<begin_ref )
				throw std::out_of_range( __FUNCTION__ );

			return *(element_pointer_type)ptr_ref;
		}
		operator element_pointer_type()
		{
			if( bytelen_ref<sizeof( element_type ) || ptr_ref<begin_ref )
				throw std::out_of_range( __FUNCTION__ );

			return element_pointer_type(ptr_ref);
		}
		TRef& operator += ( size_t count )
		{
			if( bytelen_ref < count * sizeof(element_type) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );

			bytelen_ref -= count * sizeof( element_type );
			(*(element_type**)&ptr_ref) += count;

			return *this;
		}
		TRef& operator -= ( size_t count )
		{
			if( this->begin_ref > ((element_type*)ptr_ref) - count )
				throw std::out_of_range( __FUNCTION__ " index out of range" );

			bytelen_ref += count * sizeof( element_type );
			(*(element_type**)&ptr_ref) -= count;

			return *this;
		}
		TRef& operator ++()
		{
			if( bytelen_ref < sizeof(element_type) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );

			bytelen_ref -= sizeof(element_type);
			++(*(element_type**)&ptr_ref);

			return *this;
		}
		element_pointer_type operator ++(int)
		{
			element_pointer_type retvalue = (element_pointer_type) ptr_ref;
			++(*this);
			return retvalue;
		}
		TRef& operator --()
		{
			if( this->begin_ref > ((element_type*)ptr_ref) - 1 )
				throw std::out_of_range( __FUNCTION__ " index out of range" );

			bytelen_ref += sizeof(element_type);
			--(*(element_type**)&ptr_ref);

			return *this;
		}
		element_pointer_type operator --(int)
		{
			element_pointer_type retvalue = (element_pointer_type) ptr_ref;
			--(*this);
			return retvalue;
		}
		TRef& operator=( TRef const & r ) = default;
	};//template <typename element_t, typename ptr_void_t> class TRef //

	template <>class ptr_array<void const*>
	{
	public:
		typedef void const *	pointer_type;
		typedef pointer_type	pointer_t;
		typedef void const		element_type;

	private:
		void*			membegin= nullptr;
		void*			pointer	= nullptr;
		size_t			bytelen	= 0;

	public:
		size_t				ElementCount() const
		{
			return 0;
		}
		size_t				ByteLen() const
		{
			return bytelen;
		}
		size_t				UsedByteLen() const
		{
			return (__int8*)pointer - (__int8*)membegin;
		}

		template<typename T> operator T const *() const
		{
			return (T const*)pointer;
		}
		operator void const *() const
		{
			ASSERT(pointer==nullptr || this->ByteLen());
			return (void*)pointer;
		}

		void const * begin() const
		{
			return (void*)pointer;
		}
		void const * end() const
		{
			return (void const *)((__int8 const *)pointer + ByteLen());
		}

		void const * memorystartpos() const
		{
			return membegin;
		}

		template<typename T> TRef<T, pointer_type> useas()
		{
			 return TRef<T,pointer_type> ( *(pointer_type *)&pointer, bytelen, *(pointer_type *)&membegin );
		}


		ptr_array() {}
		ptr_array( pointer_type p, size_t ByteLen, pointer_type b=nullptr )
			: pointer( (void*)p )
			, bytelen( ByteLen )
			, membegin( b ? (void*)b : (void*)p )
		{
		}
		ptr_array( ptr_array const & ) = default;
		template<typename otherpointer> ptr_array( ptr_array<otherpointer> const & r )
			: pointer( (void*)(typename ptr_array<otherpointer>::pointer_type)r ) 
			, bytelen( r.ByteLen() )
			, membegin( r.memorystartpos() )
		{
			static_assert( WS::is_const_pointer<otherpointer>::value==false || WS::is_const_pointer<pointer_type>::value, "U const* kann nicht zu T * konvertiert werden" );
		}

		ptr_array	operator+( size_t pointer_um_anzahl_bytes_weitersetzen ) const
		{
			if( pointer_um_anzahl_bytes_weitersetzen > ByteLen() )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ptr_array( (__int8*)pointer + pointer_um_anzahl_bytes_weitersetzen, ByteLen() - pointer_um_anzahl_bytes_weitersetzen, (__int8*)membegin );
		}
		ptr_array&	operator+=( size_t pointer_um_anzahl_bytes_weitersetzen ) 
		{
			return *this = operator+( pointer_um_anzahl_bytes_weitersetzen );
		}
		ptr_array	operator+( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) const
		{
			return operator+( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
		}
		ptr_array&	operator+=( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) 
		{
			return *this = operator+( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
		}

		ptr_array	operator-( size_t pointer_um_anzahl_bytes_weitersetzen ) const
		{
			//ASSERT( (__int8*)pointer >= (__int8*)membegin );//kann nie anders sein, sonst exception
			if( pointer_um_anzahl_bytes_weitersetzen > size_t((__int8*)pointer - (__int8*)membegin) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ptr_array( (__int8*)pointer - pointer_um_anzahl_bytes_weitersetzen, ByteLen() + pointer_um_anzahl_bytes_weitersetzen, (__int8*)membegin );
		}
		ptr_array&	operator-=( size_t pointer_um_anzahl_bytes_weitersetzen ) 
		{
			return *this = operator-( pointer_um_anzahl_bytes_weitersetzen );
		}
		ptr_array	operator-( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) const
		{
			if( pointer_um_anzahl_bytes_weitersetzen >= 0 )
				return operator-( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
			else
				return operator+( static_cast<size_t>(-pointer_um_anzahl_bytes_weitersetzen) );
		}
		ptr_array&	operator-=( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) 
		{
			if( pointer_um_anzahl_bytes_weitersetzen >= 0 )
				return *this = operator-( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
			else
				return *this = operator+( static_cast<size_t>(-pointer_um_anzahl_bytes_weitersetzen) );
		}
	};//template <>class ptr_array<void const*>

	template <>class ptr_array<void*>
	{
	public:
		typedef void*			pointer_type;
		typedef pointer_type	pointer_t;
		typedef void			element_type;

	private:
		void*			membegin= nullptr;	//fix
		void*			pointer	= nullptr;	//laufzeiger
		size_t			bytelen = 0;

	public:
		size_t				ElementCount() const
		{
			return 0;
		}
		size_t				ByteLen() const
		{
			return bytelen;
		}
		size_t				UsedByteLen() const
		{
			return (__int8*)pointer - (__int8*)membegin;
		}
		template<typename T> operator T*() const
		{
			ASSERT(pointer==nullptr || sizeof(T)<=this->ByteLen());
			return (T*)pointer;
		}
		operator void*() const
		{
			ASSERT(pointer==nullptr || this->ByteLen());
			return (void*)pointer;
		}
		void* begin() const
		{
			return (void*)pointer;
		}
		void* end() const
		{
			return (void*)((__int8*)pointer + ByteLen());
		}
		void* memorystartpos() const
		{
			return membegin;
		}
		template<typename T> TRef<T, pointer_type> useas()
		{
			return TRef<T,pointer_type>( (pointer_type)pointer, bytelen, (pointer_type)membegin );
		}

		ptr_array() {};
		ptr_array( pointer_type p, size_t ByteLen, pointer_type b=nullptr )
			: pointer( p )
			, bytelen( ByteLen )
			, membegin( b ? b : p )
		{
		}
		ptr_array( ptr_array const & ) = default;
		template<typename otherpointer> ptr_array( ptr_array<otherpointer> const & r )
			: pointer( (void*)(typename ptr_array<otherpointer>::pointer_type)r ) 
			, bytelen( r.ByteLen() )
			, membegin( r.memorystartpos() )
		{
			static_assert( WS::is_const_pointer<otherpointer>::value==false || WS::is_const_pointer<pointer_type>::value, "U const* kann nicht zu T * konvertiert werden" );
		}


		ptr_array	operator+( size_t pointer_um_anzahl_bytes_weitersetzen ) const
		{
			if( pointer_um_anzahl_bytes_weitersetzen > ByteLen() )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ptr_array( (__int8*)pointer + pointer_um_anzahl_bytes_weitersetzen, ByteLen() - pointer_um_anzahl_bytes_weitersetzen, (__int8*)membegin );
		}
		ptr_array&	operator+=( size_t pointer_um_anzahl_bytes_weitersetzen ) 
		{
			return *this = operator+( pointer_um_anzahl_bytes_weitersetzen );
		}
		ptr_array	operator+( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) const
		{
			if( pointer_um_anzahl_bytes_weitersetzen >= 0 )
				return operator+( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
			else
				return operator-( static_cast<size_t>(-pointer_um_anzahl_bytes_weitersetzen) );
		}
		ptr_array&	operator+=( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) 
		{
			if( pointer_um_anzahl_bytes_weitersetzen >= 0 )
				return *this = operator+( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
			else
				return *this = operator-( static_cast<size_t>(-pointer_um_anzahl_bytes_weitersetzen) );
		}
		ptr_array&	operator+=( ptr_array<void*> const & append ) 
		{
			if( ByteLen() < append.ByteLen() )
				throw std::out_of_range( __FUNCTION__ " ziel hat zu wenig speicher" );
			memcpy( this->pointer, append.pointer, append.ByteLen() );
			operator+=( append.ByteLen() );
			return *this;
		}
		ptr_array&	operator+=( ptr_array<void const*> const & append )
		{
			if( ByteLen() < append.ByteLen() )
				throw std::out_of_range( __FUNCTION__ " ziel hat zu wenig speicher" );
			memcpy( this->pointer, static_cast<void const*>(append), append.ByteLen() );
			operator+=( append.ByteLen() );
			return *this;
		}
		template<typename otherpointer>ptr_array&	operator+=(ptr_array<otherpointer> const & append)
		{
			return operator+=(ptr_array<void const*>(append));
		}


		ptr_array	operator-( size_t pointer_um_anzahl_bytes_weitersetzen ) const
		{
			//ASSERT( (__int8*)pointer >= (__int8*)membegin );//kann nie anders sein, sonst exception
			if( pointer_um_anzahl_bytes_weitersetzen > size_t((__int8*)pointer - (__int8*)membegin) )
				throw std::out_of_range( __FUNCTION__ " index out of range" );
			return ptr_array( (__int8*)pointer - pointer_um_anzahl_bytes_weitersetzen, ByteLen() + pointer_um_anzahl_bytes_weitersetzen, (__int8*)membegin );
		}
		ptr_array&	operator-=( size_t pointer_um_anzahl_bytes_weitersetzen ) 
		{
			return *this = operator-( pointer_um_anzahl_bytes_weitersetzen );
		}
		ptr_array	operator-( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) const
		{
			if( pointer_um_anzahl_bytes_weitersetzen >= 0 )
				return operator-( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
			else
				return operator+( static_cast<size_t>(-pointer_um_anzahl_bytes_weitersetzen) );
		}
		ptr_array&	operator-=( INT_PTR pointer_um_anzahl_bytes_weitersetzen ) 
		{
			if( pointer_um_anzahl_bytes_weitersetzen >= 0 )
				return *this = operator-( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
			else
				return *this = operator+( static_cast<size_t>(pointer_um_anzahl_bytes_weitersetzen) );
		}

	};

	template<typename Tout, typename Tin> ptr_array<Tout> Create_ptr_array(Tin pointer, size_t size)
	{
		return ptr_array<Tout>(ptr_array<Tin>(pointer, size));
	}
	template<typename T> ptr_array<T> Create_ptr_array(T pointer, size_t size)
	{
		return ptr_array<T>(pointer, size);
	}
	template<typename T, size_t size> ptr_array<T*> Create_ptr_array(T (&arr)[size] )
	{
		return ptr_array<T*>(arr, size);
	}
	template<typename Tout, typename Tin, size_t size> ptr_array<Tout> Create_ptr_array(Tin(&arr)[size])
	{
		return ptr_array<Tout>(ptr_array<Tin*>(arr, size));
	}
}

#ifdef UNDEF_ASSERT
#	undef ASSERT
#endif
