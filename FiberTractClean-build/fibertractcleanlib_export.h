
#ifndef FIBERTRACTCLEANLIB_EXPORT_H
#define FIBERTRACTCLEANLIB_EXPORT_H

#ifdef FIBERTRACTCLEANLIB_STATIC_DEFINE
#  define FIBERTRACTCLEANLIB_EXPORT
#  define FIBERTRACTCLEANLIB_TEMPLATE_EXPORT
#  define FIBERTRACTCLEANLIB_NO_EXPORT
#else
#  ifndef FIBERTRACTCLEANLIB_EXPORT
#    ifdef FiberTractCleanLib_EXPORTS
        /* We are building this library */
#      define FIBERTRACTCLEANLIB_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define FIBERTRACTCLEANLIB_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef FIBERTRACTCLEANLIB_TEMPLATE_EXPORT
#    ifdef FiberTractCleanLib_EXPORTS
        /* We are building this library */
#      define FIBERTRACTCLEANLIB_TEMPLATE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define FIBERTRACTCLEANLIB_TEMPLATE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef FIBERTRACTCLEANLIB_NO_EXPORT
#    define FIBERTRACTCLEANLIB_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef FIBERTRACTCLEANLIB_DEPRECATED
#  define FIBERTRACTCLEANLIB_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef FIBERTRACTCLEANLIB_DEPRECATED_EXPORT
#  define FIBERTRACTCLEANLIB_DEPRECATED_EXPORT FIBERTRACTCLEANLIB_EXPORT FIBERTRACTCLEANLIB_DEPRECATED
#endif

#ifndef FIBERTRACTCLEANLIB_DEPRECATED_NO_EXPORT
#  define FIBERTRACTCLEANLIB_DEPRECATED_NO_EXPORT FIBERTRACTCLEANLIB_NO_EXPORT FIBERTRACTCLEANLIB_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define FIBERTRACTCLEANLIB_NO_DEPRECATED
#endif

#endif
