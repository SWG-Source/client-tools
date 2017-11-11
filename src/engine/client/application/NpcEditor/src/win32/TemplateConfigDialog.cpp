// ============================================================================
//
// TemplateConfigDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "TemplateConfigDialog.h"
#include "TemplateConfigDialog.moc"
#include "QtUtility.h"
#include "ObjectTemplateWriter.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedUtility/DataTable.h"

#include <map>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtabwidget.h>

namespace TemplateConfigDialogNamespace
{
	void setComboBoxIndexFromText(QComboBox * combo, const char * text);

	char const * c_npcEditorSection = "NpcEditor";
	char const * c_nameGeneratorType = "nameGeneratorType";
	char const * c_gender = "gender";
	char const * c_species = "species";
	char const * c_defaultWeapon = "defaultWeapon";
	char const * c_objectName = "objectName";
	char const * c_lookAtText = "lookAtText";
	char const * c_scaleMin = "scaleMin";
	char const * c_scaleMax = "scaleMax";
	char const * c_detailedDescriptionA = "detailedDescriptionA";
	char const * c_detailedDescriptionB = "detailedDescriptionB";

	char const * c_defaultWeaponDirectory = "defaultWeaponDirectory";
	char const * c_defaultWeaponRoot = "defaultWeaponRoot";
}

using namespace TemplateConfigDialogNamespace;

// ============================================================================
//
// TemplateConfigDialog
//
// ============================================================================

void TemplateConfigDialogNamespace::setComboBoxIndexFromText(QComboBox * combo, const char * text)
{
	for(int i = 0; i < combo->count(); ++i)
		if(combo->text(i) == text)
		{
			combo->setCurrentItem(i);
			return;
		}

	DEBUG_REPORT_LOG(true, ("TemplateConfigDialogNamespace::setComboIndexFromText() - Count not find text [%s] in combo box.\n", text));
}

// ----------------------------------------------------------------------------

TemplateConfigDialog::TemplateConfigDialog(QWidget *myParent, char const *windowName)
 : BaseTemplateConfigDialog(myParent, windowName)
 , m_npcDataTable(0)
{
	IGNORE_RETURN(connect(m_pushButtonServerBrowserDefaultWeapon, SIGNAL(released()), this, SLOT(slotServerBrowseWeaponReleased())));

	//-- these will be implemented in the future
    m_pushButtonServerEditFile->setHidden(true);
    m_pushButtonSharedEditFile->setHidden(true);
}

// ----------------------------------------------------------------------------

TemplateConfigDialog::~TemplateConfigDialog()
{
	m_npcDataTable = 0;
}

// ----------------------------------------------------------------------------

void TemplateConfigDialog::setupControls(DataTable * npcDataTable,
										 ObjectTemplateWriter::ObjectTemplateParameterMap * serverParameters,
										 ObjectTemplateWriter::ObjectTemplateParameterMap * sharedParameters)
{
	//-- Config file driven controls
	QtUtility::fillComboBoxFromConfigFile(c_npcEditorSection, c_nameGeneratorType, 64, m_comboBoxServerNameGeneratorType);
	QtUtility::fillComboBoxFromConfigFile(c_npcEditorSection, c_gender, 64, m_comboBoxSharedGender);
	QtUtility::fillComboBoxFromConfigFile(c_npcEditorSection, c_species, 512, m_comboBoxSharedSpecies);

	setNpcDataTable(npcDataTable);

	//-- Server Parameters
	setComboBoxIndexFromText(m_comboBoxServerNameGeneratorType, (*serverParameters)[c_nameGeneratorType].c_str());

	m_lineEditServerDefaultWeapon->setText((*serverParameters)[c_defaultWeapon].c_str());

	//-- Shared Parameters
	setComboBoxIndexFromText(m_comboBoxSharedSpecies, (*sharedParameters)[c_species].c_str());
	setComboBoxIndexFromText(m_comboBoxSharedGender, (*sharedParameters)[c_gender].c_str());

	m_lineEditObjectName->setText((*sharedParameters)[c_objectName].c_str());
	m_lineEditLookAtText->setText((*sharedParameters)[c_lookAtText].c_str());

	m_lineEditScaleMin->setText((*sharedParameters)[c_scaleMin].c_str());
	m_lineEditScaleMax->setText((*sharedParameters)[c_scaleMax].c_str());

	m_lineEditDescriptionA->setText((*sharedParameters)[c_detailedDescriptionA].c_str());
	m_lineEditDescriptionB->setText((*sharedParameters)[c_detailedDescriptionB].c_str());
}

// ----------------------------------------------------------------------------

void TemplateConfigDialog::updateParameters(ObjectTemplateWriter::ObjectTemplateParameterMap * serverParameters,
											ObjectTemplateWriter::ObjectTemplateParameterMap * sharedParameters) const
{
	//-- Server Parameters
	(*serverParameters)[c_nameGeneratorType] = m_comboBoxServerNameGeneratorType->currentText();
	(*serverParameters)[c_defaultWeapon] = m_lineEditServerDefaultWeapon->text();

	//-- Shared Parameters
	(*sharedParameters)[c_species] = m_comboBoxSharedSpecies->currentText();
	(*sharedParameters)[c_gender] = m_comboBoxSharedGender->currentText();

	(*sharedParameters)[c_objectName] = m_lineEditObjectName->text();
	(*sharedParameters)[c_lookAtText] = m_lineEditLookAtText->text();

	(*sharedParameters)[c_scaleMin] = m_lineEditScaleMin->text();
	(*sharedParameters)[c_scaleMax] = m_lineEditScaleMax->text();

	(*sharedParameters)[c_detailedDescriptionA] = m_lineEditDescriptionA->text();
	(*sharedParameters)[c_detailedDescriptionB] = m_lineEditDescriptionB->text();
}

// ----------------------------------------------------------------------------

void TemplateConfigDialog::slotServerBrowseWeaponReleased()
{
	QString defaultDirectory = ConfigFile::getKeyString(c_npcEditorSection, c_defaultWeaponDirectory, 0, 0);

	QString selectedFileName(QFileDialog::getOpenFileName(defaultDirectory, "Iff files (*.iff)", this, "WeaponFileDialog", "Set Default Weapon File"));
	
	if(!selectedFileName.isNull())
	{
		QtUtility::makeFileNameRelative(&selectedFileName, c_npcEditorSection, c_defaultWeaponRoot);

		m_lineEditServerDefaultWeapon->setText(selectedFileName);
	}
}

// ============================================================================
