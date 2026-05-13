
#pragma once

/*
MIT License

Copyright (c) 2026 Werner Schiwietz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

------------------------------------------------------------------------------

compare.h

Small utility for pragmatic total ordering in application code.

Design goals:
- Simple comparison API returning -1 / 0 / 1
- Direct usability with standard algorithms via WS::less
- Special handling for floating-point NaN values
- Treat -0.0 and +0.0 as equal
- Prefer operator<=> when available, otherwise fall back to operator<

Floating-point ordering rules:
- All NaN values are considered equal
- NaN is ordered after all non-NaN values
- Non-NaN values use the type's natural ordering

This utility is intended for everyday application development, not for
bit-level IEEE-754 total ordering or numerical analysis edge cases.
*/


/// <summary>
/// WS::comparer kommt mit floatingpoint typen klar, die bei NaN Werten unsortierbar sind
/// für andere datentype müssten unten evtl. spezialfälle angelegt werden, oder spezialisierungen angelegt werden
/// mit dem less-objekt kann z.b. std::sort direkt aufgerufen werden
/// </summary>

#include <compare>
#include <cmath>
#include <concepts>
#include <type_traits>

namespace WS
{
    template <typename>
    inline constexpr bool dependent_false_v = false;

    template <typename T>
    struct total_order_fallback
    {
        static int compare(const T&, const T&)
        {
            //bitte selbst implementieren. siehe total_order_fallback für floating_point
            static_assert(dependent_false_v<T>, "Für diesen Typ gibt es keine totale Ordnung im unordered-Fall.");
            return 0;
        }
    };

    template <std::floating_point T>
    struct total_order_fallback<T>
    {
        static int compare(const T& a, const T& b)
        {
            //sonderbehandlung IEEE-754 NaN Werte
            const bool a_nan = std::isnan(a);
            const bool b_nan = std::isnan(b);

            if (a_nan && b_nan) return 0;
            if (a_nan) return 1;
            if (b_nan) return -1;
            return 0;
        }
    };

    //alle bekannten rückgabewerte vom operator<=>
    template <typename R>
    concept supported_ordering =
        std::same_as<std::remove_cvref_t<R>, std::strong_ordering> ||
        std::same_as<std::remove_cvref_t<R>, std::weak_ordering> ||
        std::same_as<std::remove_cvref_t<R>, std::partial_ordering>;

    //operator <=> muss einen in supported_ordering aufgeführten typen liefern
    template <typename T>
    concept supported_three_way_comparable =
        requires(const T & a, const T & b)
    {
        { a <=> b } -> supported_ordering;
    };

    //zumindest ein sauberer operator< muss vorhanden sein. wobei gelten sollte wenn a<b dann ist nicht b<a. wenn nicht a<b und nicht b<a, dann a==b, also trichotomie
    template <typename T>
    concept less_than_comparable =
        requires(const T & a, const T & b)
    {
        { a < b } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept comparable =
        supported_three_way_comparable<T> || less_than_comparable<T>;

    template <comparable T>
    int compare(const T& a, const T& b)
    {
        if constexpr (supported_three_way_comparable<T>)
        {
            auto r = (a <=> b);
            using R = std::remove_cvref_t<decltype(r)>;

            if constexpr (std::same_as<R, std::partial_ordering>)
            {
                //darum geht es in den ganzen code. sonderbehandlung für nicht trichotomie einhaltende vergleichsfunktionen
                if (r == std::partial_ordering::unordered)
                    return total_order_fallback<T>::compare(a, b);
            }

            if (r < 0) return -1;
            if (r > 0) return 1;
            return 0;
        }
        else
        {
            if (a < b) return -1;
            if (b < a) return 1;
            return 0;
        }
    }

    struct less_fn
    {
        template <typename T>
        constexpr bool operator()(T const& a, T const& b) const
        {
            return compare(a, b) < 0;
        }
    };

    inline constexpr less_fn less{};
}
