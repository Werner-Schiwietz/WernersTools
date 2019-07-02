#pragma once

#include <functional>

namespace WrapedCalls
{
	//std::bin kommt mit funktion die WINAPI (evtl. andere modifier auch) in der definition haben nicht klar

	inline HGLOBAL GlobalFree( HGLOBAL h ){
		return ::GlobalFree( h );}
	inline BOOL GlobalUnlock( HGLOBAL h ){
		return ::GlobalUnlock( h );}
};

template <typename ret_type> class DestructorCall
{
protected:
	std::function<ret_type ()> _fn;
public:
	DestructorCall( DestructorCall const & ) = delete;
	DestructorCall( DestructorCall && ) = delete;
	DestructorCall& operator=( DestructorCall const & ) = delete;
	DestructorCall& operator=( DestructorCall && ) = delete;

	DestructorCall( std::function<ret_type ()> const & fn ):_fn(fn){}
	~DestructorCall()
	{
		try
		{
			_fn();
		}
		catch(...)
		{
		}
	}
};

//erlaubt sind void (*)( void ) funktionen, die mit SetAction gesetzt werden können. Das objekt kann als rvalue weitergereicht werden
class DestructorCall2 : DestructorCall<void>
{
public:
	DestructorCall2( std::function<void()> const & fn=[]()->void{}) : DestructorCall( fn ){}
	DestructorCall2( DestructorCall2 const & ) = delete;
	DestructorCall2& operator=( DestructorCall2 const & ) = delete;
	DestructorCall2( DestructorCall2 && r ) : DestructorCall( _fn = std::move(r._fn) )
	{
		r._fn = []()->void{};
	}
	DestructorCall2& operator=( DestructorCall2 && r ) 
	{
		_fn();//ggf. letzte resource freigeben
		_fn = std::move(r._fn);
		r._fn = []()->void{};
		return *this;
	}
	void SetAction( std::function<void()> const & fn )
	{
		_fn();//ggf. letzte resource freigeben
		_fn = fn;
	}
};

//using z.b. DestructorRestore<bool> RestoreIt( this->m_fFlag=true, false );
template <class T> class DestructorRestore
{
public:
	DestructorRestore( T& rTValue, T TRestoreValue )
		: m_rTValue( rTValue )
		, m_TRestoreValue( TRestoreValue )
		{ }
	~DestructorRestore( ){ 
		try
		{
			m_rTValue = m_TRestoreValue;
		}
		catch(...)
		{
		}
	}
private:
	T&	m_rTValue;
	T	m_TRestoreValue;
};// class DestructorRestore


///////////////////////////////////////
///test und anwenundungsbeispiel
//#define __TEST_DestructorCall
#ifdef __TEST_DestructorCall
class boolcontainer
{
	bool _f;
public:
	boolcontainer( bool fInitial=false ):_f(fInitial){}
	void funktion( bool f ){
		_f=f;}
};

int __DestructorCallTest()
{
	boolcontainer objekt;
	DestructorCall<void> autoObjektRestore( std::bind( &boolcontainer::funktion, &objekt, false ) );
	objekt.funktion( true );

	//HGLOBAL hData = GlobalAlloc( GMEM_MOVEABLE, 100 );
	//DestructorCall<void> autoFreeMem( std::bind( &WrapedCalls::GlobalFree, hData ) );


	return 0;
}
int n__DestructorCallTest=__DestructorCallTest();
#endif// __TEST_DestructorCall
///test und anwenundungsbeispiel
///////////////////////////////////////
