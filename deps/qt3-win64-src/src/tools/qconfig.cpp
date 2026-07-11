#include <qglobal.h>
#include <qcstring.h>
#include <string.h>
#include <stdlib.h>
/* Install paths from configure */

static char install_prefix         [267];
static char QT_INSTALL_BINS        [267];
static char QT_INSTALL_DOCS        [267];
static char QT_INSTALL_HEADERS     [267];
static char QT_INSTALL_LIBS        [267];
static char QT_INSTALL_PLUGINS     [267];
static char QT_INSTALL_DATA        [267];
static char QT_INSTALL_TRANSLATIONS[267];
static char QT_INSTALL_SYSCONF     [267];

/* strlen( "qt_xxxpath=" ) == 11 */
Q_EXPORT const char *qInstallPath()             { return install_prefix          + 11; }
Q_EXPORT const char *qInstallPathDocs()         { return QT_INSTALL_DOCS         + 11; }
Q_EXPORT const char *qInstallPathHeaders()      { return QT_INSTALL_HEADERS      + 11; }
Q_EXPORT const char *qInstallPathLibs()         { return QT_INSTALL_LIBS         + 11; }
Q_EXPORT const char *qInstallPathBins()         { return QT_INSTALL_BINS         + 11; }
Q_EXPORT const char *qInstallPathPlugins()      { return QT_INSTALL_PLUGINS      + 11; }
Q_EXPORT const char *qInstallPathData()         { return QT_INSTALL_DATA         + 11; }
Q_EXPORT const char *qInstallPathTranslations() { return QT_INSTALL_TRANSLATIONS + 11; }
Q_EXPORT const char *qInstallPathSysconf()      { return QT_INSTALL_SYSCONF      + 11; }

class QConfigInit {
public:
 QConfigInit()
 { 
  QCString qtdir = getenv("QTDIR");
  qtdir.replace("\\","/");
  QCString a;
  a = QCString("qt_nstpath=") + qtdir;                   strcpy(install_prefix         ,a.data()); 
  a = QCString("qt_binpath=") + qtdir + "/bin";          strcpy(QT_INSTALL_BINS        ,a.data()); 
  a = QCString("qt_docpath=") + qtdir + "/doc";          strcpy(QT_INSTALL_DOCS        ,a.data()); 
  a = QCString("qt_hdrpath=") + qtdir + "/include";      strcpy(QT_INSTALL_HEADERS     ,a.data()); 
  a = QCString("qt_libpath=") + qtdir + "/lib";          strcpy(QT_INSTALL_LIBS        ,a.data()); 
  a = QCString("qt_plgpath=") + qtdir + "/plugins";      strcpy(QT_INSTALL_PLUGINS     ,a.data()); 
  a = QCString("qt_datpath=") + qtdir;                   strcpy(QT_INSTALL_DATA        ,a.data()); 
  a = QCString("qt_trnpath=") + qtdir + "/translations"; strcpy(QT_INSTALL_TRANSLATIONS,a.data()); 
  a = QCString("qt_cnfpath=") + qtdir;                   strcpy(QT_INSTALL_SYSCONF     ,a.data()); 
 } 
};
QConfigInit x;
