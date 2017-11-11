// ============================================================================
//
// TemplateVerificationDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateVerificationDialog_H
#define INCLUDED_TemplateVerificationDialog_H

#include "BaseTemplateVerificationDialog.h"

//-----------------------------------------------------------------------------
class TemplateVerificationDialog : public BaseTemplateVerificationDialog
{
	Q_OBJECT

public:

	TemplateVerificationDialog(QWidget *parent, const char *name, int const templateCount, int const directoryCount);

protected slots:

	void slotFirstLineEditReturnPressed();
	void slotLastLineEditReturnPressed();
	void slotCancelPushButtonClicked();
	void slotOpenPushButtonClicked();
	void slotCheckAllRadioButtonClicked();
	void slotCheckOneRadioButtonClicked();
	void slotCheckSubsetRadioButtonClicked();

signals:

protected:

private:

	int const m_templateMin;
	int const m_templateMax;

	TemplateVerificationDialog &operator =(TemplateVerificationDialog const &);
};

// ============================================================================

#endif // INCLUDED_TemplateVerificationDialog_H
