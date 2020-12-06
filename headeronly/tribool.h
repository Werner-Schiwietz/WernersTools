#pragma once
//Copyright (c) 2020 Werner Schiwietz
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

#include <stdexcept>
// WS::tribool kapselt die stati false, true und invalid
// invalid ist ein wert, wenn er mit dem default-ctor erzeugt wurde, bzw durch zuweisung eines invalid tribool.
// operator bool() und ähnliche werfen std::exception, wenn wert invalid ist. vorher also ggf. mit valid() pruefen
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
