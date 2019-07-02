#pragma once

//by werner schiwietz
//Funktionbeschreibung: Dtor_call fuehrt im Destruktor oder explizit durch Release eine Methode/Funktion/Lamda/std::function oder operator() aus
//
//(einfache erzeugeung eines objekts per CreateDtorCall, siehe unten)
//Dtor_call bekommt im konstruktor 
//	einen zeiger auf ein objekt (das objekt muss selbstverständlich länger leben als Dtor_callFirstDtor selbst) oder einen shared_pointer(der ist life-time sicher)
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

#include <tuple>
#include <memory>


namespace Dtor_call_Helper
{	//quelle variadic parameter http://stackoverflow.com/questions/16868129/how-to-store-variadic-template-arguments

	//generiere_sequenz_type<N> bedeutet, der compiler (also kein laufzeitaufwand) erzeugt einen datentyp index<0,1,2,... N-1>.
	//die templateparameter des index-datentyps werden benutzt um ein tuple in seine einzelteile zerlegt einer methode zu uebergeben
	template <size_t... integer_sequenz> struct index{}; 
    template <size_t N, size_t... integer_sequenz> struct generiere_sequenz_type : generiere_sequenz_type<N - 1, N - 1, integer_sequenz...> {};
    template <size_t... integer_sequenz> struct generiere_sequenz_type<0, integer_sequenz...> : index<integer_sequenz...>{};

	template <typename Objekt,typename Method,typename... Args,size_t... integer_sequenz> auto call_func( Objekt * obj, Method method, std::tuple<Args...>& tup, index<integer_sequenz...>)
	{
		return (obj->*method)( std::get<integer_sequenz>(tup)... );//uebergibt den tuple in einzelteilen der methode
	}
	template <typename Function,typename... Args,size_t... integer_sequenz> auto call_func( void *, Function function, std::tuple<Args...>& tup, index<integer_sequenz...>)
	{
		return (function)( std::get<integer_sequenz>(tup)... );//uebergibt den tuple in einzelteilen der funktion
	}
	template <typename Objekt,typename Method,typename... Args> auto call_func(Objekt * obj, Method method, std::tuple<Args...>& tup)
	{
		return call_func(obj, method, tup, generiere_sequenz_type<sizeof...(Args)>{});
	}
}

namespace
{
	template<typename ReleaseMethod_type, bool moveable=std::is_move_assignable<ReleaseMethod_type>::value> struct _helper;
	template<typename ReleaseMethod_type> struct _helper<ReleaseMethod_type,true> 
	{
		static void swap( ReleaseMethod_type &l, ReleaseMethod_type &r)
		{
			std::swap(l,r);
		}
	};
	template<typename ReleaseMethod_type> struct _helper<ReleaseMethod_type,false>
	{
		static void swap( ReleaseMethod_type &l, ReleaseMethod_type &r) = delete;
	};
}

//wenn ihr diese klasse benutzen wollt, macht euch gedanken, was bei der zweiten zuweisung passieren soll. beziehungsweise bei nebenläufiger benutzung
//z.zt. gibt es drei versionen
// FirstDtor, LastDtor, OnlyFirstCtor 
//schaut euch an was die tun, oder fragt werner wenn ihr euch nicht sicher seid

enum class enumDtor_call_Movetype
{
	FirstDtor,	  //standard, bei erneuter zuweisung wird der vorherige dtor-call ausgelöst
	LastDtor,	  //bei erneuter zuweisung wird der dtor-call des zugewiesenen sofort ausgelöst, this bleibt unveraendert. z.b. fuer funktionen fällt mir nichts ein, was nicht auch mit OnlyFirstCtor machbar waere
	OnlyFirstCtor //bei erneuter zuweisung wird der dtor-call des zugewiesenen nicht(nie) ausgelöst, this bleibt unveraendert. z.b. fuer CDC::SetFont(Old), da dabei i.d.r. der immer nur der erste font wieder hergestellt werden soll
};
template <enumDtor_call_Movetype eMovetype, typename LockedObjekt_type,typename ReleaseMethod_type,typename ... ReleaseArgs> class Dtor_call
{
	std::shared_ptr<LockedObjekt_type>	LockedPtr;	//dient nur dazu ggf. den referenzcounter des shared_ptr zu erhoehen
	LockedObjekt_type*					pLocked		= nullptr;
	ReleaseMethod_type					fnRelease;	//lambdas nicht assign-able
	std::tuple<ReleaseArgs...>			releaseargs;//parameter fuer releasemethode zwischenspeichern
	bool								release		= false;

	template <bool enabled_shared=std::is_base_of<std::enable_shared_from_this<LockedObjekt_type>,LockedObjekt_type>::value> LockedObjekt_type* SetSharedPtr( LockedObjekt_type* pLocked );
	template <> LockedObjekt_type* SetSharedPtr<true>( LockedObjekt_type* pLocked )
	{
		try
		{
			//sollte LockedObjekt_type von enable_shared_from_this abgeleitet sein, sollte shared_from_this funktionieren. 
			//dazu muesste pLocked aber auch als shared_ptr angelegt worden sein, sonst exception (c++17, aber msvc c++14 wirft auch exception).
			//mit c++17 haette wir mehr moeglichkeiten
			LockedPtr = pLocked->shared_from_this();
		}
		catch(...)
		{
#			ifdef ASSERT
				ASSERT( "LockedObjekt_type ist zwar von enable_shared_from_this abgeleitet, aber nicht als shared_ptr angelegt worden");
#			endif
		}
		return pLocked;
	}
	template <> LockedObjekt_type* SetSharedPtr<false>( LockedObjekt_type* pLocked )
	{
		return pLocked;
	}
public:
	Dtor_call( LockedObjekt_type* pLocked, ReleaseMethod_type fnRelease, ReleaseArgs ... releaseargs ) 
		: pLocked(SetSharedPtr(pLocked))
		, fnRelease(fnRelease)
		, releaseargs(releaseargs...)
		, release(true)
	{}
	Dtor_call( std::shared_ptr<LockedObjekt_type> LockedPtr, ReleaseMethod_type fnRelease, ReleaseArgs ... releaseargs ) 
		: pLocked(LockedPtr.get())
		, fnRelease(fnRelease)
		, releaseargs(releaseargs...)
		, release(true)
		, LockedPtr(LockedPtr)
	{
#		ifdef ASSERT
			ASSERT( pLocked );
#		endif
	}
	~Dtor_call()
	{
		try
		{
			CallitNow();
		}
		catch(...)
		{
#			ifdef ASSERT
				ASSERT( __FUNCTION__ " exception" );
#			endif
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
		if( this->pLocked == r.pLocked )
			Move<eMovetype>(std::move(r));
		else
			//objekt sind nicht die selben, also auf jeden fall release aufs this->pLocked machen
			Move<enumDtor_call_Movetype::FirstDtor>(std::move(r));
		return *this;
	}

	void swap(Dtor_call & r)
	{
		static_assert( std::is_move_assignable<ReleaseMethod_type>::value, "es gibt einen move-assign irgendwo auf this der nicht funktioniert. Der Compiler zeigt die Ctor-Zeile, nicht die Zuweisung selbst. z.B. Lambda-Funktionen koennen nicht zugewiesen werden" );// ggf error C2338: 
		std::swap(this->fnRelease,r.fnRelease);

		std::swap(this->pLocked,r.pLocked);
		std::swap(this->release,r.release);
		std::swap(this->releaseargs,r.releaseargs);
		std::swap(this->LockedPtr,r.LockedPtr);
	}
public:
	bool Empty() const
	{
		return this->release==false;
	}
	template <typename return_type> bool CallitNow( return_type & retvalue )//wenn rueckgabewert true, dann enthaelt retvalue den rueckgabewert der gerufenen funktion. nur maximal einmal, erfolgreich aufrufbar
	{
		if( release )
		{
			release = false;
			retvalue = Dtor_call_Helper::call_func( this->pLocked, this->fnRelease, this->releaseargs );
			this->LockedPtr.reset();//ggf. vorhandenes referenziertes obj wird nicht mehr gebraucht
			return true;
		}
		return false;
	}
	void CallitNow()
	{
		if( release )
		{
			release = false;
			Dtor_call_Helper::call_func( this->pLocked, this->fnRelease, this->releaseargs );
			this->LockedPtr.reset();//ggf. vorhandenes referenziertes obj wird nicht mehr gebraucht
		}
	}
	void Release()//same as CallitNow
	{
		CallitNow();
	}
	void Discard()//führt die funktion/methode nicht (nie) aus
	{
		release = false;
		this->LockedPtr.reset();//ggf. vorhandenes referenziertes obj wird nicht mehr gebraucht
	}

protected:
	template<enumDtor_call_Movetype eMovetype>void Move(Dtor_call && r ) = delete;
	template<> void Move<enumDtor_call_Movetype::FirstDtor>(Dtor_call && r )
	{
		if( this != &r )
		{
			Dtor_call temp;//move_assign würde zu endlosrekusion führen
			temp.swap( r );
			this->swap( temp );
		}
	}
	template<> void Move<enumDtor_call_Movetype::LastDtor>(Dtor_call && r )
	{
		if( this->Empty() )
		{
			swap( r );
		}
		else if( this != &r )
		{
			Dtor_call temp;
			temp.swap( r );
		}
	}
	template<> void Move<enumDtor_call_Movetype::OnlyFirstCtor>(Dtor_call && r )
	{
		if( this->Empty() )
		{
			swap( r );
		}
		else if( this != &r )
		{
			Dtor_call temp;
			temp.swap( r );
			temp.Discard();
		}
	}
};

template<enumDtor_call_Movetype eMovetype=enumDtor_call_Movetype::FirstDtor, typename ReleaseFunction,typename ... ReleaseArgs> 
inline auto CreateDtorCallFn( ReleaseFunction fnRelease, ReleaseArgs ... releaseargs ) //dass kann mein template nicht selbst aufloesen, also funktion mit pararater. das springt CreateDtorCall in funktion mit LockedObjekt_type
{
	return Dtor_call<eMovetype,void,ReleaseFunction,ReleaseArgs...>( (void*)nullptr, fnRelease, releaseargs ... );
}
template<enumDtor_call_Movetype eMovetype=enumDtor_call_Movetype::FirstDtor, typename ReleaseFunction,typename ... ReleaseArgs> 
inline auto CreateDtorCall( ReleaseFunction fnRelease, ReleaseArgs ... releaseargs ) 
{
	return Dtor_call<eMovetype,void,ReleaseFunction,ReleaseArgs...>( (void*)nullptr, fnRelease, releaseargs ... );
}

template<enumDtor_call_Movetype eMovetype=enumDtor_call_Movetype::FirstDtor, typename LockedObjekt_type,typename ReleaseMethod_type,typename ... ReleaseArgs>
inline auto CreateDtorCall( LockedObjekt_type* pLocked, ReleaseMethod_type fnRelease, ReleaseArgs ... releaseargs )
{
	return Dtor_call<eMovetype,LockedObjekt_type,ReleaseMethod_type,ReleaseArgs...>( pLocked, fnRelease, releaseargs ... );
}
template<enumDtor_call_Movetype eMovetype=enumDtor_call_Movetype::FirstDtor, typename LockedObjekt_type,typename ReleaseMethod_type,typename ... ReleaseArgs> 
inline auto CreateDtorCall( std::shared_ptr<LockedObjekt_type> LockedPtr, ReleaseMethod_type fnRelease, ReleaseArgs ... releaseargs )
{
	return Dtor_call<eMovetype,LockedObjekt_type,ReleaseMethod_type,ReleaseArgs...>( LockedPtr, fnRelease, releaseargs ... );
}
