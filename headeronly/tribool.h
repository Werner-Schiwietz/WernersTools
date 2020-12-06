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

#include <stdexcept>
// WS::tribool kapselt die stati false, true und invalid
// invalid ist ein wert, wenn er mit dem default-ctor erzeugt wurde, bzw durch zuweisung eines invalid tribool.
// operator bool() und �hnliche werfen std::exception, wenn wert invalid ist. vorher also ggf. mit valid() pruefen
// vergleichsoperatoren vergleichen alle drei moeglichen werte, also keine excpetion wenn ein oder beide werte invalid sind (invalid==invlid)==(false==false)==(true==true)==true
//
//siehe auch UT_tribool.cpp

namespace WS
{
	class tribool//boost tribool gefaellt mir nicht
	{
		unsigned __int8 value=0xff;
	public:
		tribool() = default;
		tribool(bool value) : value(value){}
		tribool( tribool const & r ) : value(r.value){}
		tribool& operator=( tribool const & ) = default;
#		pragma warning(suppress:4458)
		tribool& operator=( bool value )
		{
			this->value = value?1:0;
			return *this;
		}

		bool valid()const
		{
			return this->value==0 || this->value==1;
		}
		bool operator==( bool r )const //(invalid==invalid)==true
		{
			return *this==tribool(r);
		}
		friend bool operator==( bool l, tribool const &r )
		{
			return tribool(l)==r;
		}
		bool operator!=( bool r )const //(invalid==invalid)==true
		{
			return *this!=tribool(r);
		}
		bool operator==( tribool const & r )const //(invalid==invalid)==true
		{
			return this->value==r.value;
		}
		bool operator!=( tribool const & r )const //(invalid==invalid)==true
		{
			return this->value!=r.value;
		}
		operator bool()const
		{
			if( valid()==false )
				throw std::runtime_error( "tribool::operator bool, invalid" );
			return this->value ? true : false;
		}
		bool operator !()const
		{
			return ! this->operator bool();
		}
	};
}
