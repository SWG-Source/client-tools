// ======================================================================
//
// AnimationMessageCollector.cpp
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "AnimationMessageCollector.h"

#include "maya/MDagPath.h"
#include "maya/MFnAttribute.h"
#include "maya/MFnDagNode.h"
#include "maya/MPlug.h"
#include "maya/MString.h"
#include "Messenger.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

#define MESSENGER_REJECT_CONSTRUCTOR(x, y) \
	if (x) \
	{ \
		MESSENGER_LOG(y); \
		return; \
	}

#define MESSENGER_REJECT_ERRORCOUNT(x, y) \
	if (x) \
	{ \
		MESSENGER_LOG(y); \
		++m_errorCount; \
		return; \
	}

// ======================================================================

class AnimationMessageCollector::Message
{
public:

	explicit Message(const MString &messageName, const MObject &object);

	void                  addSignalFrameNumber(int frameNumber);

	const CrcLowerString &getName() const;
	const IntVector      &getSignalFrameNumbers() const;
	const MObject        &getObject() const;

private:

	// disabled
	Message();
	Message(const Message&);             //lint -esym(754, Message::Message)   // unreferenced // preventative
	Message &operator =(const Message&); //lint -esym(754, Message::operator=) // unreferenced // preventative

private:

	CrcLowerString  m_name;
	MObject         m_object;
	IntVector       m_signalFrameNumbers;

};

// ======================================================================

class AnimationMessageCollector::SignalCollector
{
public:

	SignalCollector(const MFnDagNode &fnDagNode, int frameNumber);

	void  operator()(Message *message);
	int   getErrorCount() const;

private:

	// disabled
	SignalCollector();
	SignalCollector &operator =(const SignalCollector&); //lint -esym(754, SignalCollector::operator=) // unreferenced // preventative

private:

	const MFnDagNode &m_fnDagNode;
	int               m_frameNumber;
	int               m_errorCount;

};

// ======================================================================

static Messenger *messenger;

// ======================================================================

const MString   AnimationMessageCollector::ms_mayaMessageAttributePrefix       = "soe_am_";
const unsigned  AnimationMessageCollector::ms_mayaMessageAttributePrefixLength = ms_mayaMessageAttributePrefix.length();

// ======================================================================
// class AnimationMessageCollector::Message
// ======================================================================

AnimationMessageCollector::Message::Message(const MString &messageName, const MObject &object)
:	m_name(messageName.asChar()),
	m_object(object),
	m_signalFrameNumbers()
{
}

// ----------------------------------------------------------------------

void AnimationMessageCollector::Message::addSignalFrameNumber(int frameNumber)
{
	m_signalFrameNumbers.push_back(frameNumber);
}

// ----------------------------------------------------------------------

inline const CrcLowerString &AnimationMessageCollector::Message::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline const AnimationMessageCollector::IntVector &AnimationMessageCollector::Message::getSignalFrameNumbers() const
{
	return m_signalFrameNumbers;
}

// ----------------------------------------------------------------------

inline const MObject &AnimationMessageCollector::Message::getObject() const
{
	return m_object;
}

// ======================================================================
// class AnimationMessageCollector::SignalCollector
// ======================================================================

AnimationMessageCollector::SignalCollector::SignalCollector(const MFnDagNode &fnDagNode, int frameNumber)
:	m_fnDagNode(fnDagNode),
	m_frameNumber(frameNumber),
	m_errorCount(0)
{
}

// ----------------------------------------------------------------------

void AnimationMessageCollector::SignalCollector::operator()(Message *message)
{
	NOT_NULL(message);

	//-- get the plug for the message
	MStatus status;

	MPlug plug = m_fnDagNode.findPlug(message->getObject(), &status);
	MESSENGER_REJECT_ERRORCOUNT(!status, ("failed to find plug for message [%s]\n", message->getName().getString()));

	//-- retrieve message value
	bool messageSignaled = false;
	status               = plug.getValue(messageSignaled );
	MESSENGER_REJECT_ERRORCOUNT(!status, ("failed to get bool value for attribute [%s]\n", message->getName().getString()));

	//-- if the message is "on" (true), we signal it this frame
	if (messageSignaled)
		message->addSignalFrameNumber(m_frameNumber);
}

// ----------------------------------------------------------------------

inline int AnimationMessageCollector::SignalCollector::getErrorCount() const
{
	return m_errorCount;
}

// ======================================================================
// class AnimationMessageCollector
// ======================================================================

void AnimationMessageCollector::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ======================================================================

AnimationMessageCollector::AnimationMessageCollector(const MDagPath &node, bool &success):	
	m_node(node),
	m_messages(new MessageVector()),
	m_collectingData(true)
{
	DEBUG_FATAL(!messenger, ("AnimationMessageCollector not installed"));
	success = false;

	//-- find message attributes associated with the given dag node
	// get the dag node function set
	MStatus     status;
	MFnDagNode  fnDagNode(m_node, &status);
	MESSENGER_REJECT_CONSTRUCTOR(!status, ("failed to initialize MFnDagNode [%s]\n", status.errorString().asChar()));

	// check each attribute associated with the dag node
	const unsigned attributeCount = fnDagNode.attributeCount(&status);
	MESSENGER_REJECT_CONSTRUCTOR(!status, ("failed to get attribute count [%s]\n", status.errorString().asChar()));

	MFnAttribute  fnAttribute;
	for (unsigned i = 0; i < attributeCount; ++i)
	{
		//-- get the attribute function set
		MObject attributeObject = fnDagNode.attribute(i, &status);
		MESSENGER_REJECT_CONSTRUCTOR(!status, ("failed to get attribute [%u] [%s]\n", i, status.errorString().asChar()));

		status = fnAttribute.setObject(attributeObject);
		MESSENGER_REJECT_CONSTRUCTOR(!status, ("failed to set MFnAttributeobject [%s]\n", status.errorString().asChar()));

		//-- handle message construction from this attribute within this dag node
		const bool checkResult = performMessageConstruction(fnAttribute);
		MESSENGER_REJECT_CONSTRUCTOR(!checkResult, ("performMessageConstruction failed for attribute index %u\n", i));
	}
	
	// no errors
	success = true;
}

// ----------------------------------------------------------------------

AnimationMessageCollector::~AnimationMessageCollector()
{
	std::for_each(m_messages->begin(), m_messages->end(), PointerDeleter());
	delete m_messages;
}

// ----------------------------------------------------------------------

bool AnimationMessageCollector::collectFrameMessageSignals(int frameNumber)
{
	MESSENGER_REJECT(!m_collectingData, ("collectFrameMessageSignals(): data collection has already ended!\n"));

	//-- mark each message that is currently "on" this frame
	// grab dag node
	MStatus     status;

	MFnDagNode  fnDagNode(m_node, &status);
	MESSENGER_REJECT(!status, ("failed to initialize MFnDagNode [%s]\n", status.errorString().asChar()));

	SignalCollector signalCollector = std::for_each(m_messages->begin(), m_messages->end(), SignalCollector(fnDagNode, frameNumber));

	// no errors if signal collector received no errors
	return (signalCollector.getErrorCount() == 0);
}

// ----------------------------------------------------------------------

bool AnimationMessageCollector::endDataCollection()
{
	MESSENGER_REJECT(!m_collectingData, ("endDataCollection(): data collection has already ended!\n"));

	//-- Set to false so user can now access the data.  I do this so I can guarantee that I've done post-collection processing below.
	m_collectingData = false;

	//-- Check for the same message being triggered 3 or more frames in a row.  Return false if this is the case.
	int const messageCount = getMessageCount();
	for (int i = 0; i < messageCount; ++i)
	{
		IntVector const &signalFrameNumbers = getMessageSignalFrameNumbers(i);
		int const signalCount = static_cast<int>(signalFrameNumbers.size());

		int previousFrameNumber = std::numeric_limits<int>::min();
		int contiguousCount = 1;

		for (int signalIndex = 0; signalIndex < signalCount; ++signalIndex)
		{
			//-- Update contiguous counter.
			int const frameNumber = signalFrameNumbers[static_cast<IntVector::size_type>(signalIndex)];
			if (frameNumber <= previousFrameNumber + 1)
				++contiguousCount;
			else
				contiguousCount = 1;

			//-- Reject on too many in a row.
			MESSENGER_REJECT(contiguousCount >= 3, ("animation message [%s] signaled 3 or more frames in a row. Fix by keying off the message on the frame after it gets signaled.\n", getMessageName(i).getString()));

			//-- Update last frame number.
			previousFrameNumber = frameNumber;
		}
	}

	// Success.
	return true;
}

// ----------------------------------------------------------------------

int AnimationMessageCollector::getMessageCount() const
{
	if (m_collectingData)
	{
		MESSENGER_LOG_ERROR(("getMessageCount(): still collecting message data, returning 0!  Forgot to call endDataCollection()?\n"));
		return 0;
	}

	return static_cast<int>(m_messages->size());
}

// ----------------------------------------------------------------------

const CrcLowerString &AnimationMessageCollector::getMessageName(int index) const
{
	if (m_collectingData)
	{
		MESSENGER_LOG_ERROR(("getMessageName(): still collecting message data, returning NULL!  Forgot to call endDataCollection()?\n"));
		return CrcLowerString::empty;
	}

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_messages->size()));
	return (*m_messages)[static_cast<size_t>(index)]->getName();
}

// ----------------------------------------------------------------------

const AnimationMessageCollector::IntVector &AnimationMessageCollector::getMessageSignalFrameNumbers(int index) const
{
	if (m_collectingData)
	{
		static IntVector  emptyIntVector;

		MESSENGER_LOG_ERROR(("getMessageSignalFrameNumbers(): still collecting message data, returning empty vector!  Forgot to call endDataCollection()?\n"));
		return emptyIntVector;
	}

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_messages->size()));
	return (*m_messages)[static_cast<size_t>(index)]->getSignalFrameNumbers();
}

// ======================================================================

bool AnimationMessageCollector::performMessageConstruction(const MFnAttribute &fnAttribute)
{
	MStatus  status;

	//-- check if name starts with one of our magic strings
	MString attributeName = fnAttribute.name(&status);
	MESSENGER_REJECT(!status, ("failed to get attribute name\n"));

	const unsigned attributeNameLength = attributeName.length();
	IGNORE_RETURN(attributeName.toLowerCase());

	if (
		(attributeNameLength > ms_mayaMessageAttributePrefixLength) && 
		(attributeName.substring(0, static_cast<int>(ms_mayaMessageAttributePrefixLength - 1)) == ms_mayaMessageAttributePrefix))
	{
		//-- add message to list of messages
		// retrieve the message name
		MString messageName = attributeName.substring(static_cast<int>(ms_mayaMessageAttributePrefixLength), static_cast<int>(attributeNameLength - 1));
		
		// create the message
		m_messages->push_back(new Message(messageName, fnAttribute.object()));
	}

	// no errors
	return true;
}

// ======================================================================
