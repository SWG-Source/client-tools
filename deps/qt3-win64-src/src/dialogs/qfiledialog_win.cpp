/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the Qt 3 compatibility classes of the Qt Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include "qapplication.h"
#include "private/qapplication_p.h"
#include "qt_windows.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qdir.h"
#include "qstringlist.h"
#include "qlibrary.h"

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#include "shlobj.h"

#ifndef Q_OS_TEMP
#include <commdlg.h>
#endif

// Don't remove the lines below!
//
// resolving the W methods manually is needed, because Windows 95 doesn't include
// these methods in Shell32.lib (not even stubs!), so you'd get an unresolved symbol
// when Qt calls getEsistingDirectory(), etc.
typedef LPITEMIDLIST (WINAPI *PtrSHBrowseForFolder)(BROWSEINFO*);
static PtrSHBrowseForFolder ptrSHBrowseForFolder = 0;
typedef BOOL (WINAPI *PtrSHGetPathFromIDList)(LPITEMIDLIST,LPWSTR);
static PtrSHGetPathFromIDList ptrSHGetPathFromIDList = 0;

static void resolveLibs()
{
#ifndef Q_OS_TEMP
    static bool triedResolve = false;

    if (!triedResolve) {
#ifdef QT_THREAD_SUPPORT
        // protect initialization
        QMutexLocker locker(qt_global_mutexpool ?
                             qt_global_mutexpool->get(&triedResolve) : 0);
        // check triedResolve again, since another thread may have already
        // done the initialization
        if (triedResolve) {
            // another thread did initialize the security function pointers,
            // so we shouldn't do it again.
            return;
        }
#endif

        triedResolve = true;
        if (qt_winunicode) {
            QLibrary lib("shell32");
            ptrSHBrowseForFolder = (PtrSHBrowseForFolder) lib.resolve("SHBrowseForFolderW");
            ptrSHGetPathFromIDList = (PtrSHGetPathFromIDList) lib.resolve("SHGetPathFromIDListW");
        }
    }
#endif
}
#ifdef Q_OS_TEMP
#define PtrSHBrowseForFolder SHBrowseForFolder ;
#define PtrSHGetPathFromIDList SHGetPathFromIDList;
#endif


extern const char qt_file_dialog_filter_reg_exp[]; // defined in qfiledialog.cpp

const unsigned int maxNameLen = 1023;
const unsigned int maxMultiLen = 65535;

// Returns the wildcard part of a filter.
static QString extractFilter(const QString& rawFilter)
{
    QString result = rawFilter;
    QRegExp r(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    int index = r.search(result);
    if (index >= 0)
        result = r.cap(2);
    return result.replace(QChar(' '), QChar(';'));
}

// Makes a list of filters from ;;-separated text.
static QStringList makeFiltersList(const QString &filter)
{
    QString f(filter);

    if (f.isEmpty())
        f = QFileDialog::tr("All Files (*.*)");

    if (f.isEmpty())
        return QStringList();

    int i = f.find(";;", 0);
    QString sep(";;");
    if (i == -1) {
        if (f.find("\n", 0) != -1) {
            sep = "\n";
            i = f.find(sep, 0);
        }
    }

    return QStringList::split(sep, f );
}

// Makes a NUL-oriented Windows filter from a Qt filter.
static QString winFilter(const QString& filter)
{
    QStringList filterLst = makeFiltersList(filter);
    QStringList::Iterator it = filterLst.begin();
    QString winfilters;
    for (; it != filterLst.end(); ++it) {
        winfilters += *it;
        winfilters += QChar::null;
        winfilters += extractFilter(*it);
        winfilters += QChar::null;
    }
    winfilters += QChar::null;
    return winfilters;
}

static QString selFilter(const QString& filter, DWORD idx)
{
    QStringList filterLst = makeFiltersList(filter);
    return filterLst[(int)idx - 1];
}

#ifndef Q_OS_TEMP
// Static vars for OFNA funcs:
static QByteArray aInitDir;
static QByteArray aInitSel;
static QByteArray aTitle;
static QByteArray aFilter;
// Use ANSI strings and API

// If you change this, then make sure you change makeOFN (below) too
static
OPENFILENAMEA* makeOFNA(QWidget* parent,
                         const QString& initialSelection,
                         const QString& initialDirectory,
                         const QString& title,
                         const QString& filters,
                         QFileDialog::Mode mode)
{
    if (parent)
        parent = parent->topLevelWidget();
    else
        parent = qApp->activeWindow();

    aTitle = title.local8Bit();
    aInitDir = QDir::convertSeparators(initialDirectory).local8Bit();
    if (initialSelection.isEmpty())
        aInitSel = QCString("");
    else
        aInitSel = QDir::convertSeparators(initialSelection).local8Bit();
    unsigned int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    aInitSel.resize(maxLen + 1);                // make room for return value
    aFilter = filters.local8Bit();

    OPENFILENAMEA* ofn = new OPENFILENAMEA;
    memset(ofn, 0, sizeof(OPENFILENAMEA));

#if defined(Q_CC_BOR) && (WINVER >= 0x0500) && (_WIN32_WINNT >= 0x0500)
    // according to the MSDN, this should also be necessary for MSVC, but
    // OPENFILENAME_SIZE_VERSION_400A is in not Microsoft header, as it seems
    if (QApplication::winVersion()==Qt::WV_NT || QApplication::winVersion()&Qt::WV_DOS_based) {
        ofn->lStructSize= OPENFILENAME_SIZE_VERSION_400A;
    } else {
        ofn->lStructSize= sizeof(OPENFILENAMEA);
    }
#else
    ofn->lStructSize        = sizeof(OPENFILENAMEA);
#endif
    ofn->hwndOwner        = parent ? parent->winId() : 0;
    ofn->lpstrFilter        = aFilter;
    ofn->lpstrFile        = aInitSel.data();
    ofn->nMaxFile        = maxLen;
    ofn->lpstrInitialDir = aInitDir.data();
    ofn->lpstrTitle        = aTitle.data();
    ofn->Flags                = (OFN_NOCHANGEDIR | OFN_HIDEREADONLY);

    if (mode == QFileDialog::ExistingFile ||
         mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    if (mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_ALLOWMULTISELECT | OFN_EXPLORER);

    return ofn;
}

static void cleanUpOFNA(OPENFILENAMEA** ofn)
{
    delete *ofn;
    *ofn = 0;
}
#endif

static QString tFilters, tTitle, tInitDir;

#ifdef UNICODE
// If you change this, then make sure you change makeOFNA (above) too
static
OPENFILENAME* makeOFN(QWidget* parent,
                       const QString& initialSelection,
                       const QString& initialDirectory,
                       const QString& title,
                       const QString& filters,
                       QFileDialog::Mode mode)
{
    if (parent)
        parent = parent->topLevelWidget();
    else
        parent = qApp->activeWindow();

    tInitDir = QDir::convertSeparators(initialDirectory);
    tFilters = filters;
    tTitle = title;
    QString initSel = QDir::convertSeparators(initialSelection);

    unsigned int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    TCHAR *tInitSel = new TCHAR[maxLen+1];
    if (initSel.length() > 0 && initSel.length() <= maxLen)
        memcpy(tInitSel, initSel.ucs2(), (initSel.length()+1)*sizeof(QChar));
    else
        tInitSel[0] = 0;

    OPENFILENAME* ofn = new OPENFILENAME;
    memset(ofn, 0, sizeof(OPENFILENAME));

#if defined(Q_CC_BOR) && (WINVER >= 0x0500) && (_WIN32_WINNT >= 0x0500)
    // according to the MSDN, this should also be necessary for MSVC, but
    // OPENFILENAME_SIZE_VERSION_400 is in not Microsoft header, as it seems
    if (QApplication::winVersion()==Qt::WV_NT || QApplication::winVersion()&Qt::WV_DOS_based) {
        ofn->lStructSize= OPENFILENAME_SIZE_VERSION_400;
    } else {
        ofn->lStructSize        = sizeof(OPENFILENAME);
    }
#else
    ofn->lStructSize        = sizeof(OPENFILENAME);
#endif
    ofn->hwndOwner        = parent ? parent->winId() : 0;
    ofn->lpstrFilter        = (TCHAR *)tFilters.ucs2();
    ofn->lpstrFile        = tInitSel;
    ofn->nMaxFile        = maxLen;
    ofn->lpstrInitialDir = (TCHAR *)tInitDir.ucs2();
    ofn->lpstrTitle        = (TCHAR *)tTitle.ucs2();
    ofn->Flags                = (OFN_NOCHANGEDIR | OFN_HIDEREADONLY);

    if (mode == QFileDialog::ExistingFile ||
         mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    if (mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_ALLOWMULTISELECT | OFN_EXPLORER);

    return ofn;
}


static void cleanUpOFN(OPENFILENAME** ofn)
{
    delete (*ofn)->lpstrFile;
    delete *ofn;
    *ofn = 0;
}

#endif // UNICODE

QString QFileDialog::winGetOpenFileName(const QString &initialSelection,
                                         const QString &filter,
                                         QString* initialDirectory,
                                         QWidget *parent, const char* /*name*/,
                                         const QString& caption,
                                         QString* selectedFilter)
{
    QString result;

    QString isel = initialSelection;

    if (initialDirectory && initialDirectory->left(5) == "file:")
        initialDirectory->remove(0, 5);
    QFileInfo fi(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.dirPath(true);
        if (isel.isEmpty())
            isel = fi.fileName();
    }

    if (!fi.exists())
        *initialDirectory = QDir::homeDirPath();

    QString title = caption;
    if (title.isNull())
        title = tr("Open");

    DWORD selFilIdx;

    int idx = 0;
    if (selectedFilter && !selectedFilter->isEmpty()) {
        QStringList filterLst = makeFiltersList(filter);
        idx = filterLst.findIndex(*selectedFilter);
    }

    if (parent) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(parent, &e);
        qt_enter_modal(parent);
    }
    QT_WA({
        // Use Unicode strings and API
        OPENFILENAME* ofn = makeOFN(parent, isel,
                                     *initialDirectory, title,
                                     winFilter(filter), ExistingFile);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileName(ofn)) {
            result = QString::fromUcs2((ushort*)ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        cleanUpOFN(&ofn);
    } , {
        // Use ANSI strings and API
        OPENFILENAMEA* ofn = makeOFNA(parent, isel,
                                       *initialDirectory, title,
                                       winFilter(filter), ExistingFile);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileNameA(ofn)) {
            result = QString::fromLocal8Bit(ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        cleanUpOFNA(&ofn);
    });
    if (parent) {
        qt_leave_modal(parent);
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(parent, &e);
    }

    if (result.isEmpty()) {
        return result;
    }
    else {
        QFileInfo fi(result);
        *initialDirectory = fi.dirPath();
        if (selectedFilter)
            *selectedFilter = selFilter(filter, selFilIdx);
        return fi.absFilePath();
    }
}


QString QFileDialog::winGetSaveFileName(const QString &initialSelection,
                                         const QString &filter,
                                         QString* initialDirectory,
                                         QWidget *parent, const char* /*name*/,
                                         const QString& caption,
                                         QString* selectedFilter)
{
    QString result;

    QString isel = initialSelection;
    if (initialDirectory && initialDirectory->left(5) == "file:")
        initialDirectory->remove(0, 5);
    QFileInfo fi(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.dirPath(true);
        if (isel.isEmpty())
            isel = fi.fileName();
    }

    if (!fi.exists())
        *initialDirectory = QDir::homeDirPath();

    QString title = caption;
    if (title.isNull())
        title = tr("Save As");

    DWORD selFilIdx;

    int idx = 0;
    if (selectedFilter && !selectedFilter->isEmpty()) {
        QStringList filterLst = makeFiltersList(filter);
        idx = filterLst.findIndex(*selectedFilter);
    }

    if (parent) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(parent, &e);
        qt_enter_modal(parent);
    }
    QT_WA({
        // Use Unicode strings and API
        OPENFILENAME* ofn = makeOFN(parent, isel,
                                     *initialDirectory, title,
                                     winFilter(filter), AnyFile);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetSaveFileName(ofn)) {
            result = QString::fromUcs2((ushort*)ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        cleanUpOFN(&ofn);
    } , {
        // Use ANSI strings and API
        OPENFILENAMEA* ofn = makeOFNA(parent, isel,
                                       *initialDirectory, title,
                                       winFilter(filter), AnyFile);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetSaveFileNameA(ofn)) {
            result = QString::fromLocal8Bit(ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        cleanUpOFNA(&ofn);
    });
    if (parent) {
        qt_leave_modal(parent);
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(parent, &e);
    }

    if (result.isEmpty()) {
        return result;
    }
    else {
        QFileInfo fi(result);
        *initialDirectory = fi.dirPath();
        if (selectedFilter)
            *selectedFilter = selFilter(filter, selFilIdx);
        return fi.absFilePath();
    }
}



QStringList QFileDialog::winGetOpenFileNames(const QString &filter,
                                              QString* initialDirectory,
                                              QWidget *parent,
                                              const char* /*name*/,
                                              const QString& caption,
                                              QString* selectedFilter)
{
    QStringList result;
    QFileInfo fi;
    QDir dir;
    QString isel;

    if (initialDirectory && initialDirectory->left(5) == "file:")
        initialDirectory->remove(0, 5);
    fi = QFileInfo(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.dirPath(true);
        isel = fi.fileName();
    }

    if (!fi.exists())
        *initialDirectory = QDir::homeDirPath();

    QString title = caption;
    if (title.isNull())
        title = tr("Open ");

    DWORD selFilIdx;

    int idx = 0;
    if (selectedFilter && !selectedFilter->isEmpty()) {
        QStringList filterLst = makeFiltersList(filter);
        idx = filterLst.findIndex(*selectedFilter);
    }

    if (parent) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(parent, &e);
        qt_enter_modal(parent);
    }
    QT_WA({
        OPENFILENAME* ofn = makeOFN(parent, isel,
                                     *initialDirectory, title,
                                     winFilter(filter), ExistingFiles);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileName(ofn)) {
            QString fileOrDir = QString::fromUcs2((ushort*)ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
            int offset = fileOrDir.length() + 1;
            if (ofn->lpstrFile[offset] == 0) {
                // Only one file selected; has full path
                fi.setFile(fileOrDir);
                QString res = fi.absFilePath();
                if (!res.isEmpty())
                    result.append(res);
            }
            else {
                // Several files selected; first string is path
                dir.setPath(fileOrDir);
                QString f;
                while(!(f = QString::fromUcs2((ushort*)ofn->lpstrFile+offset)).isEmpty()) {
                    fi.setFile(dir, f);
                    QString res = fi.absFilePath();
                    if (!res.isEmpty())
                        result.append(res);
                    offset += f.length() + 1;
                }
            }
        }
        cleanUpOFN(&ofn);
    } , {
        OPENFILENAMEA* ofn = makeOFNA(parent, isel,
                                       *initialDirectory, title,
                                       winFilter(filter), ExistingFiles);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileNameA(ofn)) {
            QCString fileOrDir(ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
            int offset = fileOrDir.length() + 1;
            if (ofn->lpstrFile[offset] == '\0') {
                // Only one file selected; has full path
                fi.setFile(QString::fromLocal8Bit(fileOrDir));
                QString res = fi.absFilePath();
                if (!res.isEmpty())
                    result.append(res);
            }
            else {
                // Several files selected; first string is path
                dir.setPath(QString::fromLocal8Bit(fileOrDir));
                QCString f;
                while(!(f = QCString(ofn->lpstrFile + offset)).isEmpty()) {
                    fi.setFile(dir, QString::fromLocal8Bit(f));
                    QString res = fi.absFilePath();
                    if (!res.isEmpty())
                        result.append(res);
                    offset += f.length() + 1;
                }
            }
            cleanUpOFNA(&ofn);
        }
    });
    if (parent) {
        qt_leave_modal(parent);
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(parent, &e);
    }

    if (!result.isEmpty()) {
        *initialDirectory = fi.dirPath();    // only save the path if there is a result
        if (selectedFilter)
            *selectedFilter = selFilter(filter, selFilIdx);
    }
    return result;
}

// MFC Directory Dialog. Contrib: Steve Williams (minor parts from Scott Powers)

static int __stdcall winGetExistDirCallbackProc(HWND hwnd,
                                                UINT uMsg,
                                                LPARAM lParam,
                                                LPARAM lpData)
{
#ifndef Q_OS_TEMP
    if (uMsg == BFFM_INITIALIZED && lpData != 0) {
        QString *initDir = (QString *)(lpData);
        if (!initDir->isEmpty()) {
            // ### Lars asks: is this correct for the A version????
            QT_WA({
                SendMessage(hwnd, BFFM_SETSELECTION, true, Q_ULONG(initDir->ucs2()));
            } , {
                SendMessageA(hwnd, BFFM_SETSELECTION, true, Q_ULONG(initDir->ucs2()));
            });
        }
    } else if (uMsg == BFFM_SELCHANGED) {
        QT_WA({
            resolveLibs();
            TCHAR path[MAX_PATH];
            ptrSHGetPathFromIDList(LPITEMIDLIST(lParam), path);
            QString tmpStr = QString::fromUcs2((ushort*)path);
            if (!tmpStr.isEmpty())
                SendMessage(hwnd, BFFM_ENABLEOK, 1, 1);
            else
                SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
            SendMessage(hwnd, BFFM_SETSTATUSTEXT, 1, Q_ULONG(path));
        } , {
            char path[MAX_PATH];
            SHGetPathFromIDListA(LPITEMIDLIST(lParam), path);
            QString tmpStr = QString::fromLocal8Bit(path);
            if (!tmpStr.isEmpty())
                SendMessageA(hwnd, BFFM_ENABLEOK, 1, 1);
            else
                SendMessageA(hwnd, BFFM_ENABLEOK, 0, 0);
            SendMessageA(hwnd, BFFM_SETSTATUSTEXT, 1, Q_ULONG(path));
        });
    }
#endif
    return 0;
}

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE     0x0040   // Use the new dialog layout with the ability to resize
#endif


QString QFileDialog::winGetExistingDirectory(const QString& initialDirectory,
                                             QWidget *parent,
                                             const char* /*name*/,
                                             const QString& caption)
{
#ifndef Q_OS_TEMP
    QString currentDir = QDir::currentDirPath();
    QString result;
    if (parent)
        parent = parent->topLevelWidget();
    else
        parent = qApp->activeWindow();
    QString title = caption;
    if (title.isNull())
        title = tr("Select a Directory");

    if (parent) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(parent, &e);
        qt_enter_modal(parent);
    }
    QT_WA({
        resolveLibs();
        QString initDir = QDir::convertSeparators(initialDirectory);
        TCHAR path[MAX_PATH];
        TCHAR initPath[MAX_PATH];
        initPath[0] = 0;
        path[0] = 0;
        tTitle = title;
        BROWSEINFO bi;
        bi.hwndOwner = (parent ? parent->winId() : 0);
        bi.pidlRoot = NULL;
        bi.lpszTitle = (TCHAR*)tTitle.ucs2();
        bi.pszDisplayName = initPath;
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE;
        bi.lpfn = winGetExistDirCallbackProc;
        bi.lParam = Q_ULONG(&initDir);
        LPITEMIDLIST pItemIDList = ptrSHBrowseForFolder(&bi);
        if (pItemIDList) {
            ptrSHGetPathFromIDList(pItemIDList, path);
            IMalloc *pMalloc;
            if (SHGetMalloc(&pMalloc) != NOERROR)
                result = QString("");
            else {
                pMalloc->Free(pItemIDList);
                pMalloc->Release();
                result = QString::fromUcs2((ushort*)path);
            }
        } else
            result = QString("");
        tTitle = QString("");
    } , {
        QString initDir = QDir::convertSeparators(initialDirectory);
        char path[MAX_PATH];
        char initPath[MAX_PATH];
        QByteArray ctitle = title.local8Bit();
        initPath[0]=0;
        path[0]=0;
        BROWSEINFOA bi;
        bi.hwndOwner = (parent ? parent->winId() : 0);
        bi.pidlRoot = NULL;
        bi.lpszTitle = ctitle;
        bi.pszDisplayName = initPath;
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE;
        bi.lpfn = winGetExistDirCallbackProc;
        bi.lParam = Q_ULONG(&initDir);
        LPITEMIDLIST pItemIDList = SHBrowseForFolderA(&bi);
        if (pItemIDList) {
            SHGetPathFromIDListA(pItemIDList, path);
            IMalloc *pMalloc;
            if (SHGetMalloc(&pMalloc) != NOERROR)
                result = QString("");
            else {
                pMalloc->Free(pItemIDList);
                pMalloc->Release();
                result = QString::fromLocal8Bit(path);
            }
        } else
            result = QString("");
    });
    if (parent) {
        qt_leave_modal(parent);
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(parent, &e);
    }

    // Due to a bug on Windows Me, we need to reset the current
    // directory
    if ((qWinVersion() == Qt::WV_98 || qWinVersion() == Qt::WV_Me) && QDir::currentDirPath() != currentDir)
        QDir::setCurrent(currentDir);

    if (!result.isEmpty())
        result.replace("\\", "/");
    return result;
#else
    return QString();
#endif
}


#endif
