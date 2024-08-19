#pragma once
#define is_is_char_type_defined

namespace WS
{
    template<typename T>        struct _is_char_type                                                  : std::false_type{};
    template<>                                                                   struct _is_char_type<char>                   : std::true_type{};
    template<>                                                                   struct _is_char_type<wchar_t>                  : std::true_type{};
    template<typename T>        using is_char_type = _is_char_type<std::remove_cv_t<std::remove_reference_t<T>>>;
    template<typename T>        static bool constexpr is_char_type_v = is_char_type<T>::value;

    static_assert( is_char_type_v<char> );
    static_assert( is_char_type_v<char const> );
    static_assert( is_char_type_v<char const *> == false );
    static_assert( is_char_type_v<char *> == false );
    static_assert( is_char_type_v<char const * const > == false );
    static_assert( is_char_type_v<char const &> == true );
    static_assert( is_char_type_v<const char> );
    static_assert( is_char_type_v<wchar_t> );
    static_assert( is_char_type_v<unsigned char> == false );
    static_assert( std::is_same_v<char,__int8> == true );//???
    static_assert( is_char_type_v<__int8> == std::is_same_v<char,__int8> );
}