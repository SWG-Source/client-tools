#include <qglobal.h>
#include <qconfig.h>

#ifndef QT_POINTER_SIZE
# ifdef Q_OS_WIN32
#  define QT_POINTER_SIZE 4
# elif Q_OS_WIN64
#  define QT_POINTER_SIZE 8
# else
#  error This platform is unsupported
# endif
#endif /* QT_POINTER_SIZE */

#define SQLITE_PTR_SZ QT_POINTER_SIZE

#ifdef UNICODE
# undef UNICODE
#endif

#ifdef Q_CC_MSVC
# pragma warning(disable: 4018)
# pragma warning(disable: 4761)
#endif
