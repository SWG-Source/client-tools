/****************************************************************************
** $Id: qsound_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#define QT_CLEAN_NAMESPACE

#include "qsound.h"

#ifndef QT_NO_SOUND

#include "qptrdict.h"
#include "qsocketnotifier.h"
#include "qapplication.h"


#ifdef QT_NAS_SUPPORT

#include <audio/audiolib.h>
#include <audio/soundlib.h>

static AuServer *nas=0;

static AuBool eventPred(AuServer *, AuEvent *e, AuPointer p)
{
    if (e && (e->type == AuEventTypeElementNotify)) {
	if (e->auelementnotify.flow == *((AuFlowID *)p))
	    return TRUE;
    }
    return FALSE;
}

class QAuBucketNAS : public QAuBucket {
public:
    QAuBucketNAS(AuBucketID b, AuFlowID f = 0) : id(b), flow(f), stopped(TRUE), numplaying(0) { }
    ~QAuBucketNAS()
    {
	if ( nas ) {
	    AuSync(nas, FALSE);
	    AuDestroyBucket(nas, id, NULL);

	    AuEvent ev;
	    while (AuScanEvents(nas, AuEventsQueuedAfterFlush, TRUE, eventPred, &flow, &ev))
		;
        }
    }

    AuBucketID id;
    AuFlowID flow;
    bool     stopped;
    int      numplaying;
};

class QAuServerNAS : public QAuServer {
    Q_OBJECT

    QSocketNotifier* sn;

public:
    QAuServerNAS(QObject* parent);
    ~QAuServerNAS();

    void init(QSound*);
    void play(const QString& filename);
    void play(QSound*);
    void stop(QSound*);
    bool okay();
    void setDone(QSound*);

public slots:
    void dataReceived();
    void soundDestroyed(QObject *o);

private:
    QAuBucketNAS* bucket(QSound* s)
    {
	return (QAuBucketNAS*)QAuServer::bucket(s);
    }
};

QAuServerNAS::QAuServerNAS(QObject* parent) :
    QAuServer(parent,"Network Audio System")
{
    nas = AuOpenServer(NULL, 0, NULL, 0, NULL, NULL);
    if (nas) {
	AuSetCloseDownMode(nas, AuCloseDownDestroy, NULL);
	// Ask Qt for async messages...
	sn=new QSocketNotifier(AuServerConnectionNumber(nas),
		QSocketNotifier::Read);
	QObject::connect(sn, SIGNAL(activated(int)),
		this, SLOT(dataReceived()));
    } else {
	sn = 0;
    }
}

QAuServerNAS::~QAuServerNAS()
{
    if ( nas )
	AuCloseServer( nas );
    delete sn;
    nas = 0;
}

static QPtrDict<void> *inprogress=0;

void QAuServerNAS::soundDestroyed(QObject *o)
{
    if (inprogress) {
	QSound *so = static_cast<QSound *>(o);
	while (inprogress->remove(so))
	    ; // Loop while remove returns TRUE
    }
}

void QAuServerNAS::play(const QString& filename)
{
    if (nas) {
	int iv=100;
	AuFixedPoint volume=AuFixedPointFromFraction(iv,100);
	AuSoundPlayFromFile(nas, filename, AuNone, volume, NULL, NULL, NULL, NULL, NULL, NULL);
	AuFlush(nas);
	dataReceived();
	AuFlush(nas);
	qApp->flushX();
    }
}

static void callback( AuServer*, AuEventHandlerRec*, AuEvent* e, AuPointer p)
{
    if ( inprogress->find(p) && e ) {
	if (e->type==AuEventTypeElementNotify &&
		    e->auelementnotify.kind==AuElementNotifyKindState) {
	    if ( e->auelementnotify.cur_state == AuStateStop )
                ((QAuServerNAS*)inprogress->find(p))->setDone((QSound*)p);
	}
    }
}

void QAuServerNAS::setDone(QSound* s)
{
    if (nas) {
        decLoop(s);
        if (s->loopsRemaining() && !bucket(s)->stopped) {
	    bucket(s)->stopped = TRUE;
            play(s);
        } else {
	    if (--(bucket(s)->numplaying) == 0)
		bucket(s)->stopped = TRUE;
            inprogress->remove(s);
        }
    }
}

void QAuServerNAS::play(QSound* s)
{
    if (nas) {
	++(bucket(s)->numplaying);
	if (!bucket(s)->stopped) {
	    stop(s);
	}

	bucket(s)->stopped = FALSE;
	if ( !inprogress )
	    inprogress = new QPtrDict<void>;
	inprogress->insert(s,(void*)this);
	int iv=100;
	AuFixedPoint volume=AuFixedPointFromFraction(iv,100);
        QAuBucketNAS *b = bucket(s);
        AuSoundPlayFromBucket(nas, b->id, AuNone, volume,
                              callback, s, 0, &b->flow, NULL, NULL, NULL);
	AuFlush(nas);
	dataReceived();
	AuFlush(nas);
	qApp->flushX();
    }
}

void QAuServerNAS::stop(QSound* s)
{
    if (nas && !bucket(s)->stopped) {
	bucket(s)->stopped = TRUE;
        AuStopFlow(nas, bucket(s)->flow, NULL);
        AuFlush(nas);
	dataReceived();
	AuFlush(nas);
	qApp->flushX();
    }
}

void QAuServerNAS::init(QSound* s)
{
    connect(s, SIGNAL(destroyed(QObject *)),
	    this, SLOT(soundDestroyed(QObject *)));

    if ( nas ) {
        AuBucketID b_id =
            AuSoundCreateBucketFromFile(nas, s->fileName(),
                                        0 /*AuAccessAllMasks*/, NULL, NULL);
	setBucket(s, new QAuBucketNAS(b_id));
    }
}

bool QAuServerNAS::okay()
{
    return !!nas;
}

void QAuServerNAS::dataReceived()
{
    AuHandleEvents(nas);
}

#include "qsound_x11.moc"

#endif


class QAuServerNull : public QAuServer {
public:
    QAuServerNull(QObject* parent);

    void play(const QString&) { }
    void play(QSound*s) { while(decLoop(s) > 0) /* nothing */ ; }
    void stop(QSound*) { }
    bool okay() { return FALSE; }
};

QAuServerNull::QAuServerNull(QObject* parent) :
    QAuServer(parent,"Null Audio Server")
{
}


QAuServer* qt_new_audio_server()
{
#ifdef QT_NAS_SUPPORT
    QAuServer* s=new QAuServerNAS(qApp);
    if (s->okay()) {
	return s;
    } else {
	delete s;
    }
#endif
    return new QAuServerNull(qApp);
}

#endif // QT_NO_SOUND
