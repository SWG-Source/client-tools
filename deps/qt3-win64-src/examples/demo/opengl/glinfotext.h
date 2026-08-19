#include <qlayout.h>
#include <qtextview.h>
#include <qfont.h>
#include "glinfo.h"

class GLInfoText : public QWidget
{
public:
    GLInfoText(QWidget *parent) 
	: QWidget(parent)
    {
	view = new QTextView(this);
	view->setFont(QFont("courier", 10));
	view->setWordWrap(QTextEdit::NoWrap);
	QHBoxLayout *l = new QHBoxLayout(this);
	l->addWidget(view);
	GLInfo info;
	view->setText(info.info());
    }
    
private:
    QTextView *view;
};
