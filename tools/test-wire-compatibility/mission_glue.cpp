// Test glue only: the trivial value-holder members of MessageQueueMissionListResponse.
// MessageQueueMissionListResponse.cpp is not compiled because it registers a controller-message
// factory (runtime infrastructure). Every serializer under test is the checkout's real code.
#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
MessageQueueMissionListResponse::MessageQueueMissionListResponse() : m_response(), m_sequenceId(0), m_bountyTerminal(false) {}
MessageQueueMissionListResponse::MessageQueueMissionListResponse(const DataVector & v, const uint8 s, const bool b) : m_response(v), m_sequenceId(s), m_bountyTerminal(b) {}
MessageQueueMissionListResponse::~MessageQueueMissionListResponse() {}
void MessageQueueMissionListResponse::set(const DataVector & r, const uint8 s, const bool b) { m_response=r; m_sequenceId=s; m_bountyTerminal=b; }
// Localized-text lookup is display-only (StringId::localize); never reached by pack/unpack.
#include "LocalizationManager.h"
LocalizationManager & LocalizationManager::getManager() { std::abort(); }
LocalizationManager & LocalizationManager::getManager(Unicode::NarrowString) { std::abort(); }
LocalizationManager::StringValueCode LocalizationManager::getLocalizedStringValue(StringId const &, Unicode::String &, bool) { std::abort(); }
// MessageQueue::Data base and the class's pooled allocator (runtime infrastructure).
#include "sharedFoundation/MessageQueue.h"
MessageQueue::Data::Data() {}
MessageQueue::Data::~Data() {}
void MessageQueueMissionListResponse::operator delete(void *p) { ::operator delete(p); }
