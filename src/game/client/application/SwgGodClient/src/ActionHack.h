// ======================================================================
//
// ActionHack.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionHack_H
#define INCLUDED_ActionHack_H

// ======================================================================

#include <qaction.h>

/**
 * ActionHack is a class that implements some functionality that QAction is conspicuously missing.
 * Mainly, it allows clients to 'trigger' the Action's functionality directly, without
 * needing to know about the slots that implement the functionality.
 * @see doActivate ()
 * @see doToggle (bool)
 */
class ActionHack : public QAction
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit ActionHack(QObject* parent, const char* name = 0, bool toggle = FALSE);
	ActionHack(const QString & text, const QIconSet & icon, const QString & menuText, int accel, QObject* parent, const char* name = 0, bool toggle = FALSE);
	ActionHack(const QString & text, const QString & menuText, int accel, QObject* parent, const char* name = 0, bool toggle = FALSE);

public slots:
	void doActivate ();
	void doToggle   (bool b);

private:
	//disabled
	ActionHack();
	ActionHack(const ActionHack & rhs);
	ActionHack& operator=(const ActionHack & rhs);
};
//----------------------------------------------------------------------
//-- shortcuts to workaround qt enums as key accels

#define QT_ACCEL(a)      (static_cast<int>(Qt::a))
#define QT_ACCEL2(a,b)   (QT_ACCEL(a)    + QT_ACCEL(b))
#define QT_ACCEL3(a,b,c) (QT_ACCEL2(a,b) + QT_ACCEL(c))

// ======================================================================

#endif
