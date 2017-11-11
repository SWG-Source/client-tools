// ======================================================================
//
// MayaAnimationList.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaAnimationList.h"

#include <algorithm>
#include <map>
#include <list>
#include "maya/MDagPath.h"
#include "maya/MFnAttribute.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MObject.h"
#include "maya/MPlug.h"
#include "maya/MString.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "sharedFoundation/PointerDeleter.h"

// ======================================================================

namespace
{
	const char *const ms_firstFrameAttributePrefix = "soe_ff_";
	const char *const ms_lastFrameAttributePrefix  = "soe_lf_";
	const char *const ms_exportNodeAttributePrefix = "soe_en_";

	const int         ms_bogusFrameNumber          = -10000;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool       ms_installed;

	Messenger *messenger;

	MString   *ms_mayaFirstFrameAttributePrefix;
	unsigned   ms_mayaFirstFrameAttributePrefixLength;

	MString   *ms_mayaLastFrameAttributePrefix;
	unsigned   ms_mayaLastFrameAttributePrefixLength;

	MString   *ms_mayaExportNodeAttributePrefix;
	unsigned   ms_mayaExportNodeAttributePrefixLength;
}

// ======================================================================
// class MayaAnimationList::AnimationInfo
// ======================================================================

MayaAnimationList::AnimationInfo::AnimationInfo(const char *animationName)
:	m_animationName(animationName),
	m_exportNodeName(),
	m_firstFrame(ms_bogusFrameNumber),
	m_lastFrame(ms_bogusFrameNumber)
{
}

// ----------------------------------------------------------------------

MayaAnimationList::AnimationInfo::AnimationInfo(const char *animationName, const char *exportNodeName, int firstFrameNumber, int lastFrameNumber)
:	m_animationName(animationName),
	m_exportNodeName(exportNodeName),
	m_firstFrame(firstFrameNumber),
	m_lastFrame(lastFrameNumber)
{
}

// ======================================================================
// class MayaAnimationList
// ======================================================================

void MayaAnimationList::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("MayaAnimationList already installed"));

	messenger = newMessenger;

	ms_mayaFirstFrameAttributePrefix       = new MString(ms_firstFrameAttributePrefix);
	ms_mayaFirstFrameAttributePrefixLength = ms_mayaFirstFrameAttributePrefix->length();

	ms_mayaLastFrameAttributePrefix        = new MString(ms_lastFrameAttributePrefix);
	ms_mayaLastFrameAttributePrefixLength  = ms_mayaLastFrameAttributePrefix->length();

	ms_mayaExportNodeAttributePrefix       = new MString(ms_exportNodeAttributePrefix);
	ms_mayaExportNodeAttributePrefixLength = ms_mayaExportNodeAttributePrefix->length();

	ms_installed = true;
}

// ----------------------------------------------------------------------

void MayaAnimationList::remove()
{
	DEBUG_FATAL(!ms_installed, ("MayaAnimationList not installed"));
	
	delete ms_mayaFirstFrameAttributePrefix;
	ms_mayaFirstFrameAttributePrefix = 0;

	delete ms_mayaLastFrameAttributePrefix;
	ms_mayaLastFrameAttributePrefix = 0;

	delete ms_mayaExportNodeAttributePrefix;

}

// ======================================================================

MayaAnimationList::MayaAnimationList(MDagPath &storageDagPath, MStatus *status)
:	m_animationInfoMap(new AnimationInfoMap()),
	m_storageDependencyNode(new MFnDependencyNode())
{
	DEBUG_FATAL(!ms_installed, ("MayaAnimationList not installed"));

	MStatus localStatus;

	//-- get maya object from dag node
	MObject object = storageDagPath.node(&localStatus);
	if (localStatus)
	{
		//-- set object for the DN function set
		localStatus = m_storageDependencyNode->setObject(object);
		if (localStatus)
		{
			// -- load the animation map from the node
			const bool result = loadMapFromStorage();
			if (!result)
				localStatus = MStatus::kFailure;
		}
	}

	//-- log any errors
	if (!localStatus)
		MESSENGER_LOG_ERROR(("failed to create maya animation list from node [%s]\n", storageDagPath.partialPathName().asChar()));

	if (status)
		*status = localStatus;
}

// ----------------------------------------------------------------------

MayaAnimationList::~MayaAnimationList()
{
	delete m_storageDependencyNode;

	std::for_each(m_animationInfoMap->begin(), m_animationInfoMap->end(), PointerDeleterPairSecond());
	delete m_animationInfoMap;
}

// ----------------------------------------------------------------------

bool MayaAnimationList::findOrCreateAnimationInfo(const CrcLowerString &animationName, AnimationInfo **animationInfo)
{
	NOT_NULL(animationInfo);

	AnimationInfoMap::iterator it = m_animationInfoMap->find(&animationName);
	if (it == m_animationInfoMap->end())
	{
		// create it and add to animation map
		AnimationInfo *const newAnimationInfo = new AnimationInfo(animationName.getString());
		std::pair<AnimationInfoMap::iterator, bool> insertResult = m_animationInfoMap->insert(AnimationInfoMap::value_type(&newAnimationInfo->m_animationName, newAnimationInfo));

		if (!insertResult.second)
		{
			delete newAnimationInfo;
			MESSENGER_LOG(("both find and insert failed for animation list entry [%s]\n", animationName.getString()));
		}
		it = insertResult.first;
	}

	*animationInfo = it->second;
	return true;
}

// ----------------------------------------------------------------------

bool MayaAnimationList::loadMapFromStorage()
{
	MESSENGER_INDENT;
	MStatus status;

	//-- clear out existing map
	std::for_each(m_animationInfoMap->begin(), m_animationInfoMap->end(), PointerDeleterPairSecond());
	m_animationInfoMap->clear();	

	//-- find each attribute that matches the prefix for specifying the beginning of an animation range
	{
		const unsigned attributeCount = m_storageDependencyNode->attributeCount(&status);
		MESSENGER_REJECT(!status, ("failed to get attribute count for animation list storage node\n"));

		MFnAttribute    fnAttribute;
		CrcLowerString  animationName("");

		for (unsigned i = 0; i < attributeCount; ++i)
		{
			//-- get the attribute
			MObject attributeObject = m_storageDependencyNode->attribute(i, &status);
			MESSENGER_REJECT(!status, ("failed to get attribute [%u]\n", i));

			status = fnAttribute.setObject(attributeObject);
			MESSENGER_REJECT(!status, ("failed to set object into MFnAttribute\n"));

			//-- check if name starts with one of our magic strings
			MString attributeName = fnAttribute.name(&status);
			MESSENGER_REJECT(!status, ("failed to get attribute name\n"));
			const unsigned attributeNameLength = attributeName.length();
			IGNORE_RETURN(attributeName.toLowerCase());

			if (
				(attributeNameLength > ms_mayaFirstFrameAttributePrefixLength) && 
				(attributeName.substring(0, static_cast<int>(ms_mayaFirstFrameAttributePrefixLength-1)) == *ms_mayaFirstFrameAttributePrefix))
			{
				//-- this attribute specifies the first frame for an animation

				// get the AnimationInfo record for this
				MString mayaAnimationName = attributeName.substring(static_cast<int>(ms_mayaFirstFrameAttributePrefixLength), static_cast<int>(attributeNameLength-1));
				animationName.setString(mayaAnimationName.asChar());

				AnimationInfo *animationInfo = 0;
				const bool     focResult     = findOrCreateAnimationInfo(animationName, &animationInfo);
				MESSENGER_REJECT(!focResult, ("failed to find or create animation info for [%s]\n", animationName.getString()));
				NOT_NULL(animationInfo);

				//-- get the value for this attribute
				MPlug plug = m_storageDependencyNode->findPlug(attributeObject, &status);
				MESSENGER_REJECT(!status, ("failed to find plug for attribute [%s]\n", attributeName.asChar()));

				int value  = 0;
				status     = plug.getValue(value);
				MESSENGER_REJECT(!status, ("failed to get long value for attribute [%s]\n", attributeName.asChar()));

				animationInfo->m_firstFrame = value;
			}
			else if (
				(attributeNameLength > ms_mayaLastFrameAttributePrefixLength) && 
				(attributeName.substring(0, static_cast<int>(ms_mayaLastFrameAttributePrefixLength-1)) == *ms_mayaLastFrameAttributePrefix))
			{
				//-- this attribute specifies the last frame for an animation

				// get the AnimationInfo record for this
				MString mayaAnimationName = attributeName.substring(static_cast<int>(ms_mayaLastFrameAttributePrefixLength), static_cast<int>(attributeNameLength-1));
				animationName.setString(mayaAnimationName.asChar());

				AnimationInfo *animationInfo = 0;
				const bool     focResult     = findOrCreateAnimationInfo(animationName, &animationInfo);
				MESSENGER_REJECT(!focResult, ("failed to find or create animation info for [%s]\n", animationName.getString()));
				NOT_NULL(animationInfo);

				//-- get the value for this attribute
				MPlug plug = m_storageDependencyNode->findPlug(attributeObject, &status);
				MESSENGER_REJECT(!status, ("failed to find plug for attribute [%s]\n", attributeName.asChar()));

				int value  = 0;
				status     = plug.getValue(value);
				MESSENGER_REJECT(!status, ("failed to get long value for attribute [%s]\n", attributeName.asChar()));

				animationInfo->m_lastFrame = value;
			}
			else if (
				(attributeNameLength > ms_mayaExportNodeAttributePrefixLength) && 
				(attributeName.substring(0, static_cast<int>(ms_mayaExportNodeAttributePrefixLength-1)) == *ms_mayaExportNodeAttributePrefix))
			{
				//-- this attribute specifies the node to select for exporting animation

				// get the AnimationInfo record for this
				MString mayaAnimationName = attributeName.substring(static_cast<int>(ms_mayaExportNodeAttributePrefixLength), static_cast<int>(attributeNameLength-1));
				animationName.setString(mayaAnimationName.asChar());

				AnimationInfo *animationInfo = 0;
				const bool     focResult     = findOrCreateAnimationInfo(animationName, &animationInfo);
				MESSENGER_REJECT(!focResult, ("failed to find or create animation info for [%s]\n", animationName.getString()));
				NOT_NULL(animationInfo);

				//-- get the value for this attribute
				MPlug plug = m_storageDependencyNode->findPlug(attributeObject, &status);
				MESSENGER_REJECT(!status, ("failed to find plug for attribute [%s]\n", attributeName.asChar()));

				MString  value;
				status = plug.getValue(value);
				MESSENGER_REJECT(!status, ("failed to get string value for attribute [%s]\n", attributeName.asChar()));

				animationInfo->m_exportNodeName = value.asChar();
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool MayaAnimationList::deleteStorageEntries()
{
	typedef std::list<MObject>    ObjectList;
	typedef ObjectList::iterator  ObjectListIterator;

	MESSENGER_INDENT;
	MStatus status;

	//-- find each attribute that matches the prefix for specifying the beginning of an animation range
	const unsigned attributeCount = m_storageDependencyNode->attributeCount(&status);
	MESSENGER_REJECT(!status, ("failed to get attribute count for animation list storage node\n"));

	ObjectList  attributesToDelete;

	MString attributeName;
	for (unsigned i = 0; i < attributeCount; ++i)
	{
		bool deleteThisAttribute = false;

		//-- get the attribute
		MObject attributeObject = m_storageDependencyNode->attribute(i, &status);
		MESSENGER_REJECT(!status, ("failed to get attribute [%u]\n", i));

		{
			MFnAttribute fnAttribute(attributeObject, &status);
			//status = fnAttribute.setObject(attributeObject);
			MESSENGER_REJECT(!status, ("failed to set object into MFnAttribute\n"));

			//-- check if name starts with one of our magic strings
			attributeName = fnAttribute.name(&status);
			MESSENGER_REJECT(!status, ("failed to get attribute name\n"));
			const unsigned attributeNameLength = attributeName.length();
			IGNORE_RETURN(attributeName.toLowerCase());

			if (
				(attributeNameLength > ms_mayaFirstFrameAttributePrefixLength) && 
				(attributeName.substring(0, static_cast<int>(ms_mayaFirstFrameAttributePrefixLength-1)) == *ms_mayaFirstFrameAttributePrefix))
			{
				//-- this attribute specifies the first frame for an animation
				deleteThisAttribute = true;
			}
			else if (
				(attributeNameLength > ms_mayaLastFrameAttributePrefixLength) && 
				(attributeName.substring(0, static_cast<int>(ms_mayaLastFrameAttributePrefixLength-1)) == *ms_mayaLastFrameAttributePrefix))
			{
				//-- this attribute specifies the last frame for an animation
				deleteThisAttribute = true;
			}
			else if (
				(attributeNameLength > ms_mayaExportNodeAttributePrefixLength) && 
				(attributeName.substring(0, static_cast<int>(ms_mayaExportNodeAttributePrefixLength-1)) == *ms_mayaExportNodeAttributePrefix))
			{
				//-- this attribute specifies the node to select for exporting animation
				deleteThisAttribute = true;
			}
		}

		if (deleteThisAttribute)
		{
			attributesToDelete.push_back(attributeObject);
		}
	}

	//-- now delete them
	const ObjectListIterator itEnd = attributesToDelete.end();
	for (ObjectListIterator it = attributesToDelete.begin(); it != itEnd; ++it)
	{
		MObject &object = *it;
		status = m_storageDependencyNode->removeAttribute(object);
		MayaUtility::clearMayaObjectHack(&object);
		MESSENGER_REJECT(!status, ("failed to remove attribute [%s] from animation list storage node\n", attributeName.asChar()));
	}

	return true;
}

// ----------------------------------------------------------------------

bool MayaAnimationList::saveMapToStorage()
{
	MESSENGER_INDENT;
	MStatus status;

	//-- delete existing storage nodes related to the animation list
	const bool deleteResult = deleteStorageEntries();
	MESSENGER_REJECT(!deleteResult, ("failed to delete existing storage entries\n"));

	//-- save current animation info list to storage node
	AnimationInfoMap::const_iterator itEnd = m_animationInfoMap->end();
	for (AnimationInfoMap::const_iterator it = m_animationInfoMap->begin(); it != itEnd; ++it)
	{
		AnimationInfo &animationInfo = *NON_NULL(it->second);

		MString attributeName;
		MString shortName;
		//-- add first frame attribute
		{
			// create the attribute
			attributeName      = ms_firstFrameAttributePrefix;
			attributeName     += animationInfo.m_animationName.getString();

			shortName          = animationInfo.m_animationName.getString();
			shortName         += " (f)";

			MFnNumericAttribute  fnNumericAttribute;
			MObject attribute  = fnNumericAttribute.create(attributeName, shortName, MFnNumericData::kLong);

			// add attribute to storage node
			status = m_storageDependencyNode->addAttribute(attribute);
			MESSENGER_REJECT(!status, ("failed to add attribute [%s] to storage node\n", attributeName.asChar()));

			// set value
			MPlug plug = m_storageDependencyNode->findPlug(attribute, &status);
			MESSENGER_REJECT(!status, ("failed to find plug for attribute that we just added [%s]\n", attributeName.asChar()));

			long value = static_cast<long>(animationInfo.m_firstFrame);
			status     = plug.setValue(value);
			MESSENGER_REJECT(!status, ("failed to set value [%d] for attribute [%s]\n", value, attributeName.asChar()));
		}

		//-- add last frame attribute
		{
			// create the attribute
			attributeName      = ms_lastFrameAttributePrefix;
			attributeName     += animationInfo.m_animationName.getString();

			shortName          = animationInfo.m_animationName.getString();
			shortName         += " (l)";

			MFnNumericAttribute  fnNumericAttribute;
			MObject attribute  = fnNumericAttribute.create(attributeName, shortName, MFnNumericData::kLong);

			// add attribute to storage node
			status = m_storageDependencyNode->addAttribute(attribute);
			MESSENGER_REJECT(!status, ("failed to add attribute [%s] to storage node\n", attributeName.asChar()));

			// set value
			MPlug plug = m_storageDependencyNode->findPlug(attribute, &status);
			MESSENGER_REJECT(!status, ("failed to find plug for attribute that we just added [%s]\n", attributeName.asChar()));

			long value = static_cast<long>(animationInfo.m_lastFrame);
			status     = plug.setValue(value);
			MESSENGER_REJECT(!status, ("failed to set value [%d] for attribute [%s]\n", value, attributeName.asChar()));
		}

		//-- add export node name attribute
		{
			// create the attribute
			attributeName      = ms_exportNodeAttributePrefix;
			attributeName     += animationInfo.m_animationName.getString();

			shortName          = animationInfo.m_animationName.getString();
			shortName         += " (n)";

			MFnTypedAttribute    fnTypedAttribute;
			MObject attribute  = fnTypedAttribute.create(attributeName, shortName, MFnData::kString);

			// add attribute to storage node
			status = m_storageDependencyNode->addAttribute(attribute);
			MESSENGER_REJECT(!status, ("failed to add attribute [%s] to storage node\n", attributeName.asChar()));

			// set value
			MPlug plug = m_storageDependencyNode->findPlug(attribute, &status);
			MESSENGER_REJECT(!status, ("failed to find plug for attribute that we just added [%s]\n", attributeName.asChar()));

			MString value = animationInfo.m_exportNodeName.c_str();
			status        = plug.setValue(value);
			MESSENGER_REJECT(!status, ("failed to set value [%s] for attribute [%s]\n", value.asChar(), attributeName.asChar()));
		}
	}

	return true;
}

// ======================================================================
