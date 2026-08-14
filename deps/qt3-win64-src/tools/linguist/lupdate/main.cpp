/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <metatranslator.h>
#include <proparser.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <errno.h>
#include <string.h>

// defined in fetchtr.cpp
extern void fetchtr_cpp( const char *fileName, MetaTranslator *tor,
			 const char *defaultContext, bool mustExist );
extern void fetchtr_ui( const char *fileName, MetaTranslator *tor,
			const char *defaultContext, bool mustExist );

// defined in merge.cpp
extern void merge( MetaTranslator *tor, const MetaTranslator *virginTor,
		   bool verbose );

typedef QValueList<MetaTranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
	     "    lupdate [options] project-file\n"
	     "    lupdate [options] source-files -ts ts-files\n"
	     "Options:\n"
	     "    -help  Display this information and exit\n"
	     "    -noobsolete\n"
	     "           Drop all obsolete strings\n"
	     "    -verbose\n"
	     "           Explain what is being done\n"
	     "    -version\n"
	     "           Display the version of lupdate and exit\n" );
}

static void updateTsFiles( const MetaTranslator& fetchedTor,
			   const QStringList& tsFileNames, const QString& codec,
			   bool noObsolete, bool verbose )
{
    QStringList::ConstIterator t = tsFileNames.begin();
    while ( t != tsFileNames.end() ) {
	MetaTranslator tor;
	tor.load( *t );
	if ( !codec.isEmpty() )
	    tor.setCodec( codec );
	if ( verbose )
	    fprintf( stderr, "Updating '%s'...\n", (*t).latin1() );
	merge( &tor, &fetchedTor, verbose );
	if ( noObsolete )
	    tor.stripObsoleteMessages();
	tor.stripEmptyContexts();
	if ( !tor.save(*t) )
	    fprintf( stderr, "lupdate error: Cannot save '%s': %s\n",
		     (*t).latin1(), strerror(errno) );
	++t;
    }
}

int main( int argc, char **argv )
{
    QString defaultContext = "@default";
    MetaTranslator fetchedTor;
    QCString codec;
    QStringList tsFileNames;

    bool verbose = FALSE;
    bool noObsolete = FALSE;
    bool metSomething = FALSE;
    int numFiles = 0;
    bool standardSyntax = TRUE;
    bool metTsFlag = FALSE;

    int i;

    for ( i = 1; i < argc; i++ ) {
	if ( qstrcmp(argv[i], "-ts") == 0 )
	    standardSyntax = FALSE;
    }

    for ( i = 1; i < argc; i++ ) {
	if ( qstrcmp(argv[i], "-help") == 0 ) {
	    printUsage();
	    return 0;
	} else if ( qstrcmp(argv[i], "-noobsolete") == 0 ) {
	    noObsolete = TRUE;
	    continue;
	} else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
	    verbose = TRUE;
	    continue;
	} else if ( qstrcmp(argv[i], "-version") == 0 ) {
	    fprintf( stderr, "lupdate version %s\n", QT_VERSION_STR );
	    return 0;
	} else if ( qstrcmp(argv[i], "-ts") == 0 ) {
	    metTsFlag = TRUE;
	    continue;
	}

	numFiles++;

	QString fullText;

	if ( !metTsFlag ) {
	    QFile f( argv[i] );
	    if ( !f.open(IO_ReadOnly) ) {
		fprintf( stderr, "lupdate error: Cannot open file '%s': %s\n",
			 argv[i], strerror(errno) );
		return 1;
	    }

	    QTextStream t( &f );
	    fullText = t.read();
	    f.close();
	}

	QString oldDir = QDir::currentDirPath();
	QDir::setCurrent( QFileInfo(argv[i]).dirPath() );

	if ( standardSyntax ) {
	    fetchedTor = MetaTranslator();
	    codec.truncate( 0 );
	    tsFileNames.clear();

	    QMap<QString, QString> tagMap = proFileTagMap( fullText );
	    QMap<QString, QString>::Iterator it;

	    for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
        	QStringList toks = QStringList::split( ' ', it.data() );
		QStringList::Iterator t;

        	for ( t = toks.begin(); t != toks.end(); ++t ) {
                    if ( it.key() == "HEADERS" || it.key() == "SOURCES" ) {
                	fetchtr_cpp( *t, &fetchedTor, defaultContext, TRUE );
                	metSomething = TRUE;
                    } else if ( it.key() == "INTERFACES" ||
				it.key() == "FORMS" ) {
                	fetchtr_ui( *t, &fetchedTor, defaultContext, TRUE );
			fetchtr_cpp( *t + ".h", &fetchedTor, defaultContext,
				     FALSE );
                	metSomething = TRUE;
                    } else if ( it.key() == "TRANSLATIONS" ) {
                	tsFileNames.append( *t );
                	metSomething = TRUE;
                    } else if ( it.key() == "CODEC" ||
				it.key() == "DEFAULTCODEC" ) {
                	codec = (*t).latin1();
                    }
        	}
            }

	    updateTsFiles( fetchedTor, tsFileNames, codec, noObsolete,
			   verbose );

	    if ( !metSomething ) {
		fprintf( stderr,
			 "lupdate warning: File '%s' does not look like a"
			 " project file\n",
			 argv[i] );
	    } else if ( tsFileNames.isEmpty() ) {
		fprintf( stderr,
			 "lupdate warning: Met no 'TRANSLATIONS' entry in"
			 " project file '%s'\n",
			 argv[i] );
	    }
	} else {
	    if ( metTsFlag ) {
		if ( QString(argv[i]).lower().endsWith(".ts") ) {
		    QFileInfo fi( argv[i] );
		    if ( !fi.exists() || fi.isWritable() ) {
			tsFileNames.append( argv[i] );
		    } else {
			fprintf( stderr,
				 "lupdate warning: For some reason, I cannot"
				 " save '%s'\n",
				 argv[i] );
		    }
		} else {
		    fprintf( stderr,
			     "lupdate error: File '%s' lacks .ts extension\n",
			     argv[i] );
		}
	    } else {
		QFileInfo fi(argv[i]);
		if ( QString(argv[i]).lower().endsWith(".ui") ) {
        	    fetchtr_ui( fi.fileName(), &fetchedTor, defaultContext, TRUE );
		    fetchtr_cpp( QString(fi.fileName()) + ".h", &fetchedTor,
				 defaultContext, FALSE );
		} else {
        	    fetchtr_cpp( fi.fileName(), &fetchedTor, defaultContext, TRUE );
		}
	    }
	}
	QDir::setCurrent( oldDir );
    }

    if ( !standardSyntax )
	updateTsFiles( fetchedTor, tsFileNames, codec, noObsolete, verbose );

    if ( numFiles == 0 ) {
	printUsage();
	return 1;
    }
    return 0;
}
