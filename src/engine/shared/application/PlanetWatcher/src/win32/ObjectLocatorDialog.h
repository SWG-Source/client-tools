// ============================================================================
//
// ObjectLocatorDialog.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ObjectLocatorDialog_H
#define INCLUDED_ObjectLocatorDialog_H

#include "BaseObjectLocatorDialog.h"

//-----------------------------------------------------------------------------
class ObjectLocatorDialog : public BaseObjectLocatorDialog
{
	Q_OBJECT

public:

	ObjectLocatorDialog(QWidget *parent, char const *name);

	const char * getObjectId() const;

protected:

	virtual void keyPressEvent(QKeyEvent *keyEvent);

private slots:

	void slotAcceptPushButtonClicked();
};

// ============================================================================

#endif // INCLUDED_ObjectLocatorDialog_H
