/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#include <qpainter.h>
#include "ledmeter.h"

KALedMeter::KALedMeter( QWidget *parent ) : QFrame( parent )
{
    mCRanges.setAutoDelete( TRUE );
    mRange = 100;
    mCount = 20;
    mCurrentCount = 0;
    mValue = 0;
    setMinimumWidth( mCount * 2 + frameWidth() );
}

void KALedMeter::setRange( int r )
{
    mRange = r;
    if ( mRange < 1 )
        mRange = 1;
    setValue( mValue );
    update();
}

void KALedMeter::setCount( int c )
{
    mCount = c;
    if ( mCount < 1 )
        mCount = 1;
    setMinimumWidth( mCount * 2 + frameWidth() );
    calcColorRanges();
    setValue( mValue );
    update();
}

void KALedMeter::setValue( int v )
{
    mValue = v;
    if ( mValue > mRange )
        mValue = mRange;
    else if ( mValue < 0 )
        mValue = 0;
    int c = ( mValue + mRange / mCount - 1 ) * mCount / mRange;
    if ( c != mCurrentCount )
    {
        mCurrentCount = c;
        update();
    }
}

void KALedMeter::addColorRange( int pc, const QColor &c )
{
    ColorRange *cr = new ColorRange;
    cr->mPc = pc;
    cr->mColor = c;
    mCRanges.append( cr );
    calcColorRanges();
}

void KALedMeter::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    int w = ( width() - frameWidth() - 2 ) / mCount * mCount;
    w += frameWidth() + 2;
    setFrameRect( QRect( 0, 0, w, height() ) );
}

void KALedMeter::drawContents( QPainter *p )
{
    QRect b = contentsRect();

    unsigned cidx = 0;
    int ncol = mCount;
    QColor col = colorGroup().foreground();
   
    if ( !mCRanges.isEmpty() )
    {
        col = mCRanges.at( cidx )->mColor;
        ncol = mCRanges.at( cidx )->mValue;
    }
    p->setBrush( col );
    p->setPen( col );

    int lw = b.width() / mCount;
    int lx = b.left() + 1;
    for ( int i = 0; i < mCurrentCount; i++, lx += lw )
    {
        if ( i > ncol )
        {
            if ( ++cidx < mCRanges.count() )
            {
                col = mCRanges.at( cidx )->mColor;
                ncol = mCRanges.at( cidx )->mValue;
                p->setBrush( col );
                p->setPen( col );
            }
        }

        p->drawRect( lx, b.top() + 1, lw - 1, b.height() - 2 );
    }
}

void KALedMeter::calcColorRanges()
{
    int prev = 0;
    ColorRange *cr;
    for ( cr = mCRanges.first(); cr; cr = mCRanges.next() )
    {
        cr->mValue = prev + cr->mPc * mCount / 100;
        prev = cr->mValue;
    }
}

