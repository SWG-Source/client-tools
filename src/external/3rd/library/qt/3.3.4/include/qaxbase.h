/****************************************************************************
** $Id: qt/qaxbase.h   3.3.4   edited Nov 17 2003 $
**
** Declaration of the QAxBase class
**
** Copyright (C) 2001-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef UNICODE
#define UNICODE
#endif

#ifndef QAXBASE_H
#define QAXBASE_H

#include <qvariant.h>
#include <qobject.h>

struct IUnknown;
struct QUuid;
class QAxEventSink;
class QAxObject;
class QAxBasePrivate;

class QAxBase
{
#ifdef Q_QDOC
#error "The Symbol Q_QDOC is reserved for documentation purposes."
    Q_PROPERTY( QString control READ control WRITE setControl )
#endif
public:
#ifndef Q_QDOC
    typedef QMap<QCString, QVariant> PropertyBag;
#endif

    QAxBase( IUnknown *iface = 0 );
    virtual ~QAxBase();

    QString control() const;

    long queryInterface( const QUuid &, void** ) const;

    QVariant dynamicCall( const QCString&, const QVariant &v1 = QVariant(), 
					   const QVariant &v2 = QVariant(),
					   const QVariant &v3 = QVariant(),
					   const QVariant &v4 = QVariant(),
					   const QVariant &v5 = QVariant(),
					   const QVariant &v6 = QVariant(),
					   const QVariant &v7 = QVariant(),
					   const QVariant &v8 = QVariant() );
    QVariant dynamicCall( const QCString&, QValueList<QVariant> &vars );
    QAxObject *querySubObject( const QCString &name, const QVariant &v1 = QVariant(),
					    const QVariant &v2 = QVariant(),
					    const QVariant &v3 = QVariant(),
					    const QVariant &v4 = QVariant(),
					    const QVariant &v5 = QVariant(),
					    const QVariant &v6 = QVariant(),
					    const QVariant &v7 = QVariant(),
					    const QVariant &v8 = QVariant() );

    virtual QMetaObject *metaObject() const;
    virtual bool qt_invoke( int, QUObject* );
    virtual bool qt_property( int, int, QVariant* );
    virtual bool qt_emit( int, QUObject* ) = 0;
    virtual const char *className() const = 0;
    virtual QObject *qObject() = 0;

    PropertyBag propertyBag() const;
    void setPropertyBag( const PropertyBag& );

    QString generateDocumentation();

    virtual bool propertyWritable( const char* ) const;
    virtual void setPropertyWritable( const char*, bool );

    bool isNull() const;

    QVariant asVariant() const;

#ifdef Q_QDOC
#error "The Symbol Q_QDOC is reserved for documentation purposes."
    enum PropertyBag {};
signals:
    void signal(const QString&,int,void*);
    void propertyChanged(const QString&);
    void exception(int,const QString&,const QString&,const QString&);
#endif

public:
    virtual void clear();
    bool setControl( const QString& );

    void disableMetaObject();
    void disableClassInfo();
    void disableEventSink();

protected:
    virtual bool initialize( IUnknown** ptr );
    bool initializeRemote(IUnknown** ptr);
    bool initializeLicensed(IUnknown** ptr);
    bool initializeActive(IUnknown** ptr);

private:
    bool initializeLicensedHelper(void *factory, const QString &key, IUnknown **ptr);
    QAxBasePrivate *d;

    static QMetaObject *staticMetaObject() { return 0; }
    virtual QMetaObject *parentMetaObject() const = 0;
    bool internalInvoke( const QCString &name, void *out, QVariant var[], QCString &type );

    QString ctrl;
};

inline QString QAxBase::generateDocumentation()
{
    extern QString qax_generateDocumentation(QAxBase *, QAxBasePrivate *);
    return qax_generateDocumentation(this, d);
}

#ifndef QT_NO_DATASTREAM
inline QDataStream &operator >>( QDataStream &s, QAxBase &c )
{
    QAxBase::PropertyBag bag;
    c.qObject()->blockSignals( TRUE );
    QString control;
    s >> control;
    c.setControl( control );
    s >> bag;
    c.setPropertyBag( bag );
    c.qObject()->blockSignals( FALSE );

    return s;
}

inline QDataStream &operator <<( QDataStream &s, const QAxBase &c )
{
    QAxBase::PropertyBag bag = c.propertyBag();
    s << c.control();
    s << bag;

    return s;
}
#endif // QT_NO_DATASTREAM

#endif // QAXBASE_H
