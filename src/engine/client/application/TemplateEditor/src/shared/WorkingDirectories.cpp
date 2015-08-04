// ---------------------------------------------------------------------------=
//
//	WorkingDirectories.cpp
//
//	Copyright Sony Online Entertainment
//
// ---------------------------------------------------------------------------=

#include "FirstTemplateEditor.h"
#include "WorkingDirectories.h"

#include "sharedTemplateDefinition/templateData.h"
#include "sharedTemplateDefinition/templateGlobals.h"
#include "templateLoader.h"

//-----------------------------------------------------------------------------
//namespace WorkingDirectories
//{
//	const QDir &
//	getCurrentDirectory()
//	{
//		static QDir swgBase(QDir::current());
//		static QDir currentDir;
//
//		// Make sure swgBase points to base swg folder
//		while(swgBase.dirName() != "swg")
//		{
//			bool cdUpSucceeded = swgBase.cdUp();
//
//			if(!cdUpSucceeded)
//			{
//				break;
//			}
//		}
//
//		TemplateData *currentDefinition = TemplateLoader::getTemplateData();
//
//		switch(currentDefinition->getTemplateLocation())
//		{
//			case LOC_CLIENT:
//				{
//					currentDir.setPath(swgBase.absPath() + client);
//				}
//				break;
//			case LOC_SERVER:
//				{
//					currentDir.setPath(swgBase.absPath() + server);
//				}
//				break;
//			case LOC_SHARED:
//				{
//					currentDir.setPath(swgBase.absPath() + shared);
//				}
//				break;
//			default:
//				{
//					currentDir.setPath(swgBase.absPath());
//				}
//				break;
//		}
//
//		return currentDir;
//	}
//}

// ---------------------------------------------------------------------------=
