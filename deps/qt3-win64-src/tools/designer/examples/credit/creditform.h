#include "creditformbase.h"

class CreditForm : public CreditFormBase
{
    Q_OBJECT
public:
    CreditForm( QWidget* parent = 0, const char* name = 0, 
		bool modal = FALSE, WFlags fl = 0 );
    ~CreditForm();
public slots:
    void setAmount();
};
