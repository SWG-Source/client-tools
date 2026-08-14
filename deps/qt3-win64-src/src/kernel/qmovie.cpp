/****************************************************************************
** $Id: qmovie.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of movie classes
**
** Created : 970617
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
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

// #define QT_SAVE_MOVIE_HACK

#include "qtimer.h"
#include "qpainter.h"
#include "qptrlist.h"
#include "qbitmap.h"
#include "qmovie.h"
#include "qfile.h"
#include "qbuffer.h"
#include "qobject.h"
#include "qpixmapcache.h"

#ifndef QT_NO_MOVIE

#ifdef Q_WS_QWS
#include "qgfx_qws.h"
#endif

#include "qasyncio.h"
#include "qasyncimageio.h"

#include <stdlib.h>

/*!
    \class QMovie qmovie.h
    \brief The QMovie class provides incremental loading of animations or images, signalling as it progresses.

    \ingroup images
    \ingroup graphics
    \ingroup multimedia
    \mainclass

    The simplest way to display a QMovie is to use a QLabel and
    QLabel::setMovie().

    A QMovie provides a QPixmap as the framePixmap(); connections can
    be made via connectResize() and connectUpdate() to receive
    notification of size and pixmap changes. All decoding is driven
    by the normal event-processing mechanisms.

    The movie begins playing as soon as the QMovie is created
    (actually, once control returns to the event loop). When the last
    frame in the movie has been played, it may loop back to the start
    if such looping is defined in the input source.

    QMovie objects are explicitly shared. This means that a QMovie
    copied from another QMovie will be displaying the same frame at
    all times. If one shared movie pauses, all pause. To make \e
    independent movies, they must be constructed separately.

    The set of data formats supported by QMovie is determined by the
    decoder factories that have been installed; the format of the
    input is determined as the input is decoded.

    The supported formats are MNG (if Qt is configured with MNG
    support enabled) and GIF (if Qt is configured with GIF support
    enabled, see qgif.h).

    If Qt is configured to support GIF reading, we are required to
    state that "The Graphics Interchange Format(c) is the Copyright
    property of CompuServe Incorporated. GIF(sm) is a Service Mark
    property of CompuServe Incorporated.

    \warning If you are in a country that recognizes software patents
    and in which Unisys holds a patent on LZW compression and/or
    decompression and you want to use GIF, Unisys may require you to
    license that technology. Such countries include Canada, Japan,
    the USA, France, Germany, Italy and the UK.

    GIF support may be removed completely in a future version of Qt.
    We recommend using the MNG or PNG format.

    \img qmovie.png QMovie

    \sa QLabel::setMovie()
*/

/*!
    \enum QMovie::Status

    \value SourceEmpty
    \value UnrecognizedFormat
    \value Paused
    \value EndOfFrame
    \value EndOfLoop
    \value EndOfMovie
    \value SpeedChanged
*/

class QMoviePrivate : public QObject, public QShared,
		      private QDataSink, private QImageConsumer
{
    Q_OBJECT

public: // for QMovie

    // Creates a null Private
    QMoviePrivate();

    // NOTE:  The ownership of the QDataSource is transferred to the Private
    QMoviePrivate(QDataSource* src, QMovie* movie, int bufsize);

    virtual ~QMoviePrivate();

    bool isNull() const;

    // Initialize, possibly to the null state
    void init(bool fully);
    void flushBuffer();
    void updatePixmapFromImage();
    void updatePixmapFromImage(const QPoint& off, const QRect& area);
    void showChanges();

    // This as QImageConsumer
    void changed(const QRect& rect);
    void end();
    void preFrameDone(); //util func
    void frameDone();
    void frameDone(const QPoint&, const QRect& rect);
    void restartTimer();
    void setLooping(int l);
    void setFramePeriod(int milliseconds);
    void setSize(int w, int h);

    // This as QDataSink
    int readyToReceive();
    void receive(const uchar* b, int bytecount);
    void eof();
    void pause();

signals:
    void sizeChanged(const QSize&);
    void areaChanged(const QRect&);
    void dataStatus(int);

public slots:
    void refresh();

public:
    QMovie *that;
    QWidget * display_widget;

    QImageDecoder *decoder;

    // Cyclic buffer
    int buf_size;
    uchar *buffer;
    int buf_r, buf_w, buf_usage;

    int framenumber;
    int frameperiod;
    int speed;
    QTimer *frametimer;
    int lasttimerinterval;
    int loop;
    bool movie_ended;
    bool dirty_cache;
    bool waitingForFrameTick;
    int stepping;
    QRect changed_area;
    QRect valid_area;
    QDataPump *pump;
    QDataSource *source;
    QPixmap mypixmap;
    QBitmap mymask;
    QColor bg;

    int error;
    bool empty;

#ifdef QT_SAVE_MOVIE_HACK
  bool save_image;
  int image_number;
#endif
};


QMoviePrivate::QMoviePrivate()
{
    dirty_cache = FALSE;
    buffer = 0;
    pump = 0;
    source = 0;
    decoder = 0;
    display_widget=0;
    buf_size = 0;
    init(FALSE);
}

// NOTE:  The ownership of the QDataSource is transferred to the Private
QMoviePrivate::QMoviePrivate(QDataSource* src, QMovie* movie, int bufsize) :
    that(movie),
    buf_size(bufsize)
{
    frametimer = new QTimer(this);
    pump = src ? new QDataPump(src, this) : 0;
    QObject::connect(frametimer, SIGNAL(timeout()), this, SLOT(refresh()));
    dirty_cache = FALSE;
    source = src;
    buffer = 0;
    decoder = 0;
    speed = 100;
    display_widget=0;
    init(TRUE);
}

QMoviePrivate::~QMoviePrivate()
{
    if ( buffer )				// Avoid purify complaint
	delete [] buffer;
    delete pump;
    delete decoder;
    delete source;

    // Too bad.. but better be safe than sorry
    if ( dirty_cache )
        QPixmapCache::clear();
}

bool QMoviePrivate::isNull() const
{
    return !buf_size;
}

// Initialize. Only actually allocate any space if \a fully is TRUE,
// otherwise, just enough to be a valid null Private.
void QMoviePrivate::init(bool fully)
{
#ifdef QT_SAVE_MOVIE_HACK
    save_image = TRUE;
    image_number = 0;
#endif

    buf_usage = buf_r = buf_w = 0;
    if ( buffer )				// Avoid purify complaint
	delete [] buffer;
    buffer = fully ? new uchar[buf_size] : 0;
    if ( buffer )
	memset( buffer, 0, buf_size );

    delete decoder;
    decoder = fully ? new QImageDecoder(this) : 0;

#ifdef AVOID_OPEN_FDS
    if ( source && !source->isOpen() )
	source->open(IO_ReadOnly);
#endif

    waitingForFrameTick = FALSE;
    stepping = -1;
    framenumber = 0;
    frameperiod = -1;
    if (fully) frametimer->stop();
    lasttimerinterval = -1;
    changed_area.setRect(0,0,-1,-1);
    valid_area = changed_area;
    loop = -1;
    movie_ended = FALSE;
    error = 0;
    empty = TRUE;
}

void QMoviePrivate::flushBuffer()
{
    int used;
    while (buf_usage && !waitingForFrameTick && stepping != 0 && !error) {
	used = decoder->decode(buffer + buf_r, QMIN(buf_usage, buf_size - buf_r));
	if (used <= 0) {
	    if ( used < 0 ) {
		error = 1;
		emit dataStatus(QMovie::UnrecognizedFormat);
	    }
	    break;
	}
	buf_r = (buf_r + used) % buf_size;
	buf_usage -= used;
    }

    // Some formats, like MNG, can make stuff happen without any extra data.
    // Only do this if the movie hasn't ended, however or we'll never get the end of loop signal.
    if (!movie_ended) {
	used = decoder->decode(buffer + buf_r, 0);
	if (used <= 0) {
	    if ( used < 0 ) {
		error = 1;
		emit dataStatus(QMovie::UnrecognizedFormat);
	    }
	}
    }

    if (error)
	frametimer->stop();
    maybeReady();
}

void QMoviePrivate::updatePixmapFromImage()
{
    if (changed_area.isEmpty()) return;
    updatePixmapFromImage(QPoint(0,0),changed_area);
}

void QMoviePrivate::updatePixmapFromImage(const QPoint& off,
						const QRect& area)
{
    // Create temporary QImage to hold the part we want
    const QImage& gimg = decoder->image();
    QImage img = gimg.copy(area);

#ifdef QT_SAVE_MOVIE_HACK
    if ( save_image ) {
	QString name;
	name.sprintf("movie%i.ppm",image_number++);
	gimg.save( name, "PPM" );
    }
#endif

    // Resize to size of image
    if (mypixmap.width() != gimg.width() || mypixmap.height() != gimg.height())
	mypixmap.resize(gimg.width(), gimg.height());

    // Convert to pixmap and paste that onto myself
    QPixmap lines;

#ifndef QT_NO_SPRINTF
    if (!(frameperiod < 0 && loop == -1)) {
        // its an animation, lets see if we converted
        // this frame already.
        QString key;
        key.sprintf( "%08lx:%04d", ( long )this, framenumber );
        if ( !QPixmapCache::find( key, lines ) ) {
            lines.convertFromImage(img, Qt::ColorOnly);
            QPixmapCache::insert( key, lines );
            dirty_cache = TRUE;
        }
    } else
#endif
    {
        lines.convertFromImage(img, Qt::ColorOnly);
    }

    if (bg.isValid()) {
	QPainter p;
	p.begin(&mypixmap);
	p.fillRect(area, bg);
	p.drawPixmap(area, lines);
	p.end();
    } else {
	if (gimg.hasAlphaBuffer()) {
	    // Resize to size of image
	    if (mymask.isNull()) {
		mymask.resize(gimg.width(), gimg.height());
		mymask.fill( Qt::color1 );
	    }
	}
	mypixmap.setMask(QBitmap()); // Remove reference to my mask
	copyBlt( &mypixmap, area.left(), area.top(),
		 &lines, off.x(), off.y(), area.width(), area.height() );
    }

#ifdef Q_WS_QWS
    if(display_widget) {
	QGfx * mygfx=display_widget->graphicsContext();
	if(mygfx) {
	    double xscale,yscale;
	    xscale=display_widget->width();
	    yscale=display_widget->height();
	    xscale=xscale/((double)mypixmap.width());
	    yscale=yscale/((double)mypixmap.height());
	    double xh,yh;
	    xh=xscale*((double)area.left());
	    yh=yscale*((double)area.top());
	    mygfx->setSource(&mypixmap);
	    mygfx->setAlphaType(QGfx::IgnoreAlpha);
	    mygfx->stretchBlt(0,0,display_widget->width(),
			      display_widget->height(),mypixmap.width(),
			      mypixmap.height());
	    delete mygfx;
	}
    }
#endif
}

void QMoviePrivate::showChanges()
{
    if (changed_area.isValid()) {
	updatePixmapFromImage();

	valid_area = valid_area.unite(changed_area);
	emit areaChanged(changed_area);

	changed_area.setWidth(-1); // make empty
    }
}

// Private as QImageConsumer
void QMoviePrivate::changed(const QRect& rect)
{
    if (!frametimer->isActive())
	frametimer->start(0);
    changed_area = changed_area.unite(rect);
}

void QMoviePrivate::end()
{
    movie_ended = TRUE;
}

void QMoviePrivate::preFrameDone()
{
    if (stepping > 0) {
	stepping--;
	if (!stepping) {
	    frametimer->stop();
	    emit dataStatus( QMovie::Paused );
	}
    } else {
	waitingForFrameTick = TRUE;
	restartTimer();
    }
}
void QMoviePrivate::frameDone()
{
    preFrameDone();
    showChanges();
    emit dataStatus(QMovie::EndOfFrame);
    framenumber++;
}
void QMoviePrivate::frameDone(const QPoint& p,
				const QRect& rect)
{
    preFrameDone();
    const QImage& gimg = decoder->image();
    QPoint point = p - gimg.offset();
    if (framenumber==0)
	emit sizeChanged(gimg.size());
    valid_area = valid_area.unite(QRect(point,rect.size()));
    updatePixmapFromImage(point,rect);
    emit areaChanged(QRect(point,rect.size()));
    emit dataStatus(QMovie::EndOfFrame);
    framenumber++;
}

void QMoviePrivate::restartTimer()
{
    if (speed > 0) {
	int i = frameperiod >= 0 ? frameperiod * 100/speed : 0;
	if ( i != lasttimerinterval || !frametimer->isActive() ) {
	    lasttimerinterval = i;
	    frametimer->start( i );
	}
    } else {
	frametimer->stop();
    }
}

void QMoviePrivate::setLooping(int nloops)
{
    if (loop == -1) { // Only if we don't already know how many loops!
	if (source && source->rewindable()) {
	    source->enableRewind(TRUE);
	    loop = nloops;
	} else {
	    // Cannot loop from this source
	    loop = -2;
	}
    }
}

void QMoviePrivate::setFramePeriod(int milliseconds)
{
    // Animation:  only show complete frame
    frameperiod = milliseconds;
    if (stepping<0 && frameperiod >= 0) restartTimer();
}

void QMoviePrivate::setSize(int w, int h)
{
    if (mypixmap.width() != w || mypixmap.height() != h) {
	mypixmap.resize(w, h);
	emit sizeChanged(QSize(w, h));
    }
}


// Private as QDataSink

int QMoviePrivate::readyToReceive()
{
    // Could pre-fill buffer, but more efficient to just leave the
    // data back at the source.
    return (waitingForFrameTick || !stepping || buf_usage || error)
	? 0 : buf_size;
}

void QMoviePrivate::receive(const uchar* b, int bytecount)
{
    if ( bytecount ) empty = FALSE;

    while (bytecount && !waitingForFrameTick && stepping != 0) {
	int used = decoder->decode(b, bytecount);
	if (used<=0) {
	    if ( used < 0 ) {
		error = 1;
		emit dataStatus(QMovie::UnrecognizedFormat);
	    }
	    break;
	}
	b+=used;
	bytecount-=used;
    }

    // Append unused to buffer
    while (bytecount--) {
	buffer[buf_w] = *b++;
	buf_w = (buf_w+1)%buf_size;
	buf_usage++;
    }
}

void QMoviePrivate::eof()
{
    if ( !movie_ended )
	return;

    if ( empty )
	emit dataStatus(QMovie::SourceEmpty);

#ifdef QT_SAVE_MOVIE_HACK
    save_image = FALSE;
#endif

    emit dataStatus(QMovie::EndOfLoop);

    if (loop >= 0) {
	if (loop) {
	    loop--;
	    if (!loop) return;
	}
	delete decoder;
	decoder = new QImageDecoder(this);
	source->rewind();
	framenumber = 0;
	movie_ended = FALSE;
    } else {
	delete decoder;
	decoder = 0;
	if ( buffer )				// Avoid purify complaint
	    delete [] buffer;
	buffer = 0;
	emit dataStatus(QMovie::EndOfMovie);
#ifdef AVOID_OPEN_FDS
	if ( source )
	    source->close();
#endif
    }
}

void QMoviePrivate::pause()
{
    if ( stepping ) {
	stepping = 0;
	frametimer->stop();
	emit dataStatus( QMovie::Paused );
    }
}

void QMoviePrivate::refresh()
{
    if (!decoder) {
	frametimer->stop();
	return;
    }

    if (frameperiod < 0 && loop == -1) {
	// Only show changes if probably not an animation
	showChanges();
    }

    if (!buf_usage) {
	frametimer->stop();
    }

    waitingForFrameTick = FALSE;
    flushBuffer();
}

///////////////// End of Private /////////////////





/*!
    Constructs a null QMovie. The only interesting thing to do with
    such a movie is to assign another movie to it.

    \sa isNull()
*/
QMovie::QMovie()
{
    d = new QMoviePrivate();
}

/*!
    Constructs a QMovie with an external data source. You should later
    call pushData() to send incoming animation data to the movie.

    The \a bufsize argument sets the maximum amount of data the movie
    will transfer from the data source per event loop. The lower this
    value, the better interleaved the movie playback will be with
    other event processing, but the slower the overall processing will
    be.

    \sa pushData()
*/
QMovie::QMovie(int bufsize)
{
    d = new QMoviePrivate(0, this, bufsize);
}

/*!
    Returns the maximum amount of data that can currently be pushed
    into the movie by a call to pushData(). This is affected by the
    initial buffer size, but varies as the movie plays and data is
    consumed.
*/
int QMovie::pushSpace() const
{
    return d->readyToReceive();
}

/*!
    Pushes \a length bytes from \a data into the movie. \a length must
    be no more than the amount returned by pushSpace() since the
    previous call to pushData().
*/
void QMovie::pushData(const uchar* data, int length)
{
    d->receive(data,length);
}

#ifdef Q_WS_QWS // ##### Temporary performance experiment
/*!
    \internal
*/
void QMovie::setDisplayWidget(QWidget * w)
{
    d->display_widget=w;
}
#endif

/*!
    Constructs a QMovie that reads an image sequence from the given
    data source, \a src. The source must be allocated dynamically,
    because QMovie will take ownership of it and will destroy it when
    the movie is destroyed. The movie starts playing as soon as event
    processing continues.

    The \a bufsize argument sets the maximum amount of data the movie
    will transfer from the data source per event loop. The lower this
    value, the better interleaved the movie playback will be with
    other event processing, but the slower the overall processing will
    be.
*/
QMovie::QMovie(QDataSource* src, int bufsize)
{
    d = new QMoviePrivate(src, this, bufsize);
}

/*!
    Constructs a QMovie that reads an image sequence from the file, \a
    fileName.

    The \a bufsize argument sets the maximum amount of data the movie
    will transfer from the data source per event loop. The lower this
    value, the better interleaved the movie playback will be with
    other event processing, but the slower the overall processing will
    be.
*/
QMovie::QMovie(const QString &fileName, int bufsize)
{
    QFile* file = new QFile(fileName);
    if ( !fileName.isEmpty() )
 	file->open(IO_ReadOnly);
    d = new QMoviePrivate(new QIODeviceSource(file, bufsize), this, bufsize);
}

/*!
    Constructs a QMovie that reads an image sequence from the byte
    array, \a data.

    The \a bufsize argument sets the maximum amount of data the movie
    will transfer from the data source per event loop. The lower this
    value, the better interleaved the movie playback will be with
    other event processing, but the slower the overall processing will
    be.
*/
QMovie::QMovie(QByteArray data, int bufsize)
{
    QBuffer* buffer = new QBuffer(data);
    buffer->open(IO_ReadOnly);
    d = new QMoviePrivate(new QIODeviceSource(buffer, bufsize), this, bufsize);
}

/*!
    Constructs a movie that uses the same data as movie \a movie.
    QMovies use explicit sharing, so operations on the copy will
    affect both.
*/
QMovie::QMovie(const QMovie& movie)
{
    d = movie.d;
    d->ref();
}

/*!
    Destroys the QMovie. If this is the last reference to the data of
    the movie, the data is deallocated.
*/
QMovie::~QMovie()
{
    if (d->deref()) delete d;
}

/*!
    Returns TRUE if the movie is null; otherwise returns FALSE.
*/
bool QMovie::isNull() const
{
    return d->isNull();
}

/*!
    Makes this movie use the same data as movie \a movie. QMovies use
    explicit sharing.
*/
QMovie& QMovie::operator=(const QMovie& movie)
{
    movie.d->ref();
    if (d->deref()) delete d;
    d = movie.d;
    return *this;
}


/*!
    Sets the background color of the pixmap to \a c. If the background
    color isValid(), the pixmap will never have a mask because the
    background color will be used in transparent regions of the image.

    \sa backgroundColor()
*/
void QMovie::setBackgroundColor(const QColor& c)
{
    d->bg = c;
}

/*!
    Returns the background color of the movie set by
    setBackgroundColor().
*/
const QColor& QMovie::backgroundColor() const
{
    return d->bg;
}

/*!
    Returns the area of the pixmap for which pixels have been
    generated.
*/
const QRect& QMovie::getValidRect() const
{
    return d->valid_area;
}

/*!
    Returns the current frame of the movie, as a QPixmap. It is not
    generally useful to keep a copy of this pixmap. It is better to
    keep a copy of the QMovie and get the framePixmap() only when
    needed for drawing.

    \sa frameImage()
*/
const QPixmap& QMovie::framePixmap() const
{
    return d->mypixmap;
}

/*!
    Returns the current frame of the movie, as a QImage. It is not
    generally useful to keep a copy of this image. Also note that you
    must not call this function if the movie is finished(), since by
    then the image will not be available.

    \sa framePixmap()
*/
const QImage& QMovie::frameImage() const
{
    return d->decoder->image();
}

/*!
    Returns the number of steps remaining after a call to step(). If
    the movie is paused, steps() returns 0. If it's running normally
    or is finished, steps() returns a negative number.
*/
int QMovie::steps() const
{
    return d->stepping;
}

/*!
    Returns the number of times EndOfFrame has been emitted since the
    start of the current loop of the movie. Thus, before any
    EndOfFrame has been emitted the value will be 0; within slots
    processing the first signal, frameNumber() will be 1, and so on.
*/
int QMovie::frameNumber() const { return d->framenumber; }

/*!
    Returns TRUE if the image is paused; otherwise returns FALSE.
*/
bool QMovie::paused() const
{
    return d->stepping == 0;
}

/*!
    Returns TRUE if the image is no longer playing: this happens when
    all loops of all frames are complete; otherwise returns FALSE.
*/
bool QMovie::finished() const
{
    return !d->decoder;
}

/*!
    Returns TRUE if the image is not single-stepping, not paused, and
    not finished; otherwise returns FALSE.
*/
bool QMovie::running() const
{
    return d->stepping<0 && d->decoder;
}

/*!
    Pauses the progress of the animation.

    \sa unpause()
*/
void QMovie::pause()
{
    d->pause();
}

/*!
    Unpauses the progress of the animation.

    \sa pause()
*/
void QMovie::unpause()
{
    if ( d->stepping >= 0 )	{
	if (d->isNull())
	    return;
	d->stepping = -1;
	d->restartTimer();
    }
}

/*!
    \overload

    Steps forward, showing \a steps frames, and then pauses.
*/
void QMovie::step(int steps)
{
    if (d->isNull())
	return;
    d->stepping = steps;
    d->frametimer->start(0);
    d->waitingForFrameTick = FALSE; // Full speed ahead!
}

/*!
    Steps forward 1 frame and then pauses.
*/
void QMovie::step()
{
    step(1);
}

/*!
    Rewinds the movie to the beginning. If the movie has not been
    paused, it begins playing again.
*/
void QMovie::restart()
{
    if (d->isNull())
	return;
    if (d->source->rewindable()) {
	d->source->enableRewind(TRUE);
	d->source->rewind();
	int s = d->stepping;
	d->init(TRUE);
	if ( s>0 )
	    step(s);
	else if ( s==0 )
	    pause();
    }
}

/*!
    Returns the movie's play speed as a percentage. The default is 100
    percent.

    \sa setSpeed()
*/
int QMovie::speed() const
{
    return d->speed;
}

/*!
    Sets the movie's play speed as a percentage, to \a percent. This
    is a percentage of the speed dictated by the input data format.
    The default is 100 percent.
*/
void QMovie::setSpeed(int percent)
{
    int oldspeed = d->speed;
    if ( oldspeed != percent && percent >= 0 ) {
	d->speed = percent;
	// Restart timer only if really needed
	if (d->stepping < 0) {
	    if ( !percent || !oldspeed    // To or from zero
		 || oldspeed*4 / percent > 4   // More than 20% slower
		 || percent*4 / oldspeed > 4   // More than 20% faster
		 )
		d->restartTimer();
	}
    }
}

/*!
    Connects the \a{receiver}'s \a member of type \c{void member(const
    QSize&)} so that it is signalled when the movie changes size.

    Note that due to the explicit sharing of QMovie objects, these
    connections persist until they are explicitly disconnected with
    disconnectResize() or until \e every shared copy of the movie is
    deleted.
*/
void QMovie::connectResize(QObject* receiver, const char *member)
{
    QObject::connect(d, SIGNAL(sizeChanged(const QSize&)), receiver, member);
}

/*!
    Disconnects the \a{receiver}'s \a member (or all members if \a
    member is zero) that were previously connected by connectResize().
*/
void QMovie::disconnectResize(QObject* receiver, const char *member)
{
    QObject::disconnect(d, SIGNAL(sizeChanged(const QSize&)), receiver, member);
}

/*!
    Connects the \a{receiver}'s \a member of type \c{void member(const
    QRect&)} so that it is signalled when an area of the framePixmap()
    has changed since the previous frame.

    Note that due to the explicit sharing of QMovie objects, these
    connections persist until they are explicitly disconnected with
    disconnectUpdate() or until \e every shared copy of the movie is
    deleted.
*/
void QMovie::connectUpdate(QObject* receiver, const char *member)
{
    QObject::connect(d, SIGNAL(areaChanged(const QRect&)), receiver, member);
}

/*!
    Disconnects the \a{receiver}'s \a member (or all members if \q
    member is zero) that were previously connected by connectUpdate().
*/
void QMovie::disconnectUpdate(QObject* receiver, const char *member)
{
    QObject::disconnect(d, SIGNAL(areaChanged(const QRect&)), receiver, member);
}

/*!
    Connects the \a{receiver}'s \a member, of type \c{void
    member(int)} so that it is signalled when the movie changes
    status. The status codes are negative for errors and positive for
    information.

    \table
    \header \i Status Code \i Meaning
    \row \i QMovie::SourceEmpty
	 \i signalled if the input cannot be read.
    \row \i QMovie::UnrecognizedFormat
	 \i signalled if the input data is unrecognized.
    \row \i QMovie::Paused
	  \i signalled when the movie is paused by a call to paused()
	  or by after \link step() stepping \endlink pauses.
    \row \i QMovie::EndOfFrame
	 \i signalled at end-of-frame after any update and Paused signals.
    \row \i QMovie::EndOfLoop
	 \i signalled at end-of-loop, after any update signals,
	 EndOfFrame - but before EndOfMovie.
    \row \i QMovie::EndOfMovie
	 \i signalled when the movie completes and is not about to loop.
    \endtable

    More status messages may be added in the future, so a general test
    for errors would test for negative.

    Note that due to the explicit sharing of QMovie objects, these
    connections persist until they are explicitly disconnected with
    disconnectStatus() or until \e every shared copy of the movie is
    deleted.
*/
void QMovie::connectStatus(QObject* receiver, const char *member)
{
    QObject::connect(d, SIGNAL(dataStatus(int)), receiver, member);
}

/*!
    Disconnects the \a{receiver}'s \a member (or all members if \a
    member is zero) that were previously connected by connectStatus().
*/
void QMovie::disconnectStatus(QObject* receiver, const char *member)
{
    QObject::disconnect(d, SIGNAL(dataStatus(int)), receiver, member);
}


#include "qmovie.moc"

#endif	// QT_NO_MOVIE
