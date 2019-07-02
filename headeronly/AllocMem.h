#pragma once

#include "headeronly\Ptr_Array.h"

#include <memory>

#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ = 0
struct IWPMemFileAlloc
{
	typedef void* HANDLE;

	enum class KnownInterface
	{
		Heap,		//schnellsten, aber ggf. auf 1,7GB bzw 3,6GB bei Win32 beschränkt
		FileMap,	//auf die größe des arbeitspeichers begrenzt
		AWE,		//dont use this. braucht police + adminrechte und kann das BS destabilisieren
		Local,		//dont use this. benutzt LocalAlloc nur um zu zeigen, dass es das nicht bringt
	};

	struct LockedMem
	{
		HANDLE						handle = NULL;
		WP::ptr_array<void*>		ptr;

		LockedMem( HANDLE handle, void* ptr, size_t bytes) 
			: handle(handle)
			, ptr( ptr, bytes ){}
		LockedMem( HANDLE handle, WP::ptr_array<void*>ptr )
			: handle(handle)
			, ptr( ptr){}

		LockedMem(){}
		LockedMem( LockedMem const & ) = delete;
		LockedMem( LockedMem && r )
		{
			swap( r );
		}
		LockedMem& operator=( LockedMem const & ) = delete;
		LockedMem& operator=( LockedMem && r )
		{
			LockedMem temp( std::move(r) );
			swap( temp );
			return *this;
		}

		operator WP::ptr_array<void*>()
		{
			return this->ptr;
		}
		void swap( LockedMem & r )
		{
			std::swap(this->handle,r.handle);
			std::swap(this->ptr,r.ptr);
		}
	};

	virtual ~IWPMemFileAlloc() = default;
	
	virtual HANDLE					AllocMem() _INTERFACE_FUNCTION_;
	virtual LockedMem				LockMem(HANDLE) _INTERFACE_FUNCTION_;
	virtual void					UnlockMem(LockedMem&&) _INTERFACE_FUNCTION_;
	virtual HANDLE					FreeMem(HANDLE) _INTERFACE_FUNCTION_;

	virtual size_t					BytesPerBlock() const _INTERFACE_FUNCTION_;
	virtual bool					valid(HANDLE) const _INTERFACE_FUNCTION_;
};
#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ override

template <IWPMemFileAlloc::KnownInterface Interface, typename ... Args> std::shared_ptr<IWPMemFileAlloc> CreateIMem( Args ... args );

struct IKeyValuePairDB;
std::shared_ptr<IWPMemFileAlloc> CreateIMem( IKeyValuePairDB const & schnittstelle);
std::shared_ptr<IWPMemFileAlloc> CreateIMem( );
