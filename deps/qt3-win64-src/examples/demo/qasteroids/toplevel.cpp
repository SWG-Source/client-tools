/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */
//	--- toplevel.cpp ---
#include <qaccel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>

#include <qapplication.h>

#include "toplevel.h"
#include "ledmeter.h"


#define SB_SCORE	1
#define SB_LEVEL	2
#define SB_SHIPS	3

struct SLevel
{
    int    nrocks;
    double rockSpeed;
};

#define MAX_LEVELS	16

SLevel levels[MAX_LEVELS] =
{
    { 1, 0.4 },
    { 1, 0.6 },
    { 2, 0.5 },
    { 2, 0.7 },
    { 2, 0.8 },
    { 3, 0.6 },
    { 3, 0.7 },
    { 3, 0.8 },
    { 4, 0.6 },
    { 4, 0.7 },
    { 4, 0.8 },
    { 5, 0.7 },
    { 5, 0.8 },
    { 5, 0.9 },
    { 5, 1.0 }
};

const char *soundEvents[] =
{
    "ShipDestroyed",
    "RockDestroyed",
    0
};

const char *soundDefaults[] =
{
    "Explosion.wav",
    "ploop.wav",
    0
};


KAstTopLevel::KAstTopLevel( QWidget *parent, const char *name )
    : QMainWindow( parent, name, 0 )
{
    QWidget *border = new QWidget( this );
    border->setBackgroundColor( black );
    setCentralWidget( border );

    QVBoxLayout *borderLayout = new QVBoxLayout( border );
    borderLayout->addStretch( 1 );

    QWidget *mainWin = new QWidget( border );
    mainWin->setFixedSize(640, 480);
    borderLayout->addWidget( mainWin, 0, AlignHCenter );

    borderLayout->addStretch( 1 );

    view = new KAsteroidsView( mainWin );
    view->setFocusPolicy( StrongFocus );
    connect( view, SIGNAL( shipKilled() ), SLOT( slotShipKilled() ) );
    connect( view, SIGNAL( rockHit(int) ), SLOT( slotRockHit(int) ) );
    connect( view, SIGNAL( rocksRemoved() ), SLOT( slotRocksRemoved() ) );
    connect( view, SIGNAL( updateVitals() ), SLOT( slotUpdateVitals() ) );

    QVBoxLayout *vb = new QVBoxLayout( mainWin );
    QHBoxLayout *hb = new QHBoxLayout;
    QHBoxLayout *hbd = new QHBoxLayout;
    vb->addLayout( hb );

    QFont labelFont( "helvetica", 24 );
    QColorGroup grp( darkGreen, black, QColor( 128, 128, 128 ),
	    QColor( 64, 64, 64 ), black, darkGreen, black );
    QPalette pal( grp, grp, grp );

    mainWin->setPalette( pal );

    hb->addSpacing( 10 );

    QLabel *label;
    label = new QLabel( tr("Score"), mainWin );
    label->setFont( labelFont );
    label->setPalette( pal );
    label->setFixedWidth( label->sizeHint().width() );
    hb->addWidget( label );

    scoreLCD = new QLCDNumber( 6, mainWin );
    scoreLCD->setFrameStyle( QFrame::NoFrame );
    scoreLCD->setSegmentStyle( QLCDNumber::Flat );
    scoreLCD->setFixedWidth( 150 );
    scoreLCD->setPalette( pal );
    hb->addWidget( scoreLCD );
    hb->addStretch( 10 );

    label = new QLabel( tr("Level"), mainWin );
    label->setFont( labelFont );
    label->setPalette( pal );
    label->setFixedWidth( label->sizeHint().width() );
    hb->addWidget( label );

    levelLCD = new QLCDNumber( 2, mainWin );
    levelLCD->setFrameStyle( QFrame::NoFrame );
    levelLCD->setSegmentStyle( QLCDNumber::Flat );
    levelLCD->setFixedWidth( 70 );
    levelLCD->setPalette( pal );
    hb->addWidget( levelLCD );
    hb->addStretch( 10 );

    label = new QLabel( tr("Ships"), mainWin );
    label->setFont( labelFont );
    label->setFixedWidth( label->sizeHint().width() );
    label->setPalette( pal );
    hb->addWidget( label );

    shipsLCD = new QLCDNumber( 1, mainWin );
    shipsLCD->setFrameStyle( QFrame::NoFrame );
    shipsLCD->setSegmentStyle( QLCDNumber::Flat );
    shipsLCD->setFixedWidth( 40 );
    shipsLCD->setPalette( pal );
    hb->addWidget( shipsLCD );

    hb->addStrut( 30 );

    vb->addWidget( view, 10 );

// -- bottom layout:
    vb->addLayout( hbd );

    QFont smallFont( "helvetica", 14 );
    hbd->addSpacing( 10 );

    QString sprites_prefix = "qasteroids/sprites/";
/*
    label = new QLabel( tr( "T" ), mainWin );
    label->setFont( smallFont );
    label->setFixedWidth( label->sizeHint().width() );
    label->setPalette( pal );
    hbd->addWidget( label );

    teleportsLCD = new QLCDNumber( 1, mainWin );
    teleportsLCD->setFrameStyle( QFrame::NoFrame );
    teleportsLCD->setSegmentStyle( QLCDNumber::Flat );
    teleportsLCD->setPalette( pal );
    teleportsLCD->setFixedHeight( 20 );
    hbd->addWidget( teleportsLCD );

    hbd->addSpacing( 10 );
*/
    QPixmap pm( sprites_prefix + "powerups/brake.png" );
    label = new QLabel( mainWin );
    label->setPixmap( pm );
    label->setFixedWidth( label->sizeHint().width() );
    label->setPalette( pal );
    hbd->addWidget( label );

    brakesLCD = new QLCDNumber( 1, mainWin );
    brakesLCD->setFrameStyle( QFrame::NoFrame );
    brakesLCD->setSegmentStyle( QLCDNumber::Flat );
    brakesLCD->setPalette( pal );
    brakesLCD->setFixedHeight( 20 );
    hbd->addWidget( brakesLCD );

    hbd->addSpacing( 10 );

    pm.load( sprites_prefix + "powerups/shield.png" );
    label = new QLabel( mainWin );
    label->setPixmap( pm );
    label->setFixedWidth( label->sizeHint().width() );
    label->setPalette( pal );
    hbd->addWidget( label );

    shieldLCD = new QLCDNumber( 1, mainWin );
    shieldLCD->setFrameStyle( QFrame::NoFrame );
    shieldLCD->setSegmentStyle( QLCDNumber::Flat );
    shieldLCD->setPalette( pal );
    shieldLCD->setFixedHeight( 20 );
    hbd->addWidget( shieldLCD );

    hbd->addSpacing( 10 );

    pm.load( sprites_prefix + "powerups/shoot.png" );
    label = new QLabel( mainWin );
    label->setPixmap( pm );
    label->setFixedWidth( label->sizeHint().width() );
    label->setPalette( pal );
    hbd->addWidget( label );

    shootLCD = new QLCDNumber( 1, mainWin );
    shootLCD->setFrameStyle( QFrame::NoFrame );
    shootLCD->setSegmentStyle( QLCDNumber::Flat );
    shootLCD->setPalette( pal );
    shootLCD->setFixedHeight( 20 );
    hbd->addWidget( shootLCD );

    hbd->addStretch( 1 );

    label = new QLabel( tr( "Fuel" ), mainWin );
    label->setFont( smallFont );
    label->setFixedWidth( label->sizeHint().width() + 10 );
    label->setPalette( pal );
    hbd->addWidget( label );

    powerMeter = new KALedMeter( mainWin );
    powerMeter->setFrameStyle( QFrame::Box | QFrame::Plain );
    powerMeter->setRange( MAX_POWER_LEVEL );
    powerMeter->addColorRange( 10, darkRed );
    powerMeter->addColorRange( 20, QColor(160, 96, 0) );
    powerMeter->addColorRange( 70, darkGreen );
    powerMeter->setCount( 40 );
    powerMeter->setPalette( pal );
    powerMeter->setFixedSize( 200, 12 );
    hbd->addWidget( powerMeter );

    shipsRemain = 3;
    showHiscores = FALSE;

    actions.insert( Qt::Key_Up, Thrust );
    actions.insert( Qt::Key_Left, RotateLeft );
    actions.insert( Qt::Key_Right, RotateRight );
    actions.insert( Qt::Key_Space, Shoot );
    actions.insert( Qt::Key_Z, Teleport );
    actions.insert( Qt::Key_X, Brake );
    actions.insert( Qt::Key_S, Shield );
    actions.insert( Qt::Key_P, Pause );
    actions.insert( Qt::Key_L, Launch );
    actions.insert( Qt::Key_N, NewGame );

    view->showText( tr( "Press N to start playing" ), yellow );
}

KAstTopLevel::~KAstTopLevel()
{
}

void KAstTopLevel::playSound( const char * )
{
}

void KAstTopLevel::keyPressEvent( QKeyEvent *event )
{
    if ( event->isAutoRepeat() || !actions.contains( event->key() ) )
    {
        event->ignore();
        return;
    }

    Action a = actions[ event->key() ];

    switch ( a )
    {
        case RotateLeft:
            view->rotateLeft( TRUE );
            break;

        case RotateRight:
            view->rotateRight( TRUE );
            break;

        case Thrust:
            view->thrust( TRUE );
            break;

        case Shoot:
            view->shoot( TRUE );
            break;

        case Shield:
            view->setShield( TRUE );
            break;

        case Teleport:
            view->teleport( TRUE );
            break;

        case Brake:
            view->brake( TRUE );
            break;

        default:
            event->ignore();
            return;
    }
    event->accept();
}

void KAstTopLevel::keyReleaseEvent( QKeyEvent *event )
{
    if ( event->isAutoRepeat() || !actions.contains( event->key() ) )
    {
        event->ignore();
        return;
    }

    Action a = actions[ event->key() ];

    switch ( a )
    {
        case RotateLeft:
            view->rotateLeft( FALSE );
            break;

        case RotateRight:
            view->rotateRight( FALSE );
            break;

        case Thrust:
            view->thrust( FALSE );
            break;

        case Shoot:
            view->shoot( FALSE );
            break;

        case Brake:
            view->brake( FALSE );
            break;

        case Shield:
            view->setShield( FALSE );
            break;

        case Teleport:
            view->teleport( FALSE );
            break;

        case Launch:
            if ( waitShip )
            {
                view->newShip();
                waitShip = FALSE;
                view->hideText();
            }
            else
            {
                event->ignore();
                return;
            }
            break;

	case NewGame:
	    slotNewGame();
	    break;
/*
        case Pause:
            {
                view->pause( TRUE );
                QMessageBox::information( this,
                                          tr("KAsteroids is paused"),
                                          tr("Paused") );
                view->pause( FALSE );
            }
            break;
*/
        default:
            event->ignore();
            return;
    }

    event->accept();
}

void KAstTopLevel::showEvent( QShowEvent *e )
{
    QMainWindow::showEvent( e );
    view->pause( FALSE );
    view->setFocus();
}

void KAstTopLevel::hideEvent( QHideEvent *e )
{
    QMainWindow::hideEvent( e );
    view->pause( TRUE );
}

void KAstTopLevel::slotNewGame()
{
    score = 0;
    shipsRemain = SB_SHIPS;
    scoreLCD->display( 0 );
    level = 0;
    levelLCD->display( level+1 );
    shipsLCD->display( shipsRemain-1 );
    view->newGame();
    view->setRockSpeed( levels[0].rockSpeed );
    view->addRocks( levels[0].nrocks );
//    view->showText( tr( "Press L to launch." ), yellow );
    view->newShip();
    waitShip = FALSE;
    view->hideText();
    isPaused = FALSE;
}

void KAstTopLevel::slotShipKilled()
{
    shipsRemain--;
    shipsLCD->display( shipsRemain-1 );

    playSound( "ShipDestroyed" );

    if ( shipsRemain )
    {
        waitShip = TRUE;
        view->showText( tr( "Ship Destroyed. Press L to launch."), yellow );
    }
    else
    {
        view->showText( tr("Game Over!"), red );
        view->endGame();
	doStats();
//        highscore->addEntry( score, level, showHiscores );
    }
}

void KAstTopLevel::slotRockHit( int size )
{
    switch ( size )
    {
	case 0:
	    score += 10;
	     break;

	case 1:
	    score += 20;
	    break;

	default:
	    score += 40;
      }

    playSound( "RockDestroyed" );

    scoreLCD->display( score );
}

void KAstTopLevel::slotRocksRemoved()
{
    level++;

    if ( level >= MAX_LEVELS )
	level = MAX_LEVELS - 1;

    view->setRockSpeed( levels[level-1].rockSpeed );
    view->addRocks( levels[level-1].nrocks );

    levelLCD->display( level+1 );
}

void KAstTopLevel::doStats()
{
    QString r( "0.00" );
    if ( view->shots() )
	 r = QString::number( (double)view->hits() / view->shots() * 100.0,
			     'g', 2 );

/* multi-line text broken in Qt 3
    QString s = tr( "Game Over\n\nShots fired:\t%1\n  Hit:\t%2\n  Missed:\t%3\nHit ratio:\t%4 %\n\nPress N for a new game" )
      .arg(view->shots()).arg(view->hits())
      .arg(view->shots() - view->hits())
      .arg(r);
*/

    view->showText( "Game Over.   Press N for a new game.", yellow, FALSE );
}

void KAstTopLevel::slotUpdateVitals()
{
    brakesLCD->display( view->brakeCount() );
    shieldLCD->display( view->shieldCount() );
    shootLCD->display( view->shootCount() );
//    teleportsLCD->display( view->teleportCount() );
    powerMeter->setValue( view->power() );
}
