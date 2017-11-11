// ======================================================================
//
// FormWindow.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "FormWindow.h"
#include "FormWindow.moc"

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include "clientGame/FormManagerClient.h"

#include "QLayout.h"
#include "QLabel.h"
#include "QLineEdit.h"
#include "QMessageBox.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace FormWindowNamespace
{
	static const int cs_leftBuffer = 20;
	static const int cs_betweenBuffer = 40;
	static const int cs_widgetHeight = 20;
	static const int cs_verticalBufferSize = 10;
}
using namespace FormWindowNamespace;

// ======================================================================

FormWindow::FormWindow(QWidget * theParent, char const * theName)
: BaseFormWindow (theParent, theName),
  m_displayReason(FormManager::CREATE_OBJECT),
  m_currentForm(NULL),
  m_fieldMap(new std::map<FormManager::Field const *, QWidget * >),
  m_orderedFieldList(new std::vector<FormManager::Field const *>),
  m_serverTemplateName(),
  m_cursorPositionInWorld(new Vector),
  m_cellIdPositionInWorld(),
  m_object(NULL)
{
}

//-----------------------------------------------------------------------

FormWindow::~FormWindow()
{
	delete m_orderedFieldList;
	delete m_fieldMap;
	delete m_cursorPositionInWorld;
	m_currentForm = NULL;
	m_object = NULL;
}

//-----------------------------------------------------------------------

void FormWindow::setDisplayReason(FormManager::Command const & reason)
{
	m_displayReason = reason;
}

//-----------------------------------------------------------------------

void FormWindow::setCurrentForm(FormManager::Form const & form)
{
	m_currentForm = &form;
}

//-----------------------------------------------------------------------

void FormWindow::setServerObjectTemplate(std::string const & templateName)
{
	m_serverTemplateName = templateName;
}

//-----------------------------------------------------------------------

void FormWindow::setCreatePosition(Vector const & pos, NetworkId const & cellId)
{
	if(m_cursorPositionInWorld)
		*m_cursorPositionInWorld = pos;
	m_cellIdPositionInWorld = cellId;
}

//-----------------------------------------------------------------------

void FormWindow::setObject(Object const * const object)
{
	m_object = object;
}

//-----------------------------------------------------------------------

void FormWindow::populatePage()
{
	if(!m_currentForm)
	{
		DEBUG_FATAL(true, ("FormWindow::populateData - m_currentForm needs to be filled out beforehand!"));
		return; //lint !e527 reachable in release
	}

	buildUI(*m_currentForm);
}

//-----------------------------------------------------------------------

void FormWindow::populatePage(FormManager::UnpackedFormData const & unpackedData)
{
	if(!m_currentForm)
	{
		DEBUG_FATAL(true, ("FormWindow::populateData - m_currentForm needs to be filled out beforehand!"));
		return; //lint !e527 reachable in release
	}

	buildUI(*m_currentForm, unpackedData);
}

//-----------------------------------------------------------------------

void FormWindow::buildUI(FormManager::Form const & form)
{
	std::vector<FormManager::Field const *> const & fields = form.getOrderedFieldList();

	int currentY = cs_verticalBufferSize;

	for(std::vector<FormManager::Field const *>::const_iterator i = fields.begin(); i != fields.end(); ++i)
	{
		FormManager::Field const * const field = *i;
		if(field)
			currentY = addField(*field, currentY, "") + cs_verticalBufferSize;
	}
}

//-----------------------------------------------------------------------

void FormWindow::buildUI(FormManager::Form const & form, FormManager::UnpackedFormData const & unpackedData)
{
	std::vector<FormManager::Field const *> const & fields = form.getOrderedFieldList();

	int currentY = cs_verticalBufferSize;

	for(std::vector<FormManager::Field const *>::const_iterator i = fields.begin(); i != fields.end(); ++i)
	{
		FormManager::Field const * const field = *i;
		if(field)
		{
			FormManager::UnpackedFormData::const_iterator i2 = unpackedData.find(field->getName());
			if(i2 != unpackedData.end())
			{
				std::vector<std::string> values = i2->second;
				if(!values.empty())
					currentY = addField(*field, currentY, values[0]/*TODO handle lists*/) + cs_verticalBufferSize;
			}
		}
	}
}

//-----------------------------------------------------------------------

int FormWindow::addField(FormManager::Field const & field, int yLocation, std::string const & value)
{
	QLabel * const label = new QLabel(this);
	label->setText(field.getExternalName().c_str());
	label->setMinimumHeight(cs_widgetHeight);
	label->setMaximumHeight(cs_widgetHeight);
	label->move(cs_leftBuffer, yLocation);

	QLineEdit* const edit = new QLineEdit(this);
	edit->setMinimumHeight(cs_widgetHeight);
	edit->setMaximumHeight(cs_widgetHeight);
	edit->move(cs_leftBuffer + label->width() + cs_betweenBuffer, yLocation);

	if(value.empty())
		edit->setText(field.getDefaultValue().c_str());
	else
		edit->setText(value.c_str());

	(*m_fieldMap)[&field] = edit;
	m_orderedFieldList->push_back(&field);

	return yLocation + label->height();
}

//-----------------------------------------------------------------------

bool FormWindow::gatherAndSendData()
{
	FormManager::UnpackedFormData dataMap;
	std::map<std::string, std::string> pendingData;

	std::vector<std::string> values;
	for(std::map<FormManager::Field const *, QWidget *>::iterator i = m_fieldMap->begin(); i != m_fieldMap->end(); ++i)
	{
		FormManager::Field const * const field = i->first;
		if(!field)
		{
			DEBUG_FATAL(true, ("Field is null"));
			return false; //lint !e527 reachable in release
		}
		QWidget * const widget = i->second;
		if(!widget)
		{
			DEBUG_FATAL(true, ("widget is null"));
			return false; //lint !e527 reachable in release
		}
		std::string const & fieldName = field->getName();
		std::string const fieldValueString = getFieldValue(widget);

		/*TODO handle lists*/
		values.clear();
		values.push_back(fieldValueString.empty() ? FormManager::getEmptyStringPlaceholder(): fieldValueString);
		dataMap[fieldName] = values;
		pendingData[fieldName] = fieldValueString;
	}

	if(!validateData(pendingData))
		return false;

	//the the message to tell script to create the object with the given data
	if(m_displayReason == FormManager::CREATE_OBJECT)
	{
		if(m_cursorPositionInWorld)
			FormManagerClient::sendCreateObjectData(m_serverTemplateName, *m_cursorPositionInWorld, m_cellIdPositionInWorld, dataMap);
	}
	//tell script to edit the object with the given data
	else if(m_displayReason == FormManager::EDIT_OBJECT)
	{
		NetworkId const & nid = m_object ? m_object->getNetworkId() : NetworkId::cms_invalid;
		if(nid != NetworkId::cms_invalid)
			FormManagerClient::sendEditObjectData(nid, dataMap);
	}
	else
	{
		DEBUG_FATAL(true, ("Unknown display reason encountered"));
		return false; //lint !e527 reachable in release
	}
	return true;
}

//-----------------------------------------------------------------------

std::string FormWindow::getFieldValue(QWidget const * const widget) const
{
	static std::string emptyString;
	if(!widget)
		return emptyString;

	QLineEdit const * const edit = dynamic_cast<QLineEdit const * const>(widget);
	if(edit)
		return std::string(edit->text().latin1());
	else
		return emptyString;
}

//-----------------------------------------------------------------------

void FormWindow::onOkPressed()
{
	if(gatherAndSendData())
		accept();
}

//-----------------------------------------------------------------------

void FormWindow::onCancelPressed()
{
	reject();
}

//-----------------------------------------------------------------------

bool FormWindow::validateData(std::map<std::string, std::string> const & pendingData)
{
	if(!m_orderedFieldList || !m_fieldMap)
		return false;
	for(std::vector<FormManager::Field const *>::iterator i = m_orderedFieldList->begin(); i != m_orderedFieldList->end(); ++i)
	{
		FormManager::Field const * const f = *i;
		if(f)
		{
			std::map<FormManager::Field const *, QWidget *>::iterator i2 = m_fieldMap->find(f);
			if(i2 != m_fieldMap->end())
			{
				QWidget * const widget = i2->second;
				if(!widget)
					return false;
				std::string const value = getFieldValue(widget);
				std::string errorString;
				bool const result = validateField(*f, value, pendingData, errorString);
				if(!result)
				{
					IGNORE_RETURN(QMessageBox::information (0, "Validation Failed", errorString.c_str(), QMessageBox::Ok));
					widget->setFocus();

					QLineEdit * const lineEdit = dynamic_cast<QLineEdit * const>(widget);
					if(lineEdit)
					{
						lineEdit->selectAll();
					}

					return false;
				}
			}
			else
				return false;
		}
		else
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------

bool FormWindow::validateField(FormManager::Field const & field, std::string const & value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const
{
	if(field.isIntType())
		return validateFieldAsInt(field, value, pendingData, errorString);
	else if(field.isFloatType())
		return validateFieldAsFloat(field, value, pendingData, errorString);
	else if(field.isStringType())
		return validateFieldAsString(field, value, pendingData, errorString);
	else
	{
		DEBUG_FATAL(true, ("Unknown field type encountered in FormWindow::validateField"));
		return false; //lint !e527 reachable in release
	}
}

//-----------------------------------------------------------------------

bool FormWindow::validateFieldAsInt(FormManager::Field const & field, std::string const & value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const
{
	if(!field.isIntType())
	{
		DEBUG_FATAL(true, ("field is not int type in FormWindow::validateFieldAsInt"));
		return false; //lint !e527 reachable in release
	}

	if(value.empty())
	{
		errorString = std::string("Field: \"") + field.getExternalName() + std::string("\" must be a valid int.");
		return false; //lint !e527 reachable in release
	}

	if(field.isArrayType())
	{
		//TODO handle arrays
		return true;
	}
	else
	{
		int const i = atoi(value.c_str());
		return field.isValidValue(i, pendingData, errorString);
	}
}

//-----------------------------------------------------------------------

bool FormWindow::validateFieldAsFloat(FormManager::Field const & field, std::string const & value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const
{
	if(!field.isFloatType())
	{
		DEBUG_FATAL(true, ("field is not float type in FormWindow::validateFieldAsFloat"));
		return false; //lint !e527 reachable in release
	}

	if(value.empty())
	{
		errorString = std::string("Field: \"") + field.getExternalName() + std::string("\" must be a valid float.");
		return false;
	}

	if(field.isArrayType())
	{
		//TODO handle arrays
		return true;
	}
	else
	{ 
		float const f = static_cast<float>(atof(value.c_str()));
		return field.isValidValue(f, pendingData, errorString);
	}
}

//-----------------------------------------------------------------------

bool FormWindow::validateFieldAsString(FormManager::Field const & field, std::string const & value, std::map<std::string, std::string> const & pendingData, std::string & errorString /*OUT*/) const
{
	if(!field.isStringType())
	{
		DEBUG_FATAL(true, ("field is not string type in FormWindow::validateFieldAsString"));
		return false; //lint !e527 reachable in release
	}

	for(std::vector<FormManager::OtherValidationRules>::const_iterator i = field.getOtherValidationRules().begin(); i != field.getOtherValidationRules().end(); ++i)
	{
		if(*i == FormManager::NOT_EMPTY)
		{
			if(value.empty())
			{
				errorString = std::string("Field: \"") + field.getExternalName() + std::string("\" must not be empty.");
				return false;
			}
		}
	}

	if(field.isArrayType())
	{
		//TODO handle arrays
		return true;
	}
	else
	{
		return field.isValidValue(value, pendingData, errorString);
	}
}

// ======================================================================
