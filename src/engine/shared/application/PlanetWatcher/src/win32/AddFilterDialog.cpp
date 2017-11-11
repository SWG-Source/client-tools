// ============================================================================
//
// AddFilterDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "AddFilterDialog.h"
#include "AddFilterDialog.moc"

#include "FilterManager.h"

#include <vector>

///////////////////////////////////////////////////////////////////////////////
//
// AddFilterDialog
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------

AddFilterDialog::AddFilterDialog(QWidget *parent, char const *name)
 : BaseFilterDialog(parent, name)
 , m_selectedFilterType()
 , m_selectedCondition()
 , m_selectedValue()
{
	connect(m_connectPushAddFilterButton, SIGNAL(clicked()), this, SLOT(slotAcceptPushButtonClicked()));
	connect(m_filterTypesComboBox, SIGNAL(highlighted(const QString &)), this, SLOT(slotFilterTypesComboBoxHighlighted(const QString &)));

	//fill in the filter types combo box
	m_filterTypesComboBox->clear();
	std::vector<std::string> filterTypes;
	FilterManager::getFilterTypes(filterTypes);
	for(std::vector<std::string>::const_iterator i = filterTypes.begin(); i != filterTypes.end(); ++i)
	{
		m_filterTypesComboBox->insertItem(i->c_str());
	}

	m_conditionComboBox->clear();

	//initialized the conditions combo box with the first selected filter type
	slotFilterTypesComboBoxHighlighted(m_filterTypesComboBox->currentText());

	m_filterTypesComboBox->setFocus();
}

//-----------------------------------------------------------------------------

void AddFilterDialog::slotAcceptPushButtonClicked()
{
	if(m_valueLineEdit->text().length() == 0)
	{
		QMessageBox::warning(this, "No value", "Please set a filter value.", "OK");
		return;
	}

	std::string const & filterType = m_filterTypesComboBox->currentText().latin1();
	std::string const & condition  = m_conditionComboBox->currentText().latin1();
	std::string const & value      = m_valueLineEdit->text().latin1();

	if(!FilterManager::validateValue(filterType, value))
	{
		std::string const & validationFailedMessage = FilterManager::getValidationFailedMessage(filterType);
		QMessageBox::warning(this, "Invalid entry", validationFailedMessage.c_str(), "OK");
		return;
	}

	m_selectedFilterType = filterType;
	m_selectedCondition  = condition;
	m_selectedValue      = value;
	accept();
}

//-----------------------------------------------------------------------------

void AddFilterDialog::slotFilterTypesComboBoxHighlighted(const QString & qText)
{
	m_conditionComboBox->clear();
	std::string const & filterType = qText.latin1();
	std::vector<std::string> conditions;
	FilterManager::getConditions(conditions, filterType);
	for(std::vector<std::string>::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
	{
		m_conditionComboBox->insertItem(i->c_str());
	}
}

//-----------------------------------------------------------------------------

void AddFilterDialog::setEditing(std::string const & filterType, std::string const & condition, std::string const & value)
{
	m_connectPushAddFilterButton->setText("Set");
	m_filterTypesComboBox->setCurrentText(filterType.c_str());
	m_conditionComboBox->setCurrentText(condition.c_str());
	m_valueLineEdit->setText(value.c_str());
	m_valueLineEdit->setFocus();
}

// ============================================================================
