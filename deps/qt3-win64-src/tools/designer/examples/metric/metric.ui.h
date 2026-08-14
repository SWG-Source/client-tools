/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions respectively slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qvalidator.h>

void ConversionForm::init()
{
    numberLineEdit->setValidator( new QDoubleValidator( numberLineEdit ) );
    numberLineEdit->setText( "10" );
    convert();
    numberLineEdit->selectAll();
}

void ConversionForm::convert()
{
    enum MetricUnits {
	Kilometers,
	Meters,
	Centimeters,
	Millimeters
    };
    enum OldUnits {
	Miles,
	Yards,
	Feet,
	Inches
    };

    // Retrieve the input
    double input = numberLineEdit->text().toDouble();
    double scaledInput = input;

    // internally convert the input to millimeters
    switch ( fromComboBox->currentItem() ) {
    case Kilometers:
	scaledInput *= 1000000;
	break;
    case Meters:
	scaledInput *= 1000;
	break;
    case Centimeters:
	scaledInput *= 10;
	break;
    }

    //convert to inches
    double result = scaledInput * 0.0393701;

    switch ( toComboBox->currentItem() ) {
    case Miles:
	result /= 63360;
	break;
    case Yards:
	result /= 36;
	break;
    case Feet:
	result /= 12;
	break;
    }

    // set the result
    int decimals = decimalsSpinBox->value();
    resultLineEdit->setText( QString::number( result, 'f', decimals ) );
    numberLineEdit->setText( QString::number( input, 'f', decimals ) );
}

