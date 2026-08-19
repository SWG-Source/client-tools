void EditBookForm::init()
{
    QSqlQuery query( "SELECT surname, id FROM author ORDER BY surname;" );    
    while ( query.next() ) {
	ComboBoxAuthor->insertItem( query.value( 0 ).toString() ); 
	int id = query.value( 1 ).toInt();
	mapAuthor( query.value( 0 ).toString(), id, TRUE );
    }
}

void EditBookForm::beforeUpdateBook( QSqlRecord * buffer )
{
    int id;
    mapAuthor( ComboBoxAuthor->currentText(), id, FALSE );
    buffer->setValue( "authorid", id );
}

void EditBookForm::mapAuthor( const QString & name, int & id, bool populate )
{
    if ( populate ) 
	authorMap[ name ] = id;
    else
	id = authorMap[ name ];
}

void EditBookForm::primeInsertBook( QSqlRecord * buffer )
{
    QSqlQuery q;  
    q.exec( "UPDATE sequence SET sequence = sequence + 1 WHERE tablename='book';" );  
    q.exec( "SELECT sequence FROM sequence WHERE tablename='book';" );  
    if ( q.next() ) {  
	buffer->setValue( "id", q.value( 0 ) );  
    }  
}

void EditBookForm::primeUpdateBook( QSqlRecord * buffer )
{
    int id = buffer->value( "authorid" ).toInt();
    for ( int i = 0; i < ComboBoxAuthor->count(); i++ ) {
	QString author = ComboBoxAuthor->text( i );
	if ( authorMap.contains( author ) && authorMap[author] == id ) {
	    ComboBoxAuthor->setCurrentItem( i ) ;
	    break;
	}
    }
}

