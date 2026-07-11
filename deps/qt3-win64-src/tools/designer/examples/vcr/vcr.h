#ifndef VCR_H
#define VCR_H
#include <qwidget.h>

class Vcr : public QWidget
{
    Q_OBJECT
public:
    Vcr( QWidget *parent = 0, const char *name = 0 );
    ~Vcr() {}
signals:
    void rewind();
    void play();
    void next();
    void stop();
};
#endif
