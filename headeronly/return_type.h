#pragma once


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
	template<typename value_type,typename error_code=return_type_error_code> struct return_type : compare_bool
	{
		using value_t = value_type;
		using error_code_t = error_code;
		value_t			value {};
		error_code_t	error_code = error_code_t(-1);

		return_type() noexcept(noexcept(value_t{})) {}
		return_type(error_code_t error_code) noexcept(noexcept(value_t{})) : error_code(error_code) {}
		template<typename T>return_type(T && value) noexcept(noexcept(std::decay_t<value_t>(std::forward<T>(value)))) : value(std::forward<T>(value)),error_code(error_code_t(0)) {}

		//bool operator==(value_t const & r) const { return toValueType()==r;}
		//bool operator!=(value_t const & r) const { return toValueType()!=r;}

		auto & toValueType() const &					{return this->value;} 
		auto & toValueType() &							{return this->value;} 
		auto && toValueType() &&						{return std::move(this->value);}//ohoh nutzung ohne prüfung ob valid? value nach aufruf evtl. leer aber valid-status bleibt ggf. true
		operator value_t const & () const &				{return this->value;} 
		operator value_t & () &							{return this->value;} 
		operator value_t && () &&						{return std::move(this->value);}//ohoh nutzung ohne prüfung ob valid? value nach aufruf evtl. leer aber valid-status bleibt ggf. true
		bool Valid() const override						{return this->error_code==error_code_t(0);}
	};
}
