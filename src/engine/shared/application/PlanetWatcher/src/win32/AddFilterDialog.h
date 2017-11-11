// ============================================================================
//
// AddFilterDialog.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_AddFilterDialog_H
#define INCLUDED_AddFilterDialog_H

#include "BaseFilterDialog.h"

#include <string>

//-----------------------------------------------------------------------------
class AddFilterDialog : public BaseFilterDialog
{
	Q_OBJECT

public:
	AddFilterDialog(QWidget *parent, char const *name);
	void setEditing(std::string const & filterType, std::string const & condition, std::string const & value);

	std::string m_selectedFilterType;
	std::string m_selectedCondition;
	std::string m_selectedValue;

protected:

private slots:
	void slotAcceptPushButtonClicked();
	void slotFilterTypesComboBoxHighlighted(const QString &);

private:
};

// ============================================================================

#endif // INCLUDED_AddFilterDialog_H
