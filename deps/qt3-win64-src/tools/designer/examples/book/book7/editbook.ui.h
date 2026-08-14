void EditBookForm::init()
{
    QSqlQuery query( "SELECT surname FROM author ORDER BY surname;" );    
    while ( query.next() ) 
	ComboBoxAuthor->insertItem( query.value( 0 ).toString()); 
}

void EditBookForm::beforeUpdateBook( QSqlRecord * buffer )
{
    QSqlQuery query( "SELECT id FROM author WHERE surname ='" + 
	ComboBoxAuthor->currentText() + "';" );
    if ( query.next() )
	buffer->setValue( "authorid", query.value( 0 ) );
}

void EditBookForm::primeInsertBook( QSqlRecord * buffer )
{
    QSqlQuery query;  
    query.exec( "UPDATE sequence SET sequence = sequence + 1 WHERE tablename='book';" );  
    query.exec( "SELECT sequence FROM sequence WHERE tablename='book';" );  
    if ( query.next() ) {  
	buffer->setValue( "id", query.value( 0 ) );  
    }  
}

void EditBookForm::primeUpdateBook( QSqlRecord * buffer )
{
    // Who is this book's author?
    QSqlQuery query( "SELECT surname FROM author WHERE id='" +  
	buffer->value( "authorid" ).toString() + "';" ); 
    QString author = "";    
    if ( query.next() )
	author = query.value( 0 ).toString();
    // Set the ComboBox to the right author
    for ( int i = 0; i < ComboBoxAuthor->count(); i++ ) {
	if ( ComboBoxAuthor->text( i ) == author ) {
	    ComboBoxAuthor->setCurrentItem( i ) ;
	    break;
	}
    }
}

