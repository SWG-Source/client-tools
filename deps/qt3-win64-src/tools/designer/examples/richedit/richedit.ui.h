void EditorForm::init()
{
    textEdit->setFocus();  
 
    QFontDatabase fonts;
    fontComboBox->insertStringList( fonts.families() );
    QString font = textEdit->family();
    font = font.lower();
    for ( int i = 0 ; i < fontComboBox->count(); i++ ) {
	if ( font == fontComboBox->text( i ) ) {
	    fontComboBox->setCurrentItem( i );
	    break;
	}
    }
}

void EditorForm::fileExit()
{
    if ( saveAndContinue( "Exit" ) )
	qApp->exit();
}

void EditorForm::fileNew()
{
    if ( saveAndContinue( "New" ) )
    	textEdit->clear();
}

void EditorForm::fileOpen()
{
    if ( saveAndContinue( "Open" ) ) {
    	QString fn( QFileDialog::getOpenFileName( 
			QString::null, 
			"Rich Text Files (*.htm*)", this ) );   
	if ( !fn.isEmpty() ) {
	    fileName = fn;
	    QFile file( fileName );
	    if ( file.open( IO_ReadOnly ) ) {
		QTextStream ts( &file );
		textEdit->setText( ts.read() );
	    }
	}
    }
}

void EditorForm::fileSave()
{
    if ( fileName.isEmpty() ) {
	fileSaveAs();
    } else {
	QFile f( fileName );
	if ( f.open( IO_WriteOnly ) ) {
	    QTextStream ts( &f );
	    ts << textEdit->text();
	    textEdit->setModified( FALSE );
	}
    }
}

void EditorForm::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName( 
    				"", "Rich Text Files (*.htm*)", this );
    if ( !fn.isEmpty() ) {
	fileName = fn;
	fileSave();
    }
}

void EditorForm::helpAbout()
{
    
}

void EditorForm::helpContents()
{
    
}

void EditorForm::helpIndex()
{
    
}

void EditorForm::changeAlignment(QAction * align)
{
    if ( align == leftAlignAction ) 
	textEdit->setAlignment( Qt::AlignLeft );
    else if ( align == rightAlignAction )
	textEdit->setAlignment( Qt::AlignRight );
    else if ( align == centerAlignAction )
	textEdit->setAlignment( Qt::AlignCenter );
}

int EditorForm::saveAndContinue(const QString & action)
{
    int continueAction = 1;
    
    if ( textEdit->isModified() ) {  
	switch( QMessageBox::information(     
		this, "Rich Edit",   
		"The document contains unsaved changes.\n"   
		"Do you want to save the changes?",   
		"&Save", "&Don't Save", "&Cancel " + action,   
		0, // Enter == button 0   
		2 ) ) { // Escape == button 2   
	case 0: // Save; continue  
     		fileSave();  
        	break;   
    	case 1: // Do not save; continue
        	break;   
    	case 2: // Cancel
		continueAction = 0;
        	break;  
    	}	  
    }  
    
    return continueAction;
}

