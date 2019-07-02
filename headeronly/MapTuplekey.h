#pragma once

#include <map>
#include <tuple>

namespace WP
{
	template<typename value_t,typename ... key_types > class MapTuplekey : public std::map<std::tuple<key_types...>, value_t>
	{
	public:
		bool get(value_t & value_OutParam, std::tuple<key_types...> key )
		{
			auto iter = find(key);
			if (iter != end())
			{
				value_OutParam = iter->second;
				return true;
			}
			return false ;
		}
		auto get(value_t & value_OutParam, key_types ... keyvalues)
		{
			return get( value_OutParam, key(std::forward<key_types>(keyvalues)...) );
		}
		struct Get_type
		{
			bool found;
			value_t value;

			template<typename value_t>operator value_t() const//ohne template gaebe es compilefehler, wenn value_t bool waere. !!!bedeutet aber, wenn value_t bool ist muss explicit auf .value zugegriffen werden!!!!
			{
				return value;
			}
			operator bool() const
			{
				return found;
			}
		} get(std::tuple<key_types...> key)
		{
			Get_type retvalue;
			retvalue.found = get( retvalue.value, key);
			return retvalue;
		}
		auto get(key_types ... keyvalues)
		{
			return get( key(std::forward<key_types>(keyvalues)...) );
		}

		std::tuple<key_types...> make_key(key_types ... keyvalues)
		{
			return std::forward_as_tuple(keyvalues ...);
		}
		//value_t& operator[](key_types ... keyvalues)//mist, mag der compiler nicht
		auto& operator()(key_types ... keyvalues)
		{
			return operator[](key(keyvalues...));
		}
	};

	//wer nicht weiﬂ was er tut, sollte diesen converter nie benutzen. der compiler kann euch hier nicht mehr helfen.
	template<typename ret_t=void*, typename fn_t> ret_t UnsafeCast(fn_t fn )
	{
		union Convert
		{
			ret_t		pv;
			fn_t		pfn;
		};
		Convert  convert;
		convert.pfn = fn;
		return convert.pv;
	};
}