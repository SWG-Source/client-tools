/* $Id: viewer.cpp 1439 2006-05-05 18:17:01Z chehrlic $ */

#include "viewer.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qlayout.h>

Viewer::Viewer()
       :QWidget()
{
    setFontSubstitutions();

    QString greeting_heb = QString::fromUtf8( "\327\251\327\234\327\225\327\235" );
    QString greeting_ru = QString::fromUtf8( "\320\227\320\264\321\200\320\260\320\262\321\201\321\202\320\262\321\203\320\271\321\202\320\265" );
    QString greeting_en( "Hello" );

    greetings = new QTextView( this, "textview" );

    greetings->setText( greeting_en + "\n" +
                       greeting_ru + "\n" +
                       greeting_heb );

    fontInfo = new QTextView( this, "fontinfo" );

    setDefault();

    defaultButton = new QPushButton( "Default", this,
                                                   "pushbutton1" );
    defaultButton->setFont( QFont( "times" ) );
    connect( defaultButton, SIGNAL( clicked() ),
             this, SLOT( setDefault() ) );

    sansSerifButton = new QPushButton( "Sans Serif", this,
                                                     "pushbutton2" );
    sansSerifButton->setFont( QFont( "Helvetica", 12 ) );
    connect( sansSerifButton, SIGNAL( clicked() ),
             this, SLOT( setSansSerif() ) );

    italicsButton = new QPushButton( "Italics", this,
                                                   "pushbutton3" );
    italicsButton->setFont( QFont( "lucida", 12, QFont::Bold, TRUE ) );
    connect( italicsButton, SIGNAL( clicked() ),
             this, SLOT( setItalics() ) );

    layout();
}

void Viewer::setDefault()
{
    QFont font( "Bavaria" );
    font.setPointSize( 24 );

    font.setWeight( QFont::Bold );
    font.setUnderline( TRUE );

    greetings->setFont( font );

    showFontInfo( font );
}

void Viewer::setSansSerif()
{
    QFont font( "Newyork", 18 );
    font.setStyleHint( QFont::SansSerif );
    greetings->setFont( font );

    showFontInfo( font );
}

void Viewer::setItalics()
{
    QFont font( "Tokyo" );
    font.setPointSize( 32 );
    font.setWeight( QFont::Bold );
    font.setItalic( TRUE );

    greetings->setFont( font );

    showFontInfo( font );
}

void Viewer::showFontInfo( QFont & font )
{
    QFontInfo info( font );

    QString messageText;
    messageText = "Font requested: \"" +
                  font.family() + "\" " +
                  QString::number( font.pointSize() ) + "pt<BR>" +
                  "Font used: \"" +
                  info.family() + "\" " +
                  QString::number( info.pointSize() ) + "pt<P>";

    QStringList substitutions = QFont::substitutes( font.family() );

    if ( ! substitutions.isEmpty() ){
	messageText += "The following substitutions exist for " + \
		       font.family() + ":<UL>";

	QStringList::Iterator i = substitutions.begin();
	while ( i != substitutions.end() ){
	    messageText += "<LI>\"" + (* i) + "\"";
	    i++;
	}
	 messageText += "</UL>";
    } else {
	messageText += "No substitutions exist for " + \
		       font.family() + ".";
    }

    fontInfo->setText( messageText );
}

void Viewer::setFontSubstitutions()
{
    QStringList substitutes;
    substitutes.append( "Times" );
    substitutes +=  "Mincho",
    substitutes << "Arabic Newspaper" << "crox";

    QFont::insertSubstitutions( "Bavaria", substitutes );

    QFont::insertSubstitution( "Tokyo", "Lucida" );
}


// For those who prefer to use Qt Designer for creating GUIs
// the following function might not be of particular interest:
// all it does is creating the widget layout.

void Viewer::layout()
{
    QHBoxLayout * textViewContainer = new QHBoxLayout();
    textViewContainer->addWidget( greetings );
    textViewContainer->addWidget( fontInfo );

    QHBoxLayout * buttonContainer = new QHBoxLayout();

    buttonContainer->addWidget( defaultButton );
    buttonContainer->addWidget( sansSerifButton );
    buttonContainer->addWidget( italicsButton );

    int maxButtonHeight = defaultButton->height();

    if ( sansSerifButton->height() > maxButtonHeight )
	maxButtonHeight = sansSerifButton->height();
    if ( italicsButton->height() > maxButtonHeight )
        maxButtonHeight = italicsButton->height();

    defaultButton->setFixedHeight( maxButtonHeight );
    sansSerifButton->setFixedHeight( maxButtonHeight );
    italicsButton->setFixedHeight( maxButtonHeight );

    QVBoxLayout * container = new QVBoxLayout( this );
    container->addLayout( textViewContainer );
    container->addLayout( buttonContainer );

    resize( 700, 250 );
}
