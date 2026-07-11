void MainForm::init()
{

}

void MainForm::destroy()
{

}

void MainForm::creditDialog()
{
    QDialog *creditForm = (QDialog *)
    	QWidgetFactory::create( "../credit/creditformbase.ui" );
    // Set up the dynamic dialog here
    
    if ( creditForm->exec() ) {
	// The user accepted, act accordingly
	QSpinBox *amount = (QSpinBox *) creditForm->child( "amountSpinBox", "QSpinBox" );
	if ( amount )
	    ratingTextLabel->setText( amount->text() ); 
    }
    delete creditForm;
}

