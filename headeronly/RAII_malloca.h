#pragma once 
//Copyright (c) 2020 Werner Schiwietz Werner(dot)githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

#include "Ptr_Array.h"

namespace WS
{
	/// <summary>
	/// RAII_malloca stellt ein RAII-Objekt für die Funktionsgruppe
	/// _malloca  _freea 
	/// zur verfügung
	/// 
	/// da es speicher ggf auf den Stack allokiert sind Kopien und Zuweisungen explizit verhindert
	/// </summary>
	class RAII_malloca 
	{
	public:
		void*	ptr{};
		size_t	bytelen{};
		~RAII_malloca() { release(); }
		RAII_malloca( size_t bytes ) noexcept(false); //seht in der hilfe zu _malloca die exception-hinweise (SEH) ein

		void release()
		{
			if(this->ptr)
			{
				_freea( this->ptr );
				this->ptr = nullptr;
				bytelen = 0;
			}
		}

		ptr_array<void*> get() && = delete;
		ptr_array<void*> get() & { return ptr_array<void*>{this->ptr,this->bytelen}; } //nur auf lvalues sinnvoll. der anwender muss sicherstellen, das this lange genug lebt
		operator ptr_array<void*>() & { return get(); } //nur auf lvalues sinnvoll. der anwender muss sicherstellen, das this lange genug lebt
		operator ptr_array<void*>() && = delete;

		RAII_malloca( RAII_malloca const & ) = delete;
		RAII_malloca( RAII_malloca && r) = delete;
		RAII_malloca& operator=( RAII_malloca const & ) = delete;
		RAII_malloca& operator=( RAII_malloca && r) = delete;
	};
}

#ifndef RAII_malloca_SEH
	namespace WS
	{
		inline RAII_malloca::RAII_malloca( size_t bytes ) noexcept(false)
		{
			this->ptr = _malloca( bytes);
			if( this->ptr )
				this->bytelen = bytes;
		}
	}
#else
	//__try usw ist MS quatsch. 
	// ich denke da sollte man lieber mit /EHa und _set_se_translator arbeite. ich habe aber keine lust mich mit den quatsch auseinander zu setzen
	#include <excpt.h>
	namespace WS
	{
		inline RAII_malloca::RAII_malloca( size_t bytes ) noexcept(false)
		{
			__try
			{
				this->ptr = _malloca( bytes);
				if( this->ptr )
					this->bytelen = bytes;
			}
			__finally
			{
				if( AbnormalTermination() )
					throw std::exception( __FUNCTION__ " exception thrown" );
			}
		}
	}
#endif