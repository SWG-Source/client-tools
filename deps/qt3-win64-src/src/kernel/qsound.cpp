/****************************************************************************
** $Id: qsound.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QSound class and QAuServer internal class
**
** Created : 000117
**
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
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

#include "qsound.h"

#ifndef QT_NO_SOUND

#include "qptrlist.h"

static QPtrList<QAuServer> *servers=0;

QAuServer::QAuServer(QObject* parent, const char* name) :
    QObject(parent,name)
{
    if ( !servers ) {
	servers = new QPtrList<QAuServer>;
	// ### add cleanup
    }
    servers->prepend(this);
}

QAuServer::~QAuServer()
{
    servers->remove(this);
    if ( servers->count() == 0 ) {
	delete servers;
	servers = 0;
    }
}

void QAuServer::play(const QString& filename)
{
    QSound s(filename);
    play(&s);
}

extern QAuServer* qt_new_audio_server();

static QAuServer& server()
{
    if (!servers) qt_new_audio_server();
    return *servers->first();
}

class QSoundData {
public:
    QSoundData(const QString& fname) :
	filename(fname), bucket(0), looprem(0), looptotal(1)
    {
    }

    ~QSoundData()
    {
	delete bucket;
    }

    QString filename;
    QAuBucket* bucket;
    int looprem;
    int looptotal;
};

/*!
    \class QSound qsound.h
    \brief The QSound class provides access to the platform audio facilities.

    \ingroup multimedia
    \mainclass

    Qt provides the most commonly required audio operation in GUI
    applications: asynchronously playing a sound file. This is most
    easily accomplished with a single call:
    \code
	QSound::play("mysounds/bells.wav");
    \endcode

    A second API is provided in which a QSound object is created from
    a sound file and is played later:
    \code
	QSound bells("mysounds/bells.wav");

	bells.play();
    \endcode

    Sounds played using the second model may use more memory but play
    more immediately than sounds played using the first model,
    depending on the underlying platform audio facilities.

    On Microsoft Windows the underlying multimedia system is used;
    only WAVE format sound files are supported.

    On X11 the \link ftp://ftp.x.org/contrib/audio/nas/ Network Audio
    System\endlink is used if available, otherwise all operations work
    silently. NAS supports WAVE and AU files.

    On Macintosh, ironically, we use QT (\link
    http://quicktime.apple.com QuickTime\endlink) for sound, this
    means all QuickTime formats are supported by Qt/Mac.

    On Qt/Embedded, a built-in mixing sound server is used, which
    accesses \c /dev/dsp directly. Only the WAVE format is supported.

    The availability of sound can be tested with
    QSound::isAvailable().
*/

/*!
    \fn static bool QSound::available()

    Returns TRUE if sound support is available; otherwise returns FALSE.
*/

/*!
    Plays the sound in a file called \a filename.
*/
void QSound::play(const QString& filename)
{
    server().play(filename);
}

/*!
    Constructs a QSound that can quickly play the sound in a file
    named \a filename.

    This may use more memory than the static \c play function.

    The \a parent and \a name arguments (default 0) are passed on to
    the QObject constructor.
*/
QSound::QSound(const QString& filename, QObject* parent, const char* name) :
    QObject(parent,name),
    d(new QSoundData(filename))
{
    server().init(this);
}

/*!
    Destroys the sound object. If the sound is not finished playing stop() is called on it.

    \sa stop() isFinished()
*/
QSound::~QSound()
{
    if ( !isFinished() )
	stop();
    delete d;
}

/*!
    Returns TRUE if the sound has finished playing; otherwise returns FALSE.

    \warning On Windows this function always returns TRUE for unlooped sounds.
*/
bool QSound::isFinished() const
{
    return d->looprem == 0;
}

/*!
    \overload

    Starts the sound playing. The function returns immediately.
    Depending on the platform audio facilities, other sounds may stop
    or may be mixed with the new sound.

    The sound can be played again at any time, possibly mixing or
    replacing previous plays of the sound.
*/
void QSound::play()
{
    d->looprem = d->looptotal;
    server().play(this);
}

/*!
    Returns the number of times the sound will play.
*/
int QSound::loops() const
{
    return d->looptotal;
}

/*!
    Returns the number of times the sound will loop. This value
    decreases each time the sound loops.
*/
int QSound::loopsRemaining() const
{
    return d->looprem;
}

/*!
    Sets the sound to repeat \a l times when it is played. Passing the
    value -1 will cause the sound to loop indefinitely.

    \sa loops()
*/
void QSound::setLoops(int l)
{
    d->looptotal = l;
}

/*!
    Returns the filename associated with the sound.
*/
QString QSound::fileName() const
{
    return d->filename;
}

/*!
    Stops the sound playing.

    On Windows the current loop will finish if a sound is played
    in a loop.

    \sa play()
*/
void QSound::stop()
{
    server().stop(this);
}


/*!
    Returns TRUE if sound facilities exist on the platform; otherwise
    returns FALSE. An application may choose either to notify the user
    if sound is crucial to the application or to operate silently
    without bothering the user.

    If no sound is available, all QSound operations work silently and
    quickly.
*/
bool QSound::isAvailable()
{
    return server().okay();
}

/*!
    Sets the internal bucket record of sound \a s to \a b, deleting
    any previous setting.
*/
void QAuServer::setBucket(QSound* s, QAuBucket* b)
{
    delete s->d->bucket;
    s->d->bucket = b;
}

/*!
    Returns the internal bucket record of sound \a s.
*/
QAuBucket* QAuServer::bucket(QSound* s)
{
    return s->d->bucket;
}

/*!
    Decrements the QSound::loopRemaining() value for sound \a s,
    returning the result.
*/
int QAuServer::decLoop(QSound* s)
{
    if ( s->d->looprem > 0 )
	--s->d->looprem;
    return s->d->looprem;
}

/*!
    Initializes the sound. The default implementation does nothing.
*/
void QAuServer::init(QSound*)
{
}

QAuBucket::~QAuBucket()
{
}

#endif // QT_NO_SOUND
