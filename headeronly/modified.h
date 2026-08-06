#pragma once

/// struct modified und struct notmodified dienen zum komfortablen erkennen, ob eine Zuweisung den ursprünglichen  Wert verändert
/// ist eine spielerei, aber nett
///
/// der verwendete Datentyp muss einen zuweisung-operator und einen vergleichs-operator haben t1 = t2 und t1 == t2 müssen compilierbar sein
///
/// usage C++17
/// if( WS::modified{dest} = source ){...doing}
/// if( WS::notmodified{dest} = source ){...doing}
/// usage C++14
/// if( WS::modified<decltype(dest)>{dest} = source ){...doing}
/// if( WS::notmodified<decltype(dest)>{dest} = source ){...doing}

#include <type_traits> //std::is_const_v

namespace WS
{
    template<typename T> requires (not std::is_const_v<T>) struct modified;
    template<typename T> requires (not std::is_const_v<T>) struct notmodified;
}

template<typename T> requires (not std::is_const_v<T>) struct WS::modified
{
    T& value;
    modified(T& value) :value(value) {}
    bool operator=(T const& v) && //only rvalue desired
        requires requires { this->value == v; this->value = v; }
    {
        if (this->value == v)
            return false;
        this->value = v;
        return true;
    }
    bool operator=(T&& v) && //only rvalue desired
        requires requires { this->value == v; this->value = std::move(v); }
    {
        bool retvalue = not (this->value == v);
        this->value = std::move(v);
        return retvalue;
    }
};
template<typename T> requires (not std::is_const_v<T>) struct WS::notmodified : modified<T>
{
    using base_t = modified<T>;
    using base_t::base_t;     //use ctor
    bool operator=(T const& v) && //only rvalue desired
        requires requires { this->value == v; this->value = v; }
    {
        return not std::move(*this).base_t::operator=(v);
    }
    bool operator=(T&& v) && //only rvalue desired
        requires requires { this->value == v; this->value = std::move(v); }
    {
        return not std::move(*this).base_t::operator=(std::move(v));
    }
};