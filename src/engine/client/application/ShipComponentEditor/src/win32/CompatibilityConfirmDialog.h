//======================================================================
//
// CompatibilityConfirmDialog.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CompatibilityConfirmDialog_H
#define INCLUDED_CompatibilityConfirmDialog_H

//======================================================================

#include "BaseCompatibilityConfirmDialog.h"

//-----------------------------------------------------------------------------

class CompatibilityConfirmDialog : public BaseCompatibilityConfirmDialog
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	typedef stdvector<std::string>::fwd StringVector;

	CompatibilityConfirmDialog(QWidget *parent, char const *name, std::string const & oldCompat, std::string const & newCompat, StringVector const & chassisSlotsAffected, StringVector const & componentsAffected);
	~CompatibilityConfirmDialog();

	bool showAndTell();

public slots:

	void onPushButtonOkClicked();
	void onPushButtonCancelClicked();

signals:

protected:

private:

	void populateLists(StringVector const & chassisSlotsAffected, StringVector const & componentsAffected);
	void setupSignals();

private:

	bool m_ok;
};

//======================================================================

#endif
