#pragma once
///Copyright (c) 2023 Werner Schiwietz Werner.githubpublic(at)gisbw(dot)de
//Jedem, der eine Kopie dieser Software und der zugehörigen Dokumentationsdateien (die "Software") erhält, wird hiermit kostenlos die Erlaubnis erteilt, 
//ohne Einschränkung mit der Software zu handeln, einschließlich und ohne Einschränkung der Rechte zur Nutzung, zum Kopieren, Ändern, Zusammenführen, Veröffentlichen, 
//Verteilen, Unterlizenzieren und/oder Verkaufen von Kopien der Software, und Personen, denen die Software zur Verfügung gestellt wird, dies unter den folgenden Bedingungen zu gestatten:
//Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen Kopien oder wesentlichen Teilen der Software enthalten sein.
//DIE SOFTWARE WIRD OHNE MÄNGELGEWÄHR UND OHNE JEGLICHE AUSDRÜCKLICHE ODER STILLSCHWEIGENDE GEWÄHRLEISTUNG, EINSCHLIEßLICH, ABER NICHT BESCHRÄNKT AUF
//DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT, DER EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND DER NICHTVERLETZUNG VON RECHTEN DRITTER, ZUR VERFÜGUNG GESTELLT. 
//DIE AUTOREN ODER URHEBERRECHTSINHABER SIND IN KEINEM FALL HAFTBAR FÜR ANSPRÜCHE, SCHÄDEN ODER ANDERE VERPFLICHTUNGEN, OB IN EINER VERTRAGS- ODER 
//HAFTUNGSKLAGE, EINER UNERLAUBTEN HANDLUNG ODER ANDERWEITIG, DIE SICH AUS, AUS ODER IN VERBINDUNG MIT DER SOFTWARE ODER DER NUTZUNG ODER ANDEREN 
//GESCHÄFTEN MIT DER SOFTWARE ERGEBEN. 

/// <summary>
/// WS::bitfeld<type_name> zerlegt per begin/end einen integralen datentyp in seine einzelen bits
/// WS::bitfeld_split<type_name> zerlegt den "wert" in firstbit und die restlichen bits
/// WS::union_bitfeld<type_name> bildte die vereinigungsmenge der bits
/// WS::intersect_bitfeld<type_name> bildte die schnittsmenge der bits
/// WS::invert_bitfeld<type_name> invertierte die bits
/// </summary>
/// 

#include "to_underlying.h"

namespace WS
{
	template<typename bitfeld_type> struct bitfeld_split
	{
		using bitfeld_t = bitfeld_type;
		bitfeld_t	first_bit{};
		bitfeld_t	rest{};

		bitfeld_split(){}
		bitfeld_split(bitfeld_t const & bits,bitfeld_t const &first_bit)
		{
			if( WS::to_integral(bits) )
			{
				for( this->first_bit = first_bit; WS::to_integral(this->first_bit); this->first_bit = bitfeld_t( WS::to_integral(this->first_bit) << 1 ) )
				{
					if( WS::to_integral(this->first_bit) & WS::to_integral(bits) )
					{
						this->rest = bitfeld_t( WS::to_integral(bits) & (~WS::to_integral(this->first_bit)) );
						return;
					}
				}
			}
		}
		bitfeld_split(bitfeld_t const & bits) : bitfeld_split(bits,bitfeld_t(1)){}

		bool						operator==(bitfeld_split const & r) const {return this->first_bit==r.first_bit;}
		bool						operator!=(bitfeld_split const & r) const {return !operator==(r);}

		bool						is_multibit() const {return this->first_bit && this->rest;}

		bitfeld_t					getFirstBit() const {return this->first_bit;}
		bitfeld_split<bitfeld_t>	getRest() const {return bitfeld_split<bitfeld_t>{this->rest,bitfeld_t(WS::to_integral(this->first_bit)<<1)};}
		bitfeld_t					getValue() const {return this->first_bit | this->rest;}
	};
	//WS::split_bits(value); liefert ein WS::bitfeld_split ohne template-parameter zu benötigen
	template<typename bitfeld_t> auto split_bits(bitfeld_t bits){return bitfeld_split<bitfeld_t>{bits};}

	template<typename bitfeld_type> struct bitfeld_iterator
	{
		using bitfeld_t = bitfeld_type;
		bitfeld_split<bitfeld_t> data;

		bitfeld_iterator():data(){}
		bitfeld_iterator(bitfeld_t bitfeld):data(bitfeld){}

		bitfeld_iterator& operator++() &{ this->data=this->data.getRest();return *this;}
		bitfeld_iterator operator++(int){auto retvalue=*this;operator++();return retvalue;}
		
		bitfeld_t operator*() const{return data.getFirstBit();}
		bitfeld_t getFirstBit() const {return data.getFirstBit();}
		operator bitfeld_t() const { return operator*();}//führt zu error C2593: 'operator ==' is ambiguous mit bool, lol

		//operator bool() const { return static_cast<bool>(getFirstBit());}//führt zu error C2593: 'operator ==' is ambiguous mit bitfeld_t, lol

		bool	operator==(bitfeld_iterator const & r)const{ return this->data == r.data;}
		bool	operator!=(bitfeld_iterator const & r)const{ return this->data != r.data;}

		bool is_multibit() const{return this->data.is_multibit();}
	};

	//dient in erster Linie dem iterieren über die bits, die arbeit macht WS::bitfeld_split
	template<typename bitfeld_t> struct bitfeld 
	{
		bitfeld_t value{};
		bitfeld() {}
		explicit bitfeld(bitfeld_t const & r) : value(r){}

		operator bitfeld_t() const {return value;}

		auto		begin() const {return bitfeld_iterator<bitfeld_t>{this->value};}
		auto		end() const {return bitfeld_iterator<bitfeld_t>{};}

		bool		is_multibit() const { return begin().data.is_multibit(); }
	};
	//WS::make_bitfeld(value);liefert ein WS::bitfeld ohne template-parameter zu benötigen
	template<typename bitfeld_t> auto make_bitfeld(bitfeld_t bits){return bitfeld<bitfeld_t>{bits};}
}

namespace WS
{
	template<typename bitfeld_t> bitfeld_t Union( bitfeld_t const & l, bitfeld_t const & r ) //l | r
	{
		return bitfeld_t{static_cast<decltype(WS::to_integral(l))>(WS::to_integral(l) | WS::to_integral(r))};
	}
	template<typename bitfeld_t> bitfeld_t Intersect( bitfeld_t const & l, bitfeld_t const & r ) //l & r
	{
		return bitfeld_t{static_cast<decltype(WS::to_integral(l))>(WS::to_integral(l) & WS::to_integral(r))};
	}
	template<typename bitfeld_t> bitfeld_t Invert( bitfeld_t const & r ) //~r
	{
		return bitfeld_t{static_cast<decltype(WS::to_integral(r))>(~WS::to_integral(r))};
	}
	template<typename bitfeld_t> bitfeld_t Without( bitfeld_t const & l, bitfeld_t const & r )//l & ~r //schneidet r aus l heraus und liefert das den Rest von l
	{
		return bitfeld_t{static_cast<decltype(WS::to_integral(l))>(WS::to_integral(l) & ~WS::to_integral(r))};
	}
}
