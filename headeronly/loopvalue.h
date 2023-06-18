//#pragma once
//Copyright (c) 2020 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 


/// usage C++17
/// z.b. per range based for
/// 
/// for( auto index : WS::loopvalue(7,5) )
/// { doing von 7 --> 5 also {7,6,5} }
/// 
/// or 
/// 
/// for( WS::loopvalue index{unsigned __int8(0),unsigned __int8(255)}; index.isvalid(); ++index)
/// { doing }

#pragma once

#include <stdexcept>

namespace WS
{
	/// <summary>
	/// loopvalue<loopvalue_type> die loopvalue-variable vom typ loopvalue_type läuft von start_wert bis einschließlich ende_wert. benutzt werden loopvalue_type::operator++() bzw loopvalue_type::operator--()
	/// beim zugriff auf einen "invalid()" wert wird eine std::out_of_range exception geworfen
	/// operator++ ruft next(), also intern ++ oder -- auf die zugrunde liegende loopvalue-variable. es geht nur in eine richtung
	/// </summary>
	/// <typeparam name="loopvalue_type"></typeparam>
	template<typename loopvalue_type>
	struct loopvalue
	{   using loopvalue_t = loopvalue_type;
	private:
		mutable bool		valid = false;
		mutable loopvalue_t i;                //i nimmt alle werte zwischen start_wert und ende_wert an, incl. der beiden grenzwerte.
		loopvalue_t const   start_wert;
		loopvalue_t const   ende_wert;
		bool const          inc;              //wenn true wird operator++ sonst operator-- benutzt. default ohne überlauf (also wenn e<s false sonst true)
	public:
		loopvalue( loopvalue_t start_wert, loopvalue_t ende_wert, bool defaultdirection ) : start_wert(start_wert), ende_wert(ende_wert), i(start_wert), inc(defaultdirection?!(ende_wert<start_wert):(ende_wert<start_wert)), valid(true){}//inc default so, das ohne überlauf gearbeitet wird
		loopvalue( loopvalue_t start_wert, loopvalue_t ende_wert ) : loopvalue( start_wert,ende_wert, true ){}
		loopvalue() : loopvalue( loopvalue_t{}, loopvalue_t{} ){}
		loopvalue( loopvalue const & ) = default;

		loopvalue_t const & to_underlying() const &
		{
			if( isinvalid() )
				throw std::out_of_range{__FUNCTION__ " loopvalue is invalid"};
			return i;
		}
		operator loopvalue_t const &() const & {return to_underlying();}

		bool next() const
		{
			if(i!=ende_wert)//ende erreicht -> wird invalid. geht nur so, anders würde loopvalue<unsigned char>(0,255) nicht funktionieren
			{
				if( inc )
					++i; 
				else
					--i;
			}
			else
				valid = false;
			return isvalid();
		}
		loopvalue const &	operator++() const	& { next(); return *this; }
		loopvalue &			operator++()		& { next(); return *this; }
		loopvalue			operator++(int) const { auto retvalue{*this};next();return retvalue; }

		bool                isvalid() const{ return valid; }
		bool                isinvalid() const{ return !valid; }

		bool operator==(WS::loopvalue<loopvalue_t> const & r) const
		{
			if(    this->start_wert  != r.start_wert
				|| this->ende_wert   != r.ende_wert
				|| this->inc         != r.inc )
			{
				throw std::exception( __FUNCTION__ " loopvalue differing");
			}
			return this->i		== r.i
				&& this->valid	== r.valid;
		}
		bool operator!=(WS::loopvalue<loopvalue_t> const & r) const { return !operator==(r); }

		template<typename loopvalue_type> 
		struct const_iterator
		{	using loopvalue_t = loopvalue_type;
			WS::loopvalue<loopvalue_t> loopvalue;

			const_iterator(){};
			const_iterator(WS::loopvalue<loopvalue_t> loopvalue) : loopvalue(loopvalue){}
			const_iterator(WS::loopvalue<loopvalue_t> loopvalue,bool) : loopvalue(loopvalue){this->loopvalue.i=this->loopvalue.ende_wert;this->loopvalue.valid = false;}//für end

			bool operator==( const_iterator const & r ) const { return this->loopvalue == r.loopvalue; }
			bool operator!=( const_iterator const & r ) const { return this->loopvalue != r.loopvalue; }
			const_iterator& operator++() &
			{
				(void)loopvalue.next();
				return *this;
			}
			const_iterator operator++(int)
			{
				auto retvalue = *this;
				++(*this);
				return retvalue;
			}

			loopvalue_t const & operator*() const &
			{
				if( this->loopvalue.isinvalid() )
					throw std::out_of_range{ __FUNCTION__ " end defereced"};
				return this->loopvalue.i;
			}
			loopvalue_t const * operator->() const &
			{
				if( this->loopvalue.isinvalid() )
					throw std::out_of_range{ __FUNCTION__ " end defereced"};
				return &this->loopvalue.i;
			}
		};
		const_iterator<loopvalue_t> begin() const{ return const_iterator<loopvalue_t>{*this}; }
		const_iterator<loopvalue_t> end()   const{ return const_iterator<loopvalue_t>{*this,false}; }
	};
}
