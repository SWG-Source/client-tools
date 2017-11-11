/****************************************************************************
** $Id: qt/qdragobject.h   3.3.4   edited May 27 2003 $
**
** Definition of QDragObject
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QDRAGOBJECT_H
#define QDRAGOBJECT_H

class QWidget;
class QTextDragPrivate;
class QDragObjectData;
class QStoredDragData;
class QImageDragData;

#ifndef QT_H
#include "qobject.h"
#include "qimage.h"
#include "qstrlist.h"
#include "qcolor.h"
#endif // QT_H

#ifndef QT_NO_MIME

class Q_EXPORT QDragObject: public QObject, public QMimeSource {
    Q_OBJECT
public:
    QDragObject( QWidget * dragSource = 0, const char * name = 0 );
    virtual ~QDragObject();

#ifndef QT_NO_DRAGANDDROP
    bool drag();
    bool dragMove();
    void dragCopy();
    void dragLink();

    virtual void setPixmap(QPixmap);
    virtual void setPixmap(QPixmap, const QPoint& hotspot);
    QPixmap pixmap() const;
    QPoint pixmapHotSpot() const;
#endif

    QWidget * source();
    static QWidget * target();

    static void setTarget(QWidget*);

#ifndef QT_NO_DRAGANDDROP
    enum DragMode { DragDefault, DragCopy, DragMove, DragLink, DragCopyOrMove };

protected:
    virtual bool drag(DragMode);
#endif

private:
    QDragObjectData * d;
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QDragObject( const QDragObject & );
    QDragObject &operator=( const QDragObject & );
#endif
};

class Q_EXPORT QStoredDrag: public QDragObject {
    Q_OBJECT
    QStoredDragData * d;

public:
    QStoredDrag( const char * mimeType,
		 QWidget * dragSource = 0, const char * name = 0 );
    ~QStoredDrag();

    virtual void setEncodedData( const QByteArray & );

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QStoredDrag( const QStoredDrag & );
    QStoredDrag &operator=( const QStoredDrag & );
#endif
};

class Q_EXPORT QTextDrag: public QDragObject {
    Q_OBJECT
    QTextDragPrivate* d;
public:
    QTextDrag( const QString &,
	       QWidget * dragSource = 0, const char * name = 0 );
    QTextDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QTextDrag();

    virtual void setText( const QString &);
    virtual void setSubtype( const QCString &);

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode( const QMimeSource* e );
    static bool decode( const QMimeSource* e, QString& s );
    static bool decode( const QMimeSource* e, QString& s, QCString& subtype );

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QTextDrag( const QTextDrag & );
    QTextDrag &operator=( const QTextDrag & );
#endif
};

class Q_EXPORT QImageDrag: public QDragObject {
    Q_OBJECT
    QImage img;
    QStrList ofmts;
    QImageDragData* d;

public:
    QImageDrag( QImage image, QWidget * dragSource = 0, const char * name = 0 );
    QImageDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QImageDrag();

    virtual void setImage( QImage image );

    const char * format(int i) const;
    virtual QByteArray encodedData(const char*) const;

    static bool canDecode( const QMimeSource* e );
    static bool decode( const QMimeSource* e, QImage& i );
    static bool decode( const QMimeSource* e, QPixmap& i );

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QImageDrag( const QImageDrag & );
    QImageDrag &operator=( const QImageDrag & );
#endif
};


class Q_EXPORT QUriDrag: public QStoredDrag {
    Q_OBJECT

public:
    QUriDrag( QStrList uris, QWidget * dragSource = 0, const char * name = 0 );
    QUriDrag( QWidget * dragSource = 0, const char * name = 0 );
    ~QUriDrag();

    void setFilenames( const QStringList & fnames ) { setFileNames( fnames ); }
    void setFileNames( const QStringList & fnames );
    void setUnicodeUris( const QStringList & uuris );
    virtual void setUris( QStrList uris );

    static QString uriToLocalFile(const char*);
    static QCString localFileToUri(const QString&);
    static QString uriToUnicodeUri(const char*);
    static QCString unicodeUriToUri(const QString&);
    static bool canDecode( const QMimeSource* e );
    static bool decode( const QMimeSource* e, QStrList& i );
    static bool decodeToUnicodeUris( const QMimeSource* e, QStringList& i );
    static bool decodeLocalFiles( const QMimeSource* e, QStringList& i );

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QUriDrag( const QUriDrag & );
    QUriDrag &operator=( const QUriDrag & );
#endif
};

class Q_EXPORT QColorDrag : public QStoredDrag
{
    Q_OBJECT
    QColor color;

public:
    QColorDrag( const QColor &col, QWidget *dragsource = 0, const char *name = 0 );
    QColorDrag( QWidget * dragSource = 0, const char * name = 0 );
    void setColor( const QColor &col );

    static bool canDecode( QMimeSource * );
    static bool decode( QMimeSource *, QColor &col );

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QColorDrag( const QColorDrag & );
    QColorDrag &operator=( const QColorDrag & );
#endif
};

#ifndef QT_NO_COMPAT
typedef QUriDrag QUrlDrag;
#endif

#ifndef QT_NO_DRAGANDDROP

// QDragManager is not part of the public API.  It is defined in a
// header file simply so different .cpp files can implement different
// member functions.
//

class Q_EXPORT QDragManager: public QObject {
    Q_OBJECT

private:
    QDragManager();
    ~QDragManager();
    // only friend classes can use QDragManager.
    friend class QDragObject;
    friend class QDragMoveEvent;
    friend class QDropEvent;
    friend class QApplication;

    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent* );

    bool drag( QDragObject *, QDragObject::DragMode );

    void cancel( bool deleteSource = TRUE );
    void move( const QPoint & );
    void drop();
    void updatePixmap();

private:
    QDragObject * object;
    void updateMode( ButtonState newstate );
    void updateCursor();

    QWidget * dragSource;
    QWidget * dropWidget;
    bool beingCancelled;
    bool restoreCursor;
    bool willDrop;

    QPixmap *pm_cursor;
    int n_cursor;
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QDragManager( const QDragManager & );
    QDragManager &operator=( const QDragManager & );
#endif
};

#endif

#endif // QT_NO_MIME

#endif // QDRAGOBJECT_H
