#include <qradiobutton.h>
#include <qspinbox.h>
#include "receiver.h"

void Receiver::setParent( QDialog *parent )
{
    p = parent;
    setAmount();
}

void Receiver::setAmount() 
{
    QSpinBox *amount = 
	(QSpinBox *) p->child( "amountSpinBox", "QSpinBox" );

    QRadioButton *radio = 
	(QRadioButton *) p->child( "stdRadioButton", "QRadioButton" );
    if ( radio && radio->isChecked() ) {
	if ( amount )
	    amount->setValue( amount->maxValue() / 2 );
	return;
    }

    radio = 
	(QRadioButton *) p->child( "noneRadioButton", "QRadioButton" );
    if ( radio && radio->isChecked() )
	if ( amount )
	    amount->setValue( amount->minValue() );
}
