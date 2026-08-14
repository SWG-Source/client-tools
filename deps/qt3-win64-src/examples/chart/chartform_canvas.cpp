#include "canvastext.h"
#include "chartform.h"

#include <qbrush.h>
#include <qcanvas.h>

#include <math.h> // sin, cos

#ifndef M_PI
#define M_PI 3.1415
#endif

void ChartForm::drawElements()
{
    QCanvasItemList list = m_canvas->allItems();
    for ( QCanvasItemList::iterator it = list.begin(); it != list.end(); ++it )
	delete *it;

	// 360 * 16 for pies; Qt works with 16ths of degrees
    int scaleFactor = m_chartType == PIE ? 5760 :
			m_chartType == VERTICAL_BAR ? m_canvas->height() :
			    m_canvas->width();
    double biggest = 0.0;
    int count = 0;
    double total = 0.0;
    static double scales[MAX_ELEMENTS];

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    double value = m_elements[i].value();
	    count++;
	    total += value;
	    if ( value > biggest )
		biggest = value;
	    scales[i] = m_elements[i].value() * scaleFactor;
	}
    }

    if ( count ) {
	    // 2nd loop because of total and biggest
	for ( int i = 0; i < MAX_ELEMENTS; ++i )
	    if ( m_elements[i].isValid() )
		if ( m_chartType == PIE )
		    scales[i] = (m_elements[i].value() * scaleFactor) / total;
		else
		    scales[i] = (m_elements[i].value() * scaleFactor) / biggest;

	switch ( m_chartType ) {
	    case PIE:
		drawPieChart( scales, total, count );
		break;
	    case VERTICAL_BAR:
		drawVerticalBarChart( scales, total, count );
		break;
	    case HORIZONTAL_BAR:
		drawHorizontalBarChart( scales, total, count );
		break;
	}
    }

    m_canvas->update();
}


void ChartForm::drawPieChart( const double scales[], double total, int )
{
    double width = m_canvas->width();
    double height = m_canvas->height();
    int size = int(width > height ? height : width);
    int x = int(width / 2);
    int y = int(height / 2);
    int angle = 0;

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasEllipse *arc = new QCanvasEllipse(
					    size, size, angle, extent, m_canvas );
	    arc->setX( x );
	    arc->setY( y );
	    arc->setZ( 0 );
	    arc->setBrush( QBrush( m_elements[i].valueColor(),
				   BrushStyle(m_elements[i].valuePattern()) ) );
	    arc->show();
	    angle += extent;
	    QString label = m_elements[i].label();
	    if ( !label.isEmpty() || m_addValues != NO ) {
		label = valueLabel( label, m_elements[i].value(), total );
		CanvasText *text = new CanvasText( i, label, m_font, m_canvas );
		double proX = m_elements[i].proX( PIE );
		double proY = m_elements[i].proY( PIE );
		if ( proX < 0 || proY < 0 ) {
		    // Find the centre of the pie segment
		    QRect rect = arc->boundingRect();
		    proX = ( rect.width() / 2 ) + rect.x();
		    proY = ( rect.height() / 2 ) + rect.y();
		    // Centre text over the centre of the pie segment
		    rect = text->boundingRect();
		    proX -= ( rect.width() / 2 );
		    proY -= ( rect.height() / 2 );
		    // Make proportional
		    proX /= width;
		    proY /= height;
		}
		text->setColor( m_elements[i].labelColor() );
		text->setX( proX * width );
		text->setY( proY * height );
		text->setZ( 1 );
		text->show();
		m_elements[i].setProX( PIE, proX );
		m_elements[i].setProY( PIE, proY );
	    }
	}
    }
}


void ChartForm::drawVerticalBarChart(
	const double scales[], double total, int count )
{
    double width = m_canvas->width();
    double height = m_canvas->height();
    int prowidth = int(width / count);
    int x = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    int y = int(height - extent);
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    x, y, prowidth, extent, m_canvas );
	    rect->setBrush( QBrush( m_elements[i].valueColor(),
				    BrushStyle(m_elements[i].valuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = m_elements[i].label();
	    if ( !label.isEmpty() || m_addValues != NO ) {
		double proX = m_elements[i].proX( VERTICAL_BAR );
		double proY = m_elements[i].proY( VERTICAL_BAR );
		if ( proX < 0 || proY < 0 ) {
		    proX = x / width;
		    proY = y / height;
		}
		label = valueLabel( label, m_elements[i].value(), total );
		CanvasText *text = new CanvasText( i, label, m_font, m_canvas );
		text->setColor( m_elements[i].labelColor() );
		text->setX( proX * width );
		text->setY( proY * height );
		text->setZ( 1 );
		text->show();
		m_elements[i].setProX( VERTICAL_BAR, proX );
		m_elements[i].setProY( VERTICAL_BAR, proY );
	    }
	    x += prowidth;
	}
    }
}


void ChartForm::drawHorizontalBarChart(
	const double scales[], double total, int count )
{
    double width = m_canvas->width();
    double height = m_canvas->height();
    int proheight = int(height / count);
    int y = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( m_elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    0, y, extent, proheight, m_canvas );
	    rect->setBrush( QBrush( m_elements[i].valueColor(),
				    BrushStyle(m_elements[i].valuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = m_elements[i].label();
	    if ( !label.isEmpty() || m_addValues != NO ) {
		double proX = m_elements[i].proX( HORIZONTAL_BAR );
		double proY = m_elements[i].proY( HORIZONTAL_BAR );
		if ( proX < 0 || proY < 0 ) {
		    proX = 0;
		    proY = y / height;
		}
		label = valueLabel( label, m_elements[i].value(), total );
		CanvasText *text = new CanvasText( i, label, m_font, m_canvas );
		text->setColor( m_elements[i].labelColor() );
		text->setX( proX * width );
		text->setY( proY * height );
		text->setZ( 1 );
		text->show();
		m_elements[i].setProX( HORIZONTAL_BAR, proX );
		m_elements[i].setProY( HORIZONTAL_BAR, proY );
	    }
	    y += proheight;
	}
    }
}


QString ChartForm::valueLabel(
	    const QString& label, double value, double total )
{
    if ( m_addValues == NO )
	return label;

    QString newLabel = label;
    if ( !label.isEmpty() )
	if ( m_chartType == VERTICAL_BAR )
	    newLabel += '\n';
	else
	    newLabel += ' ';
    if ( m_addValues == YES )
	newLabel += QString::number( value, 'f', m_decimalPlaces );
    else if ( m_addValues == AS_PERCENTAGE )
	newLabel += QString::number( (value / total) * 100, 'f', m_decimalPlaces )
		    + '%';
    return newLabel;
}

