#include <iostream>
#include <qobject.h>
#include "myobject.h"

MyObject::MyObject()
    : QObject()
{
    std::cout << "MyObject::MyObject()\n";
}

MyObject::~MyObject()
{
    qDebug("MyObject::~MyObject()");
}
