#pragma once
/// struct getold dient zum setzen eines wertes und weiterarbeiten mit dem alten wert
/// ist eine spielerei, aber nett
///
/// usage C++17
/// auto oldvalue = WS::getold{value} = newvalue;
/// usage C++14
/// auto oldvalue = WS::getold<decltype(value)>{value} = newvalue;

namespace WS
{
    template<typename T> struct getold
    {
        T& value;
        getold(T& value) :value(value) {}

        T operator=(T v)&& //only rvalue desired
        {
            std::swap(value, v);
            return std::move(v);
        }
    };
}