#include "vcr.h"
#include <qpushbutton.h>
#include <qlayout.h>

static const char * rewind_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"................",
".++..........++.",
".++........++++.",
".++......++++++.",
".++....++++++++.",
".++..++++++++++.",
".++++++++++++++.",
".++++++++++++++.",
".++++++++++++++.",
".++++++++++++++.",
".++..++++++++++.",
".++....++++++++.",
".++......++++++.",
".++........++++.",
".++.........+++.",
"................"};

static const char * play_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"................",
".++.............",
".++++...........",
".++++++.........",
".++++++++.......",
".++++++++++.....",
".++++++++++++...",
".+++++++++++++..",
".+++++++++++++..",
".++++++++++++...",
".++++++++++.....",
".++++++++.......",
".++++++.........",
".++++...........",
".+++............",
"................"};

static const char * next_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"................",
".++.....+.......",
".+++....++......",
".++++...+++.....",
".+++++..++++....",
".++++++.+++++...",
".+++++++++++++..",
".++++++++++++++.",
".++++++++++++++.",
".+++++++++++++..",
".++++++.+++++...",
".+++++..++++....",
".++++...+++.....",
".+++....++......",
".++.....+.......",
"................"};

static const char * stop_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"................",
".++++++++++++++.",
".++++++++++++++.",
".++++++++++++++.",
".+++........+++.",
".+++........+++.",
".+++........+++.",
".+++........+++.",
".+++........+++.",
".+++........+++.",
".+++........+++.",
".+++........+++.",
".++++++++++++++.",
".++++++++++++++.",
".++++++++++++++.",
"................"};


Vcr::Vcr( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    QPushButton *rewind = new QPushButton( QPixmap( rewind_xpm ), 0, this, "vcr_rewind" );
    layout->addWidget( rewind );
    connect( rewind, SIGNAL(clicked()), SIGNAL(rewind()) ); 

    QPushButton *play = new QPushButton( QPixmap( play_xpm ), 0, this, "vcr_play" );
    layout->addWidget( play );
    connect( play, SIGNAL(clicked()), SIGNAL(play()) ); 

    QPushButton *next = new QPushButton( QPixmap( next_xpm ), 0, this, "vcr_next" );
    layout->addWidget( next );
    connect( next, SIGNAL(clicked()), SIGNAL(next()) ); 

    QPushButton *stop = new QPushButton( QPixmap( stop_xpm ), 0, this, "vcr_stop" );
    layout->addWidget( stop );
    connect( stop, SIGNAL(clicked()), SIGNAL(stop()) ); 
}


