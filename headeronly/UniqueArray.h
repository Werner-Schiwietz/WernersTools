#pragma once


#include "Ptr_Array.h"
namespace WP
{
	template<typename element_t> struct UniqueArrayMemoryallocDefault 
	{
		using element_type = element_t;
		ptr_array<element_type*> allocmem( size_t elemente )
		{
			return ptr_array<element_type*>( new element_type[elemente], elemente );
		}
		void freemem( ptr_array<element_type*> & ptr )
		{
			delete [] (element_type*)ptr.memorystartpos();
			ptr = ptr_array<element_type*>();
		}
	};
	template<> struct UniqueArrayMemoryallocDefault<void>//spezialisierung fuer void. macht alloc und free statt new[] und delete[]
	{
		using element_type = void;
		ptr_array<element_type*> allocmem( size_t elemente ) 
		{
			return ptr_array<element_type*>(malloc(elemente),elemente );
		}
		void freemem( ptr_array<element_type*> & ptr ) 
		{
			element_type* p =  ptr.memorystartpos();
			free( p );
			ptr = ptr_array<element_type*>();
		}
	};

	//UniqueArrayMemoryAccess_t muss nur die beiden methoden allocmem und freemem bereitstellen
	template<typename element_t,typename MemoryAccess_t=UniqueArrayMemoryallocDefault<element_t>> class UniqueArray final
	{
	public:
		using element_type			= element_t;
		using MemoryAccess_type		= MemoryAccess_t;
	private:
		ptr_array<element_type*>	data;
		MemoryAccess_type			iManageMemory;
	public:

		UniqueArray() = default;
		UniqueArray( UniqueArray const & ) = delete;
		UniqueArray( UniqueArray && r ) : UniqueArray()
		{
			swap( r );
		}
		UniqueArray& operator=( UniqueArray const & ) = delete;
		UniqueArray& operator=( UniqueArray && r )
		{
			UniqueArray temp( std::move(*this) );
			swap( r );
			return *this;
		}

		void swap( UniqueArray & r )
		{
			std::swap( this->data, r.data );
			std::swap( this->iManageMemory, r.iManageMemory );
		}

		~UniqueArray()
		{
			getManageMemory().freemem( data );
		}
		UniqueArray( size_t elemente )
		{
			data = getManageMemory().allocmem( elemente );
		}
		template<typename MemoryAccess_t>UniqueArray( size_t elemente, MemoryAccess_t && iManageMemory )//perfect forward spart konstruktoren
			: iManageMemory( std::forward<MemoryAccess_t>(iManageMemory) )
		{
			data = getManageMemory().allocmem( elemente );
		}


		MemoryAccess_type & getManageMemory()
		{
			return this->iManageMemory;
		}
		ptr_array<element_type*> get()
		{
			return data;//kopie, aktuelle schreibpos immer 0
		}
		template<typename return_type> ptr_array<return_type*> get()
		{
			return ptr_array<return_type*>( data );
		}
	//private:
		//<<DONT USE release()>> macht wenig sinn, da den speicher sinnvoll nur MemoryAccess_type freigeben kann, und der aufrufer dafuer sorgen müsste, dass das passiert, was ja aber nicht passieren wird. siehe UT_UniqueArrayRelease
		ptr_array<element_t*> release()
		{
			ptr_array<element_t*> retvalue;
			std::swap( retvalue, data );
			return retvalue;
		}
	public:
		operator ptr_array<element_t*> ()
		{
			return data;//kopie, aktuelle schreibpos immer 0
		}
		operator ptr_array<element_t const *> ()
		{
			return data;//kopie, aktuelle schreibpos immer 0
		}
		operator element_t* ()
		{
			return data;//kopie, aktuelle schreibpos immer 0
		}
		auto & operator[](size_t index)
		{
			return data[index];
		}
		auto & operator[](size_t index) const
		{
			return data[index];
		}
		auto & operator[](INT_PTR index)
		{
			return data[index];
		}
		auto & operator[](INT_PTR index) const
		{
			return data[index];
		}
		size_t elemente() const
		{
			return data.ElementCount();
		}
		size_t bytes() const
		{
			return data.ByteLen();
		}


		auto begin()
		{
			return data.begin();
		}
		auto end()
		{
			return data.end();
		}
		auto begin() const
		{
			return data.begin();
		}
		auto end() const
		{
			return data.end();
		}
	};
}