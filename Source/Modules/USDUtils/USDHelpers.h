#pragma once

#if defined(_MSC_VER)
#define BEGIN_DISABLE_USD_WARNINGS                                                                   \
    __pragma(warning(push)) __pragma(warning(disable : 4003)) /* Not enough macro arguments */       \
        __pragma(warning(disable : 4244))                     /* Conversion possible loss of data */ \
        __pragma(warning(disable : 4267))                     /* Conversion possible loss of data */ \
        __pragma(warning(disable : 4305))                     /* Truncation double to float */       \
        __pragma(warning(disable : 5033))                     /* 'register' storage class specifier deprecated */
#define END_DISABLE_USD_WARNINGS __pragma(warning(pop))
#elif defined(__clang__)
#define BEGIN_DISABLE_USD_WARNINGS __pragma(clang diagnostic push) __pragma(clang diagnostic ignored "-Wignored-attributes")
#define END_DISABLE_USD_WARNINGS __pragma(clang diagnostic pop)
#elif defined(__GNUC__)
#define BEGIN_DISABLE_USD_WARNINGS _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wignored-attributes\"")
_Pragma("GCC diagnostic ignored \"-Wparentheses\"")
#define END_DISABLE_USD_WARNINGS _Pragma("GCC diagnostic pop")
#else
#define BEGIN_DISABLE_USD_WARNINGS
#define END_DISABLE_USD_WARNINGS
#endif
