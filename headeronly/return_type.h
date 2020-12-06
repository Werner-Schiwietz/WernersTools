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

namespace WS
{
	struct compare_bool
	{
		virtual bool Valid() const = 0;
		operator bool() const { return Valid();}
		bool operator!() const { return !operator bool();}
		bool operator==(bool r) const { return operator bool()==r;}
		bool operator!=(bool r) const { return operator bool()!=r;}
		friend bool operator==(bool l, compare_bool const & r) { return r==l;}
		friend bool operator!=(bool l, compare_bool const & r) { return r!=l;}
	};
	enum class return_type_error_code : __int8
	{
		invalid = -1, //-1 allgemeiner fehler, oder nicht initialisiert
		none,		  //0 bedeutet immer kein fehler
	};
	template<typename value_type,typename error_code_type=return_type_error_code> struct return_type : compare_bool
	{
		using value_t = value_type;
		using error_code_t = error_code_type;
		value_t			value {};
		error_code_t	error_code = error_code_t(-1);

		return_type() noexcept(noexcept(value_t{})) {}
		return_type(error_code_t error_code) noexcept(noexcept(value_t{})) : error_code(error_code) {}
		template<typename T>return_type(T && value) noexcept(noexcept(std::decay_t<value_t>(std::forward<T>(value)))) : value(std::forward<T>(value)),error_code(error_code_t(0)) {}

		//bool operator==(value_t const & r) const { return toValueType()==r;}
		//bool operator!=(value_t const & r) const { return toValueType()!=r;}

		auto & toValueType() const &					{return this->value;} 
		auto & toValueType() &							{return this->value;} 
		auto && toValueType() &&						{return std::move(this->value);}//ohoh nutzung ohne pr�fung ob valid? value nach aufruf evtl. leer aber valid-status bleibt ggf. true
		operator value_t const & () const &				{return this->value;} 
		operator value_t & () &							{return this->value;} 
		operator value_t && () &&						{return std::move(this->value);}//ohoh nutzung ohne pr�fung ob valid? value nach aufruf evtl. leer aber valid-status bleibt ggf. true
		bool Valid() const override						{return this->error_code==error_code_t(0);}
	};
}
