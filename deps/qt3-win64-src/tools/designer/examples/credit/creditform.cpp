#include <qradiobutton.h>
#include <qspinbox.h>
#include "creditform.h"

CreditForm::CreditForm( QWidget* parent, const char* name, 
			bool modal, WFlags fl )
    : CreditFormBase( parent, name, modal, fl ) 
{  
    setAmount(); 
}

CreditForm::~CreditForm() { /* NOOP */ }

void CreditForm::setAmount() 
{
    if ( stdRadioButton->isChecked() )
	amountSpinBox->setValue( amountSpinBox->maxValue() / 2 );
    else if ( noneRadioButton->isChecked() )
	amountSpinBox->setValue( amountSpinBox->minValue() );
}
