#pragma once

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

//erlaubt sind void (*)( void ) funktionen, die mit SetAction gesetzt werden k�nnen. Das objekt kann als rvalue weitergereicht werden
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
