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
// Base infrastructure only: allow LoginClusterStatus construction, with NO command CRC
// registration. Its real .cpp and all AutoByteStream/AutoArray/element serializers run.
// This deliberately tests bounded message data, not the complete network packet.
// ChatOnRequestLog is still not constructed.
#include "sharedNetworkMessages/GameNetworkMessage.h"
GameNetworkMessage::GameNetworkMessage(std::string const &name) {
 if (name != "LoginClusterStatus") std::abort();
}
GameNetworkMessage::~GameNetworkMessage() {}
#include "sharedMessageDispatch/Message.h"
MessageDispatch::MessageBase::MessageBase(char const *) : type(0) {}
MessageDispatch::MessageBase::~MessageBase() {}
// Pooled allocation and message registration are runtime infrastructure. The harness keeps
// each pool's element size in the opaque m_allocator field and uses plain heap blocks.
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include <cstdint>
MemoryBlockManager::MemoryBlockManager(char const *name, bool shared, int elementSize, int, int, int)
: m_name(name), m_shared(shared), m_currentNumberOfElements(0),
  m_allocator(reinterpret_cast<Allocator *>(static_cast<intptr_t>(elementSize))) {}
MemoryBlockManager::~MemoryBlockManager() {}
void *MemoryBlockManager::allocate(bool) { return ::operator new(static_cast<size_t>(reinterpret_cast<intptr_t>(m_allocator))); }
void MemoryBlockManager::free(void *p) { ::operator delete(p); }
void ExitChain::add(Function, char const *, int, bool) {}
void ControllerMessageFactory::registerControllerMessageHandler(int32, ControllerMessagePackFunction, ControllerMessageUnpackFunction, bool) {}
