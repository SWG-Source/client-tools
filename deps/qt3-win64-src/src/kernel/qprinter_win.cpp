/****************************************************************************
** $Id: qprinter_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QPrinter class for Unix
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Tom and Timi Cecka
** Copyright (C) 2004 Andreas Hausladen
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Peter Kuemmel
** Copyright (C) 2004 Simon Rutishauser
** Copyright (C) 2005 Christian Ehrlicher
** Copyright (C) 2005 Trolltech AS (parts are from gpl'd qt4/win32)
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qplatformdefs.h"

#include "qt_windows.h"

#ifndef QT_NO_PRINTER
#include <math.h>

#include "qprinter.h"
#include "qprinter_p.h"

#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qrect.h"
#include "qwmatrix.h"

// QPrinter states
#define PST_IDLE        0
#define PST_ACTIVE      1
#define PST_ERROR       2
#define PST_ABORTED     3

static const struct {
    int winSizeName;
    QPrinter::PageSize qtSizeName;
} dmMapping[] = {
    { DMPAPER_LETTER,             QPrinter::Letter },
    { DMPAPER_LETTERSMALL,        QPrinter::Letter },
    { DMPAPER_TABLOID,            QPrinter::Tabloid },
    { DMPAPER_LEDGER,             QPrinter::Ledger },
    { DMPAPER_LEGAL,              QPrinter::Legal },
    { DMPAPER_EXECUTIVE,          QPrinter::Executive },
    { DMPAPER_A3,                 QPrinter::A3 },
    { DMPAPER_A4,                 QPrinter::A4 },
    { DMPAPER_A4SMALL,            QPrinter::A4 },
    { DMPAPER_A5,                 QPrinter::A5 },
    { DMPAPER_B4,                 QPrinter::B4 },
    { DMPAPER_B5,                 QPrinter::B5 },
    { DMPAPER_FOLIO,              QPrinter::Folio },
    { DMPAPER_ENV_10,             QPrinter::Comm10E },
    { DMPAPER_ENV_DL,             QPrinter::DLE },
    { DMPAPER_ENV_C3,             QPrinter::C5E },
    { DMPAPER_LETTER_EXTRA,       QPrinter::Letter },
    { DMPAPER_LEGAL_EXTRA,        QPrinter::Legal },
    { DMPAPER_TABLOID_EXTRA,      QPrinter::Tabloid },
    { DMPAPER_A4_EXTRA,           QPrinter::A4},
    { DMPAPER_LETTER_TRANSVERSE,  QPrinter::Letter},
    { DMPAPER_A4_TRANSVERSE,      QPrinter::A4},
    { DMPAPER_LETTER_EXTRA_TRANSVERSE, QPrinter::Letter },
    { DMPAPER_A_PLUS,             QPrinter::A4 },
    { DMPAPER_B_PLUS,             QPrinter::A3 },
    { DMPAPER_LETTER_PLUS,        QPrinter::Letter },
    { DMPAPER_A4_PLUS,            QPrinter::A4 },
    { DMPAPER_A5_TRANSVERSE,      QPrinter::A5 },
    { DMPAPER_B5_TRANSVERSE,      QPrinter::B5 },
    { DMPAPER_A3_EXTRA,           QPrinter::A3 },
    { DMPAPER_A5_EXTRA,           QPrinter::A5 },
    { DMPAPER_B5_EXTRA,           QPrinter::B5 },
    { DMPAPER_A2,                 QPrinter::A2 },
    { DMPAPER_A3_TRANSVERSE,      QPrinter::A3 },
    { DMPAPER_A3_EXTRA_TRANSVERSE,QPrinter::A3 },
    { 0, QPrinter::Custom }
};

static QPrinter::PageSize mapDevmodePageSize(int s)
{
    int i = 0;
    while ((dmMapping[i].winSizeName > 0) && (dmMapping[i].winSizeName != s))
        i++;
    return dmMapping[i].qtSizeName;
}

static int mapPageSizeDevmode(QPrinter::PageSize s)
{
    int i = 0;
 while ((dmMapping[i].winSizeName > 0) && (dmMapping[i].qtSizeName != s))
	i++;
    return dmMapping[i].winSizeName;
}

static struct {
    int winSourceName;
    QPrinter::PaperSource qtSourceName;
}  sources[] = {
    { DMBIN_ONLYONE,        QPrinter::OnlyOne },
    { DMBIN_LOWER,          QPrinter::Lower },
    { DMBIN_MIDDLE,         QPrinter::Middle },
    { DMBIN_MANUAL,         QPrinter::Manual },
    { DMBIN_ENVELOPE,       QPrinter::Envelope },
    { DMBIN_ENVMANUAL,      QPrinter::EnvelopeManual },
    { DMBIN_AUTO,           QPrinter::Auto },
    { DMBIN_TRACTOR,        QPrinter::Tractor },
    { DMBIN_SMALLFMT,       QPrinter::SmallFormat },
    { DMBIN_LARGEFMT,       QPrinter::LargeFormat },
    { DMBIN_LARGECAPACITY,  QPrinter::LargeCapacity },
    { DMBIN_CASSETTE,       QPrinter::Cassette },
    { DMBIN_FORMSOURCE,     QPrinter::FormSource },
    { 0, (QPrinter::PaperSource) -1 }
};

static QPrinter::PaperSource mapDevmodePaperSource(int s)
{
    int i = 0;
    while ((sources[i].winSourceName > 0) && (sources[i].winSourceName != s))
        i++;
    return sources[i].winSourceName ? sources[i].qtSourceName : (QPrinter::PaperSource) s;
}

static int mapPaperSourceDevmode(QPrinter::PaperSource s)
{
    int i = 0;
    while ((sources[i].qtSourceName >= 0) && (sources[i].qtSourceName != s))
        i++;
    return sources[i].winSourceName ? sources[i].winSourceName : s;
}


class QWin32PrinterPrivate : public QPrinterPrivate
{
public:
    HANDLE hPrinter;

    HGLOBAL globalDevMode;
    void  *devMode;
    void  *pInfo;

    HDC    hdc;

    QPrinter::PrinterMode mode;

    int resolution;

    QRect devPageRect;
    QRect devPaperRect;
    float stretch_x;
    float stretch_y;

    int dpi_x;
    int dpi_y;
    int dpi_display;

    bool marginsSpecified;
    uint topMargin;
    uint leftMargin;
    uint bottomMargin;
    uint rightMargin;
    // Printer info
    QString name;
    QString program;
    QString port;
    
    QWidget *p;
    
    bool printToFile;
    int minPage;
    int maxPage;
    int fromPage;
    int toPage;
    
    bool bNeedReinit;

public:
    QWin32PrinterPrivate();
    ~QWin32PrinterPrivate();
    void initialize();
    void initHDC();
    void release();
    void queryDefault();
    bool begin(const QString &output_filename, const QString &doc_name, int &state);
    bool end(int &state);

    inline DEVMODEW *devModeW() const { return (DEVMODEW*) devMode; }
    inline DEVMODEA *devModeA() const { return (DEVMODEA*) devMode; }

    inline PRINTER_INFO_2W *pInfoW() { return (PRINTER_INFO_2W*) pInfo; };
    inline PRINTER_INFO_2A *pInfoA() { return (PRINTER_INFO_2A*) pInfo; };

    inline bool resetDC() {
        QT_WA( {
            hdc = ResetDCW(hdc, devModeW());
        }, {
            hdc = ResetDCA(hdc, devModeA());
        } );
        return hdc != 0;
    }
    
    HGLOBAL *createDevNames();
    void readDevnames(HGLOBAL globalDevnames);
    void readDevmode(HGLOBAL globalDevmode);

    int showPageSetupDialog();
    int showPrintDialog();

    QRect pageRect();
    QRect paperRect();
    
    QWidget *getParent() { return p; }
    void setParent(QWidget *parent) { p = parent; }
};

QWin32PrinterPrivate::QWin32PrinterPrivate()
    : hPrinter(0), globalDevMode(0), hdc(0), p(NULL),
      devMode(0), pInfo(0),
      fromPage(0), toPage(0), minPage(0), maxPage(0),
      printToFile(false)
{
    printerOptions = 0;
    printRange = QPrinter::AllPages;
    
    bNeedReinit = ( QApplication::winVersion() < Qt::WV_NT_based );
}

QWin32PrinterPrivate::~QWin32PrinterPrivate()
{
    release();
}

void QWin32PrinterPrivate::initialize()
{
    if (hdc)
        release();
    Q_ASSERT(!hPrinter);
    Q_ASSERT(!hdc);
    Q_ASSERT(!devMode);
    Q_ASSERT(!pInfo);
    
    if (name.isEmpty())
        return;

    bool ok;
    QT_WA( {
        ok = OpenPrinterW((LPWSTR)name.ucs2(), (LPHANDLE)&hPrinter, 0);
    }, {
        ok = OpenPrinterA((LPSTR)name.latin1(), (LPHANDLE)&hPrinter, 0);
    } );

    if (!ok) {
        qWarning("QPrinterPrivate::initialize: OpenPrinter failed");
        return;
    }

    DWORD infoSize, numBytes;
    QT_WA( {
        GetPrinterW(hPrinter, 2, NULL, 0, &infoSize);
        pInfo = malloc(infoSize);
        if (!GetPrinterW(hPrinter, 2, (LPBYTE)pInfo, infoSize, &numBytes)) {
            qWarning("QPrinterPrivate::initialize: GetPrinter failed");
            return;
        }
    }, {
        GetPrinterA(hPrinter, 2, NULL, 0, &infoSize);
        pInfo = malloc(infoSize);
        if (!GetPrinterA(hPrinter, 2, (LPBYTE)pInfo, infoSize, &numBytes)) {
            qWarning("QPrinterPrivate::initialize: GetPrinter failed");
            return;
        }
    });

    QT_WA( {
        devMode = pInfoW()->pDevMode;
    }, {
        devMode = pInfoA()->pDevMode;
    } );

    QT_WA( {
        hdc = CreateDC(reinterpret_cast<const wchar_t *>(name.ucs2()),
                       reinterpret_cast<const wchar_t *>(name.ucs2()), 0, devModeW());
    }, {
        hdc = CreateDCA(name.latin1(), name.latin1(), 0, devModeA());
    } );

    Q_ASSERT(hPrinter);
    Q_ASSERT(devMode);
    Q_ASSERT(pInfo);

    initHDC();
}

void QWin32PrinterPrivate::initHDC()
{
    Q_ASSERT(hdc);

    dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
    dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
    dpi_display = GetDeviceCaps(qt_display_dc(), LOGPIXELSY);

    switch(mode) {
    case QPrinter::Compatible:
    case QPrinter::ScreenResolution:
        resolution = dpi_display;
        stretch_x = dpi_x / double(dpi_display);
        stretch_y = dpi_y / double(dpi_display);
	break;
    case QPrinter::PrinterResolution:
    case QPrinter::HighResolution:
        resolution = dpi_y;
        stretch_x = 1;
        stretch_y = 1;
	break;
    default:
        break;
    }

    devPaperRect = QRect(0, 0,
                         GetDeviceCaps(hdc, PHYSICALWIDTH),
                         GetDeviceCaps(hdc, PHYSICALHEIGHT));

    devPageRect = QRect(GetDeviceCaps(hdc, PHYSICALOFFSETX),
                        GetDeviceCaps(hdc, PHYSICALOFFSETY),
                        GetDeviceCaps(hdc, HORZRES),
                        GetDeviceCaps(hdc, VERTRES));
}

void QWin32PrinterPrivate::release()
{
    Q_ASSERT(hdc);

    if (globalDevMode) { // Devmode comes from print dialog
        GlobalUnlock(globalDevMode);
    } else {            // Devmode comes from initialize...
        // devMode is a part of the same memory block as pInfo so one free is enough...
        free(pInfo);
        ClosePrinter(hPrinter);
    }
    DeleteDC(hdc);

    hdc = 0;
    hPrinter = 0;
    pInfo = 0;
    devMode = 0;
}

void QWin32PrinterPrivate::queryDefault()
{
    /* Read the default printer name, driver and port with the intuitive function
     * Strings "windows" and "device" are specified in the MSDN under EnumPrinters()
     */
    QString noPrinters("qt_no_printers");
    QString output;
    QT_WA({
        ushort buffer[256];
        GetProfileStringW(L"windows", L"device",
                          reinterpret_cast<const wchar_t *>(noPrinters.ucs2()),
                          reinterpret_cast<wchar_t *>(buffer), 256);
        output = QString::fromUcs2(buffer);
        if (output == noPrinters) { // no printers
            qWarning("System has no default printer, are any printers installed?");
            return;
        }
    }, {
        char buffer[256];
        GetProfileStringA("windows", "device", noPrinters.latin1(), buffer, 256);
        output = QString::fromLocal8Bit(buffer);
        if (output == noPrinters) { // no printers
            qWarning("System has no default printer, are any printers installed?");
            return;
        }
    });
    QStringList info = QStringList::split( QChar( ',' ), output );
    name = info[0];
    program = info[1];
    port = info[2];
}

bool QWin32PrinterPrivate::begin(const QString &output_filename, const QString &doc_name, int &state)
{
    bool ok = state == PST_IDLE;

    if(!hdc)
        return false;

    QT_WA({
        DOCINFO di;
        memset(&di, 0, sizeof(DOCINFO));
        di.cbSize = sizeof(DOCINFO);
        di.lpszDocName = reinterpret_cast<const wchar_t *>(doc_name.ucs2());
        if (printToFile && !output_filename.isEmpty())
            di.lpszOutput = reinterpret_cast<const wchar_t *>(output_filename.ucs2());
        if (ok && StartDoc(hdc, &di) == SP_ERROR) {
            qWarning("QWin32PrinterPrivate::begin: StartDoc failed");
            ok = false;
        }
    } , {
        DOCINFOA di;
        memset(&di, 0, sizeof(DOCINFOA));
        di.cbSize = sizeof(DOCINFOA);
        QByteArray docNameA = doc_name.local8Bit();
        di.lpszDocName = docNameA.data();
        QCString outfileA = output_filename.local8Bit();
        if (printToFile && !output_filename.isEmpty())
            di.lpszOutput = outfileA;
        if (ok && StartDocA(hdc, &di) == SP_ERROR) {
            qWarning("QWin32PrinterPrivate::begin: StartDoc failed");
            ok = false;
        }
    });
    
    if (ok && StartPage(hdc) == SP_ERROR) {
        qWarning("QWin32PrinterPrivate::begin: StartPage failed");
        ok = false;
    }
    if (!ok) {
        if (hdc) {
            DeleteDC(hdc);
            hdc = 0;
        }
        state = PST_IDLE;
    } else {
        state = PST_ACTIVE;
    }
    return ok;
}

bool QWin32PrinterPrivate::end(int &state)
{
    if ( hdc ) {
        if (state == PST_ABORTED) {
            AbortDoc( hdc );
        } else {
            EndPage( hdc ); // end; printing done
            EndDoc( hdc );
        }
    }
    state = PST_IDLE;
    return true;
}

HGLOBAL *QWin32PrinterPrivate::createDevNames()
{
    QT_WA( { 
        int size = sizeof(DEVNAMES)
                   + program.length() * 2 + 2
                   + name.length() * 2 + 2
                   + port.length() * 2 + 2;
        HGLOBAL *hGlobal = (HGLOBAL *) GlobalAlloc(GMEM_MOVEABLE, size);
        DEVNAMES *dn = (DEVNAMES*) GlobalLock(hGlobal);

        dn->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
        dn->wDeviceOffset = dn->wDriverOffset + program.length() + 1;
        dn->wOutputOffset = dn->wDeviceOffset + name.length() + 1;

        memcpy((ushort*)dn + dn->wDriverOffset, program.ucs2(), program.length() * 2 + 2);
        memcpy((ushort*)dn + dn->wDeviceOffset, name.ucs2(), name.length() * 2 + 2);
        memcpy((ushort*)dn + dn->wOutputOffset, port.ucs2(), port.length() * 2 + 2);
        dn->wDefault = 0;

        GlobalUnlock(hGlobal);

        return hGlobal;
    }, { 
        int size = sizeof(DEVNAMES)
                   + program.length() + 2
                   + name.length() + 2
                   + port.length() + 2;
        HGLOBAL *hGlobal = (HGLOBAL *) GlobalAlloc(GMEM_MOVEABLE, size);
        DEVNAMES *dn = (DEVNAMES*) GlobalLock(hGlobal);

        dn->wDriverOffset = sizeof(DEVNAMES);
        dn->wDeviceOffset = dn->wDriverOffset + program.length() + 1;
        dn->wOutputOffset = dn->wDeviceOffset + name.length() + 1;

        memcpy((char*)dn + dn->wDriverOffset, program.latin1(), program.length() + 2);
        memcpy((char*)dn + dn->wDeviceOffset, name.latin1(), name.length() + 2);
        memcpy((char*)dn + dn->wOutputOffset, port.latin1(), port.length() + 2);
        dn->wDefault = 0;

        GlobalUnlock(hGlobal);
        return hGlobal;
    } );
}

void QWin32PrinterPrivate::readDevnames(HGLOBAL globalDevnames)
{
    if (globalDevnames) {
        QT_WA( {
            DEVNAMES *dn = (DEVNAMES*) GlobalLock(globalDevnames);
            name = QString::fromUcs2((ushort*)(dn) + dn->wDeviceOffset);
            port = QString::fromUcs2((ushort*)(dn) + dn->wOutputOffset);
            program = QString::fromUcs2((ushort*)(dn) + dn->wDriverOffset);
            GlobalUnlock(globalDevnames);
        }, {
            DEVNAMES *dn = (DEVNAMES*) GlobalLock(globalDevnames);
            name = QString::fromLatin1((char*)(dn) + dn->wDeviceOffset);
            port = QString::fromLatin1((char*)(dn) + dn->wOutputOffset);
            program = QString::fromLatin1((char*)(dn) + dn->wDriverOffset);
            GlobalUnlock(globalDevnames);
        } );
    }
}

void QWin32PrinterPrivate::readDevmode(HGLOBAL globalDevmode)
{
    if (globalDevmode) {
        QT_WA( {
            DEVMODE *dm = (DEVMODE*) GlobalLock(globalDevmode);
            release();
            globalDevMode = globalDevmode;
            devMode = dm;
            hdc = CreateDC(reinterpret_cast<const wchar_t *>(program.ucs2()),
                           reinterpret_cast<const wchar_t *>(name.ucs2()), 0, dm);
        }, {
            DEVMODEA *dm = (DEVMODEA*) GlobalLock(globalDevmode);
            release();
            globalDevMode = globalDevmode;
            devMode = dm;
            hdc = CreateDCA(program.latin1(), name.latin1(), 0, dm);
        } );
    }

    initHDC();
}

int QWin32PrinterPrivate::showPageSetupDialog()
{
    PAGESETUPDLG psd;
    memset(&psd, 0, sizeof(PAGESETUPDLG));
    psd.lStructSize = sizeof(PAGESETUPDLG);
    psd.hDevMode = devMode;
    HGLOBAL *tempDevNames = createDevNames();
    psd.hDevNames = tempDevNames;

    QWidget *parent = getParent();
    parent = parent ? parent->topLevelWidget() : qApp->activeWindow();
    psd.hwndOwner = parent ? parent->winId() : 0;

    QRect paperR = paperRect();
    QRect pageR = pageRect();

    psd.rtMargin.left   = paperR.left() - pageR.left();
    psd.rtMargin.top    = paperR.top() - pageR.top();
    psd.rtMargin.right  = paperR.right() - pageR.right();
    psd.rtMargin.bottom = paperR.bottom() - pageR.bottom();

    psd.Flags = PSD_INHUNDREDTHSOFMILLIMETERS
                | PSD_MARGINS;

    bool result = PageSetupDlg(&psd);

    // ### margins too...

    if (result) {
        readDevnames(psd.hDevNames);
        readDevmode(psd.hDevMode);
    }

    GlobalFree(tempDevNames);

    return result;
}

QRect QWin32PrinterPrivate::pageRect()
{
    return QWMatrix(1/stretch_x, 0, 0, 1/stretch_y, 0, 0).mapRect(devPageRect);
}

QRect QWin32PrinterPrivate::paperRect()
{
    return QWMatrix(1/stretch_x, 0, 0, 1/stretch_y, 0, 0).mapRect(devPaperRect);
}

/*****************************************************************************
  QPrinter member functions
 *****************************************************************************/
/*!
    Constructs a printer paint device with mode \a m.

    \sa QPrinter::PrinterMode
*/

QPrinter::QPrinter( PrinterMode m )
        : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    state = PST_IDLE;

    QWin32PrinterPrivate *dW32;
    d = dW32 = new QWin32PrinterPrivate;

    dW32->mode = m;
    dW32->queryDefault();
    dW32->initialize();

    page_order = FirstPageFirst;
    from_pg = to_pg = min_pg = max_pg = 0;
    state = PST_IDLE;
    output_file = FALSE;
    to_edge = FALSE;

    int psize,o,cm,psrc;
	if ( QT_WA_INLINE( dW32->devModeW() != 0, dW32->devModeA() != 0 ) ) {
		QT_WA(  hdevmode = dW32->devModeW();
				psize = dW32->devModeW()->dmPaperSize;
				o = dW32->devModeW()->dmOrientation;
				cm = dW32->devModeW()->dmColor;
				ncopies = dW32->devModeW()->dmCopies;
				psrc = dW32->devModeW()->dmDefaultSource;
				,
				hdevmode = dW32->devModeA();
				psize = dW32->devModeA()->dmPaperSize;
				o = dW32->devModeA()->dmOrientation;
				cm = dW32->devModeA()->dmColor;
				ncopies = dW32->devModeA()->dmCopies;
				psrc = dW32->devModeA()->dmDefaultSource;
		)
	} else {
		hdevmode = 0;
		psize = 0;
		o = 0;
		cm = 0;
		ncopies = 0;
		psrc = 0;
	}

	hdevnames = 0;
    page_size = mapDevmodePageSize( psize );
    orient = o == DMORIENT_LANDSCAPE ? QPrinter::Landscape : QPrinter::Portrait;
    color_mode = cm == DMCOLOR_COLOR ? QPrinter::Color : QPrinter::GrayScale;
    paper_source = mapDevmodePaperSource( psrc );
    page_order = FirstPageFirst;
    hdc = dW32->hdc;
    state = PST_IDLE;
    res = dW32->resolution;
    printer_name = dW32->name;
}

/*!
    Destroys the printer paint device and cleans up.
*/

QPrinter::~QPrinter()
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
    delete dW32;
}


/*!
    Advances to a new page on the printer. Returns TRUE if successful;
    otherwise returns FALSE.
*/

bool QPrinter::newPage()
{
    Q_ASSERT(state==PST_ACTIVE);

    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
    Q_ASSERT (dW32);
    hdc = dW32->hdc;
    Q_ASSERT(hdc);

    bool transparent = GetBkMode(hdc) == TRANSPARENT;

    if (!EndPage(hdc)) {
        qWarning("QPrinter::newPage: EndPage failed");
        return false;
    }

    if (dW32->bNeedReinit) {
        if (!dW32->resetDC()) {
            qWarning("QPrinter::newPage: ResetDC failed");
            return false;
        }
    }

    if (!StartPage(hdc)) {
        qWarning("QPrinter::newPage: StartPage failed");
        return false;
    }

    SetTextAlign(hdc, TA_BASELINE);
    if (transparent)
        SetBkMode(hdc, TRANSPARENT);

    // ###
    return true;
}


/*!
    Aborts the print job. Returns TRUE if successful; otherwise
    returns FALSE.

    \sa aborted()
*/

bool QPrinter::abort()
{
    state = PST_ABORTED;
    return true;
}

/*!
    Returns TRUE is the printer job was aborted; otherwise returns
    FALSE.

    \sa abort()
*/

bool QPrinter::aborted() const
{
    return state == PST_ABORTED;
}

/*!
    Sets the printer name to \a name.

    The default printer will be used if no printer name is set.

    Under X11, the \c PRINTER environment variable defines the default
    printer.  Under any other window system, the window system defines
    the default printer.

    \sa printerName()
*/

void QPrinter::setPrinterName( const QString &name )
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);

    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setPrinterName: Cannot do this during printing" );
#endif
        return;
    }
    if ( !dW32 )
        return;

    printer_name = name;
    dW32->name = printer_name;
    // FIXME! does this work?
    dW32->initialize();
}

/*!
    Opens a printer setup dialog, with parent \a parent, and asks the
    user to specify which printer they wish to use and what settings
    it should have.

    Returns TRUE if the user pressed "OK" to print, or FALSE if the
    user cancelled the operation.
*/

bool QPrinter::setup( QWidget * parent )
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
    if ( !dW32 )
        return false;

	if ( QT_WA_INLINE( dW32->devModeW() == 0, dW32->devModeA() == 0 ) )
		return false;

    dW32->setParent( parent );
    dW32->fromPage = fromPage();
    dW32->toPage = toPage();
    dW32->minPage = minPage();
    dW32->maxPage = maxPage();
    QT_WA( dW32->devModeW()->dmCopies = numCopies();, dW32->devModeA()->dmCopies = numCopies(); )
    dW32->printToFile = outputToFile();

    if ( !dW32->showPrintDialog() )
        return false;

    setPrintRange( AllPages );
    if ( isOptionEnabled( QPrinter::PrintSelection ) )
        setPrintRange( Selection );
    else if ( isOptionEnabled( QPrinter::PrintPageRange ) )
        setPrintRange( PageRange );

    setOutputToFile( isOptionEnabled( QPrinter::PrintToFile ) );
    setFromTo( dW32->fromPage, dW32->toPage );
    setMinMax( dW32->minPage, dW32->maxPage );


    int psize,o,cm,psrc;
	QT_WA(  hdevmode = dW32->devModeW();
            psize = dW32->devModeW()->dmPaperSize;
            o = dW32->devModeW()->dmOrientation;
            cm = dW32->devModeW()->dmColor;
            ncopies = dW32->devModeW()->dmCopies;
            psrc = dW32->devModeW()->dmDefaultSource;
            ,
            hdevmode = dW32->devModeA();
            psize = dW32->devModeA()->dmPaperSize;
            o = dW32->devModeA()->dmOrientation;
            cm = dW32->devModeA()->dmColor;
            ncopies = dW32->devModeA()->dmCopies;
            psrc = dW32->devModeA()->dmDefaultSource;
    )

	hdevnames = 0;
    page_size = mapDevmodePageSize( psize );
    orient = DMORIENT_LANDSCAPE ? QPrinter::Landscape : QPrinter::Portrait;
    color_mode = DMCOLOR_COLOR ? QPrinter::Color : QPrinter::GrayScale;
    paper_source = mapDevmodePaperSource( psrc );
    hdc = dW32->hdc;
    state = PST_IDLE;
    res = dW32->resolution;

    return true;
}

/*!
  \internal
  Handles painter commands to the printer.
*/

bool QPrinter::cmd( int c, QPainter *paint, QPDevCmdParam *p )
{
    if ( c == PdcBegin ) {
        QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
        if (!dW32)
            return false;

        return dW32->begin( output_filename, doc_name, state );
    }

    if ( c == PdcEnd ) {
        QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
        if (!dW32)
            return false;

        return dW32->end( state );
    }

    return true;
}

/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.

  \internal
  Hard coded return values for PostScript under X.
*/

int QPrinter::metric( int m ) const
{
    int val, res = 0;
    HDC hdc = 0;

    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
    if (dW32) {
        res = dW32->resolution;
        hdc = dW32->hdc;
    }
    if ( !hdc || !res )
        return 0;

    switch ( m ) {
    case QPaintDeviceMetrics::PdmWidth:
        val = res
            * GetDeviceCaps( hdc, fullPage() ? PHYSICALWIDTH : HORZRES )
            / GetDeviceCaps( hdc, LOGPIXELSX );
        break;
    case QPaintDeviceMetrics::PdmHeight:
        val = res
            * GetDeviceCaps( hdc, fullPage() ? PHYSICALHEIGHT : VERTRES )
            / GetDeviceCaps( hdc, LOGPIXELSY );
        break;
    case QPaintDeviceMetrics::PdmDpiX:
        val = res;
        break;
    case QPaintDeviceMetrics::PdmDpiY:
        val = res;
        break;
    case QPaintDeviceMetrics::PdmPhysicalDpiX:
        val = GetDeviceCaps( hdc, LOGPIXELSX );
        break;
    case QPaintDeviceMetrics::PdmPhysicalDpiY:
        val = GetDeviceCaps( hdc, LOGPIXELSY );
        break;
    case QPaintDeviceMetrics::PdmWidthMM:
        if ( !fullPage() ) {
            val = GetDeviceCaps( hdc, HORZSIZE );
        }
        else {
            float wi = 25.4 * GetDeviceCaps( hdc, PHYSICALWIDTH );
            val = (int)( wi / GetDeviceCaps( hdc,  LOGPIXELSX ) );
        }
        break;
    case QPaintDeviceMetrics::PdmHeightMM:
        if ( !fullPage() ) {
            val = GetDeviceCaps( hdc, VERTSIZE );
        }
        else {
            float hi = 25.4 * GetDeviceCaps( hdc, PHYSICALHEIGHT );
            val = ( hi / GetDeviceCaps( hdc,  LOGPIXELSY ) );
        }
        break;
    case QPaintDeviceMetrics::PdmNumColors:
        {
        int bpp = GetDeviceCaps( hdc, BITSPIXEL );
        if( bpp == 32 )
            val = INT_MAX;
        else if( bpp <= 8 )
            val = GetDeviceCaps( hdc, NUMCOLORS );
        else
            val = 1 << ( bpp * GetDeviceCaps( hdc, PLANES ) );
    }
        break;
    case QPaintDeviceMetrics::PdmDepth:
        val = GetDeviceCaps( hdc, PLANES );
        break;
    default:
       qWarning("QPrinter::metric: Invalid metric command");
       val = 0;
    }
    return val;
}


/*!
    Returns the width of the left margin and the height of the top
    margin of the printer. On Unix, this is a best-effort guess, not
    based on perfect knowledge.

    If you have called setFullPage( TRUE ), margins().width() may be
    treated as the smallest sane left margin you can use, and
    margins().height() as the smallest sane top margin you can
    use.

    If you have called setFullPage( FALSE ) (this is the default),
    margins() is automatically subtracted from the pageSize() by
    QPrinter.

    \sa setFullPage() QPaintDeviceMetrics PageSize
*/
QSize QPrinter::margins() const
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);

    if ( dW32 )
        return QSize( dW32->leftMargin, dW32->topMargin );

    if ( orient == Portrait )
        return QSize( res / 2, res / 3 );

    return QSize( res / 3, res / 2 );
}

/*!
    \overload

    Sets \a top, \a left, \a bottom and \a right to the margins of the
    printer.  On Unix, this is a best-effort guess, not based on
    perfect knowledge.

    If you have called setFullPage( TRUE ), the four values specify
    the smallest sane margins you can use.

    If you have called setFullPage( FALSE ) (this is the default),
    the margins are automatically subtracted from the pageSize() by
    QPrinter.

    \sa setFullPage() QPaintDeviceMetrics PageSize
*/
void QPrinter::margins( uint *top, uint *left, uint *bottom, uint *right ) const
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);

    if ( !dW32 ) {
        int x = orient == Portrait ? res / 2 : res / 3;
        int y = orient == Portrait ? res / 3 : res / 2;
        *top = *bottom = y;
        *left = *right = x;
    } else {
        *top = dW32->topMargin;
        *left = dW32->leftMargin;
        *bottom = dW32->bottomMargin;
        *right = dW32->rightMargin;
    }
}

/*!
  Sets the printer margins to the sizes specified in \a top, \a left,
  \a bottom and \a right.

  This function currently only has an effect on Unix systems.

  \sa margins()
*/
void QPrinter::setMargins( uint top, uint left, uint bottom, uint right )
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);

    if ( !dW32 )
        d = dW32 = new QWin32PrinterPrivate;
    dW32->marginsSpecified = true;
    dW32->topMargin = top;
    dW32->leftMargin = left;
    dW32->bottomMargin = bottom;
    dW32->rightMargin = right;
}

void QPrinter::reinit( void )
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);

    if ( !dW32 )
        return;

    int psize = mapPageSizeDevmode( page_size );
    int o = orient == QPrinter::Landscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
    int cm = color_mode == QPrinter::Color ? DMCOLOR_COLOR : DMCOLOR_MONOCHROME;
    int psrc = mapPaperSourceDevmode( paper_source );

    if ( QT_WA_INLINE( dW32->devModeW() != 0, dW32->devModeA() != 0 ) ) {
        QT_WA(  dW32->devModeW()->dmPaperSize = psize;
                dW32->devModeW()->dmOrientation = o;
                dW32->devModeW()->dmColor = cm;
                dW32->devModeW()->dmCopies = ncopies;
                dW32->devModeW()->dmDefaultSource = psrc;
                ,
                dW32->devModeA()->dmPaperSize = psize;
                dW32->devModeA()->dmOrientation = o;
                dW32->devModeA()->dmColor = cm;
                dW32->devModeA()->dmCopies = ncopies;
                dW32->devModeA()->dmDefaultSource = psrc;
        )
    }

	hdevnames = 0;
	QT_WA( hdevmode = dW32->devModeW();, hdevmode = dW32->devModeA(); )
    hdc = dW32->hdc;
    res = dW32->resolution;
}

void QPrinter::setActive( void )
{
    state = PST_ACTIVE;
}

void QPrinter::setIdle( void )
{
    state = PST_IDLE;
}

/*!
  Windows only, using this function is not portable! Sets the windows page size
  value that is used by the DEVMODE struct. The winPageSize value must be one of
  the DMPAPER_ defines from wingdi.h.
*/
void QPrinter::setWinPageSize( short winPageSize )
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
    QT_WA(  hdevmode = dW32->devModeW();
            if( hdevmode )
                dW32->devModeW()->dmPaperSize = winPageSize;
            ,
            hdevmode = dW32->devModeA();
            if( hdevmode )
                dW32->devModeA()->dmPaperSize = winPageSize;
    )
    page_size = mapDevmodePageSize( winPageSize );
}

/*!
  Returns the Windows page size value as used by the DEVMODE struct
  (Windows only). Using this function is not portable.

  Use pageSize() to get the PageSize, e.g. 'A4', 'Letter', etc.
*/
short QPrinter::winPageSize() const
{
    QWin32PrinterPrivate *dW32 = static_cast<QWin32PrinterPrivate*>(d);
    QT_WA(  if( dW32->devModeW() )
                return dW32->devModeW()->dmPaperSize;
            ,
            if( dW32->devModeA() )
                return dW32->devModeA()->dmPaperSize;
    )
    return 0;
}

#include "qprinterdialog_win.cpp"

#endif
