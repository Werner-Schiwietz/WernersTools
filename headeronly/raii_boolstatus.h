#pragma once
///Copyright (c) 2022 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugeh�rigen Dokumentationsdateien (die "Software") erh�lt, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschr�nkung mit der Software zu handeln, einschlie�lich und ohne Einschr�nkung der Rechte zur Nutzung, zum Kopieren, �ndern, Zusammenf�hren, Ver�ffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verf�gung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis m�ssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE M�NGELGEW�HR UND OHNE JEGLICHE AUSDR�CKLICHE ODER STILLSCHWEIGENDE GEW�HRLEISTUNG, EINSCHLIE�LICH, ABER NICHT BESCHR�NKT AUF
//DIE GEW�HRLEISTUNG DER MARKTG�NGIGKEIT, DER EIGNUNG F�R EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERF�GUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR F�R ANSPR�CHE, SCH�DEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCH�FTEN MIT DER SOFTWARE ERGEBEN. 

/// <summary>
/// raii_boolstatus by werner schiwietz 2022-02-25
/// nach ctor status==true
/// nach zuweisung/kopie status==false, bis es keine kopie mehr gibt
/// reset() trennt kopie ab. kopie hat status==true orignal eine referenz weniger
/// </summary>
///
/// <example>
/// struct A
/// {
///           WS::raii_boolstatus status;
///           bool GetStatus() const{ return status.status();}
///           [[nodiscard]]WS::raii_boolstatus SetStatus(){return status;}
/// }
/// A a;
/// a.GetStaus()==true;
/// {
///           auto locka = a.SetStatus();
///           a.GetStaus()==false;
///           locka.status()==false;
/// }
/// a.GetStaus()==true;
/// </example>
///

#include <memory>
#include <atomic>

namespace WS
{
    class raii_boolstatus
    {
    public:
        using status_counter_type = int;
        using value_t = std::atomic<status_counter_type>;
    private:
        static constexpr status_counter_type unset_status() { return status_counter_type{};}
        std::shared_ptr<value_t> valuePtr;
    public:
        ~raii_boolstatus()
        {
            --(*valuePtr);//z�hlt shared-objekt herunter. wenn letzte kopie zerst�rt wird �ndert sich der status zu true
        }
        raii_boolstatus()//legt ein objekt an. operator bool liefert true
            : valuePtr(new value_t{unset_status()})
        {}
        raii_boolstatus(raii_boolstatus const & r)//legt eine kopie von objekt an. operator bool liefert false, bis objekt zerst�rt wird
            : valuePtr(r.valuePtr)
        {
            ++(*valuePtr);
        }
        raii_boolstatus& operator=( raii_boolstatus const & r ) &
        {
            --(*valuePtr);//z�hlt alten shared-objekt herunter. evtl �ndert sich dessen status
            valuePtr = r.valuePtr;
            ++(*valuePtr);//status erstmal ungleich unset_status
            return *this;
        }
        raii_boolstatus( raii_boolstatus && r ) : raii_boolstatus(r){}                      //wir k�nnen kein move gebrauchen
        raii_boolstatus& operator=( raii_boolstatus && r ) &{return operator=(r);}    //wir k�nnen kein move gebrauchen

        void reset() {*this=raii_boolstatus{};}                                            //l�st verbindung zu master, gibt dessen status frei und macht sich zu unabh�ngigen master

        status_counter_type ref_counter() const {return (*valuePtr);} //liefert die anzahl kopien. braucht niemand, kann aber zum testen hilfreich sein
        bool status() const &      { return (*valuePtr)==unset_status(); }    //nur auf lvalue aufrufbar um falschen aufruf z.b. if( obj.GetStatus() ) geht nicht. richtig if( auto status=obj.GetStatus() )
        operator bool() const & { return status(); }                               //nur auf lvalue aufrufbar um falschen aufruf z.b. if( obj.GetStatus() ) geht nicht. richtig if( auto status=obj.GetStatus() )
    };
}
