void MainForm::creditDialog()
{
    Receiver *receiver = new Receiver; 
    QDialog *creditForm = (QDialog *) 
    QWidgetFactory::create( "../credit/creditformbase.ui", receiver );   
    receiver->setParent( creditForm );   

    // Set up the dynamic dialog here 
     
    if ( creditForm->exec() ) { 
	// The user accepted, act accordingly 
	QSpinBox *amount = (QSpinBox *) creditForm->child( "amountSpinBox", "QSpinBox" ); 
	if ( amount ) 
	    ratingTextLabel->setText( amount->text() );  
    } 
    
    delete receiver; 
    delete creditForm; 
}

