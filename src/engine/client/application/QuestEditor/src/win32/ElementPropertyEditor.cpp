// ======================================================================
//
// ElementPropertyEditor.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "ElementPropertyEditor.h"

// ----------------------------------------------------------------------

#include "PropertyCompoundItem.h"
#include "PropertyList/PropertyBoolItem.h"
#include "PropertyList/PropertyDoubleItem.h"
#include "PropertyList/PropertyFileItem.h"
#include "PropertyList/PropertyIntItem.h"
#include "PropertyList/PropertyItem.h"
#include "PropertyList/PropertyListItem.h"
#include "PropertyList/PropertyTextItem.h"
#include "PropertyRelativeFileItem.h"
#include "QuestEditorConstants.h"
#include "QuestInputStringValidator.h"

// ----------------------------------------------------------------------

#include "sharedFile/Iff.h"
#include "sharedUtility/DataTable.h"

// ----------------------------------------------------------------------

#include <algorithm>

// ----------------------------------------------------------------------

#include <qregexp.h>

// ----------------------------------------------------------------------

#include "ElementPropertyEditor.moc"

// ----------------------------------------------------------------------

namespace ElementPropertyEditorNamespace
{
	// ----------------------------------------------------------------------

	bool stringPointerGreater(const char * string1, const char * string2)
	{
		return strcmp(string1, string2)>0;
	}

	// ----------------------------------------------------------------------
	
	bool stringPointerLess(const char * string1, const char * string2)
	{
		return strcmp(string1, string2)<0;
	}

	// ----------------------------------------------------------------------
	
	bool stringPointerEqual(const char * string1, const char * string2)
	{
		return strcmp(string1, string2)==0;
	}

	// ----------------------------------------------------------------------
}

using namespace ElementPropertyEditorNamespace;

// ----------------------------------------------------------------------

ElementPropertyEditor::ElementPropertyEditor(QWidget * elementPropertyEditorParent)
: PropertyList(elementPropertyEditorParent)
{
}

// ----------------------------------------------------------------------

ElementPropertyEditor::~ElementPropertyEditor()
{
}

// ----------------------------------------------------------------------

void ElementPropertyEditor::setElement(QDomElement element)
{
	clear();

	m_element = element;

	setupProperties();
}

// ----------------------------------------------------------------------

bool ElementPropertyEditor::addElementProperty(PropertyItem *& item, PropertyItem * parentItem, char const * const displayName, char const * const valueName, QDomElement configInputElement)
{
	// return value
	bool skipSetValue = false;
	bool skipInitItem = false;

	QuestEditorConfig::QuestInputType qit = QuestEditorConfig::getQITFromString(configInputElement.attribute(cs_Type));

	switch (qit)
	{
	case QuestEditorConfig::QIT_Bool:
		item = new PropertyBoolItem(this, item, parentItem, displayName);
		break;
	case QuestEditorConfig::QIT_Compound:
		// This input type is made up of multiple inputs
		item = new PropertyCompoundItem(this, item, parentItem, displayName, configInputElement);
		item->createChildren();
		item->removeChildren();
		skipSetValue = true;
		break;
	case QuestEditorConfig::QIT_DataTableColumn:
		{
			typedef std::vector<const char *> stringVector;

			//-- read the values from the data table and populate the combo box
			PropertyListItem * list = new PropertyListItem(this, item, parentItem, displayName, false);
			item = list;

			Iff iff(configInputElement.attribute(cs_DataTable), false);
			DataTable dataTable;
			dataTable.load(iff);

			int const column = dataTable.findColumnNumber(configInputElement.attribute(cs_DataTableColumn).ascii());
			stringVector strings;
			dataTable.getStringColumn(column, strings);

			std::sort(strings.begin(), strings.end(), stringPointerLess);
			stringVector::iterator uniqueEnd = std::unique(strings.begin(), strings.end(), stringPointerEqual);

			//-- add optional empty string
			if (configInputElement.attribute(cs_DontAddEmptyString) != "true")
				list->addItem("");

			for (stringVector::iterator it=strings.begin() ; it != uniqueEnd; ++it)
				list->addItem((*it));
		}
		break;
	case QuestEditorConfig::QIT_Double:
		{
			PropertyDoubleItem * doubleItem = new PropertyDoubleItem(this, item, parentItem, displayName);

			if (configInputElement.hasAttribute(cs_MinValue))
				doubleItem->setMinValue(configInputElement.attribute(cs_MinValue).toDouble());

			if (configInputElement.hasAttribute(cs_MaxValue))
				doubleItem->setMaxValue(configInputElement.attribute(cs_MaxValue).toDouble());

			item = doubleItem;
		}
		break;
	case QuestEditorConfig::QIT_Enum:
		{
			PropertyListItem * list = new PropertyListItem(this, item, parentItem, displayName, false);
			item = list;

			//-- enumerated type is a combo box with inputs pulled from the data table column type
			QDomElement configDataNode = configInputElement.parentNode().toElement();

			QString columnType = configDataNode.attribute(cs_ColumnType);

			//-- format: e(Fa=0,Fb=1,...,Fn=M)[Fa]
			QRegExp enumTypeExp("e\\(([^)]*)\\).*");
			QRegExp delimiterExp("=\\d+,?");

			if (enumTypeExp.search(columnType) != -1)
			{
				QStringList items = QStringList::split(delimiterExp, enumTypeExp.cap(1));

				for (uint i=0; i<items.count(); ++i)
				{
					list->addItem(items[i]);
				}
			}
			else
			{
				WARNING(true, ("Invalid enumerated column type [%s]\n", columnType.ascii()));
			}
		}
		break;
	case QuestEditorConfig::QIT_File:
		item = new PropertyFileItem(this, item, parentItem, displayName, configInputElement.attribute(cs_DefaultDirectory), configInputElement.attribute(cs_DefaultExtension));
		break;
	case QuestEditorConfig::QIT_List:
		{
			PropertyListItem * list = new PropertyListItem(this, item, parentItem, displayName, false);
			item = list;

			if (configInputElement.attribute(cs_AddEmptyString) == "true")
				list->addItem("");

			//-- list inputs define items that are added as combo box choices
			for (QDomNode itemNode = configInputElement.firstChild(); !itemNode.isNull(); itemNode = itemNode.nextSibling())
			{
				QDomText textChild = itemNode.firstChild().toText();
				if (!textChild.isNull())
				{
					list->addItem(textChild.nodeValue());
				}
			}
		}
		break;
	case QuestEditorConfig::QIT_Int:
		{
			PropertyIntItem * intItem = new PropertyIntItem(this, item, parentItem, displayName, true);

			if (configInputElement.hasAttribute(cs_MinValue))
				intItem->setMinValue(configInputElement.attribute(cs_MinValue).toInt());

			if (configInputElement.hasAttribute(cs_MaxValue))
				intItem->setMaxValue(configInputElement.attribute(cs_MaxValue).toInt());

			item = intItem;
		}
		break;
	case QuestEditorConfig::QIT_None:
		//-- no input control needed and no value should be set for this input on the node
		skipSetValue = true;
		skipInitItem = true;
		break;
	case QuestEditorConfig::QIT_ReadOnlyString:
		item = new PropertyTextItem(this, item, parentItem, displayName, false, false);
		item->setEnabled(false);
		break;
	case QuestEditorConfig::QIT_RelativeFile:
		{
			QString defaultDirectory = QuestEditorConfig::getFileAttribute(configInputElement, cs_DefaultDirectory, false);
			item = new PropertyRelativeFileItem(this, item, parentItem, displayName, defaultDirectory, configInputElement.attribute(cs_DefaultExtension));
		}
		break;
	case QuestEditorConfig::QIT_String:
		{
			PropertyTextItem * textItem = new PropertyTextItem(this, item, parentItem, displayName, false, false);

			bool allowSpaces = configInputElement.attribute(cs_AllowSpaces) != "false";

			textItem->setValidator(new QuestInputStringValidator(this, 0, allowSpaces));

			item = textItem;
		}
		break;
	case QuestEditorConfig::QIT_StringList:
		//-- need to construct string list using input data
		item = new PropertyTextItem(this, item, parentItem, displayName, false, false);
		break;
	default:
		WARNING(true, ("Unknown type [%s]", configInputElement.attribute(cs_Type).ascii()));
		skipSetValue = true;
		skipInitItem = true;
		break;
	}

	if (!skipInitItem)
	{
		item->setValueName(valueName);

		QDomElement configDataNode = configInputElement.parentNode().toElement();

		if (configDataNode.hasAttribute(cs_Info))
			item->setWhatsThisText(configDataNode.attribute(cs_Info));
	}

	return skipSetValue;
}

// ----------------------------------------------------------------------

void ElementPropertyEditor::clearElement()
{
	m_element.clear();

	PropertyList::clear();
}

// ----------------------------------------------------------------------

void ElementPropertyEditor::valueChanged(PropertyItem *i)
{
	QString valueString = i->value().asString();

	//-- change the value in the element corresponding to this property item
	DEBUG_REPORT_LOG_PRINT(false, ("ElementPropertyEditor::valueChanged() - Name = [%s] Value = [%s]\n", i->getValueName().ascii(), valueString.ascii()));

	QDomNode data = m_element.firstChild();

	while (!data.isNull())
	{
		if (data.nodeName() == cs_Data && data.toElement().attribute(cs_Name) == i->getValueName())
		{
			data.toElement().setAttribute(cs_Value, valueString);
			emit dirty();
			return;
		}

		data = data.nextSibling();
	}

	if (m_element.hasAttribute(i->getValueName()))
	{
		m_element.setAttribute(i->getValueName(), valueString);
		emit dirty();
		return;
	}

	FATAL(true, ("PropertyItem not found in element data [%s].\n", i->getValueName().ascii()));
}

// ----------------------------------------------------------------------

QVariant ElementPropertyEditor::getResetValue(PropertyItem *item)
{
	UNREF(item);

	//-- TODO: Should reset to default value.

	return QVariant("");
}

// ----------------------------------------------------------------------

void ElementPropertyEditor::setupInput(PropertyItem *& item, QDomNode configDataNode, PropertyItem * parentItem)
{
	QDomNode configInputNode = configDataNode.firstChild();

	while(!configInputNode.isNull())
	{
		if(configInputNode.nodeName() == cs_Input)
		{
			QDomElement dataElement = configDataNode.toElement();
			QDomElement configInputElement = configInputNode.toElement();

			QString const name = dataElement.attribute(cs_Name).ascii();
			QString const displayName = dataElement.attribute(cs_DisplayName, name).ascii();

			bool skipSetValue = addElementProperty(item, parentItem, displayName, name, configInputElement);

			if (!skipSetValue)
			{
				//-- see if it's an attribute
				if (m_element.hasAttribute(name))
				{
					item->setValue(m_element.attribute(name));
				}
				else
				{
					//-- set the input controls values
					QDomNode node = m_element.firstChild();

					while (!node.isNull())
					{
						if (node.nodeName() == cs_Data)
						{
							QDomElement element = node.toElement();

							if (element.attribute(cs_Name) == name)
							{
								item->setValue(element.attribute(cs_Value));
								break;
							}
						}

						node = node.nextSibling();
					}

					//-- if no value is defined, use the default and warn the user
					if (node.isNull())
					{
						QDomElement newDataElement = m_element.toDocument().createElement(cs_Data);

						char const * const defaultValue = dataElement.attribute(cs_DefaultValue);

						newDataElement.setAttribute(cs_Value, defaultValue);
						newDataElement.setAttribute(cs_Name, name);

						m_element.appendChild(newDataElement);

						emit dirty();

						item->setValue(defaultValue);

						DEBUG_REPORT_LOG(true, ("Element is missing [%s]. Using default [%s]\n", name.ascii(), defaultValue));
					}
				}
			}
		}

		configInputNode = configInputNode.nextSibling();
	}
}

// ----------------------------------------------------------------------

void ElementPropertyEditor::openParentCompoundInput(QDomElement configDataElement)
{
	QDomElement parentInputElement = configDataElement.parentNode().toElement();

	if (parentInputElement.nodeName() == cs_Input && parentInputElement.attribute(cs_Type) == "QIT_Compound")
	{
		QDomElement parentDataElement = parentInputElement.parentNode().toElement();
		QString name = parentDataElement.attribute(cs_Name);
		
		openParentCompoundInput(parentDataElement);
		setCurrentProperty(name, true);
		getCurrentPropertyItem()->setOpen(true);
	}
}

// ======================================================================
