#pragma once

namespace WS
{
	//Combiner verarbeiten die R�ckgabewerte der gerufenen Funktionen
	template<typename T,std::conditional_t<std::is_same<void,T>::value==false, T, int> initvalue=std::conditional_t<std::is_same<void,T>::value==false, T, int>{}> struct combiner_last 
	{	//combiner_last liefert in this->value den r�ckgabewert der letzten gerufenen Funktion, oder, wenn kein Funktion verkn�pft ist, den initvalue
		static constexpr T init_val{initvalue};
		using id_t = size_t;//typename Signal_trait<T()>::id_t;
		T		value{initvalue};
		id_t	last_signal_handler_id{};
		auto operator()(){return this->value;}
		auto operator()(T const & v_in,id_t id={})&{this->last_signal_handler_id=id;this->value = v_in;return *this;}
		auto operator()(T const & v_in,id_t id={}) && { return operator()(v_in,id); }
		operator T const &(){return value;}
	};
	template<> struct combiner_last<void,0> 
	{	//spezialisierung r�ckgabe void kann nicht gesammelt werden, weil funktionen kann kein void-parameter mit namen �bergeben werden
	};
}
