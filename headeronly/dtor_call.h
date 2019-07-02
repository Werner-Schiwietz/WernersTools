#pragma once

//by werner schiwietz
//Funktionbeschreibung: Dtor_call fuehrt im Destruktor oder explizit durch Release eine Methode/Funktion/Lamda/std::function oder operator() aus
//
//Dtor_call bekommt im konstruktor 
//	einen zeiger auf ein objekt (das objekt muss selbstverständlich länger leben als Dtor_callFirst selbst) oder einen shared_pointer(der ist life-time sicher)
//	und einen methodenpointer 
// oder 
//	void void* als objekt-pointer 
//	und einen Funktionspointer/lambda/std::function
//und die parameter der methode/function (wenn die methode keine parameter braucht werden auch keine angegeben)
//im destruktor von Dtor_call wird die methode/funktion mit den parametern aufgerufen
//
//Move-verhalten
//bei erneuter zuweisung(move) wird ggf. der lock gelöst.
//	gesteuert wird das move-verhalten durch den template-parameter enumDtor_call_Movetype
//	es gibt z.zt. 3 implementierungen. die enums beschreiben aber ziemlich genau, was passiert. funktionsweise ggf. in UT_dtor_call.cpp nach schauen
//da lambdas keine zuweisung unterstuetzen funktioniert die zuweisen fuer lambda-basierte Dtor_call nicht
//
//siehe auch winwp\BasisUnitTests\UT_dtor_call.cpp, da sind die tests, an die ich gedacht haben. dort sieht man auch, wie dieser code hilfreich eingesetzt werden kann

//einfachere erzeugung eines Dtor_call-objekts per CreateDtorCall (funktioniert aenlich wie std::bind ohne platzhalter)
//Beispiele 
// auto AutoReleaseIt =  CreateDtorCall( funktion_pointer[, parameter] );// beliebig viele parameter 0-n, die dem funktion_pointer bei aufruf uebergeben werden. als funktion_pointer sind auch lambdas und operator()-objekte moeglich
// oder OnlyFirst
// auto AutoReleaseIt =  CreateDtorCall( methoden_pointer, objekt-pointer_zur_methode[, parameter] );// beliebig viele parameter 0-n, die dem methoden_pointer bei aufruf uebergeben werden
// oder 
// auto AutoReleaseIt =  CreateDtorCall( methoden_pointer, std::shared_ptr<objekt_zur_methode>[, parameter] );// beliebig viele parameter 0-n, die dem methoden_pointer bei aufruf uebergeben werden
//
// zusaetzlich kann dem CreateDtorCall noch das gewuenschte verhalten beim move per template-parameter gesetzt werden
// auto AutoReleaseIt =  CreateDtorCall<enumDtor_call_Movetype::First>( paramter wie oben );//standard ganau das gleiche wie CreateDtorCall( paramter wie oben ) oder CreateDtorCall<>( paramter wie oben )
// enum-erklaerung
//	enumDtor_call_Movetype::First,	  //standard, bei erneuter zuweisung wird der vorherige dtor-call ausgelöst
//	enumDtor_call_Movetype::Last,	  //bei erneuter zuweisung wird der dtor-call des zugewiesenen sofort ausgelöst, this bleibt unveraendert. z.b. fuer funktionen fällt mir nichts ein, was nicht auch mit OnlyFirst machbar waere
//	enumDtor_call_Movetype::OnlyFirst //bei erneuter zuweisung wird der dtor-call des zugewiesenen nicht(nie) ausgelöst, this bleibt unveraendert. z.b. fuer CDC::SelectObject(Old), da dabei i.d.r. immer nur der erste font wieder hergestellt werden soll

#include <tuple>
#include <memory>

#include "WP_Helper.h"

#pragma push_macro("ASSERT")
#ifndef ASSERT
#	define ASSERT(E) ((void)0)
//#define VERIFY(f)          ((void)(f))
#endif

#pragma warning(push,4)

namespace Dtor_call_Helper
{	//quelle variadic parameter http://stackoverflow.com/questions/16868129/how-to-store-variadic-template-arguments

	//generiere_sequenz_type<N> bedeutet, der compiler (also kein laufzeitaufwand) erzeugt einen datentyp index<0,1,2,... N-1>. man koennte auch std::make_integer_sequence und std::integer_sequence benutzen
	//die templateparameter des index-datentyps werden benutzt um ein tuple in seine einzelteile zerlegt einer methode zu uebergeben
	template <size_t... integer_sequenz> struct index {};
	template <size_t N, size_t... integer_sequenz> struct generiere_sequenz_type : generiere_sequenz_type<N - 1, N - 1, integer_sequenz...> {};
	template <size_t... integer_sequenz> struct generiere_sequenz_type<0, integer_sequenz...> : index<integer_sequenz...> {};

#	pragma warning(suppress:4100)//compiler, was tust du
	template <typename Objekt, typename Method, typename... Args, size_t... integer_sequenz> auto call_func(Objekt * obj, Method method, std::tuple<Args...>& tup, index<integer_sequenz...>)
	{
		return (obj->*method)(std::get<integer_sequenz>(tup)...);//uebergibt den tuple in einzelteilen der methode
	}
#	pragma warning(suppress:4100)//compiler, was tust du
	template <typename Function, typename... Args, size_t... integer_sequenz> auto call_func(void *, Function function, std::tuple<Args...>& tup, index<integer_sequenz...>)
	{
		return (function)(std::get<integer_sequenz>(tup)...);//uebergibt den tuple in einzelteilen der funktion
	}
	template <typename Objekt, typename Method, typename... Args> auto call_func(Objekt * obj, Method method, std::tuple<Args...>& tup)
	{
		return call_func(obj, method, tup, generiere_sequenz_type<sizeof...(Args)>{});
	}
}

namespace
{
	template<typename ReleaseMethod_type, bool moveable = std::is_move_assignable<ReleaseMethod_type>::value> struct _helper;
	template<typename ReleaseMethod_type> struct _helper<ReleaseMethod_type, true>
	{
		static void swap(ReleaseMethod_type &l, ReleaseMethod_type &r)
		{
			std::swap(l, r);
		}
	};
	template<typename ReleaseMethod_type> struct _helper<ReleaseMethod_type, false>
	{
		static void swap(ReleaseMethod_type &l, ReleaseMethod_type &r) = delete;
	};
}

//wenn ihr diese klasse benutzen wollt, macht euch gedanken, was bei der zweiten zuweisung passieren soll. beziehungsweise bei nebenläufiger benutzung
//z.zt. gibt es drei versionen
// First, Last, OnlyFirst 
//schaut euch an was die tun, oder fragt werner wenn ihr euch nicht sicher seid
enum class enumDtor_call_Movetype
{
	First,	  //standard, bei erneuter zuweisung wird der vorherige dtor-call ausgelöst
	Last,	  //bei erneuter zuweisung und gleichem objekt wird der dtor-call des zugewiesenen sofort ausgelöst, this bleibt unveraendert. z.b. fuer funktionen fällt mir nichts ein, was nicht auch mit OnlyFirst machbar waere
	OnlyFirst //bei erneuter zuweisung und gleichem objekt wird der dtor-call des zugewiesenen nicht(nie) ausgelöst, this bleibt unveraendert. z.b. fuer CDC::SelectObject(Old), da dabei i.d.r. immer nur der erste font wieder hergestellt werden soll
};
#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ = 0
class IDtor_call//kann der auflistung von verschiedenen Dtor_call-Objekten in einem container dienen
{
public:
	virtual ~IDtor_call() = default;

	virtual bool Empty() const _INTERFACE_FUNCTION_;	//liefert ob objekt etwas yu tun hat
	virtual void Release() _INTERFACE_FUNCTION_;		//gibt die resource frei, fuehrt also die hinterlegte aktion aus
	virtual void Discard() _INTERFACE_FUNCTION_;		//
};
#undef _INTERFACE_FUNCTION_
#define _INTERFACE_FUNCTION_ override
template <enumDtor_call_Movetype eMovetype, typename LockedObjekt_type, typename ReleaseMethod_type, typename ... ReleaseArgs> class Dtor_call : public IDtor_call
{
	std::shared_ptr<LockedObjekt_type>	LockedPtr;	//dient nur dazu ggf. den referenzcounter des shared_ptr zu erhoehen
	LockedObjekt_type*					pLocked = nullptr;
	ReleaseMethod_type					fnRelease;	//lambdas nicht assign-able
	std::tuple<ReleaseArgs...>			releaseargs;//parameter fuer releasemethode zwischenspeichern
	bool								release = false;

	template <bool enabled_shared = std::is_base_of<std::enable_shared_from_this<LockedObjekt_type>, LockedObjekt_type>::value> LockedObjekt_type* SetSharedPtr(LockedObjekt_type* pLocked);
	template <> LockedObjekt_type* SetSharedPtr<true>(LockedObjekt_type* plocked)
	{
		try
		{
			//sollte LockedObjekt_type von enable_shared_from_this abgeleitet sein, sollte shared_from_this funktionieren. 
			//dazu muesste pLocked aber auch als shared_ptr angelegt worden sein, sonst exception (c++17, aber msvc c++14 wirft auch exception).
			//mit c++17 haette wir mehr moeglichkeiten
			LockedPtr = plocked->shared_from_this();
		}
		catch (...)
		{
			ASSERT("LockedObjekt_type ist zwar von enable_shared_from_this abgeleitet, aber nicht als shared_ptr angelegt worden");
		}
		return plocked;
	}
	template <> LockedObjekt_type* SetSharedPtr<false>(LockedObjekt_type* plocked)
	{
		return plocked;
	}
public:
	Dtor_call(LockedObjekt_type* pLocked, ReleaseMethod_type fnRelease, ReleaseArgs ... releaseargs)
		: pLocked(SetSharedPtr(pLocked))
		, fnRelease(fnRelease)
		, releaseargs(std::forward<ReleaseArgs>(releaseargs) ...)
		, release(true)
	{}
	Dtor_call(std::shared_ptr<LockedObjekt_type> LockedPtr, ReleaseMethod_type fnRelease, ReleaseArgs ... releaseargs)
		: pLocked(LockedPtr.get())
		, fnRelease(fnRelease)
		, releaseargs(std::forward<ReleaseArgs>(releaseargs) ...)
		, release(true)
		, LockedPtr(LockedPtr)
	{
		ASSERT(pLocked);
	}
	~Dtor_call()
	{
		try
		{
			CallitNow();
		}
		catch (...)
		{
			ASSERT(__FUNCTION__ " exception");
		}
	}
	Dtor_call() = default;
	Dtor_call(Dtor_call const &) = delete;
	Dtor_call& operator=(Dtor_call const &) = delete;
	Dtor_call(Dtor_call&& r)
	{
		*this = std::move(r);
	}
	Dtor_call& operator=(Dtor_call && r)
	{
		Move<eMovetype>(std::move(r));
		return *this;
	}

	void swap(Dtor_call & r)
	{
		static_assert(std::is_move_assignable<ReleaseMethod_type>::value, "es gibt einen move-assign irgendwo auf this der nicht funktioniert. Der Compiler zeigt die Ctor-Zeile, nicht die Zuweisung selbst. z.B. Lambda-Funktionen koennen nicht zugewiesen werden");// ggf error C2338: 
		std::swap(this->fnRelease, r.fnRelease);

		std::swap(this->pLocked, r.pLocked);
		std::swap(this->release, r.release);
		std::swap(this->releaseargs, r.releaseargs);
		std::swap(this->LockedPtr, r.LockedPtr);
	}

	std::unique_ptr<Dtor_call> MoveToHeap()
	{
		return std::make_unique<Dtor_call>(std::move(*this));
	}

public:
	template <typename return_type> bool CallitNow(return_type & retvalue)//wenn rueckgabewert true, dann enthaelt retvalue den rueckgabewert der gerufenen funktion. nur maximal einmal, erfolgreich aufrufbar
	{
		if (release)
		{
			release = false;
			retvalue = Dtor_call_Helper::call_func(this->pLocked, this->fnRelease, this->releaseargs);
			this->LockedPtr.reset();//ggf. vorhandenes referenziertes obj wird nicht mehr gebraucht
			return true;
		}
		return false;
	}
	void CallitNow()
	{
		if (release)
		{
			release = false;
			Dtor_call_Helper::call_func(this->pLocked, this->fnRelease, this->releaseargs);
			this->LockedPtr.reset();//ggf. vorhandenes referenziertes obj wird nicht mehr gebraucht
		}
	}

	bool Empty() const _INTERFACE_FUNCTION_
	{
		return this->release == false;
	}
	void Release() _INTERFACE_FUNCTION_//same as CallitNow
	{
		CallitNow();
	}
	void Discard() _INTERFACE_FUNCTION_//führt die funktion/methode nicht (nie) aus
	{
		release = false;
		this->LockedPtr.reset();//ggf. vorhandenes referenziertes obj wird nicht mehr gebraucht
	}


protected:
	template<enumDtor_call_Movetype eMovetype>void Move(Dtor_call && r) = delete;
	template<> void Move<enumDtor_call_Movetype::First>(Dtor_call && r)
	{
		if (this != &r)
		{
			Dtor_call temp;//move_ctor würde zu endlosrekusion führen
			temp.swap(r);		//leer <-> r
			this->swap(temp);	//rdaten <-> this
		}						//thisdaten frei geben
	}
	template<> void Move<enumDtor_call_Movetype::Last>(Dtor_call && r)
	{
		if (this != &r)
		{
			if (this->pLocked == r.pLocked)
			{
				if (this->Empty())
				{
					swap(r);
				}
				else
				{
					Dtor_call temp;
					temp.swap(r);
				}
			}
			else
				//objekt sind nicht die selben, also auf jeden fall release aufs this->pLocked machen
				Move<enumDtor_call_Movetype::First>(std::move(r));
		}
	}
	template<> void Move<enumDtor_call_Movetype::OnlyFirst>(Dtor_call && r)
	{
		if (this != &r)
		{
			if (this->pLocked == r.pLocked)
			{
				if (this->Empty())
				{
					swap(r);
				}
				else
				{
					Dtor_call temp;
					temp.swap(r);
					temp.Discard();
				}
			}
			else
				//objekt sind nicht die selben, also auf jeden fall release aufs this->pLocked machen
				Move<enumDtor_call_Movetype::First>(std::move(r));
		}
	}
};

template<bool is_method, enumDtor_call_Movetype eMovetype, typename function_method_type, typename ... Args>				struct _CreateDtorCall;
template<enumDtor_call_Movetype eMovetype, typename function_method_type, typename ... Args>							struct _CreateDtorCall<false, eMovetype, function_method_type, Args ...>
{
	auto operator()(function_method_type fnRelease, Args ... args)
	{
		return Dtor_call<eMovetype, void, function_method_type, Args...>((void*)nullptr, fnRelease, std::forward<Args>(args) ...);
	}
};
template<enumDtor_call_Movetype eMovetype, typename function_method_type, typename objekt_ptr_type, typename ... Args>	struct _CreateDtorCall<true, eMovetype, function_method_type, objekt_ptr_type, Args ...>
{
	auto operator()(function_method_type fnRelease, objekt_ptr_type object, Args ... args)
	{
		typedef decltype((object->*fnRelease)(args...)) methoden_type;//ist die methode auf das objekt aufrufbar? wenn nicht gibt es hier compilefehler. 
		return Dtor_call<eMovetype, std::remove_pointer_t<objekt_ptr_type>, function_method_type, Args...>(object, fnRelease, std::forward<Args>(args) ...);
	}
};
template<enumDtor_call_Movetype eMovetype = enumDtor_call_Movetype::First, typename function_method_type, typename ... Args>
auto CreateDtorCall(function_method_type fn, Args ... args)
{
	return _CreateDtorCall<std::is_member_function_pointer<function_method_type>::value, eMovetype, function_method_type, Args ...>()(fn, std::forward<Args>(args) ...);
};
template<enumDtor_call_Movetype eMovetype = enumDtor_call_Movetype::First, typename method_type, typename objekt_type, typename ... Args>
auto CreateDtorCall(method_type method, std::shared_ptr<objekt_type> objekt_ptr, Args ... args)
{
	typedef decltype((objekt_ptr.get()->*method)(std::forward<Args>(args) ...)) methoden_type;//ist die methode auf das objekt aufrufbar? wenn nicht gibt es hier compilefehler. 

	return Dtor_call<eMovetype, objekt_type, method_type, Args...>(objekt_ptr, method, std::forward<Args>(args) ...);
};


namespace WP
{
	//AutoResource ist der Versuch die Freigabe einer Resource direkt an die Reservierung der Resource zu haengen
	// die Funktion LockResource legt ein Verwaltungsobjekt fuer ein Objekt von typename resource_t an.
	// wenn das Verwaltungsobjekt zerstoert wird, wird die uebergebene restorefunktion mit dem verwalteten objekt als parameter aufgerufen
	// usage z.B. 
	//			auto hdc = LockResource( ::GetDC(NULL), std::bind( ::ReleaseDC, (HWND)NULL, std::placeholders::_1 ) );//wenn hdc zerstoert wird, wird ::ReleaseDC(NULL,hdc) aufgerufen
	//			auto bitmap = ::CreateCompatibleDC( hdc, x, y );
	//
	//			auto autoResource = LockResource<enumDtor_call_Movetype::OnlyFirst, void( COLORREF )>( [pDC]( COLORREF value )->void { pDC->SetTextColor( value ); } );
	//			auto autoResource = LockResource<void( int * )>( [&]( int * value )->void { delete value; } );//legt das objekt mit der restore-fkt an. wird erst durch SetResource scharf geschaltet
	//			autoResource.SetResource(value);//je nach gesetzen enumDtor_call_Movetype passieren mit value die angesagten dinge
	//
	//	enumDtor_call_Movetype::First,	  //standard, bei erneuter zuweisung wird die vorherige resource freigegeben. sinnvoll fuer speicher-allok/free und ähnliches
	//	enumDtor_call_Movetype::Last,	  //bei erneuter zuweisung wird die resourcefreigabe des zugewiesenen sofort ausgelöst, this bleibt unveraendert.// kann fuer referenz zaehlende Resourcen verwendet werden, braucht man wahrscheinlich nie
	//	enumDtor_call_Movetype::OnlyFirst //bei erneuter zuweisung wird die resourcefreigabe des zugewiesenen nicht(nie) ausgelöst //sinnvoll z.b. wenn eine boolscher zustand verwaltet wird oder Resource wie CDC::SelectObject() um den OldFont zu restaurieren
	template <typename resource_t, typename restorefunction_t, enumDtor_call_Movetype eMovetype = enumDtor_call_Movetype::First> struct AutoResource
	{
		typedef resource_t resource_t;
		resource_t				value;
		restorefunction_t		restore;
		bool					callrestore = false;
		~AutoResource()
		{
			try
			{
				release();
			}
			catch (...)
			{
				ASSERT(__FUNCTION__ " exception");
			}
		}
		AutoResource( restorefunction_t && restore ) : restore( std::forward<restorefunction_t>( restore ) ), callrestore( false ) {}
		AutoResource(resource_t && value, restorefunction_t && restore) : value(std::forward<resource_t>(value)), restore(std::forward<restorefunction_t>(restore)), callrestore(true) {}
		AutoResource(AutoResource const &) = delete;
		AutoResource(AutoResource && r)
		{
			static_assert(std::is_assignable<restorefunction_t, restorefunction_t>::value == true, "restorefunction ist nicht zuweisbar. mit lambdas oder referenzen geht das nicht");
			swap(r);
		}
		AutoResource& operator=(AutoResource const &) = delete;
		AutoResource& operator=(AutoResource && r) { Move<eMovetype>(std::move(r)); return *this; }

		template<enumDtor_call_Movetype =eMovetype>void SetResource( resource_t const & valueNeu ) = delete;
		template<>void SetResource<enumDtor_call_Movetype::First>( resource_t const & valueNeu )
		{
			if( callrestore )
				this->restore(this->value);
			callrestore = true;
			value = valueNeu;
		}
		template<>void SetResource<enumDtor_call_Movetype::Last>( resource_t const & valueNeu )
		{
			if( callrestore )
				this->restore( valueNeu );
		}
		template<>void SetResource<enumDtor_call_Movetype::OnlyFirst>( resource_t const & valueNeu )
		{
			if( callrestore == false )
			{
				callrestore = true;
				this->value = valueNeu;
			}
		}

		void swap(AutoResource & r)
		{
			std::swap(this->value, r.value);
			std::swap(this->restore, r.restore);
			std::swap(this->callrestore, r.callrestore);
		}

		operator resource_t& ()
		{
			return value;
		}
		resource_t& get()
		{
			return value;
		}

		void release()
		{
			if (callrestore)
			{
				restore(value);
				callrestore = false;
			}
		}
		void discard()
		{
			callrestore = false;
		}
	protected:
		template<enumDtor_call_Movetype eMovetype>void Move(AutoResource && r) = delete;
		template<> void Move<enumDtor_call_Movetype::First>(AutoResource && r)
		{
			//AutoResource temp(std::move(r));//klappt nicht mit jedem funktion-objekt
			if( this == &r )
				return;
			this->swap(r);
			r.release();
		}
		template<> void Move<enumDtor_call_Movetype::Last>(AutoResource && r)
		{
			if (this != &r)
			{
				r.release();
				//AutoResource temp(std::move(r));//sofort wieder frei geben, um ggf. refcounter runter zu zaehlen
			}
		}
		template<> void Move<enumDtor_call_Movetype::OnlyFirst>(AutoResource && r)
		{
			if (this != &r)
			{
				r.discard();						//die fregabe wird nicht durchgeführt
				//AutoResource temp(std::move(r));	//die resource wird auch nicht uebernommen z.b. fuer CDC::SelectObject wollen wir am schluss den OldValue restaurieren, also den rueckgabewert des ersten SeletecObject
			}
		}
	};

	template<typename signatur>struct template_signatur;
	template<typename return_t, typename parameter_t>struct template_signatur<return_t(parameter_t)>
	{
		typedef return_t return_t;
		typedef parameter_t parameter_t;
	};

	//soll vorab ein restoreobjekt angelegt werden, muss als erster template-parameter die signatur der restore-funktion angegeben werden
	//etwa <void(COLORREF)>
	template <typename signatur, enumDtor_call_Movetype eMovetype = enumDtor_call_Movetype::First, typename restorefunction_t> auto LockResource( restorefunction_t && restore )
	{
		return AutoResource<template_signatur<signatur>::parameter_t, restorefunction_t, eMovetype>( std::forward<restorefunction_t>( restore ) );
	}
	template <enumDtor_call_Movetype eMovetype = enumDtor_call_Movetype::First, typename resource_t, typename restorefunction_t> auto LockResource(resource_t && value, restorefunction_t && restore)
	{
		return AutoResource<resource_t, restorefunction_t, eMovetype>(std::forward<resource_t>(value), std::forward<restorefunction_t>(restore));
	}
}
#pragma warning(pop)
#pragma pop_macro("ASSERT")