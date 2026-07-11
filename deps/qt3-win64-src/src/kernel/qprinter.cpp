/**********************************************************************
** $Id: qprinter.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QPrinter class
**
** Created : 941003
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
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

#include "qprinter.h"
#include "qprinter_p.h"

#ifndef QT_NO_PRINTER

/*!
    \class QPrinter qprinter.h
    \brief The QPrinter class is a paint device that paints on a printer.

    \ingroup images
    \ingroup graphics
    \mainclass

    On Windows it uses the built-in printer drivers. On X11 it
    generates postscript and sends that to lpr, lp, or another print
    command.

    QPrinter is used in much the same way as QWidget and QPixmap are
    used. The big difference is that you must keep track of the pages.

    QPrinter supports a number of settable parameters, most of which
    can be changed by the end user when the application calls
    QPrinter::setup().

    The most important parameters are:
    \list
    \i setOrientation() tells QPrinter which page orientation to use (virtual).
    \i setPageSize() tells QPrinter what page size to expect from the
    printer.
    \i setResolution() tells QPrinter what resolution you wish the
    printer to provide (in dpi).
    \i setFullPage() tells QPrinter whether you want to deal with the
    full page or just with the part the printer can draw on. The
    default is FALSE, so that by default you should be able to paint
    on (0,0). If TRUE the origin of the coordinate system will be in
    the top left corner of the paper and most probably the printer
    will not be able to paint something there due to it's physical
    margins.
    \i setNumCopies() tells QPrinter how many copies of the document
    it should print.
    \i setMinMax() tells QPrinter and QPrintDialog what the allowed
    range for fromPage() and toPage() are.
    \endlist

    Except where noted, you can only call the set functions before
    setup(), or between QPainter::end() and setup(). (Some may take
    effect between setup() and begin(), or between begin() and end(),
    but that's strictly undocumented and such behaviour may differ
    depending on platform.)

    There are also some settings that the user sets (through the
    printer dialog) and that applications are expected to obey:

    \list

    \i pageOrder() tells the application program whether to print
    first-page-first or last-page-first.

    \i colorMode() tells the application program whether to print in
    color or grayscale. (If you print in color and the printer does
    not support color, Qt will try to approximate. The document may
    take longer to print, but the quality should not be made visibly
    poorer.)

    \i fromPage() and toPage() indicate what pages the application
    program should print.

    \i paperSource() tells the application progam which paper source
    to print from.

    \endlist

    You can of course call these functions to establish defaults
    before you ask the user through QPrinter::setup().

    Once you start printing, calling newPage() is essential. You will
    probably also need to look at the QPaintDeviceMetrics for the
    printer (see the \link simple-application.html#printersimple print
    function\endlink in the Application walk-through). In previous versions,
    paint device metrics were valid only after the QPrinter has been set
    up, i.e. after setup() has returned successfully. This is no longer
    the case and paint device metrics can be requested safely before set up.

    If you want to abort the print job, abort() will try its best to
    stop printing. It may cancel the entire job or just some of it.

    \omit Need a function to setup() without a dialog (i.e. use defaults).
    \endomit

    If your current locale converts "," to ".", you will need to set
    a locale (via the standard C setlocale() function) that doen't do this
    before using QPrinter. The "C" locale works well for this.

    The TrueType font embedding for Qt's postscript driver uses code
    by David Chappell of Trinity College Computing Center.

    \legalese

    Copyright 1995, Trinity College Computing Center.
    Written by David Chappell.

    Permission to use, copy, modify, and distribute this software and
    its documentation for any purpose and without fee is hereby
    granted, provided that the above copyright notice appear in all
    copies and that both that copyright notice and this permission
    notice appear in supporting documentation. This software is
    provided "as is" without express or implied warranty.

    TrueType font support. These functions allow PPR to generate
    PostScript fonts from Microsoft compatible TrueType font files.

    The functions in this file do most of the work to convert a
    TrueType font to a type 3 PostScript font.

    Most of the material in this file is derived from a program called
    "ttf2ps" which L. S. Ng posted to the usenet news group
    "comp.sources.postscript". The author did not provide a copyright
    notice or indicate any restrictions on use.

    Last revised 11 July 1995.

*/

/*!
    \enum QPrinter::PrinterMode

    This enum describes the mode the printer should work in. It
    basically presets a certain resolution and working mode.

    \value ScreenResolution Sets the resolution of the print device to
    the screen resolution. This has the big advantage that the results
    obtained when painting on the printer will match more or less
    exactly the visible output on the screen. It is the easiest to
    use, as font metrics on the screen and on the printer are the
    same. This is the default value. ScreenResolution will produce a
    lower quality output than HighResolution and should only be used
    for drafts.

    \value PrinterResolution Use the physical resolution of the
    printer on Windows. On Unix, set the postscript resolution to 72
    dpi.

    \value HighResolution Use printer resolution on windows, set the
    resolution of the postscript driver to 600dpi.

    \value Compatible Almost the same as PrinterResolution, but keeps
    some peculiarities of the Qt 2.x printer driver. This is useful
    for applications ported from Qt 2.x to Qt 3.x.
*/

/*!
    \enum QPrinter::Orientation

    This enum type (not to be confused with Qt::Orientation) is used
    to specify each page's orientation.

    \value Portrait the page's height is greater than its width (the
    default).

    \value Landscape the page's width is greater than its height.

    This type interacts with \l QPrinter::PageSize and
    QPrinter::setFullPage() to determine the final size of the page
    available to the application.
*/


/*!
    \enum QPrinter::PageSize

    This enum type specifies what paper size QPrinter should use.
    QPrinter does not check that the paper size is available; it just
    uses this information, together with QPrinter::Orientation and
    QPrinter::setFullPage(), to determine the printable area (see
    QPaintDeviceMetrics).

    The defined sizes (with setFullPage(TRUE)) are:

    \value A0 841 x 1189 mm This value is not supported on windows.
    \value A1 594 x 841 mm This value is not supported on windows.
    \value A2 420 x 594 mm
    \value A3 297 x 420 mm
    \value A4 210 x 297 mm, 8.26 x 11.7 inches
    \value A5 148 x 210 mm
    \value A6 105 x 148 mm
    \value A7 74 x 105 mm
    \value A8 52 x 74 mm
    \value A9 37 x 52 mm
    \value B0 1030 x 1456 mm
    \value B1 728 x 1030 mm
    \value B10 32 x 45 mm
    \value B2 515 x 728 mm
    \value B3 364 x 515 mm
    \value B4 257 x 364 mm
    \value B5 182 x 257 mm, 7.17 x 10.13 inches
    \value B6 128 x 182 mm
    \value B7 91 x 128 mm
    \value B8 64 x 91 mm
    \value B9 45 x 64 mm
    \value C5E 163 x 229 mm
    \value Comm10E 105 x 241 mm, US Common #10 Envelope
    \value DLE 110 x 220 mm
    \value Executive 7.5 x 10 inches, 191 x 254 mm
    \value Folio 210 x 330 mm
    \value Ledger 432 x 279 mm
    \value Legal 8.5 x 14 inches, 216 x 356 mm
    \value Letter 8.5 x 11 inches, 216 x 279 mm
    \value Tabloid 279 x 432 mm
    \value Custom
    \value NPageSize (internal)

    With setFullPage(FALSE) (the default), the metrics will be a bit
    smaller; how much depends on the printer in use.
*/


/*!
    \enum QPrinter::PageOrder

    This enum type is used by QPrinter to tell the application program
    how to print.

    \value FirstPageFirst  the lowest-numbered page should be printed
    first.

    \value LastPageFirst  the highest-numbered page should be printed
    first.
*/

/*!
    \enum QPrinter::ColorMode

    This enum type is used to indicate whether QPrinter should print
    in color or not.

    \value Color  print in color if available, otherwise in grayscale.

    \value GrayScale  print in grayscale, even on color printers.
    Might be a little faster than \c Color. This is the default.
*/

/*!
    \enum QPrinter::PaperSource

    This enum type specifies what paper source QPrinter is to use.
    QPrinter does not check that the paper source is available; it
    just uses this information to try and set the paper source.
    Whether it will set the paper source depends on whether the
    printer has that particular source.

    Note: this is currently only implemented for Windows.

    \value OnlyOne
    \value Lower
    \value Middle
    \value Manual
    \value Envelope
    \value EnvelopeManual
    \value Auto
    \value Tractor
    \value SmallFormat
    \value LargeFormat
    \value LargeCapacity
    \value Cassette
    \value FormSource
*/

/*!
  \enum QPrinter::PrintRange

  This enum is used to specify which print range the application
  should use to print.

  \value AllPages All pages should be printed
  \value Selection Only the selection should be printed.
  \value PageRange From page, to page option.

  \sa setPrintRange(), printRange()
*/

/*!
  \enum QPrinter::PrinterOption

  This enum describes various printer options that appear in the
  printer setup dialog. It is used to enable and disable these
  options in the setup dialog.

  \value PrintToFile Describes if print to file should be enabled.
  \value PrintSelection Describes if printing selections should be enabled.
  \value PrintPageRange Describes if printing page ranges (from, to) should
         be enabled

  \sa setOptionEnabled(), isOptionEnabled()
*/


/*!
    \fn QString QPrinter::printerName() const

    Returns the printer name. This value is initially set to the name
    of the default printer.

    \sa setPrinterName()
*/

/*!
    \fn bool QPrinter::outputToFile() const

    Returns TRUE if the output should be written to a file, or FALSE
    if the output should be sent directly to the printer. The default
    setting is FALSE.

    This function is currently only supported under X11 and Mac OS X.

    \sa setOutputToFile(), setOutputFileName()
*/

/*!
    Specifies whether the output should be written to a file or sent
    directly to the printer.

    Will output to a file if \a enable is TRUE, or will output
    directly to the printer if \a enable is FALSE.

    This function is currently only supported under X11 and Mac OS X.

    \sa outputToFile(), setOutputFileName()
*/

void QPrinter::setOutputToFile( bool enable )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setOutputToFile: Cannot do this during printing" );
#endif
        return;
    }
    output_file = enable;
}


/*!
    \fn QString QPrinter::outputFileName() const

    Returns the name of the output file. There is no default file
    name.

    \sa setOutputFileName(), setOutputToFile()
*/

/*!
    Sets the name of the output file to \a fileName.

    Setting a null or empty name (0 or "") disables output to a file,
    i.e. calls setOutputToFile(FALSE). Setting a non-empty name
    enables output to a file, i.e. calls setOutputToFile(TRUE).

    This function is currently only supported under X11.

    \sa outputFileName(), setOutputToFile()
*/

void QPrinter::setOutputFileName( const QString &fileName )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning("QPrinter::setOutputFileName: Cannot do this during printing");
#endif
        return;
    }
    output_filename = fileName;
    output_file = !output_filename.isEmpty();
}


/*!
    \fn QString QPrinter::printProgram() const

    Returns the name of the program that sends the print output to the
    printer.

    The default is to return a null string; meaning that QPrinter will
    try to be smart in a system-dependent way. On X11 only, you can
    set it to something different to use a specific print program.

    On Windows, this function returns the name of the printer device
    driver.

    \sa setPrintProgram() setPrinterSelectionOption()
*/

/*!
    Sets the name of the program that should do the print job to \a
    printProg.

    On X11, this function sets the program to call with the PostScript
    output. On other platforms, it has no effect.

    \sa printProgram()
*/

void QPrinter::setPrintProgram( const QString &printProg )
{
    print_prog = printProg;
}


/*!
    \fn QString QPrinter::docName() const

    Returns the document name.

    \sa setDocName()
*/

/*!
    Sets the document name to \a name.
*/

void QPrinter::setDocName( const QString &name )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setDocName: Cannot do this during printing" );
#endif
        return;
    }
    doc_name = name;
}


/*!
    \fn QString QPrinter::creator() const

    Returns the name of the application that created the document.

    \sa setCreator()
*/

/*!
    Sets the name of the application that created the document to \a
    creator.

    This function is only applicable to the X11 version of Qt. If no
    creator name is specified, the creator will be set to "Qt"
    followed by some version number.

    \sa creator()
*/

void QPrinter::setCreator( const QString &creator )
{
    creator_name = creator;
}


/*!
    \fn Orientation QPrinter::orientation() const

    Returns the orientation setting. The default value is \c
    QPrinter::Portrait.

    \sa setOrientation()
*/

/*!
    Sets the print orientation to \a orientation.

    The orientation can be either \c QPrinter::Portrait or \c
    QPrinter::Landscape.

    The printer driver reads this setting and prints using the
    specified orientation. On Windows this setting won't take effect
    until the printer dialog is shown (using QPrinter::setup()).

    Windows only! This option can be changed while printing and will
    take effect from the next call to newPage()

    \sa orientation()
*/

void QPrinter::setOrientation( Orientation orientation )
{
    orient = orientation;
#if defined(Q_WS_WIN)
    reinit();
#endif
}


/*!
    \fn PageSize QPrinter::pageSize() const

    Returns the printer page size. The default value is system-dependent.

    \sa setPageSize()
*/


/*!
    Sets the printer page size to \a newPageSize if that size is
    supported. The result if undefined if \a newPageSize is not
    supported.

    The default page size is system-dependent.

    This function is useful mostly for setting a default value that
    the user can override in the print dialog when you call setup().

    \sa pageSize() PageSize setFullPage() setResolution()
*/

void QPrinter::setPageSize( PageSize newPageSize )
{
    if ( newPageSize > NPageSize ) {
#if defined(QT_CHECK_STATE)
        qWarning("QPrinter::SetPageSize: illegal page size %d", newPageSize );
#endif
        return;
    }
    page_size = newPageSize;
#if defined(Q_WS_WIN)
    reinit();
#endif
}

/*!
    Sets the page order to \a newPageOrder.

    The page order can be \c QPrinter::FirstPageFirst or \c
    QPrinter::LastPageFirst. The application programmer is responsible
    for reading the page order and printing accordingly.

    This function is useful mostly for setting a default value that
    the user can override in the print dialog when you call setup().

    \bug This value is not kept in sync with the Windows or Mac OS X printer
    dialogs.
*/

void QPrinter::setPageOrder( PageOrder newPageOrder )
{
    page_order = newPageOrder;
#if defined(Q_WS_WIN)
    reinit();
#endif
}


/*!
    Returns the current page order.

    The default page order is \c FirstPageFirst.

    \bug This value is not kept in sync with the Windows or Mac OS X printer
    dialogs.
*/

QPrinter::PageOrder QPrinter::pageOrder() const
{
    return page_order;
}


/*!
    Sets the printer's color mode to \a newColorMode, which can be
    either \c Color or \c GrayScale (the default).

    \sa colorMode()
*/

void QPrinter::setColorMode( ColorMode newColorMode )
{
    color_mode = newColorMode;
#if defined(Q_WS_WIN)
    reinit();
#endif
}


/*!
    Returns the current color mode. The default color mode is \c
    Color.

    \sa setColorMode()
*/

QPrinter::ColorMode QPrinter::colorMode() const
{
    return color_mode;
}


/*!
    \fn int QPrinter::fromPage() const

    Returns the from-page setting. The default value is 0.

    If fromPage() and toPage() both return 0 this signifies 'print the
    whole document'.

    The programmer is responsible for reading this setting and
    printing accordingly.

    \sa setFromTo(), toPage()
*/

/*!
    \fn int QPrinter::toPage() const

    Returns the to-page setting. The default value is 0.

    If fromPage() and toPage() both return 0 this signifies 'print the
    whole document'.

    The programmer is responsible for reading this setting and
    printing accordingly.

    \sa setFromTo(), fromPage()
*/

/*!
    Sets the from-page and to-page settings to \a fromPage and \a
    toPage respectively.

    The from-page and to-page settings specify what pages to print.

    If fromPage() and toPage() both return 0 this signifies 'print the
    whole document'.

    This function is useful mostly to set a default value that the
    user can override in the print dialog when you call setup().

    \sa fromPage(), toPage(), setMinMax(), setup()
*/

void QPrinter::setFromTo( int fromPage, int toPage )
{
    if ( state != 0 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QPrinter::setFromTo: Cannot do this during printing" );
#endif
        return;
    }
    from_pg = fromPage;
    to_pg = toPage;
}


/*!
    \fn int QPrinter::minPage() const

    Returns the min-page setting, i.e. the lowest page number a user
    is allowed to choose. The default value is 0.

    \sa maxPage(), setMinMax() setFromTo()
*/

/*!
    \fn int QPrinter::maxPage() const

    Returns the max-page setting. A user can't choose a higher page
    number than maxPage() when they select a print range. The default
    value is 0.

    \sa minPage(), setMinMax() setFromTo()
*/

/*!
    Sets the min-page and max-page settings to \a minPage and \a
    maxPage respectively.

    The min-page and max-page restrict the from-page and to-page
    settings. When the printer setup dialog appears, the user cannot
    select a from page or a to page that are outside the range
    specified by min and max pages.

    \sa minPage(), maxPage(), setFromTo(), setup()
*/

void QPrinter::setMinMax( int minPage, int maxPage )
{
    min_pg = minPage;
    max_pg = maxPage;
    if ( from_pg == 0 || from_pg < minPage )
	from_pg = minPage;
    if ( to_pg == 0 || to_pg > maxPage )
	to_pg = maxPage;
}


/*!
    \fn int QPrinter::numCopies() const

    Returns the number of copies to be printed. The default value is 1.

    This value will return the number of times the application is
    required to print in order to match the number specified in the
    printer setup dialog. This has been done since some printer
    drivers are not capable of buffering up the copies and the
    application in those cases have to make an explicit call to the
    print code for each copy.

    \sa setNumCopies()
*/

/*!
  \fn bool QPrinter::collateCopiesEnabled() const

  \internal

  Returns TRUE if the application should provide the user with the
  option of choosing a collated printout; otherwise returns FALSE.

  Collation means that each page is printed in order, i.e. print the
  first page, then the second page, then the third page and so on, and
  then repeat this sequence for as many copies as have been requested.
  If you don't collate you get several copies of the first page, then
  several copies of the second page, then several copies of the third
  page, and so on.

  \sa setCollateCopiesEnabled() setCollateCopies() collateCopies()
*/

/*!
  \fn void QPrinter::setCollateCopiesEnabled(bool enable)

  \internal

    If \a enable is TRUE (the default) the user is given the choice of
    whether to print out multiple copies collated in the print dialog.
    If \a enable is FALSE, then collateCopies() will be ignored.

  Collation means that each page is printed in order, i.e. print the
  first page, then the second page, then the third page and so on, and
  then repeat this sequence for as many copies as have been requested.
  If you don't collate you get several copies of the first page, then
  several copies of the second page, then several copies of the third
  page, and so on.

  \sa collateCopiesEnabled() setCollateCopies() collateCopies()
*/

/*!
  \fn bool QPrinter::collateCopies() const

  \internal

  Returns TRUE if collation is turned on when multiple copies is selected.
  Returns FALSE if it is turned off when multiple copies is selected.

  \sa collateCopiesEnabled() setCollateCopiesEnabled() setCollateCopies()
*/

/*!
  \internal

  Sets the default value for collation checkbox when the print dialog appears.
  If \a on is TRUE, it will enable setCollateCopiesEnabled().
  The default value is FALSE. This value will be changed by what the
  user presses in the print dialog.

  \sa collateCopiesEnabled() setCollateCopiesEnabled() collateCopies()
*/

void QPrinter::setCollateCopies(bool on)
{
    if (!collateCopiesEnabled() && on)
	setCollateCopiesEnabled(on);
    usercolcopies = on;
}

/*!
    Sets the number of copies to be printed to \a numCopies.

    The printer driver reads this setting and prints the specified
    number of copies.

    \sa numCopies(), setup()
*/

void QPrinter::setNumCopies( int numCopies )
{
    ncopies = numCopies;
#if defined(Q_WS_WIN)
    reinit();
#endif
}


/*!
    Returns the printer options selection string. This is useful only
    if the print command has been explicitly set.

    The default value (a null string) implies that the printer should
    be selected in a system-dependent manner.

    Any other value implies that the given value should be used.

    \sa setPrinterSelectionOption()
*/

QString QPrinter::printerSelectionOption() const
{
    return option_string;
}


/*!
    Sets the printer to use \a option to select the printer. \a option
    is null by default (which implies that Qt should be smart enough
    to guess correctly), but it can be set to other values to use a
    specific printer selection option.

    If the printer selection option is changed while the printer is
    active, the current print job may or may not be affected.

    \sa printerSelectionOption()
*/

void QPrinter::setPrinterSelectionOption( const QString & option )
{
    option_string = option;
}


/*!
    Sets QPrinter to have the origin of the coordinate system at the
    top-left corner of the paper if \a fp is TRUE, or where it thinks
    the top-left corner of the printable area is if \a fp is FALSE.

    The default is FALSE. You can (probably) print on (0,0), and
    QPaintDeviceMetrics will report something smaller than the size
    indicated by PageSize. (Note that QPrinter may be wrong on Unix
    systems - it does not have perfect knowledge of the physical
    printer.)

    If you set \a fp to TRUE, QPaintDeviceMetrics will report the
    exact same size as indicated by \c PageSize, but you cannot print
    on all of that - you must take care of the output margins
    yourself.

    \sa PageSize setPageSize() QPaintDeviceMetrics fullPage()
*/

void QPrinter::setFullPage( bool fp )
{
    to_edge = fp;
}


/*!
    Returns TRUE if the origin of the printer's coordinate system is
    at the corner of the sheet and FALSE if it is at the edge of the
    printable area.

    See setFullPage() for details and caveats.

    \sa setFullPage() PageSize QPaintDeviceMetrics
*/

bool QPrinter::fullPage() const
{
    return to_edge;
}


/*!
    Requests that the printer prints at \a dpi or as near to \a dpi as
    possible.

    This setting affects the coordinate system as returned by, for
    example, QPaintDeviceMetrics and QPainter::viewport().

    The value depends on the \c PrintingMode used in the QPrinter
    constructor. By default, the dpi value of the screen is used.

    This function must be called before setup() to have an effect on
    all platforms.

    \sa resolution() setPageSize()
*/

void QPrinter::setResolution( int dpi )
{
    res = dpi;
    res_set = TRUE;
}


/*!
    Returns the current assumed resolution of the printer, as set by
    setResolution() or by the printer subsystem.

    \sa setResolution()
*/

int QPrinter::resolution() const
{
    return res;
}

/*!
    Sets the paper source setting to \a source.

    Windows only! This option can be changed while printing and will
    take effect from the next call to newPage()

    \sa paperSource()
*/

void QPrinter::setPaperSource( PaperSource source )
{
    paper_source = source;
#if defined(Q_WS_WIN)
    reinit();
#endif
}

/*!
    Returns the currently set paper source of the printer.

    \sa setPaperSource()
*/

QPrinter::PaperSource QPrinter::paperSource() const
{
    return paper_source;
}

/*!
  Sets the default selected page range to be used when the print setup
  dialog is opened to \a range. If the PageRange specified by \a range is
  currently disabled the function does nothing.

  \sa printRange()
*/
void QPrinter::setPrintRange( PrintRange range )
{
    if( range != AllPages )
	if( range == Selection
	    && !isOptionEnabled( PrintSelection ) )
	    setOptionEnabled( PrintSelection, TRUE );
	else if( range == PageRange
		 && !isOptionEnabled( PrintPageRange ) )
	    setOptionEnabled( PrintPageRange, TRUE );
    d->printRange = range;
}

/*!
  Returns the PageRange of the QPrinter. After the print setup dialog
  has been opened, this function returns the value selected by the user.

  \sa setPrintRange()
*/
QPrinter::PrintRange QPrinter::printRange() const
{
    return d->printRange;
}

/*!
    Enables the printer option with the identifier \a option if \a
    enable is TRUE, and disables option \a option if \a enable is FALSE.

    \sa isOptionEnabled()
*/
void QPrinter::setOptionEnabled( PrinterOption option, bool enable )
{
    if( enable ) {
	d->printerOptions |= ( 1 << option );
	if( ( option == PrintPageRange ) && min_pg==0 && max_pg==0 )
	    max_pg = 9999;
    } else {
	d->printerOptions &= ( ~( 1 << option ) );
    }
}

/*!
  Returns TRUE if the printer option with identifier \a option is enabled;
  otherwise returns FALSE.

  \sa setOptionEnabled()
 */
bool QPrinter::isOptionEnabled( PrinterOption option )
{
    return d->printerOptions & ( 1 << option );
}
#endif // QT_NO_PRINTER

