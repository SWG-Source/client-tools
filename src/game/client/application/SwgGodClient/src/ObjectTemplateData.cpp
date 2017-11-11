// ======================================================================
//
// ObjectTemplateData.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ObjectTemplateData.h"

#include "ConfigGodClient.h"
#include "GodClientPerforce.h"
#include "AbstractFilesystemTree.h"

#include <qmessagebox.h>

// ======================================================================

const char * const ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED = "ObjectTemplateData::DragMessages::SERVER_TEMPLATE_DRAGGED";
const char * const ObjectTemplateData::DragMessages::CLIENT_TEMPLATE_DRAGGED = "ObjectTemplateData::DragMessages::CLIENT_TEMPLATE_DRAGGED";

//----------------------------------------------------------------------

ObjectTemplateData::ObjectTemplateData() :
Singleton<ObjectTemplateData>(),
m_tree(0)
{
	m_tree = 0;
	repopulate();
}

//-----------------------------------------------------------------

ObjectTemplateData::~ObjectTemplateData()
{
	delete m_tree;
	m_tree = 0;
}

//-----------------------------------------------------------------

void  ObjectTemplateData::repopulate()
{
	if(m_tree)
	{
		delete m_tree;
		m_tree = 0;
	}

	const char * const templatePath = NON_NULL(ConfigGodClient::getData().templateServerIffPath);
	std::string result;
	m_tree = GodClientPerforce::getInstance().getFileTree(templatePath, "iff", result, GodClientPerforce::FileState_depot);

	if(!m_tree)
	{
		const std::string msg = "Unable to retrieve info from perforce:\n" + result;
		IGNORE_RETURN(QMessageBox::warning(0, "Warning", msg.c_str()));
		return;
	}
}

// ======================================================================
