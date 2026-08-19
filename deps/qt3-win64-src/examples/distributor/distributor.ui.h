/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qapplication.h>
#include <qcursor.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtimer.h>


void Distributor::init()
{
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(checkLibData()) );

    cancelButton()->setAutoDefault( FALSE );
    backButton()->setAutoDefault( FALSE );

    setNextEnabled( selectLibrary, FALSE );

    setHelpEnabled( selectLibrary, FALSE );
    setHelpEnabled( modifyPaths, FALSE );
    setHelpEnabled( verifyMods, FALSE );

    setFinishEnabled( verifyMods, TRUE );
}

void Distributor::showPage( QWidget *page )
{
    if ( page == selectLibrary ) {
	nextButton()->setDefault( TRUE );
	libFilename->setFocus();
    } else if ( page == modifyPaths ) {
	nextButton()->setDefault( TRUE );
	prefixPath->selectAll();
	prefixPath->setFocus();
    } else if ( page == verifyMods ) {
	finishButton()->setDefault( TRUE );
	finishButton()->setFocus();

	QString labeltext =
	    tr("<p><b>Current Library File:</b> %1</p>"
	       "<table border=0>"
	       "<tr><td><b>New Installation Prefix:</b></td><td>%2</td></tr>"
	       "<tr><td></td><td></td></tr>"
	       "<tr><td><b>Binaries Path:</b></td><td>%3</td></tr>"
	       "<tr><td><b>Documentation Path:</b></td><td>%4</td></tr>"
	       "<tr><td><b>Headers Path:</b></td><td>%5</td></tr>"
	       "<tr><td><b>Libraries Path:</b></td><td>%6</td></tr>"
	       "<tr><td><b>Plugins Path:</b></td><td>%7</td></tr>"
	       "<tr><td><b>Data Path:</b></td><td>%8</td></tr>"
	       "</table>"
	       "<p>Please verify that these options are correct.  Press the "
	       "<i>Finish</i> button to apply these modifications to the Qt "
	       "library.  Use the <i>Back</i> button to make corrections.  Use "
	       "the <i>Cancel</i> button to abort.</p>")
	    .arg( libFilename->text() )
	    .arg( prefixPath->text() )
	    .arg( binPath->text() )
	    .arg( docPath->text() )
	    .arg( hdrPath->text() )
	    .arg( libPath->text() )
	    .arg( plgPath->text() )
	    .arg( datPath->text() );
	textLabel4->setText( labeltext );
    }

    QWizard::showPage( page );
}

void Distributor::checkLibFilename( const QString &filename )
{
    setNextEnabled( selectLibrary, FALSE );

    QFileInfo fileinfo( filename );
    if ( ! filename.isEmpty() && fileinfo.exists() &&
	 fileinfo.isReadable() && fileinfo.isWritable() &&
	 fileinfo.isFile() && !fileinfo.isSymLink() )
	timer->start( 500, TRUE );
}

void Distributor::browseLibFilename()
{
    QString filename =
	QFileDialog::getOpenFileName( QString::null, QString::null, this );
    libFilename->setText( filename );
}

static char *find_pattern( char *h, const char *n, ulong hlen )
{
    if ( ! h || ! n || hlen == 0 )
	return 0;

#ifdef Q_OS_UNIX
    size_t nlen;
#else
    ulong nlen;
#endif

    char nc = *n++;
    nlen = strlen( n );
    char hc;

    do {
	do {
	    hc = *h++;
	    if ( hlen-- < 1 )
		return 0;
	} while ( hc != nc );

	if ( nlen > hlen )
	    return 0;
    } while ( qstrncmp( h, n, nlen ) != 0 );
    return h + nlen;
}

void Distributor::checkLibData()
{
    struct step {
	const char *key;
	QCString value;
	bool done;
    } steps[7];

    steps[0].key = "qt_nstpath=";
    steps[0].done = FALSE;

    steps[1].key = "qt_binpath=";
    steps[1].done = FALSE;

    steps[2].key = "qt_docpath=";
    steps[2].done = FALSE;

    steps[3].key = "qt_hdrpath=";
    steps[3].done = FALSE;

    steps[4].key = "qt_libpath=";
    steps[4].done = FALSE;

    steps[5].key = "qt_plgpath=";
    steps[5].done = FALSE;

    steps[6].key = "qt_datpath=";
    steps[6].done = FALSE;

    uint completed = 0;
    uint total_steps = sizeof(steps) / sizeof(step);

    QFile file( libFilename->text() );
    if ( file.open( IO_ReadOnly ) ) {
	QApplication::setOverrideCursor( WaitCursor );

	// instead of reading in the entire file, do the search in chunks
	char data[60000];
	ulong offset = 0;

	while ( ! file.atEnd() && completed < total_steps ) {
	    QApplication::eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

	    ulong len = file.readBlock( data, sizeof(data) );
	    if ( len < 267 ) {
		// not enough room to make any modifications... stop
		break;
	    }

	    for ( uint x = 0; x < total_steps; ++x ) {
		if ( steps[x].done ) continue;

		char *s = find_pattern( data, steps[x].key, len );
		if ( s ) {
		    ulong where = s - data;
		    if ( len - where < 256 ) {
			// not enough space left to write the full
			// path... move the file pointer back to just
			// before the pattern and continue
			offset += where - 11;
			file.at( offset );
			len = file.readBlock( data, sizeof(data) );
			--x; // retry the current step
			continue;
		    }

		    steps[x].value = s;
		    steps[x].done = TRUE;

		    ++completed;
		}
	    }

	    // move to the new read position
	    offset += len - 11;
	    file.at( offset );
	}

	file.close();

	QApplication::restoreOverrideCursor();
    }

    if ( completed == total_steps ) {
	setNextEnabled( selectLibrary, TRUE );

	QString prefix = QFile::decodeName( steps[0].value );
	prefixPath->setText( prefix );

	QString def_bin = prefix + QString::fromLatin1( "/bin" );
	QString def_doc = prefix + QString::fromLatin1( "/doc" );
	QString def_hdr = prefix + QString::fromLatin1( "/include" );
	QString def_lib = prefix + QString::fromLatin1( "/lib" );
	QString def_plg = prefix + QString::fromLatin1( "/plugins" );
	QString def_dat = prefix;

	QString bin = QFile::decodeName( steps[1].value );
	QString doc = QFile::decodeName( steps[2].value );
	QString hdr = QFile::decodeName( steps[3].value );
	QString lib = QFile::decodeName( steps[4].value );
	QString plg = QFile::decodeName( steps[5].value );
	QString dat = QFile::decodeName( steps[6].value );

	autoSet->setChecked( def_bin == bin &&
			     def_doc == doc &&
			     def_hdr == hdr &&
			     def_lib == lib &&
			     def_plg == plg &&
			     def_dat == dat );

	if ( ! autoSet->isChecked() ) {
	    binPath->setText( bin );
	    docPath->setText( doc );
	    hdrPath->setText( hdr );
	    libPath->setText( lib );
	    plgPath->setText( plg );
	    datPath->setText( dat );
	}
    }
}

void Distributor::checkInstallationPrefix( const QString &prefix )
{
    if ( autoSet->isChecked() ) {
	binPath->setText( prefix + QString::fromLatin1( "/bin" ) );
	docPath->setText( prefix + QString::fromLatin1( "/doc" ) );
	hdrPath->setText( prefix + QString::fromLatin1( "/include" ) );
	libPath->setText( prefix + QString::fromLatin1( "/lib" ) );
	plgPath->setText( prefix + QString::fromLatin1( "/plugins" ) );
	datPath->setText( prefix );
    }
}

void Distributor::browseInstallationPrefix()
{
    QString prefix =
	QFileDialog::getOpenFileName( QString::null, QString::null, this );
    prefixPath->setText( prefix );
}


void Distributor::toggleAutoSet( bool autoset )
{
    if ( autoset ) checkInstallationPrefix( prefixPath->text() );
}

void Distributor::accept()
{
    struct step {
	const char *key;
	QCString value;
	bool done;
    } steps[7];

    steps[0].key = "qt_nstpath=";
    steps[0].value = QFile::encodeName( prefixPath->text() );
    steps[0].done = FALSE;

    steps[1].key = "qt_binpath=";
    steps[1].value = QFile::encodeName( binPath->text() );
    steps[1].done = FALSE;

    steps[2].key = "qt_docpath=";
    steps[2].value = QFile::encodeName( docPath->text() );
    steps[2].done = FALSE;

    steps[3].key = "qt_hdrpath=";
    steps[3].value = QFile::encodeName( hdrPath->text() );
    steps[3].done = FALSE;

    steps[4].key = "qt_libpath=";
    steps[4].value = QFile::encodeName( libPath->text() );
    steps[4].done = FALSE;

    steps[5].key = "qt_plgpath=";
    steps[5].value = QFile::encodeName( plgPath->text() );
    steps[5].done = FALSE;

    steps[6].key = "qt_datpath=";
    steps[6].value = QFile::encodeName( datPath->text() );
    steps[6].done = FALSE;

    uint completed = 0;
    uint total_steps = sizeof(steps) / sizeof(step);

    QFile file( libFilename->text() );
    if ( file.open( IO_ReadWrite ) ) {
	QApplication::setOverrideCursor( WaitCursor );

	// instead of reading in the entire file, do the search in chunks
	char data[60000];
	ulong offset = 0;

	while ( ! file.atEnd() && completed < total_steps ) {
	    QApplication::eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

	    ulong len = file.readBlock( data, sizeof(data) );
	    if ( len < 267 ) {
		// not enough room to make any modifications... stop
		break;
	    }

	    uint completed_save = completed;
	    for ( uint x = 0; x < total_steps; ++x ) {
		if ( steps[x].done ) continue;

		char *s = find_pattern( data, steps[x].key, len );
		if ( s ) {
		    ulong where = s - data;
		    if ( len - where < 256 ) {
			// not enough space left to write the full
			// path... move the file pointer back to just
			// before the pattern and continue
			offset += where - 11;
			file.at( offset );
			len = file.readBlock( data, sizeof(data) );
			--x; // retry the current step
			continue;
		    }

		    qstrcpy( s, steps[x].value );
		    steps[x].done = TRUE;

		    ++completed;
		}
	    }

	    if ( completed != completed_save ) {
		// something changed...  move file pointer back to
		// where the data was read and write the new data
		file.at( offset );
		file.writeBlock( data, len );
	    }

	    // move to the new read position
	    offset += len - 11;
	    file.at( offset );
	}

	file.close();

	QApplication::restoreOverrideCursor();
    }

    if ( completed != total_steps ) {
    	QMessageBox::information( this,
				  tr("Qt Distribution Wizard"),
				  tr("<p><h3>Modifications failed.</h3></p>"
				     "<p>Please make sure that you have permission "
				     "to write the selected file, and that the library "
				     "is properly built.</p>") );
	return;
    }

    QWizard::accept();
}
