/**********************************************************************
** $Id: qt/qprinter.h   3.3.4   edited Jun 19 2003 $
**
** Definition of QPrinter class
**
** Created : 940927
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QPRINTER_H
#define QPRINTER_H

#ifndef QT_H
#include "qpaintdevice.h"
#include "qstring.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_PRINTER

#if defined(B0)
#undef B0 // Terminal hang-up.  We assume that you do not want that.
#endif

class QPrinterPrivate;

class Q_EXPORT QPrinter : public QPaintDevice
{
public:
    enum PrinterMode { ScreenResolution, PrinterResolution, HighResolution, Compatible };

    QPrinter( PrinterMode mode = ScreenResolution );
   ~QPrinter();

    enum Orientation { Portrait, Landscape };

    enum PageSize    { A4, B5, Letter, Legal, Executive,
		       A0, A1, A2, A3, A5, A6, A7, A8, A9, B0, B1,
		       B10, B2, B3, B4, B6, B7, B8, B9, C5E, Comm10E,
		       DLE, Folio, Ledger, Tabloid, Custom, NPageSize = Custom };

    enum PageOrder   { FirstPageFirst, LastPageFirst };

    enum ColorMode   { GrayScale, Color };

    enum PaperSource { OnlyOne, Lower, Middle, Manual, Envelope,
                       EnvelopeManual, Auto, Tractor, SmallFormat,
                       LargeFormat, LargeCapacity, Cassette, FormSource };

    enum PrintRange   { AllPages,
			Selection,
			PageRange };

    enum PrinterOption { PrintToFile,
			 PrintSelection,
			 PrintPageRange };

    QString printerName() const;
    virtual void setPrinterName( const QString &);
    bool outputToFile() const;
    virtual void setOutputToFile( bool );
    QString outputFileName()const;
    virtual void setOutputFileName( const QString &);

    QString printProgram() const;
    virtual void setPrintProgram( const QString &);

    QString printerSelectionOption() const;
    virtual void setPrinterSelectionOption( const QString & );

    QString docName() const;
    virtual void setDocName( const QString &);
    QString creator() const;
    virtual void setCreator( const QString &);

    Orientation orientation()   const;
    virtual void setOrientation( Orientation );
    PageSize pageSize()      const;
    virtual void setPageSize( PageSize );
#ifdef Q_WS_WIN
    void setWinPageSize( short winPageSize );
    short winPageSize() const;
#endif
#ifdef Q_WS_MAC
    bool printSetup();
    bool pageSetup();
#endif
    virtual void setPageOrder( PageOrder );
    PageOrder   pageOrder() const;

    void setResolution( int );
    int resolution() const;

    virtual void setColorMode( ColorMode );
    ColorMode   colorMode() const;

    virtual void        setFullPage( bool );
    bool                fullPage() const;
    QSize       margins()       const;
    void setMargins( uint top, uint left, uint bottom, uint right );
    void margins( uint *top, uint *left, uint *bottom, uint *right ) const;

    int         fromPage()      const;
    int         toPage()        const;
    virtual void setFromTo( int fromPage, int toPage );
    int         minPage()       const;
    int         maxPage()       const;
    virtual void setMinMax( int minPage, int maxPage );
    int         numCopies()     const;
    virtual void setNumCopies( int );

    bool	collateCopiesEnabled() const;
    void	setCollateCopiesEnabled(bool );

    bool	collateCopies() const;
    void	setCollateCopies( bool );

    PrintRange	printRange() const;
    void 	setPrintRange( PrintRange range );

    bool        newPage();
    bool        abort();
    bool        aborted()       const;

    bool        setup( QWidget *parent = 0 );

    PaperSource paperSource()   const;
    virtual void setPaperSource( PaperSource );

    void setOptionEnabled( PrinterOption, bool enable );
    bool isOptionEnabled( PrinterOption );

protected:
    bool        cmd( int, QPainter *, QPDevCmdParam * );
    int         metric( int ) const;

#if defined(Q_WS_WIN)
    virtual void        setActive();
    virtual void        setIdle();
#endif

private:
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
    QPaintDevice *pdrv;
    int         pid;
#endif
#if defined(Q_WS_MAC)
    friend class QPrinterPrivate;
    PMPageFormat pformat;
    PMPrintSettings psettings;
    PMPrintSession psession;
    bool prepare(PMPrintSettings *);
    bool prepare(PMPageFormat *);
    void interpret(PMPrintSettings *);
    void interpret(PMPageFormat *);
#endif
#if defined(Q_WS_WIN)
    void        readPdlg( void* );
    void        readPdlgA( void* );
    void	writeDevmode( Qt::HANDLE );
    void	writeDevmodeA( Qt::HANDLE );
    void	reinit();

    bool        viewOffsetDone;
    QPainter*   painter;
    Qt::HANDLE hdevmode;
    Qt::HANDLE hdevnames;
#endif

    int         state;
    QString     printer_name;
    QString     option_string;
    QString     output_filename;
    bool        output_file;
    QString     print_prog;
    QString     doc_name;
    QString     creator_name;

    PageSize    page_size;
    PaperSource paper_source;
    PageOrder   page_order;
    ColorMode   color_mode;
    Orientation orient;
    uint	to_edge : 1;
    uint	appcolcopies : 1;
    uint	usercolcopies : 1;
    uint	res_set : 1;
    short       from_pg, to_pg;
    short       min_pg,  max_pg;
    short       ncopies;
    int         res;
    QPrinterPrivate *d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPrinter( const QPrinter & );
    QPrinter &operator=( const QPrinter & );
#endif
};


inline QString QPrinter::printerName() const
{ return printer_name; }

inline bool QPrinter::outputToFile() const
{ return output_file; }

inline QString QPrinter::outputFileName() const
{ return output_filename; }

inline QString QPrinter::printProgram() const
{ return print_prog; }

inline QString QPrinter::docName() const
{ return doc_name; }

inline QString QPrinter::creator() const
{ return creator_name; }

inline QPrinter::PageSize QPrinter::pageSize() const
{ return page_size; }

inline QPrinter::Orientation QPrinter::orientation() const
{ return orient; }

inline int QPrinter::fromPage() const
{ return from_pg; }

inline int QPrinter::toPage() const
{ return to_pg; }

inline int QPrinter::minPage() const
{ return min_pg; }

inline int QPrinter::maxPage() const
{ return max_pg; }

inline int QPrinter::numCopies() const
{ return ncopies; }

inline bool QPrinter::collateCopiesEnabled() const
{ return appcolcopies; }

inline void QPrinter::setCollateCopiesEnabled(bool v)
{ appcolcopies = v; }

inline bool QPrinter::collateCopies() const
{ return usercolcopies; }


#endif // QT_NO_PRINTER

#endif // QPRINTER_H
