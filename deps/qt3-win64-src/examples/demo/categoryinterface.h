#ifndef CATEGORYINTERFACE_H
#define CATEGORYINTERFACE_H

#include <qstring.h>
#include <qiconset.h>
#include <qobject.h>

class QWidgetStack;

class CategoryInterface : public QObject
{
    Q_OBJECT

public:
    CategoryInterface( QWidgetStack *s ) : stack( s ) {}
    virtual ~CategoryInterface() {}
    virtual QString name() const = 0;
    virtual QIconSet icon() const = 0;
    virtual int numCategories() const = 0;
    virtual QString categoryName( int i ) const = 0;
    virtual QIconSet categoryIcon( int i ) const = 0;
    virtual int categoryOffset() const = 0;

public slots:
    virtual void setCurrentCategory( int i ) = 0;

protected:
    QWidgetStack *stack;

};

#endif
