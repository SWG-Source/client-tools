// ======================================================================
//
// Quest.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "Quest.h"
#include "QuestEditorConstants.h"

// ----------------------------------------------------------------------

#include <qdom.h>
#include <qfile.h>
#include <qmessagebox.h>

// ----------------------------------------------------------------------

Quest::Quest()
: QDomDocument()
{
}

// ----------------------------------------------------------------------

Quest::~Quest()
{
}

// ----------------------------------------------------------------------

bool Quest::loadQuest(char const * const filename)
{
	if (filename)
	{
		//-- read the XML file and create the DOM tree
		QFile opmlFile(filename);
		if (!opmlFile.open(IO_ReadOnly))
		{
			IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"), QObject::tr("Cannot open file %1").arg(filename)));
			return false;
		}

		if (!setContent(&opmlFile))
		{
			IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"), QObject::tr("Parsing error for file %1").arg(filename)));
			opmlFile.close();
			return false;
		}
		opmlFile.close();

		//-- @TODO: add version code here
	}
	else
	{
		QDomElement root = createElement(cs_Quest);

		IGNORE_RETURN(appendChild(root));

		root.setAttribute("version", "1.0");
	}

	return true;
}

// ----------------------------------------------------------------------

bool Quest::saveQuest(char const * const filename) const
{
	//-- read the XML file and create the DOM tree
	QFile opmlFile(filename);
	if (!opmlFile.open(IO_WriteOnly))
	{
		IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"), QObject::tr("Cannot open file %1").arg(filename)));
		return false;
	}

	QTextStream textOStream(&opmlFile);

	//-- output the header
	textOStream << "<?xml version=\"1.0\"?>" << endl;

	documentElement().save(textOStream, 4);

	/*
	{
		QDomNodeList nodes = childNodes();

		for (uint i=0; i<nodes.count(); ++i)
			nodes.item(i).save(textOStream, 4);
	}
	*/

	opmlFile.close();

	return true;
}

// ----------------------------------------------------------------------

int Quest::getUniqueTaskId() const
{
	QDomElement tasks = getTasks().toElement();
	int returnId = tasks.attribute(cs_AvailableId, "0").toInt();

	WARNING(!tasks.hasAttribute(cs_AvailableId),
		("Quest::getUniqueTaskId() - [%s] not found.", cs_AvailableId));

	tasks.setAttribute(cs_AvailableId, returnId + 1);

	return returnId;
}

// ----------------------------------------------------------------------

QDomNode Quest::getTasks() const
{
	QDomNode tasks = documentElement().firstChild();

	while(!tasks.isNull() && tasks.nodeName() != cs_Tasks)
		tasks = tasks.nextSibling();

	return tasks;
}

// ----------------------------------------------------------------------

QDomNode Quest::getList() const
{
	QDomNode list = documentElement().firstChild();

	while(!list.isNull() && list.nodeName() != cs_List)
		list = list.nextSibling();

	return list;
}

// ======================================================================
