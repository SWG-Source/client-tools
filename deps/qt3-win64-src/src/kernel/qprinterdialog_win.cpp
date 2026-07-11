/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

extern void qt_win_eatMouseMove();
extern void qt_enter_modal( QWidget *widget );
extern void qt_leave_modal( QWidget *widget );

#ifndef Q_OS_TEMP
// If you change this function, make sure you also change the unicode equivalent
static PRINTDLGA *qt_win_make_PRINTDLGA(QWidget *parent, QWin32PrinterPrivate *d, HGLOBAL *tempDevNames)
{
    PRINTDLGA *pd = new PRINTDLGA;
    memset(pd, 0, sizeof(PRINTDLGA));
    pd->lStructSize = sizeof(PRINTDLGA);

    pd->hDevMode = d->devMode;
    pd->hDevNames  = tempDevNames;

    pd->Flags = PD_RETURNDC;
    pd->Flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!(d->printerOptions & QPrinter::PrintSelection))
        pd->Flags |= PD_NOSELECTION;
    if (d->printerOptions & QPrinter::PrintPageRange) {
        pd->nMinPage = d->minPage;
        pd->nMaxPage = d->maxPage;
    }

    if(!(d->printerOptions & QPrinter::PrintToFile))
        pd->Flags |= PD_DISABLEPRINTTOFILE;

    if (d->printRange == QPrinter::Selection)
        pd->Flags |= PD_SELECTION;
    else if (d->printRange == QPrinter::PageRange)
        pd->Flags |= PD_PAGENUMS;
    else
        pd->Flags |= PD_ALLPAGES;

    // As stated by MSDN, to enable collate option when minpage==maxpage==0
    // set the PD_NOPAGENUMS flag
    if (pd->nMinPage==0 && pd->nMaxPage==0)
        pd->Flags |= PD_NOPAGENUMS;

    if (d->printToFile)
        pd->Flags |= PD_PRINTTOFILE;
    pd->hwndOwner = parent ? parent->winId() : 0;
    pd->nFromPage = QMAX(d->fromPage, d->minPage);
    pd->nToPage   = QMIN(d->toPage, d->maxPage);
    pd->nCopies = d->devModeA()->dmCopies;

    return pd;
}

// If you change this function, make sure you also change the unicode equivalent
static void qt_win_clean_up_PRINTDLGA(PRINTDLGA **pd)
{
    delete *pd;
    *pd = 0;
}

// If you change this function, make sure you also change the unicode equivalent
static void qt_win_read_back_PRINTDLGA(PRINTDLGA *pd, QWin32PrinterPrivate *d)
{
    if (pd->Flags & PD_SELECTION) {
        d->printRange = QPrinter::Selection;
        d->fromPage = 0;
        d->toPage = 0;
    } else if (pd->Flags & PD_PAGENUMS) {
        d->printRange = QPrinter::PageRange;
        d->fromPage = pd->nFromPage;
        d->toPage = pd->nToPage;
    } else {
        d->printRange = QPrinter::AllPages;
        d->fromPage = d->minPage;
        d->toPage = d->maxPage;
    }

    d->printToFile = (pd->Flags & PD_PRINTTOFILE) != 0;

    d->readDevnames(pd->hDevNames);
    d->readDevmode(pd->hDevMode);
}
#endif // Q_OS_TEMP

#ifdef UNICODE
// If you change this function, make sure you also change the ansi equivalent
static PRINTDLGW *qt_win_make_PRINTDLGW(QWidget *parent, QWin32PrinterPrivate *d, HGLOBAL *tempDevNames)
{
    PRINTDLGW *pd = new PRINTDLGW;
    memset(pd, 0, sizeof(PRINTDLGW));
    pd->lStructSize = sizeof(PRINTDLGW);

    pd->hDevMode = d->devMode;
    pd->hDevNames  = tempDevNames;

    pd->Flags = PD_RETURNDC;
    pd->Flags |= PD_USEDEVMODECOPIESANDCOLLATE;

    if (!(d->printerOptions & QPrinter::PrintSelection))
        pd->Flags |= PD_NOSELECTION;
    if (d->printerOptions & QPrinter::PrintPageRange) {
        pd->nMinPage = d->minPage;
        pd->nMaxPage = d->maxPage;
    }

    if(!(d->printerOptions & QPrinter::PrintToFile))
        pd->Flags |= PD_DISABLEPRINTTOFILE;

    if (d->printRange == QPrinter::Selection)
        pd->Flags |= PD_SELECTION;
    else if (d->printRange == QPrinter::PageRange)
        pd->Flags |= PD_PAGENUMS;
    else
        pd->Flags |= PD_ALLPAGES;

    // As stated by MSDN, to enable collate option when minpage==maxpage==0
    // set the PD_NOPAGENUMS flag
    if (pd->nMinPage==0 && pd->nMaxPage==0)
        pd->Flags |= PD_NOPAGENUMS;

    if (d->printToFile)
        pd->Flags |= PD_PRINTTOFILE;
    pd->hwndOwner = parent ? parent->winId() : 0;
    pd->nFromPage = QMAX(d->fromPage, d->minPage);
    pd->nToPage   = QMIN(d->toPage, d->maxPage);
    pd->nCopies = d->devModeW()->dmCopies;

    return pd;
}

// If you change this function, make sure you also change the ansi equivalent
static void qt_win_clean_up_PRINTDLGW(PRINTDLGW **pd)
{
    delete *pd;
    *pd = 0;
}

// If you change this function, make sure you also change the ansi equivalent
static void qt_win_read_back_PRINTDLGW(PRINTDLGW *pd, QWin32PrinterPrivate *d)
{
    if (pd->Flags & PD_SELECTION) {
        d->printRange = QPrinter::Selection;
        d->fromPage = 0;
        d->toPage = 0;
    } else if (pd->Flags & PD_PAGENUMS) {
        d->printRange = QPrinter::PageRange;
        d->fromPage = pd->nFromPage;
        d->toPage = pd->nToPage;
    } else {
        d->printRange = QPrinter::AllPages;
        d->fromPage = d->minPage;
        d->toPage = d->maxPage;
    }

    d->printToFile = (pd->Flags & PD_PRINTTOFILE) != 0;

    d->readDevnames(pd->hDevNames);
    d->readDevmode(pd->hDevMode);

}
#endif // UNICODE

int QWin32PrinterPrivate::showPrintDialog()
{
    if (!devMode) {
        qWarning("QWin32PrinterPrivate::exec(), printer not initialized");
        return false;
    }

    QWidget *parent = getParent();
    if (parent)
        parent = parent->topLevelWidget();
    else
        parent = qApp->activeWindow();

    if (parent) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(parent, &e);
        qt_enter_modal(parent);
    }

    HGLOBAL *tempDevNames = createDevNames();

    bool result;
    void *voidp;
    QT_WA({
        PRINTDLGW *pd = qt_win_make_PRINTDLGW(parent, this, tempDevNames);
        voidp = pd; // store until later
        result = PrintDlgW(pd);
        if (result && pd->hDC == 0)
            result = false;
    }, {
        PRINTDLGA *pd = qt_win_make_PRINTDLGA(parent, this, tempDevNames);
        voidp = pd; // store until later
        result = PrintDlgA(pd);
        if (result && pd->hDC == 0)
            result = false;
    });

    if (!result)
        qWarning( "Error calling PrintDlg: %d", CommDlgExtendedError() );

    if (parent) {
        qt_leave_modal(parent);
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(parent, &e);
    }

    qt_win_eatMouseMove();

    // write values back...
    if (result) {
        QT_WA({
            PRINTDLGW *pd = reinterpret_cast<PRINTDLGW *>(voidp);
            qt_win_read_back_PRINTDLGW(pd, this);
            qt_win_clean_up_PRINTDLGW(&pd);
        }, {
            PRINTDLGA *pd = reinterpret_cast<PRINTDLGA *>(voidp);
            qt_win_read_back_PRINTDLGA(pd, this);
            qt_win_clean_up_PRINTDLGA(&pd);
        });
    }

    // Cleanup...
    GlobalFree(tempDevNames);

    return result;
}
