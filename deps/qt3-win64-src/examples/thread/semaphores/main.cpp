/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <qapplication.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qthread.h>
#include <qsemaphore.h>
#include <qmutex.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qlabel.h>

#if defined(QT_NO_THREAD)
#  error Thread support not enabled.
#endif

// Use pointers to create semaphores after QApplication object!
QSemaphore* yellowSem, *greenSem;


class YellowThread : public QThread
{
public:
    YellowThread(QWidget *o)
	: receiver(o), stopped(FALSE)
    { ; }

    void run();
    void stop();


private:
    QWidget *receiver;
    QMutex mutex;
    bool stopped;
};


void YellowThread::run()
{
    for (int i = 0; i < 20; i++) {
	(*yellowSem)++;

	QCustomEvent *event = new QCustomEvent(12345);
     	event->setData(new QString("Yellow!"));
	QApplication::postEvent(receiver, event);
	msleep(200);

	(*greenSem)--;

	mutex.lock();
	if (stopped) {
	    stopped = FALSE;
	    mutex.unlock();
	    break;
	}
	mutex.unlock();
    }

    (*yellowSem)++;

    QCustomEvent *event = new QCustomEvent(12346);
    event->setData(new QString("Yellow!"));
    QApplication::postEvent(receiver, event);

    (*greenSem)--;
}

void YellowThread::stop()
{
    mutex.lock();
    stopped = TRUE;
    mutex.unlock();
}


class GreenThread: public QThread
{
public:
   GreenThread(QWidget *o)
	: receiver(o), stopped( FALSE )
    { ; }

    void run();
    void stop();


private:
    QWidget *receiver;
    QMutex mutex;
    bool stopped;
};


void GreenThread::run()
{
    for (int i = 0; i < 20; i++) {
	(*greenSem)++;

	QCustomEvent *event = new QCustomEvent(12345);
     	event->setData(new QString("Green!"));
	QApplication::postEvent(receiver, event);
	msleep(200);

	(*yellowSem)--;

	mutex.lock();
	if (stopped) {
	    stopped = FALSE;
	    mutex.unlock();
	    break;
	}
	mutex.unlock();
    }

    (*greenSem)++;

    QCustomEvent *event = new QCustomEvent(12346);
    event->setData(new QString("Green!"));
    QApplication::postEvent(receiver, event);
    msleep(10);

    (*yellowSem)--;
}

void GreenThread::stop()
{
    mutex.lock();
    stopped = TRUE;
    mutex.unlock();
}



class SemaphoreExample : public QWidget
{
    Q_OBJECT
public:
    SemaphoreExample();
    ~SemaphoreExample();

    void customEvent(QCustomEvent *);


public slots:
    void startExample();


protected:


private:
    QMultiLineEdit *mlineedit;
    QPushButton *button;
    QLabel *label;

    YellowThread yellowThread;
    GreenThread greenThread;
};


SemaphoreExample::SemaphoreExample()
    : QWidget(), yellowThread(this), greenThread(this)
{
    yellowSem = new QSemaphore(1);
    greenSem = new QSemaphore(1);

    button = new QPushButton("&Ignition!", this);
    connect(button, SIGNAL(clicked()), SLOT(startExample()));

    mlineedit = new QMultiLineEdit(this);
    label = new QLabel(this);

    QVBoxLayout *vbox = new QVBoxLayout(this, 5);
    vbox->addWidget(button);
    vbox->addWidget(mlineedit);
    vbox->addWidget(label);
}


SemaphoreExample::~SemaphoreExample()
{
    bool stopYellow = yellowThread.running(),
	  stopGreen = greenThread.running();
    if (stopYellow)
	yellowThread.stop();
    if (greenThread.running())
	greenThread.stop();
    if (stopYellow)
	yellowThread.wait();
    if (stopGreen)
	greenThread.wait();
    delete yellowSem;
    delete greenSem;
}


void SemaphoreExample::startExample()
{
    if (yellowThread.running() || greenThread.running()) {
	QMessageBox::information(this, "Sorry",
				 "The threads have not completed yet, and must finish before "
				 "they can be started again.");

	return;
    }

    mlineedit->clear();

    while (yellowSem->available() < yellowSem->total()) (*yellowSem)--;
    (*yellowSem)++;

    yellowThread.start();
    greenThread.start();
}


void SemaphoreExample::customEvent(QCustomEvent *event) {
    switch (event->type()) {
    case 12345:
	{
	    QString *s = (QString *) event->data();

	    mlineedit->append(*s);

	    if (*s == "Green!")
		label->setBackgroundColor(green);
	    else
		label->setBackgroundColor(yellow);
	    label->setText(*s);

	    delete s;

	    break;
	}

    case 12346:
	{
	    QString *s = (QString *) event->data();

	    QMessageBox::information(this, (*s) + " - Finished",
				     "The thread creating the \"" + *s +
				     "\" events has finished.");
	    delete s;

	    break;
	}

    default:
	{
	    qWarning("Unknown custom event type: %d", event->type());
	}
    }
}


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    SemaphoreExample se;
    app.setMainWidget(&se);
    se.show();
    return app.exec();
}


#include "main.moc"
