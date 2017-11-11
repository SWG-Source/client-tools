//======================================================================
//
// ChassisNewDialog.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "FirstShipComponentEditor.h"
#include "ChassisNewDialog.h"
#include "ChassisNewDialog.moc"

#include "ConfigShipComponentEditor.h"
#include "QStringUtil.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedGame/ShipChassisWritable.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include <vector>

//======================================================================

ChassisNewDialog::ChassisNewDialog(QWidget *parent, char const *name, std::string const & cloneFromChassisName) :
BaseChassisNewDialog(parent, name)
{
	m_comboCloneFrom->insertItem(QString());

	typedef stdvector<std::string>::fwd StringVector;
	StringVector sv;
	{
		ShipChassis::PersistentCrcStringVector const & shipChassisNameVector = ShipChassis::getShipChassisCrcVector();
		for (ShipChassis::PersistentCrcStringVector::const_iterator it = shipChassisNameVector.begin(); it != shipChassisNameVector.end(); ++it)
		{
			PersistentCrcString const * const pcs = *it;
			sv.push_back(pcs->getString());
		}

		std::sort(sv.begin(), sv.end());
	}

	int selectedIndex = 0;
	int index = 1;
	for (StringVector::const_iterator it = sv.begin(); it != sv.end(); ++it, ++index)
	{
		std::string const & chassisName = *it;
		m_comboCloneFrom->insertItem(chassisName.c_str());

		if (cloneFromChassisName == chassisName)
		{
			selectedIndex = index;
		}
	}

	m_comboCloneFrom->setCurrentItem(selectedIndex);
//	m_comboCloneFrom->insertItem(QString(cloneFromChassisName.c_str()));

	connect(m_lineEditName, SIGNAL(textChanged(const QString &)), SLOT(onNameTextChanged(const QString &)));
	connect(m_buttonCreate, SIGNAL(clicked()), SLOT(onButtonCreateClicked()));
}

//----------------------------------------------------------------------

ChassisNewDialog::~ChassisNewDialog()
{
}

//----------------------------------------------------------------------
//-- SLOTS
//----------------------------------------------------------------------

void ChassisNewDialog::onNameTextChanged(const QString & s)
{
	std::string const & nameStr = Unicode::toLower(QStringUtil::toString(s));

	if (nameStr.empty())
	{
		m_buttonCreate->setEnabled(false);
	}
	else
	{
		ShipChassis const * const chassis = ShipChassis::findShipChassisByName(PersistentCrcString(nameStr.c_str(), true));
		if (NULL == chassis)
		{
			QPalette pal (m_lineEditName->palette());
			pal.setColor(QColorGroup::Text, "black");
			m_lineEditName->setPalette(pal);
			m_buttonCreate->setEnabled(true);
		}
		else
		{
			QPalette pal (m_lineEditName->palette());
			pal.setColor(QColorGroup::Text, "red"), 
			m_lineEditName->setPalette(pal);
			m_buttonCreate->setEnabled(false);
		}
	}
}

//----------------------------------------------------------------------

void ChassisNewDialog::onButtonCreateClicked()
{
	std::string const & chassisName = Unicode::toLower(QStringUtil::toString(m_lineEditName->text()));
	std::string const & cloneFrom = QStringUtil::toString(m_comboCloneFrom->currentText());
	if (!cloneFrom.empty())
	{
		ShipChassis const * const chassis = ShipChassis::findShipChassisByName(PersistentCrcString(cloneFrom.c_str(), true));
		if (NULL == chassis)
		{
			WARNING(true, ("ChassisNewDialog unable to find clone chassis [%s]", cloneFrom.c_str()));
			return;
		}
		ShipChassisWritable * const shipChassisWritable = new ShipChassisWritable(*chassis, chassisName);
		shipChassisWritable->addChassis(true);

		ShipComponentAttachmentManager::copyAttachmentsForChassis(chassis->getCrc(), shipChassisWritable->getCrc());
		close();
	}
	else
	{
		ShipChassisWritable * const shipChassisWritable = new ShipChassisWritable();
		shipChassisWritable->setName(ConstCharCrcString(chassisName.c_str(), true));
		shipChassisWritable->addChassis(true);	
		close();
	}

	char buf[1024];
	size_t const buf_size = sizeof(buf);

	snprintf(buf, buf_size, "p4 add %s/datatables/space/ship_%s.tab %s/datatables/space/ship_%s.iff", 
		ConfigShipComponentEditor::getSharedPathDsrc().c_str(), chassisName.c_str(),
		ConfigShipComponentEditor::getSharedPathData().c_str(), chassisName.c_str());

	system(buf);
}

//----------------------------------------------------------------------
//-- END SLOTS
//----------------------------------------------------------------------


//======================================================================
