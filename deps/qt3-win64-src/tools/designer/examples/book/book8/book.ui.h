void BookForm::editClicked()
{
    EditBookForm *dialog = new EditBookForm( this, "Edit Book Form", TRUE );
    QSqlCursor cur( "book" );
    dialog->BookDataBrowser->setSqlCursor( &cur );
    dialog->BookDataBrowser->setFilter( BookDataTable->filter() );
    dialog->BookDataBrowser->setSort(QSqlIndex::fromStringList( 
    	BookDataTable->sort(), &cur ) );
    dialog->BookDataBrowser->refresh();
    int i = BookDataTable->currentRow();
    if ( i == -1 ) i = 0; // Always use the first row
    dialog->BookDataBrowser->seek( i );
    dialog->exec();
    delete dialog;
    BookDataTable->refresh();
}

void BookForm::newCurrentAuthor( QSqlRecord *author )
{
    BookDataTable->setFilter( "authorid=" + author->value( "id" ).toString() );  
    BookDataTable->refresh();
}

void BookForm::primeInsertAuthor( QSqlRecord *buffer )
{
    QSqlQuery q; 
    q.exec( "UPDATE sequence SET sequence = sequence + 1 WHERE tablename='author';" ); 
    q.exec( "SELECT sequence FROM sequence WHERE tablename='author';" ); 
    if ( q.next() ) { 
	buffer->setValue( "id", q.value( 0 ) ); 
    } 
}

