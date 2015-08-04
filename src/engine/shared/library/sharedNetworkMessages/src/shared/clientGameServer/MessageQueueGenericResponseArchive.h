//======================================================================
//
// MessageQueueGenericResponseArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueGenericResponseArchive_H
#define INCLUDED_MessageQueueGenericResponseArchive_H

#include "Archive/ByteStream.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MessageQueue.h"

class MessageQueueGenericResponse;

//======================================================================

class MessageQueueGenericResponseArchive
{
public:

	static MessageQueue::Data *     get (Archive::ReadIterator & source);
	static void                     put (const MessageQueue::Data * source, Archive::ByteStream & target);
};

//======================================================================

#endif
