#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

//#include <winsock2.h>
// avoid including of cygwin socket header 
//#define _SYS_SOCKET_H 1

// Get Qt defines/settings

#include "qglobal.h"

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

// DNS system header files are a mess!
// <resolv.h> includes <arpa/nameser.h>. <arpa/nameser.h> is using
// 'u_char' and includes <sys/types.h>.  Now the problem is that
// <sys/types.h> defines 'u_char' only if __USE_BSD is defined.
// __USE_BSD is defined in <features.h> if _BSD_SOURCE is defined.
#ifndef _BSD_SOURCE
#  define _BSD_SOURCE
#endif

// 1) need to reset default environment if _BSD_SOURCE is defined
// 2) need to specify POSIX thread interfaces explicitly in glibc 2.0
// 3) it seems older glibc need this to include the X/Open stuff
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <unistd.h>


// We are hot - unistd.h should have turned on the specific APIs we requested


#ifdef QT_THREAD_SUPPORT
#include <pthread.h>
#endif

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <dlfcn.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/ioctl.h>
// Cygwin does not provide <sys/ipc.h> and <sys/shm.h> because it
// doesn't support SysV IPC or shared memory. See for example:
// 	http://afni.nimh.nih.gov/afni/afniboard/messages/1725.html
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <windows.h>

#include <netinet/in.h>
#include <resolv.h>


#if !defined(QT_NO_COMPAT)
#define QT_STATBUF		struct stat
#define QT_STATBUF4TSTAT	struct stat
#define QT_STAT			::stat
#define QT_FSTAT		::fstat
#define QT_STAT_REG		S_IFREG
#define QT_STAT_DIR		S_IFDIR
#define QT_STAT_MASK		S_IFMT
#define QT_STAT_LNK		S_IFLNK
#define QT_FILENO		fileno
#define QT_OPEN			::open
#define QT_CLOSE		::close
#define QT_LSEEK		::lseek
#define QT_READ			::read
#define QT_WRITE		::write
#define QT_ACCESS		::access
#define QT_GETCWD		::getcwd
#define QT_CHDIR		::chdir
#define QT_MKDIR		::mkdir
#define QT_RMDIR		::rmdir
#define QT_OPEN_RDONLY		O_RDONLY
#define QT_OPEN_WRONLY		O_WRONLY
#define QT_OPEN_RDWR		O_RDWR
#define QT_OPEN_CREAT		O_CREAT
#define QT_OPEN_TRUNC		O_TRUNC
#define QT_OPEN_APPEND		O_APPEND
#endif

#define QT_SIGNAL_RETTYPE	void
#define QT_SIGNAL_ARGS		int
#define QT_SIGNAL_IGNORE	SIG_IGN

#ifndef QT_SOCKLEN_T
#define QT_SOCKLEN_T		socklen_t
#endif 

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)
#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf
#endif

// we use cygwin socket code, which does not define the following required definitions
#define WSAGETSELECTEVENT(l)   LOWORD(l)
#define FD_READ_BIT      0
#define FD_READ          (1 << FD_READ_BIT)
#define FD_WRITE_BIT     1
#define FD_WRITE         (1 << FD_WRITE_BIT)
#define FD_OOB_BIT       2
#define FD_OOB           (1 << FD_OOB_BIT)
#define FD_ACCEPT_BIT    3
#define FD_ACCEPT        (1 << FD_ACCEPT_BIT)
#define FD_CONNECT_BIT   4
#define FD_CONNECT       (1 << FD_CONNECT_BIT)
#define FD_CLOSE_BIT     5
#define FD_CLOSE         (1 << FD_CLOSE_BIT)
typedef unsigned int   SOCKET;
extern "C" int PASCAL WSAAsyncSelect(SOCKET,HWND,unsigned int,long);


#endif // QPLATFORMDEFS_H
