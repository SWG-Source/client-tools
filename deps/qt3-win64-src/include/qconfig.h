/* License information */
#define QT_PRODUCT_LICENSEE "Free"
#define QT_PRODUCT_LICENSE  "qt-free"

/* Build key */
#define QT_BUILD_KEY "win32-msvc2005"


/* Machine byte-order */
#define Q_BIG_ENDIAN 4321
#define Q_LITTLE_ENDIAN 1234
#define Q_BYTE_ORDER Q_LITTLE_ENDIAN

/* Compile time features */
#define QT_POINTER_SIZE 8
#ifndef QT_NO_IMAGEIO_GIF
# define QT_NO_IMAGEIO_GIF
#endif

#ifndef QT_NO_STL
# define QT_NO_STL
#endif

#ifndef QT_NO_OPENGL
# define QT_NO_OPENGL
#endif

#ifndef QT_QWINEXPORT
#define QT_QWINEXPORT
#endif
#ifndef QT_DLL
#define QT_DLL
#endif
