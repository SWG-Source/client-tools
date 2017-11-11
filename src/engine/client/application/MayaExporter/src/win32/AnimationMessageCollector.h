// ======================================================================
//
// AnimationMessageCollector.h
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationMessageCollector_H
#define INCLUDED_AnimationMessageCollector_H

// ======================================================================

class CrcLowerString;
class MDagPath;
class MFnAttribute;
class MFnDagNode;
class Messenger;
class MString;

// ======================================================================
/**
 * Provides a mechanism to collect animation messages and the frames
 * on which the messages are signaled.
 *
 * This class handles collection of animation messages signaled during
 * a Maya animation.  Messages are represented by Maya boolean attributes 
 * associated with a single Maya node.
 *
 * The class is initialized with a Maya dag path to the node containing
 * the animation messages.  Animation message attributes start with the
 * name "soe_am_" for Sony Online Entertainment Animation Message.  The
 * remaining text is the name of the animation message.  The message attribute
 * should be keyable.  It should be set "on" for every frame where the
 * message should be emitted, and set "off" for every frame where it should
 * not be emitted.  In general, most messages should be set "on" for
 * one or two frames during an animation.
 *
 * For every animation frame considered, this instance should have
 * its collectFrameMessageSignals().  It is assumed the Maya animation
 * has been moved to the new frame.
 *
 * When all animation frames have  been processed, the get* functions
 * can be used to retrieve animation message data.
 */

class AnimationMessageCollector
{
public:

	class Message;

	typedef stdvector<int>::fwd  IntVector;

public:

	static void install(Messenger *newMessenger);

public:

	AnimationMessageCollector(const MDagPath &node, bool &success);
	~AnimationMessageCollector();

	bool                  collectFrameMessageSignals(int frameNumber);
	bool                  endDataCollection();

	int                   getMessageCount() const;
	const CrcLowerString &getMessageName(int index) const;
	const IntVector      &getMessageSignalFrameNumbers(int index) const;

private:

	class SignalCollector;

	typedef stdvector<Message*>::fwd  MessageVector;

private:

	bool performMessageConstruction(const MFnAttribute &fnAttribute);

	// disabled
	AnimationMessageCollector();
	AnimationMessageCollector(const AnimationMessageCollector&);
	AnimationMessageCollector &operator =(const AnimationMessageCollector&);

private:

	static const MString   ms_mayaMessageAttributePrefix;
	static const unsigned  ms_mayaMessageAttributePrefixLength;

private:

	const MDagPath &m_node;
	MessageVector  *m_messages;
	bool            m_collectingData;

};

// ======================================================================

#endif
