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

namespace WS
{
	//Combiner verarbeiten in WS::Signal die Rückgabewerte der gerufenen Funktionen 
	template<typename T,std::conditional_t<std::is_same<void,T>::value==false, T, int> initvalue=std::conditional_t<std::is_same<void,T>::value==false, T, int>{}> struct combiner_last 
	{	//combiner_last liefert in this->value den rückgabewert des letzten Aufrufs von operator()(T), oder, wenn kein Aufruf erfolgte, den initvalue
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
	{	//spezialisierung rückgabe void kann nicht gesammelt werden, weil funktionen kann kein void-parameter mit namen übergeben werden
	};
}
