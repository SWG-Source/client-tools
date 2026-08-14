/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions use Qt Designer which will
** update this file, preserving your code. Create an init() function in place
** of a constructor, and a destroy() function in place of a destructor.
*****************************************************************************/


const int CLIP_AS_HEX = 0;
const int CLIP_AS_NAME = 1;
const int CLIP_AS_RGB = 2;
const int COL_NAME = 0;
const int COL_HEX = 1;
const int COL_WEB = 2;
const QString WINDOWS_REGISTRY = "/QtExamples";
const QString APP_KEY = "/ColorTool/";

void MainForm::init()
{
    clipboard = QApplication::clipboard();
    if ( clipboard->supportsSelection() )
	clipboard->setSelectionMode( TRUE );

    findForm = 0;
    loadSettings();
    m_filename = "";
    m_changed = FALSE;
    m_table_dirty = TRUE;
    m_icons_dirty = TRUE;
    clearData( TRUE );
}

void MainForm::clearData( bool fillWithDefaults )
{
    setCaption( "Color Tool" );

    m_colors.clear();
    m_comments.clear();

    if ( fillWithDefaults ) {
	m_colors["black"] = Qt::black;
	m_colors["blue"] = Qt::blue;
	m_colors["cyan"] = Qt::cyan;
	m_colors["darkblue"] = Qt::darkBlue;
	m_colors["darkcyan"] = Qt::darkCyan;
	m_colors["darkgray"] = Qt::darkGray;
	m_colors["darkgreen"] = Qt::darkGreen;
	m_colors["darkmagenta"] = Qt::darkMagenta;
	m_colors["darkred"] = Qt::darkRed;
	m_colors["darkyellow"] = Qt::darkYellow;
	m_colors["gray"] = Qt::gray;
	m_colors["green"] = Qt::green;
	m_colors["lightgray"] = Qt::lightGray;
	m_colors["magenta"] = Qt::magenta;
	m_colors["red"] = Qt::red;
	m_colors["white"] = Qt::white;
	m_colors["yellow"] = Qt::yellow;
    }

    populate();
}

void MainForm::populate()
{
    if ( m_table_dirty ) {
	for ( int r = 0; r < colorTable->numRows(); ++r ) {
	    for ( int c = 0; c < colorTable->numCols(); ++c ) {
		colorTable->clearCell( r, c );
	    }
	}

	colorTable->setNumRows( m_colors.count() );
	if ( ! m_colors.isEmpty() ) {
	    QPixmap pixmap( 22, 22 );
	    int row = 0;
	    QMap<QString,QColor>::ConstIterator it;
	    for ( it = m_colors.constBegin(); it != m_colors.constEnd(); ++it ) {
		QColor color = it.data();
		pixmap.fill( color );
		colorTable->setText( row, COL_NAME, it.key() );
		colorTable->setPixmap( row, COL_NAME, pixmap );
		colorTable->setText( row, COL_HEX, color.name().upper() );
		if ( m_show_web ) {
		    QCheckTableItem *item = new QCheckTableItem( colorTable, "" );
		    item->setChecked( isWebColor( color ) );
		    colorTable->setItem( row, COL_WEB, item );
		}
		row++;
	    }
	    colorTable->setCurrentCell( 0, 0 );
	}
	colorTable->adjustColumn( COL_NAME );
	colorTable->adjustColumn( COL_HEX );
	if ( m_show_web ) {
	    colorTable->showColumn( COL_WEB );
	    colorTable->adjustColumn( COL_WEB );
	}
	else
	    colorTable->hideColumn( COL_WEB );
	m_table_dirty = FALSE;
    }

    if ( m_icons_dirty ) {
	colorIconView->clear();

	QMap<QString,QColor>::ConstIterator it;
	for ( it = m_colors.constBegin(); it != m_colors.constEnd(); ++it )
	    (void) new QIconViewItem( colorIconView, it.key(),
				      colorSwatch( it.data() ) );
	m_icons_dirty = FALSE;
    }
}

QPixmap MainForm::colorSwatch( const QColor color )
{
    QPixmap pixmap( 80, 80 );
    pixmap.fill( white );
    QPainter painter;
    painter.begin( &pixmap );
    painter.setPen( NoPen );
    painter.setBrush( color );
    painter.drawEllipse( 0, 0, 80, 80 );
    painter.end();
    return pixmap;
}

void MainForm::fileNew()
{
    if ( okToClear() ) {
	m_filename = "";
	m_changed = FALSE;
	m_table_dirty = TRUE;
	m_icons_dirty = TRUE;
	clearData( FALSE );
    }
}

void MainForm::fileOpen()
{
    if ( ! okToClear() )
	return;

    QString filename = QFileDialog::getOpenFileName(
			    QString::null, "Colors (*.txt)", this,
			    "file open", "Color Tool -- File Open" );
    if ( ! filename.isEmpty() )
	load( filename );
    else
	statusBar()->message( "File Open abandoned", 2000 );
}

void MainForm::fileSave()
{
    if ( m_filename.isEmpty() ) {
	fileSaveAs();
	return;
    }

    QFile file( m_filename );
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream stream( &file );
	if ( ! m_comments.isEmpty() )
	    stream << m_comments.join( "\n" ) << "\n";
	QMap<QString,QColor>::ConstIterator it;
	for ( it = m_colors.constBegin(); it != m_colors.constEnd(); ++it ) {
	    QColor color = it.data();
	    stream << QString( "%1 %2 %3\t\t%4" ).
			arg( color.red(), 3 ).
			arg( color.green(), 3 ).
			arg( color.blue(), 3 ).
			arg( it.key() ) << "\n";
	}
	file.close();
	setCaption( QString( "Color Tool -- %1" ).arg( m_filename ) );
	statusBar()->message( QString( "Saved %1 colors to '%2'" ).
				arg( m_colors.count() ).
				arg( m_filename ), 3000 );
	m_changed = FALSE;
    }
    else
	statusBar()->message( QString( "Failed to save '%1'" ).
				arg( m_filename ), 3000 );

}

void MainForm::fileSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(
			    QString::null, "Colors (*.txt)", this,
			    "file save as", "Color Tool -- File Save As" );
    if ( ! filename.isEmpty() ) {
	int ans = 0;
	if ( QFile::exists( filename ) )
	    ans = QMessageBox::warning(
			    this, "Color Tool -- Overwrite File",
			    QString( "Overwrite\n'%1'?" ).
				arg( filename ),
			    "&Yes", "&No", QString::null, 1, 1 );
	if ( ans == 0 ) {
	    m_filename = filename;
	    fileSave();
	    return;
	}
    }
    statusBar()->message( "Saving abandoned", 2000 );
}

void MainForm::load( const QString& filename )
{
    clearData( FALSE );
    m_filename = filename;
    QRegExp regex( "^\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\S+.*)$" );
    QFile file( filename );
    if ( file.open( IO_ReadOnly ) ) {
	statusBar()->message( QString( "Loading '%1'..." ).
			      arg( filename ) );
	QTextStream stream( &file );
	QString line;
	while ( ! stream.eof() ) {
	    line = stream.readLine();
	    if ( regex.search( line ) == -1 )
		m_comments += line;
	    else
		m_colors[regex.cap( 4 )] = QColor(
					    regex.cap( 1 ).toInt(),
					    regex.cap( 2 ).toInt(),
					    regex.cap( 3 ).toInt() );
	}
	file.close();
	m_filename = filename;
	setCaption( QString( "Color Tool -- %1" ).arg( m_filename ) );
	statusBar()->message( QString( "Loaded '%1'" ).
				arg( m_filename ), 3000 );
	QWidget *visible = colorWidgetStack->visibleWidget();
	m_icons_dirty = ! ( m_table_dirty = ( visible == tablePage ) );
	populate();
	m_icons_dirty = ! ( m_table_dirty = ( visible != tablePage ) );
	m_changed = FALSE;
    }
    else
	statusBar()->message( QString( "Failed to load '%1'" ).
				arg( m_filename ), 3000 );
}


bool MainForm::okToClear()
{
    if ( m_changed ) {
	QString msg;
	if ( m_filename.isEmpty() )
	    msg = "Unnamed colors ";
	else
	    msg = QString( "Colors '%1'\n" ).arg( m_filename );
	msg += QString( "has been changed." );
	int ans = QMessageBox::information(
			this,
			"Color Tool -- Unsaved Changes",
			msg, "&Save", "Cancel", "&Abandon",
			0, 1 );
	if ( ans == 0 )
	    fileSave();
	else if ( ans == 1 )
	    return FALSE;
    }

    return TRUE;
}

void MainForm::closeEvent( QCloseEvent * )
{
    fileExit();
}

void MainForm::fileExit()
{
    if ( okToClear() ) {
	saveSettings();
	QApplication::exit( 0 );
    }
}

void MainForm::editCut()
{
    QString name;
    QWidget *visible = colorWidgetStack->visibleWidget();
    statusBar()->message( QString( "Deleting '%1'" ).arg( name ) );

    if ( visible == tablePage && colorTable->numRows() ) {
	int row = colorTable->currentRow();
	name = colorTable->text( row, 0 );
	colorTable->removeRow( colorTable->currentRow() );
	if ( row < colorTable->numRows() )
	    colorTable->setCurrentCell( row, 0 );
	else if ( colorTable->numRows() )
	    colorTable->setCurrentCell( colorTable->numRows() - 1, 0 );
	m_icons_dirty = TRUE;
    }
    else if ( visible == iconsPage && colorIconView->currentItem() ) {
	QIconViewItem *item = colorIconView->currentItem();
	name = item->text();
	if ( colorIconView->count() == 1 )
	    colorIconView->clear();
	else {
	    QIconViewItem *current = item->nextItem();
	    if ( ! current )
		current = item->prevItem();
	    delete item;
	    if ( current )
		colorIconView->setCurrentItem( current );
	    colorIconView->arrangeItemsInGrid();
	}
	m_table_dirty = TRUE;
    }

    if ( ! name.isNull() ) {
	m_colors.remove( name );
	m_changed = TRUE;
	statusBar()->message( QString( "Deleted '%1'" ).arg( name ), 5000 );
    }
    else
	statusBar()->message( QString( "Failed to delete '%1'" ).arg( name ), 5000 );
}

void MainForm::editCopy()
{
    QString text;
    QWidget *visible = colorWidgetStack->visibleWidget();

    if ( visible == tablePage && colorTable->numRows() ) {
	int row = colorTable->currentRow();
	text = colorTable->text( row, 0 );
    }
    else if ( visible == iconsPage && colorIconView->currentItem() ) {
	QIconViewItem *item = colorIconView->currentItem();
	text = item->text();
    }
    if ( ! text.isNull() ) {
	QColor color = m_colors[text];
	switch ( m_clip_as ) {
	    case CLIP_AS_HEX: text = color.name(); break;
	    case CLIP_AS_NAME: break;
	    case CLIP_AS_RGB:
		    text = QString( "%1,%2,%3" ).
			arg( color.red() ).
			arg( color.green() ).
			arg( color.blue() );
		    break;
	}
	clipboard->setText( text );
	statusBar()->message( "Copied '" + text + "' to the clipboard" );
    }
}

void MainForm::editFind()
{
    if ( ! findForm ) {
	findForm = new FindForm( this );
	connect( findForm, SIGNAL( lookfor(const QString&) ),
		 this, SLOT( lookfor(const QString&) ) );
    }
    findForm->show();
}

void MainForm::lookfor( const QString& text )
{
    if ( text.isEmpty() )
	return;
    QString ltext = text.lower();
    QWidget *visible = colorWidgetStack->visibleWidget();
    bool found = FALSE;

    if ( visible == tablePage && colorTable->numRows() ) {
	int row = colorTable->currentRow();
	for ( int i = row + 1; i < colorTable->numRows(); ++i )
	    if ( colorTable->text( i, 0 ).lower().contains( ltext ) ) {
		colorTable->setCurrentCell( i, 0 );
		colorTable->clearSelection();
		colorTable->selectRow( i );
		found = TRUE;
		break;
	}
	if ( ! found )
	    colorTable->setCurrentCell( row, 0 );

    }
    else if ( visible == iconsPage ) {
	QIconViewItem *start = colorIconView->currentItem();
	for ( QIconViewItem *item = start->nextItem(); item; item = item->nextItem() )
	    if ( item->text().lower().contains( ltext ) ) {
		colorIconView->setCurrentItem( item );
		colorIconView->ensureItemVisible( item );
		found = TRUE;
		break;
	    }
	if ( ! found && start )
	    colorIconView->setCurrentItem( start );
    }
    if ( ! found ) {
	statusBar()->message( QString( "Could not find '%1' after here" ).
			      arg( text ) );
	findForm->notfound();
    }
}



void MainForm::helpIndex()
{

}

void MainForm::helpContents()
{

}

void MainForm::helpAbout()
{

}


void MainForm::changedTableColor( int row, int )
{
    changedColor( colorTable->text( row, COL_NAME ) );
}

void MainForm::changedIconColor( QIconViewItem *item )
{
    changedColor( item->text() );
}

void MainForm::changedColor( const QString& name )
{
    QColor color = m_colors[name];
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    statusBar()->message( QString( "%1 \"%2\" (%3,%4,%5)%6 {%7 %8 %9}" ).
			  arg( name ).
			  arg( color.name().upper() ).
			  arg( r ).arg( g ).arg( b ).
			  arg( isWebColor( color ) ? " web" : "" ).
			  arg( r / 255.0, 1, 'f', 3 ).
			  arg( g / 255.0, 1, 'f', 3 ).
			  arg( b / 255.0, 1, 'f', 3 )
			  );
}


void MainForm::changeView(QAction* action)
{
    if ( action == viewTableAction )
	colorWidgetStack->raiseWidget( tablePage );
    else
	colorWidgetStack->raiseWidget( iconsPage );
}

bool MainForm::isWebColor( QColor color )
{
    int r = color.red();
    int g = color.green();
    int b = color.blue();

    return ( ( r ==   0 || r ==  51 || r == 102 ||
	       r == 153 || r == 204 || r == 255 ) &&
	     ( g ==   0 || g ==  51 || g == 102 ||
	       g == 153 || g == 204 || g == 255 ) &&
	     ( b ==   0 || b ==  51 || b == 102 ||
	       b == 153 || b == 204 || b == 255 ) );
}


void MainForm::editAdd()
{
    QColor color = white;
    if ( ! m_colors.isEmpty() ) {
	QWidget *visible = colorWidgetStack->visibleWidget();
	if ( visible == tablePage )
	    color = colorTable->text( colorTable->currentRow(),
				      colorTable->currentColumn() );
	else
	    color = colorIconView->currentItem()->text();
    }
    color = QColorDialog::getColor( color, this );
    if ( color.isValid() ) {
	QPixmap pixmap( 80, 10 );
	pixmap.fill( color );
	ColorNameForm *colorForm = new ColorNameForm( this, "color", TRUE );
	colorForm->setColors( m_colors );
	colorForm->colorLabel->setPixmap( pixmap );
	if ( colorForm->exec() ) {
	    QString name = colorForm->colorLineEdit->text();
	    m_colors[name] = color;
	    QPixmap pixmap( 22, 22 );
	    pixmap.fill( color );
	    int row = colorTable->currentRow();
	    colorTable->insertRows( row, 1 );
	    colorTable->setText( row, COL_NAME, name );
	    colorTable->setPixmap( row, COL_NAME, pixmap );
	    colorTable->setText( row, COL_HEX, color.name().upper() );
	    if ( m_show_web ) {
		QCheckTableItem *item = new QCheckTableItem( colorTable, "" );
		item->setChecked( isWebColor( color ) );
		colorTable->setItem( row, COL_WEB, item );
	    }
	    colorTable->setCurrentCell( row, 0 );

	    (void) new QIconViewItem( colorIconView, name,
				      colorSwatch( color ) );
	    m_changed = TRUE;
	}
    }
}

void MainForm::editOptions()
{
    OptionsForm *options = new OptionsForm( this, "options", TRUE );
    switch ( m_clip_as ) {
    case CLIP_AS_HEX:
	options->hexRadioButton->setChecked( TRUE );
	break;
    case CLIP_AS_NAME:
	options->nameRadioButton->setChecked( TRUE );
	break;
    case CLIP_AS_RGB:
	options->rgbRadioButton->setChecked( TRUE );
	break;
    }
    options->webCheckBox->setChecked( m_show_web );

    if ( options->exec() ) {
	if ( options->hexRadioButton->isChecked() )
	    m_clip_as = CLIP_AS_HEX;
	else if ( options->nameRadioButton->isChecked() )
	    m_clip_as = CLIP_AS_NAME;
	else if ( options->rgbRadioButton->isChecked() )
	    m_clip_as = CLIP_AS_RGB;
	m_table_dirty = m_show_web !=
			options->webCheckBox->isChecked();
	m_show_web = options->webCheckBox->isChecked();

	populate();
    }
}

void MainForm::loadSettings()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    int windowWidth = settings.readNumEntry( APP_KEY + "WindowWidth", 550 );
    int windowHeight = settings.readNumEntry( APP_KEY + "WindowHeight", 500 );
    int windowX = settings.readNumEntry( APP_KEY + "WindowX", 0 );
    int windowY = settings.readNumEntry( APP_KEY + "WindowY", 0 );
    m_clip_as = settings.readNumEntry( APP_KEY + "ClipAs", CLIP_AS_HEX );
    m_show_web = settings.readBoolEntry( APP_KEY + "ShowWeb", TRUE );
    if ( ! settings.readBoolEntry( APP_KEY + "View", TRUE ) ) {
	colorWidgetStack->raiseWidget( iconsPage );
	viewIconsAction->setOn( TRUE );
    }

    resize( windowWidth, windowHeight );
    move( windowX, windowY );
}

void MainForm::saveSettings()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    settings.writeEntry( APP_KEY + "WindowWidth", width() );
    settings.writeEntry( APP_KEY + "WindowHeight", height() );
    settings.writeEntry( APP_KEY + "WindowX", x() );
    settings.writeEntry( APP_KEY + "WindowY", y() );
    settings.writeEntry( APP_KEY + "ClipAs", m_clip_as );
    settings.writeEntry( APP_KEY + "ShowWeb", m_show_web );
    settings.writeEntry( APP_KEY + "View",
	    colorWidgetStack->visibleWidget() == tablePage );
}


void MainForm::aboutToShow()
{
    populate();
}
