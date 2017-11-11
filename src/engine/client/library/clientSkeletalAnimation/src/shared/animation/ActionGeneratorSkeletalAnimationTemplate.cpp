// ======================================================================
//
// ActionGeneratorSkeletalAnimationTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimationTemplate.h"

#include "clientSkeletalAnimation/ActionGeneratorSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateUiFactory.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedRandom/Random.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

namespace ActionGeneratorSkeletalAnimationTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_ACTN = TAG(A,C,T,N);
	Tag const TAG_AGAT = TAG(A,G,A,T);
	Tag const TAG_LOOP = TAG(L,O,O,P);

	char const *const cs_actionGeneratorAnimationElementName = "actionGeneratorAnimation";

	char const *const cs_frequencyRangeElementName           = "frequencyRange";
	char const *const cs_minTimeAttributeName                = "minTime";
	char const *const cs_maxTimeAttributeName                = "maxTime";

	char const *const cs_actionListElementName               = "actionList";
	char const *const cs_actionElementName                   = "action";
	char const *const cs_nameAttributeName                   = "name";
	char const *const cs_relativeWeightAttributeName         = "relativeWeight";

	char const *const cs_loopAnimationElementName            = "loopAnimation";
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string  s_actionName;
}

using namespace ActionGeneratorSkeletalAnimationTemplateNamespace;

// ======================================================================

bool                                ActionGeneratorSkeletalAnimationTemplate::ms_installed;
bool                                ActionGeneratorSkeletalAnimationTemplate::ms_logInfo;
SkeletalAnimationTemplateUiFactory *ActionGeneratorSkeletalAnimationTemplate::ms_uiFactory;

// ======================================================================

class ActionGeneratorSkeletalAnimationTemplate::ActionData
{
public:

	ActionData(const CrcLowerString &actionName, float relativeWeight);

	void                  setActionName(const CrcLowerString &newActionName);
	const CrcLowerString &getActionName() const;

	void                  setRelativeWeight(float relativeWeight);
	float                 getRelativeWeight() const;

	void                  setMaxPercentileRoll(float maxPercentileRoll);
	float                 getMaxPercentileRoll() const;

private:

	// Disabled.
	ActionData();

private:

	CrcLowerString  m_actionName;
	float           m_relativeWeight;
	float           m_maxPercentileRoll;

};

// ======================================================================
// class ActionGeneratorSkeletalAnimationTemplate::ActionData
// ======================================================================

ActionGeneratorSkeletalAnimationTemplate::ActionData::ActionData(const CrcLowerString &actionName, float relativeWeight) :
	m_actionName(actionName),
	m_relativeWeight(relativeWeight),
	m_maxPercentileRoll(0.0f)
{
}

// ----------------------------------------------------------------------

inline void ActionGeneratorSkeletalAnimationTemplate::ActionData::setActionName(const CrcLowerString &newActionName)
{
	if (strlen(newActionName.getString()) > 0)
		m_actionName = newActionName;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &ActionGeneratorSkeletalAnimationTemplate::ActionData::getActionName() const
{
	return m_actionName;
}

// ----------------------------------------------------------------------


inline void ActionGeneratorSkeletalAnimationTemplate::ActionData::setRelativeWeight(float relativeWeight)
{
	m_relativeWeight = relativeWeight;
}

// ----------------------------------------------------------------------

inline float ActionGeneratorSkeletalAnimationTemplate::ActionData::getRelativeWeight() const
{
	return m_relativeWeight;
}

// ----------------------------------------------------------------------

inline void ActionGeneratorSkeletalAnimationTemplate::ActionData::setMaxPercentileRoll(float maxPercentileRoll)
{
	m_maxPercentileRoll = maxPercentileRoll;
}

// ----------------------------------------------------------------------

inline float ActionGeneratorSkeletalAnimationTemplate::ActionData::getMaxPercentileRoll() const
{
	return m_maxPercentileRoll;
}

// ======================================================================
// class ActionGeneratorSkeletalAnimationTemplate: public static member functions
// ======================================================================

void ActionGeneratorSkeletalAnimationTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("ActionGeneratorSkeletalAnimationTemplate already installed"));

	//-- Register IFF reader.
	const bool result = SkeletalAnimationTemplateList::registerCreateFunction(TAG_AGAT, create);
	DEBUG_FATAL(!result, ("registerCreateFunction failed."));
	UNREF(result);

	//-- Register XML reader.
	SkeletalAnimationTemplateList::registerXmlCreateFunction(cs_actionGeneratorAnimationElementName, xmlCreate);

	DebugFlags::registerFlag(ms_logInfo, "ClientSkeletalAnimation/Character", "logActionGeneratorInfo");

	ms_installed = true;
	ExitChain::add(remove, "ActionGeneratorSkeletalAnimationTemplate");
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::setUiFactory(SkeletalAnimationTemplateUiFactory *uiFactory)
{
	DEBUG_FATAL(!ms_installed, ("ActionGeneratorSkeletalAnimationTemplate not installed."));

	//-- Check for assignment to self.
	if (ms_uiFactory == uiFactory)
		return;

	//-- Delete existing uiFactory.
	delete ms_uiFactory;

	//-- Assign new uiFactory.
	ms_uiFactory = uiFactory;
}

// ======================================================================
// class ActionGeneratorSkeletalAnimationTemplate: public member functions
// ======================================================================

SkeletalAnimation *ActionGeneratorSkeletalAnimationTemplate::fetchSkeletalAnimation(AnimationEnvironment &animationEnvironment, const TransformNameMap &transformNameMap) const
{
	SkeletalAnimation *const animation = new ActionGeneratorSkeletalAnimation(*this, animationEnvironment, transformNameMap);
	animation->fetch();

	return animation;
}

// ----------------------------------------------------------------------

float ActionGeneratorSkeletalAnimationTemplate::getElapsedTimeUntilNextAction() const
{
	return Random::randomReal(m_minActionElapsedTime, m_maxActionElapsedTime);
}

// ----------------------------------------------------------------------

const CrcLowerString &ActionGeneratorSkeletalAnimationTemplate::randomSelectActionName() const
{
	//-- Ensure there are entries.
	if (m_actionDataVector->empty())
		return CrcLowerString::empty;

	//-- Generate a number between 0 and 99.
	const float percentileRoll = Random::randomReal(100.0f);

	//-- Find the action that matches the percentile.
	const ActionDataVector::const_iterator endIt = m_actionDataVector->end();
	for (ActionDataVector::const_iterator it = m_actionDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		if (percentileRoll <= (*it)->getMaxPercentileRoll())
			return (*it)->getActionName();
	}

	//-- No action found.
	DEBUG_WARNING(true, ("No action found, percentile roll = %g, action count = %u", percentileRoll, m_actionDataVector->size()));
	return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplate *ActionGeneratorSkeletalAnimationTemplate::fetchLoopingAnimationTemplate() const
{
	if (m_loopingAnimationTemplate)
		m_loopingAnimationTemplate->fetch();

	return m_loopingAnimationTemplate;
}

// ----------------------------------------------------------------------

ActionGeneratorSkeletalAnimationTemplate::ActionGeneratorSkeletalAnimationTemplate() :
	SkeletalAnimationTemplate(CrcLowerString::empty),
	m_minActionElapsedTime(5.0f),
	m_maxActionElapsedTime(20.0f),
	m_actionDataVector(new ActionDataVector()),
	m_loopingAnimationTemplate(0)
{
}

// ----------------------------------------------------------------------

bool ActionGeneratorSkeletalAnimationTemplate::supportsWriting() const
{
	return true;
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::write(Iff &iff) const
{
	iff.insertForm(TAG_AGAT);
		iff.insertForm(TAG_0000);

			// Write action frequency data.
			iff.insertChunk(TAG_INFO);
				iff.insertChunkData(static_cast<float>(m_minActionElapsedTime));
				iff.insertChunkData(static_cast<float>(m_maxActionElapsedTime));
			iff.exitChunk(TAG_INFO);

			// Write action names and relative weights.
			if (!m_actionDataVector->empty())
			{
				iff.insertChunk(TAG_ACTN);
					// Insert # actions.
					iff.insertChunkData(static_cast<int16>(m_actionDataVector->size()));

					// Write actions.
					const ActionDataVector::const_iterator endIt = m_actionDataVector->end();
					for (ActionDataVector::const_iterator it = m_actionDataVector->begin(); it != endIt; ++it)
					{
						ActionData *const data = *it;
						NOT_NULL(data);

						iff.insertChunkString(data->getActionName().getString());
						iff.insertChunkData(static_cast<float>(data->getRelativeWeight()));
					}

				iff.exitChunk(TAG_ACTN);
			}

			// Write looping action template.
			if (m_loopingAnimationTemplate && m_loopingAnimationTemplate->supportsWriting())
			{
				iff.insertForm(TAG_LOOP);
					m_loopingAnimationTemplate->write(iff);
				iff.exitForm(TAG_LOOP);
			}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_AGAT);
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<%s>", cs_actionGeneratorAnimationElementName);
	writer.indent();
	{
		//-- Write action selection frequency info.
		writer.writeLineFormat("<%s %s='%f' %s='%f' />", cs_frequencyRangeElementName, cs_minTimeAttributeName, m_minActionElapsedTime, cs_maxTimeAttributeName, m_maxActionElapsedTime);

		//-- Write action names and weights.
		if (!m_actionDataVector->empty())
		{
			writer.writeLineFormat("<%s>", cs_actionListElementName);
			writer.indent();
			{
					// Write actions.
					const ActionDataVector::const_iterator endIt = m_actionDataVector->end();
					for (ActionDataVector::const_iterator it = m_actionDataVector->begin(); it != endIt; ++it)
					{
						ActionData *const data = *it;
						WARNING(!data, ("Action generator has NULL data!"));

						if (data)
							writer.writeLineFormat("<%s %s='%s' %s='%f' />", cs_actionElementName, cs_nameAttributeName, data->getActionName().getString(), cs_relativeWeightAttributeName, data->getRelativeWeight());
					}
			}
			writer.unindent();
			writer.writeLineFormat("</%s>", cs_actionListElementName);
		}

		//-- Write the idle (looping) animation.
		if (m_loopingAnimationTemplate && m_loopingAnimationTemplate->supportsWriting())
		{
			writer.writeLineFormat("<%s>", cs_loopAnimationElementName);
			writer.indent();
			{
				m_loopingAnimationTemplate->writeXml(writer);
			}
			writer.unindent();
			writer.writeLineFormat("</%s>", cs_loopAnimationElementName);
		}
	}
	writer.unindent();
	writer.writeLineFormat("</%s>", cs_actionGeneratorAnimationElementName);
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::setLoopingSkeletalAnimationTemplate(const SkeletalAnimationTemplate *loopingTemplate)
{
	if (loopingTemplate)
		loopingTemplate->fetch();

	if (m_loopingAnimationTemplate)
		m_loopingAnimationTemplate->release();

	m_loopingAnimationTemplate = loopingTemplate;
}

// ----------------------------------------------------------------------

int ActionGeneratorSkeletalAnimationTemplate::getActionCount() const
{
	return static_cast<int>(m_actionDataVector->size());
}

// ----------------------------------------------------------------------

const CrcLowerString &ActionGeneratorSkeletalAnimationTemplate::getActionName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getActionCount());
	return (*m_actionDataVector)[static_cast<ActionDataVector::size_type>(index)]->getActionName();
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::addNewAction(const CrcLowerString &actionName, float relativeWeight)
{
	ActionData *actionData = findActionData(actionName);
	if (actionData)
	{
		DEBUG_WARNING(true, ("tried to add new action with same name, ignoring."));
		return;
	}

	m_actionDataVector->push_back(new ActionData(actionName, relativeWeight));
	computePercentileData();
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::removeAction(const CrcLowerString &actionName)
{
	//-- Find the action data entry.
	ActionData *actionData = findActionData(actionName);
	if (!actionData)
	{
		DEBUG_WARNING(true, ("tried to remove action [%s], but not present.", actionName.getString()));
		return;
	}

	//-- Remove and delete the action data entry.
	ActionDataVector::iterator findIt = std::find(m_actionDataVector->begin(), m_actionDataVector->end(), actionData);
	if (findIt == m_actionDataVector->end())
	{
		DEBUG_WARNING(true, ("tried to remove action [%s], found name but couldn't find ActionData entry.", actionName.getString()));
		return;
	}

	IGNORE_RETURN(m_actionDataVector->erase(findIt));
	delete actionData;

	computePercentileData();
}

// ----------------------------------------------------------------------

float ActionGeneratorSkeletalAnimationTemplate::getRelativeWeight(const CrcLowerString &actionName) const
{
	const ActionData *actionData = findActionData(actionName);
	if (actionData)
		return actionData->getRelativeWeight();
	else
	{
		DEBUG_WARNING(true, ("Action name [%s] doesn't exist, can't retrieve relative weight.", actionName.getString()));
		return 0.0f;
	}
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::setRelativeWeight(const CrcLowerString &actionName, float relativeWeight)
{
	ActionData *actionData = findActionData(actionName);
	if (actionData)
	{
		actionData->setRelativeWeight(relativeWeight);
		computePercentileData();
	}
	else
		DEBUG_WARNING(true, ("Action name [%s] doesn't exist, can't set relative weight.", actionName.getString()));
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::changeActionName(const CrcLowerString &oldActionName, const CrcLowerString &newActionName)
{
	ActionData *actionData = findActionData(oldActionName);
	if (actionData)
		actionData->setActionName(newActionName);
	else
	{
		DEBUG_WARNING(true, ("Action name [%s] doesn't exist, can't set new name.", oldActionName.getString()));
	}
}

// ----------------------------------------------------------------------

const SkeletalAnimationTemplateUiFactory *ActionGeneratorSkeletalAnimationTemplate::getConstUiFactory() const
{
	return ms_uiFactory;
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::garbageCollect() const
{
	//-- Forward on garbage collection call to all applicable subordinate templates.
	if (m_loopingAnimationTemplate)
		m_loopingAnimationTemplate->garbageCollect();
}

// ======================================================================
// class ActionGeneratorSkeletalAnimationTemplate: public static member functions
// ======================================================================

void ActionGeneratorSkeletalAnimationTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("ActionGeneratorSkeletalAnimationTemplate not installed"));
	ms_installed = false;

	if (ms_uiFactory)
	{
		delete ms_uiFactory;
		ms_uiFactory = 0;
	}

	const bool result = SkeletalAnimationTemplateList::deregisterCreateFunction(TAG_AGAT);
	DEBUG_FATAL(!result, ("registerCreateFunction failed."));
	UNREF(result);

	SkeletalAnimationTemplateList::deregisterXmlCreateFunction(cs_actionGeneratorAnimationElementName);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *ActionGeneratorSkeletalAnimationTemplate::create(const CrcLowerString &name, Iff &iff)
{
	return new ActionGeneratorSkeletalAnimationTemplate(name, iff);
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *ActionGeneratorSkeletalAnimationTemplate::xmlCreate(const CrcString &name, XmlTreeNode const &treeNode, int versionNumber)
{
	return new ActionGeneratorSkeletalAnimationTemplate(name, treeNode, versionNumber);
}

// ======================================================================
// class ActionGeneratorSkeletalAnimationTemplate: private member functions
// ======================================================================

ActionGeneratorSkeletalAnimationTemplate::ActionGeneratorSkeletalAnimationTemplate(const CrcString &name, Iff &iff) :
	SkeletalAnimationTemplate(name),
	m_minActionElapsedTime(5.0f),
	m_maxActionElapsedTime(20.0f),
	m_actionDataVector(new ActionDataVector()),
	m_loopingAnimationTemplate(0)
{
	iff.enterForm(TAG_AGAT);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("ActionGeneratorSkeletalAnimationTemplate version [%s] unsupported.", buffer));
				}
		}

	iff.exitForm(TAG_AGAT);

	//-- Compute random chance percentile data from relative weight data.
	computePercentileData();
}

// ----------------------------------------------------------------------

ActionGeneratorSkeletalAnimationTemplate::ActionGeneratorSkeletalAnimationTemplate(const CrcString &name, XmlTreeNode const &xmlTreeNode, int versionNumber) :
	SkeletalAnimationTemplate(name),
	m_minActionElapsedTime(5.0f),
	m_maxActionElapsedTime(20.0f),
	m_actionDataVector(new ActionDataVector()),
	m_loopingAnimationTemplate(0)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(xmlTreeNode, versionNumber);
			break;

		default:
			FATAL(true, ("ActionGeneratorSkeletalAnimationTemplate: unsupported version number [%d].", versionNumber));
	}

	//-- Compute random chance percentile data from relative weight data.
	computePercentileData();
}

// ----------------------------------------------------------------------

ActionGeneratorSkeletalAnimationTemplate::~ActionGeneratorSkeletalAnimationTemplate()
{
	if (m_loopingAnimationTemplate)
	{
		m_loopingAnimationTemplate->release();
		m_loopingAnimationTemplate = 0;
	}

	std::for_each(m_actionDataVector->begin(), m_actionDataVector->end(), PointerDeleter());
	delete m_actionDataVector;
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		//-- Get action generation random data.
		iff.enterChunk(TAG_INFO);
			m_minActionElapsedTime = iff.read_float();
			m_maxActionElapsedTime = iff.read_float();
		iff.exitChunk(TAG_INFO);

		//-- Get action names and relative weights.
		if (iff.enterChunk(TAG_ACTN, true))
		{
			const int actionCount = static_cast<int>(iff.read_int16());
			m_actionDataVector->reserve(static_cast<ActionDataVector::size_type>(actionCount));

			char      buffer[1024];
			float     relativeWeight;

			for (int i = 0; i < actionCount; ++i)
			{
				iff.read_string(buffer, sizeof(buffer) - 1);
				relativeWeight = iff.read_float();

				m_actionDataVector->push_back(new ActionData(CrcLowerString(buffer), relativeWeight));
			}

			iff.exitChunk(TAG_ACTN);
		}

		//-- Read looping animation template.
		if (iff.enterForm(TAG_LOOP, true))
		{
			m_loopingAnimationTemplate = SkeletalAnimationTemplateList::fetch(iff);

			iff.exitForm(TAG_LOOP);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::loadXml_1(XmlTreeNode const &treeNode, int versionNumber)
{
	treeNode.assertIsElement(cs_actionGeneratorAnimationElementName);

	//-- Load frequency range for action generation.
	XmlTreeNode const frequencyRangeNode = treeNode.getFirstChildElementNode();
	frequencyRangeNode.assertIsElement(cs_frequencyRangeElementName);

	frequencyRangeNode.getElementAttributeAsFloat(cs_minTimeAttributeName, m_minActionElapsedTime);
	frequencyRangeNode.getElementAttributeAsFloat(cs_maxTimeAttributeName, m_maxActionElapsedTime);

	//-- Load the list of actions.
	XmlTreeNode nextNode = frequencyRangeNode.getNextSiblingElementNode();
	if (nextNode.isElement() && !_stricmp(nextNode.getName(), cs_actionListElementName))
	{
		float relativeWeight = 0.0f;

		for (XmlTreeNode actionNode = nextNode.getFirstChildElementNode(); !actionNode.isNull(); actionNode = actionNode.getNextSiblingElementNode())
		{
			actionNode.assertIsElement(cs_actionElementName);

			// Get action name and relative weighting.
			actionNode.getElementAttributeAsString(cs_nameAttributeName, s_actionName);
			actionNode.getElementAttributeAsFloat(cs_relativeWeightAttributeName, relativeWeight);

			m_actionDataVector->push_back(new ActionData(CrcLowerString(s_actionName.c_str()), relativeWeight));
		}

		// Move on to next node.
		nextNode = nextNode.getNextSiblingElementNode();
	}

	//-- Load loop animation.
	char const *loopAnimationNodeName = nextNode.getName();
	FATAL(!nextNode.isElement() || _stricmp(loopAnimationNodeName, cs_loopAnimationElementName), ("expecting element [%s], found another entity named [%s].", cs_loopAnimationElementName, loopAnimationNodeName));

	m_loopingAnimationTemplate = SkeletalAnimationTemplateList::fetch(nextNode.getFirstChildElementNode(), versionNumber);
}

// ----------------------------------------------------------------------

void ActionGeneratorSkeletalAnimationTemplate::computePercentileData()
{
	//-- Pass 1: accumulate total relative weight of all actions.
	float  totalRelativeWeight = 0.0f;

	{
		const ActionDataVector::const_iterator endIt = m_actionDataVector->end();
		for (ActionDataVector::const_iterator it = m_actionDataVector->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			totalRelativeWeight += (*it)->getRelativeWeight();
		}
	}

	//-- Pass 2: assign max percentile value for each action.
	float  accumulatedRelativeWeight = 0.0f;
	const float percentileMultiplier = (totalRelativeWeight > 0.0f) ? (100.0f / totalRelativeWeight) : 100.0f;

	{
		const ActionDataVector::const_iterator endIt = m_actionDataVector->end();
		for (ActionDataVector::const_iterator it = m_actionDataVector->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);

			accumulatedRelativeWeight += (*it)->getRelativeWeight();

			const float maxPercentileRoll = accumulatedRelativeWeight * percentileMultiplier;
			(*it)->setMaxPercentileRoll(maxPercentileRoll);
		}
	}
}

// ----------------------------------------------------------------------

ActionGeneratorSkeletalAnimationTemplate::ActionData *ActionGeneratorSkeletalAnimationTemplate::findActionData(const CrcLowerString &actionName)
{
	const ActionDataVector::iterator endIt = m_actionDataVector->end();
	for (ActionDataVector::iterator it = m_actionDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		if ((*it)->getActionName() == actionName)
		{
			// Found it.
			return *it;
		}
	}

	//-- Didn't find it.
	return 0;
}

// ----------------------------------------------------------------------

const ActionGeneratorSkeletalAnimationTemplate::ActionData *ActionGeneratorSkeletalAnimationTemplate::findActionData(const CrcLowerString &actionName) const
{
	const ActionDataVector::iterator endIt = m_actionDataVector->end();
	for (ActionDataVector::iterator it = m_actionDataVector->begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		if ((*it)->getActionName() == actionName)
		{
			// Found it.
			return *it;
		}
	}

	//-- Didn't find it.
	return 0;
}

// ======================================================================
