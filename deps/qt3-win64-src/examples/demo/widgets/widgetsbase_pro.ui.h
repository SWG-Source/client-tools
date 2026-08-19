#include <qobjectlist.h>

void WidgetsBase::init()
{
	timeEdit->setTime( QTime::currentTime() );
    	dateEdit->setDate( QDate::currentDate() );
}

void WidgetsBase::destroy()
{

}

void WidgetsBase::resetColors()
{
    groupBox->setPalette( palette(), FALSE );
    if(QObjectList *chldn = groupBox->queryList()) {
	for(QObject *obj=chldn->first(); obj; obj = chldn->next()) {
	    if(obj->isWidgetType()) {
		QWidget *w = (QWidget *)obj;
		if(!w->isTopLevel())
		    w->setPalette(palette(), FALSE);
	    }
	}
    }
}

void WidgetsBase::setColor( const QString & color )
{
    groupBox->setPalette( QColor( color ), FALSE );
    if(QObjectList *chldn = groupBox->queryList()) {
	for(QObject *obj=chldn->first(); obj; obj = chldn->next()) {
	    if(obj->isWidgetType()) {
		QWidget *w = (QWidget *)obj;
		if(!w->isTopLevel())
		    w->setPalette(QColor(color), FALSE);
	    }
	}
    }
}

void WidgetsBase::setColor()
{
	setColor( lineEdit->text() );
}

void WidgetsBase::updateClock()
{
	clock->setTime( timeEdit->time() );
}

void WidgetsBase::updateColorTest( const QString & color )
{
	colorTest->setPalette( QColor( color ) );
}

void WidgetsBase::updateDateTimeString()
{
	QDateTime dt;
    	dt.setDate( dateEdit->date() );
    	dt.setTime( timeEdit->time() );
    	dateTimeLabel->setText( dt.toString() );
}

