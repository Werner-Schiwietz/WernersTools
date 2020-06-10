#pragma once
//defaultParam
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

//usage DefaultParameter( Values ... ).set<enum_index>(value).set<index>(value);
//also z.B.
/*
	namespace T1
	{
		auto GetDefaultParam() { return WS::defaultParam(0, 0, 0, 0); }
		void foo2(decltype(GetDefaultParam()) parameter = GetDefaultParam())
		{
			auto p0 = parameter.get<0>(); auto p1 = parameter.get<1>(); auto p2 = parameter.get<2>(); auto p3 = parameter.get<3>();
		}
		auto xxx = []()
		{
			foo2();
			foo2(GetDefaultParam().set<0>(5).set<3>(8));
			return 0;
		}();
	}
	namespace T2
	{
		auto GetDefaultParam() { enum index_t { x, y, w, h }; return WS::defaultParam<index_t>(0, 0, 0, 0); }
		void foo2(decltype(GetDefaultParam()) parameter = GetDefaultParam())
		{
			using index_t = decltype(parameter)::index_t;
			auto p0 = parameter.get<index_t::x>(); auto p1 = parameter.get<index_t::y>(); auto p2 = parameter.get<index_t::w>();auto p3 = parameter.get<index_t::h>();
		}
		auto xxx = []()
		{
			foo2();
			using index_t = decltype(GetDefaultParam())::index_t;
			foo2(GetDefaultParam().set<index_t::x>(5).set<index_t::h>(8));
			return 0;
		}();
	}
*/

#include "type_list.h"//optional
#include <tuple>

namespace WS
{
	template<typename index_type = size_t, typename...Tn>
	struct _defaultParam
	{
		using index_t = index_type;
		using types = WS::typelist<Tn...>;//optional
		std::tuple<Tn...> values;
		_defaultParam(Tn ... defValues) : values(std::forward<Tn>(defValues)...)
		{
		}
		template<index_t index> auto& get()
		{
			return std::get<static_cast<size_t>(index)>(this->values);
		}
		template<index_t index> auto const& get() const
		{
			return std::get<static_cast<size_t>(index)>(this->values);
		}
		template<index_t index, typename value_t> _defaultParam& set(value_t&& value) &
		{
			std::get<static_cast<size_t>(index)>(this->values) = std::forward<value_t>(value);
			return *this;
		}
		template<index_t index, typename value_t> _defaultParam set(value_t&& value) &&
		{
			std::get<static_cast<size_t>(index)>(this->values) = std::forward<value_t>(value);
			return std::move(*this);
		}
	};
	//_defaultParam Creator
	//auch mit c++17 geht 
	//	_defaultParam<enum_type>(1,2,3); nicht sondern nur 
	//	_defaultParam<enum_type,int,int,int>(1,2,3);
	//	mit der vorgeschalteten funktion geht defaultParam<enum_type>(1,2,3)
	template<typename index_type = size_t, typename...Tn>auto defaultParam(Tn&& ... defValues)
	{
		return _defaultParam<index_type, Tn...>(std::forward<Tn>(defValues)...);
	}
}

