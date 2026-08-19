/****************************************************************************
** $Id: archivedialog.ui.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void ArchiveDialog::init()
{
    connect(&articleSearcher, SIGNAL(done(bool)), this, SLOT(searchDone(bool)));
    connect(&articleFetcher, SIGNAL(done(bool)), this, SLOT(fetchDone(bool)));
    connect(myListView, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(fetch(QListViewItem*)));
    connect(myLineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(myListView, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(fetch(QListViewItem*)));
    connect(myPushButton, SIGNAL(clicked()), this, SLOT(close()));
}

void ArchiveDialog::fetch( QListViewItem *it )
{
    QUrl u(it->text(1));
    articleFetcher.setHost(u.host());
    articleFetcher.get(it->text(1));
}

void ArchiveDialog::fetchDone( bool error )
{
    if (error) {
	QMessageBox::critical(this, "Error fetching",
			      "An error occurred when fetching this document: "
			      + articleFetcher.errorString(),	
			      QMessageBox::Ok, QMessageBox::NoButton);
    } else {
	myTextBrowser->setText(articleFetcher.readAll());
    }
}

void ArchiveDialog::search()
{
    if (articleSearcher.state() == QHttp::HostLookup
	|| articleSearcher.state() == QHttp::Connecting
	|| articleSearcher.state() == QHttp::Sending
	|| articleSearcher.state() == QHttp::Reading) {
	articleSearcher.abort();
    }
    
    if (myLineEdit->text() == "") {
	QMessageBox::critical(this, "Empty query",
			      "Please type a search string.",
			      QMessageBox::Ok, QMessageBox::NoButton);
    } else {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    
	articleSearcher.setHost("lists.trolltech.com");
    
	QHttpRequestHeader header("POST", "/qt-interest/search.php");
	header.setValue("Host", "lists.trolltech.com");
	header.setContentType("application/x-www-form-urlencoded");
    
	QString encodedTopic = myLineEdit->text();
	QUrl::encode(encodedTopic);
	QString searchString = "qt-interest=on&search=" + encodedTopic;

	articleSearcher.request(header, searchString.utf8());
    }

}

void ArchiveDialog::searchDone( bool error )
{
    if (error) {
	QMessageBox::critical(this, "Error searching",
			      "An error occurred when searching: "
			      + articleSearcher.errorString(),
			      QMessageBox::Ok, QMessageBox::NoButton);
    } else {
	QString result(articleSearcher.readAll());
	
	QRegExp rx("<a href=\"(http://lists\\.trolltech\\.com/qt-interest/.*)\">(.*)</a>");
	rx.setMinimal(TRUE);
	int pos = 0;
	while (pos >= 0) {
	    pos = rx.search(result, pos);
	    if (pos > -1) {
		pos += rx.matchedLength();
		new QListViewItem(myListView, rx.cap(2), rx.cap(1));
	    }
	}
    }
    
    QApplication::restoreOverrideCursor();
}
