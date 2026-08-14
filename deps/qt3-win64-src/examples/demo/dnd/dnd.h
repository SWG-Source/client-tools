#include <qpixmap.h>
#include <qmap.h>
#include "dndbase.h"

#ifndef DNDDEMO_H
#define DNDDEMO_H

class IconItem
{
public:
    IconItem( const QString& name = QString::null, const QString& icon = QString::null );

    QString name() { return _name; }
    QPixmap *pixmap() { return &_pixmap; }

    Q_DUMMY_COMPARISON_OPERATOR( IconItem )

protected:
    QPixmap loadPixmap( const QString& name );

private:
    QString _name;
    QPixmap _pixmap;
};

class DnDDemo : public DnDDemoBase
{
    Q_OBJECT

public:
    DnDDemo( QWidget* parent = 0, const char* name = 0 );
    ~DnDDemo();

    IconItem findItem( const QString& tag );

private:
    QMap<QString,IconItem> items;
};

#endif
