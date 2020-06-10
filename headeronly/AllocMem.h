#pragma once
//Copyright (c) 2020 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

#include "headeronly\Ptr_Array.h"

#include <memory>

#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ = 0
struct IWPMemFileAlloc
{
	typedef void* HANDLE;

	enum class KnownInterface
	{
		Heap,		//schnellsten, aber ggf. auf 1,7GB bzw 3,6GB bei Win32 beschr�nkt
		FileMap,	//auf die gr��e des arbeitspeichers begrenzt
		AWE,		//dont use this. braucht police + adminrechte und kann das BS destabilisieren
		Local,		//dont use this. benutzt LocalAlloc nur um zu zeigen, dass es das nicht bringt
	};

	struct LockedMem
	{
		HANDLE						handle = NULL;
		WS::ptr_array<void*>		ptr;

		LockedMem( HANDLE handle, void* ptr, size_t bytes) 
			: handle(handle)
			, ptr( ptr, bytes ){}
		LockedMem( HANDLE handle, WS::ptr_array<void*>ptr )
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

		operator WS::ptr_array<void*>()
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
