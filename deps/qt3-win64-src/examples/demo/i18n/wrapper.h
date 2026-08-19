#ifndef WRAPPER_H
#define WRAPPER_H

#include <qvbox.h>
#include <qtranslator.h>


class Wrapper : public QVBox
{
public:
    Wrapper(QWidget *parent, int i, const char *name = 0)
	: QVBox(parent, name, WDestructiveClose), translator(this), id(i)
    {
    }

    QTranslator translator;
    int id;
};


#endif // WRAPPER_H
