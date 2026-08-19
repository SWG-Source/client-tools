/****************************************************************************
** Form interface generated from reading ui file 'creditformbase.ui'
**
** Created: Fri Aug 10 09:48:10 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CREDITFORMBASE_H
#define CREDITFORMBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QPushButton;
class QRadioButton;
class QSpinBox;

class CreditFormBase : public QDialog
{ 
    Q_OBJECT

public:
    CreditFormBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CreditFormBase();

    QButtonGroup* creditButtonGroup;
    QRadioButton* stdRadioButton;
    QRadioButton* noneRadioButton;
    QRadioButton* specialRadioButton;
    QSpinBox* amountSpinBox;
    QPushButton* okPushButton;
    QPushButton* cancelPushButton;


public slots:
    virtual void setAmount();

protected:
    QVBoxLayout* CreditFormBaseLayout;
    QVBoxLayout* creditButtonGroupLayout;
    QHBoxLayout* Layout4;
};

#endif // CREDITFORMBASE_H
