// ======================================================================
//
// Conversation.cpp
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "Conversation.h"

#include "Configuration.h"
#include "ScriptGroup.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "StringIdTracker.h"

// ======================================================================
// namespace ConversationNamespace
// ======================================================================

namespace ConversationNamespace
{
	Tag const TAG_CNV = TAG3 (C,N,V);
	Tag const TAG_ITEM = TAG (I,T,E,M);
	Tag const TAG_BRAN = TAG (B,R,A,N);
	Tag const TAG_RLST = TAG (R,L,S,T);
	Tag const TAG_RESP = TAG (R,E,S,P);
	Tag const TAG_BLST = TAG (B,L,S,T);
	Tag const TAG_LSET = TAG (L,S,E,T);
}

using namespace ConversationNamespace;

// ======================================================================
// PUBLIC ConversationItem
// ======================================================================

ConversationItem::ConversationItem (StringIdTracker * stringIdTracker) :
	m_conditionFamilyId (0),
	m_negateCondition (false),
	m_actionFamilyId (0),
	m_labelFamilyId (0),
	m_linkFamilyId (0),
	m_stringId (stringIdTracker->getUniqueStringId()),
	m_text (),
	m_notes (),
	m_debug (false),
	m_expanded (false),
	m_groupEcho (false),
	m_playerAnimation (Configuration::getEmptyAnimationAction()),
	m_npcAnimation (Configuration::getEmptyAnimationAction()),
	m_useProsePackage (false),
	m_tokenTOFamilyId (0),
	m_tokenDIFamilyId (0),
	m_tokenDFFamilyId (0),
	m_parent (0),
	m_stringIdTracker(stringIdTracker)
{
}

// ----------------------------------------------------------------------

ConversationItem::~ConversationItem ()
{
	m_parent = 0;

	//-- Assign pointer to zero, delete happens in the Conversation destructor
	m_stringIdTracker = 0;
}

// ----------------------------------------------------------------------

void ConversationItem::reset ()
{
	m_conditionFamilyId = 0;
	m_negateCondition = false;
	m_actionFamilyId = 0;
	m_labelFamilyId = 0;
	m_linkFamilyId = 0;
	m_stringId = getStringIdTracker()->getUniqueStringId();
	m_text.clear ();
	m_notes.clear ();
	m_debug = false;
	m_expanded = false;
	m_groupEcho = false;
	m_playerAnimation = Configuration::getEmptyAnimationAction();
	m_npcAnimation = Configuration::getEmptyAnimationAction();
	m_useProsePackage = false;
	m_tokenTOFamilyId = 0;
	m_tokenDIFamilyId = 0;
	m_tokenDFFamilyId = 0;
}

// ----------------------------------------------------------------------

void ConversationItem::load (Iff & iff)
{
	bool stringIdIsStringCrc = false;

	iff.enterForm (TAG_ITEM);

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			stringIdIsStringCrc = true;
			load_0000 (iff);
			break;

		case TAG_0001:
			stringIdIsStringCrc = true;
			load_0001 (iff);
			break;

		case TAG_0002:
			stringIdIsStringCrc = true;
			load_0002 (iff);
			break;

		case TAG_0003:
			stringIdIsStringCrc = true;
			load_0003 (iff);
			break;

		case TAG_0004:
			stringIdIsStringCrc = true;
			load_0004 (iff);
			break;

		case TAG_0005:
			stringIdIsStringCrc = true;
			load_0005 (iff);
			break;

		case TAG_0006:
			load_0006 (iff);
			break;

		case TAG_0007:
			load_0007 (iff);
			break;

		default:
			{
				char tagBuffer [5];
				ConvertTagToString (iff.getCurrentName (), tagBuffer);

				char buffer [128];
				iff.formatLocation (buffer, sizeof (buffer));
				DEBUG_FATAL (true, ("unknown ConversationItem version %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff.exitForm (TAG_ITEM);

	if (stringIdIsStringCrc)
	{
		std::string const stringId = FormattedString<64>().sprintf("%x", Crc::calculate(getText().c_str(), getText().size()));
		if (getStringIdTracker()->isUniqueStringId(stringId))
			setStringId(stringId);
		else
			setStringId(getStringIdTracker()->getUniqueStringId());
	}
}

// ----------------------------------------------------------------------

void ConversationItem::save (Iff & iff) const
{
	save_0007(iff);
}

void ConversationItem::save_0006 (Iff & iff) const
{
	iff.insertForm (TAG_ITEM);

		iff.insertForm(TAG_0006);

			iff.insertChunk (TAG_DATA);

				iff.insertChunkData (getDebug () ? static_cast<uint8> (1) : static_cast<uint8> (0));
				iff.insertChunkData (getConditionFamilyId ());
				iff.insertChunkData (getNegateCondition () ? static_cast<uint8> (1) : static_cast<uint8> (0));
				iff.insertChunkData (getActionFamilyId ());
				iff.insertChunkData (getLabelFamilyId ());
				iff.insertChunkData (getLinkFamilyId ());
				iff.insertChunkData (getUseProsePackage () ? static_cast<uint8> (1) : static_cast<uint8> (0));
				iff.insertChunkData (getTokenTOFamilyId ());
				iff.insertChunkData (getTokenDIFamilyId ());
				iff.insertChunkData (getTokenDFFamilyId ());
				iff.insertChunkString (getPlayerAnimation ().c_str ());
				iff.insertChunkString (getNpcAnimation ().c_str ());
				iff.insertChunkString(getStringId().c_str());
				iff.insertChunkString (getText ().c_str ());
				iff.insertChunkString (getNotes ().c_str ());

			iff.exitChunk ();
	
		iff.exitForm ();

	iff.exitForm ();
}

void ConversationItem::save_0007 (Iff & iff) const
{
	iff.insertForm (TAG_ITEM);

		iff.insertForm(TAG_0007);

			iff.insertChunk (TAG_DATA);

				iff.insertChunkData (getDebug () ? static_cast<uint8> (1) : static_cast<uint8> (0));
				iff.insertChunkData (getConditionFamilyId ());
				iff.insertChunkData (getNegateCondition () ? static_cast<uint8> (1) : static_cast<uint8> (0));
				iff.insertChunkData (getGroupEcho () ? static_cast<uint8> (1) : static_cast<uint8> (0));
				iff.insertChunkData (getActionFamilyId ());
				iff.insertChunkData (getLabelFamilyId ());
				iff.insertChunkData (getLinkFamilyId ());
				iff.insertChunkData (getUseProsePackage () ? static_cast<uint8> (1) : static_cast<uint8> (0));
				iff.insertChunkData (getTokenTOFamilyId ());
				iff.insertChunkData (getTokenDIFamilyId ());
				iff.insertChunkData (getTokenDFFamilyId ());
				iff.insertChunkString (getPlayerAnimation ().c_str ());
				iff.insertChunkString (getNpcAnimation ().c_str ());
				iff.insertChunkString (getStringId().c_str());
				iff.insertChunkString (getText ().c_str ());
				iff.insertChunkString (getNotes ().c_str ());

			iff.exitChunk ();

		iff.exitForm ();

	iff.exitForm ();
}

// ----------------------------------------------------------------------

int ConversationItem::getConditionFamilyId () const
{
	return m_conditionFamilyId;
}

// ----------------------------------------------------------------------

void ConversationItem::setConditionFamilyId (int const conditionFamilyId)
{
	m_conditionFamilyId = conditionFamilyId;
}

// ----------------------------------------------------------------------

bool ConversationItem::getNegateCondition () const
{
	return m_negateCondition;
}

// ----------------------------------------------------------------------

void ConversationItem::setNegateCondition (bool const negateCondition)
{
	m_negateCondition = negateCondition;
}

// ----------------------------------------------------------------------

bool ConversationItem::getGroupEcho () const
{
	return m_groupEcho;
}

// ----------------------------------------------------------------------

void ConversationItem::setGroupEcho (bool const groupEcho)
{
	m_groupEcho = groupEcho;
}

// ----------------------------------------------------------------------

int ConversationItem::getActionFamilyId () const
{
	return m_actionFamilyId;
}

// ----------------------------------------------------------------------

void ConversationItem::setActionFamilyId (int const actionFamilyId)
{
	m_actionFamilyId = actionFamilyId;
}

// ----------------------------------------------------------------------

int ConversationItem::getLabelFamilyId () const
{
	return m_labelFamilyId;
}

// ----------------------------------------------------------------------

void ConversationItem::setLabelFamilyId (int const labelFamilyId)
{
	m_labelFamilyId = labelFamilyId;
}

// ----------------------------------------------------------------------

int ConversationItem::getLinkFamilyId () const
{
	return m_linkFamilyId;
}

// ----------------------------------------------------------------------

void ConversationItem::setLinkFamilyId (int const linkFamilyId)
{
	m_linkFamilyId = linkFamilyId;
}

// ----------------------------------------------------------------------

std::string const & ConversationItem::getStringId() const
{
	return m_stringId;
}

// ----------------------------------------------------------------------

std::string const & ConversationItem::getText () const
{
	return m_text;
}

// ----------------------------------------------------------------------

void ConversationItem::setText (std::string const & text)
{
	m_text = text;
}

// ----------------------------------------------------------------------

std::string const & ConversationItem::getNotes () const
{
	return m_notes;
}

// ----------------------------------------------------------------------

void ConversationItem::setNotes (std::string const & notes)
{
	m_notes = notes;
}

// ----------------------------------------------------------------------

bool ConversationItem::getExpanded () const
{
	return m_expanded;
}

// ----------------------------------------------------------------------

void ConversationItem::setExpanded (bool const expanded)
{
	m_expanded = expanded;
}

// ----------------------------------------------------------------------

void ConversationItem::setPlayerAnimation (std::string const & playerAnimation)
{
	m_playerAnimation = playerAnimation;
}

// ----------------------------------------------------------------------

std::string const & ConversationItem::getPlayerAnimation () const
{
	return m_playerAnimation;
}

// ----------------------------------------------------------------------

void ConversationItem::setNpcAnimation (std::string const & npcAnimation)
{
	m_npcAnimation = npcAnimation;
}

// ----------------------------------------------------------------------

std::string const & ConversationItem::getNpcAnimation () const
{
	return m_npcAnimation;
}

// ----------------------------------------------------------------------

bool ConversationItem::getDebug () const
{
	return m_debug;
}

// ----------------------------------------------------------------------

void ConversationItem::setDebug (bool const debug)
{
	m_debug = debug;
}

// ----------------------------------------------------------------------

bool ConversationItem::hasChildren () const
{
	return false;
}

// ----------------------------------------------------------------------

ConversationItem const * ConversationItem::getLink (int const linkFamilyId) const
{
	return getLabelFamilyId () == linkFamilyId ? this : 0;
}

// ----------------------------------------------------------------------

ConversationItem const * ConversationItem::getParent () const
{
	return m_parent;
}

// ----------------------------------------------------------------------

ConversationItem * ConversationItem::getParent ()
{
	return m_parent;
}

// ----------------------------------------------------------------------

bool ConversationItem::getUseProsePackage () const
{
	return m_useProsePackage;
}

// ----------------------------------------------------------------------

void ConversationItem::setUseProsePackage (bool const useProsePackage)
{
	m_useProsePackage = useProsePackage;
}

// ----------------------------------------------------------------------

int ConversationItem::getTokenTOFamilyId () const
{
	return m_tokenTOFamilyId;
}

// ----------------------------------------------------------------------

void ConversationItem::setTokenTOFamilyId (int const tokenTOFamilyId)
{
	m_tokenTOFamilyId = tokenTOFamilyId;
}

// ----------------------------------------------------------------------

int ConversationItem::getTokenDIFamilyId () const
{
	return m_tokenDIFamilyId;
}

// ----------------------------------------------------------------------

void ConversationItem::setTokenDIFamilyId (int const tokenDIFamilyId)
{
	m_tokenDIFamilyId = tokenDIFamilyId;
}

// ----------------------------------------------------------------------

int ConversationItem::getTokenDFFamilyId () const
{
	return m_tokenDFFamilyId;
}

// ----------------------------------------------------------------------

void ConversationItem::setTokenDFFamilyId (int const tokenDFFamilyId)
{
	m_tokenDFFamilyId = tokenDFFamilyId;
}

// ----------------------------------------------------------------------

StringIdTracker * ConversationItem::getStringIdTracker()
{
	return m_stringIdTracker;
}

// ----------------------------------------------------------------------

void ConversationItem::deleteStringIdTracker()
{
	delete m_stringIdTracker;
	m_stringIdTracker = 0;
}

// ======================================================================
// PROTECTED ConversationItem
// ======================================================================

void ConversationItem::setParent (ConversationItem * const parent)
{
	m_parent = parent;
}

// ======================================================================
// PRIVATE ConversationItem
// ======================================================================

void ConversationItem::setStringId(std::string const & stringId)
{
	m_stringId = stringId;

	getStringIdTracker()->insertUniqueStringId(stringId);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0000 (Iff & iff)
{
	iff.enterForm (TAG_0000);

		iff.enterChunk (TAG_DATA);

			std::string buffer;

			setConditionFamilyId (iff.read_int32 ());

			setActionFamilyId (iff.read_int32 ());

			iff.read_string (buffer);
			setText (buffer);

			iff.read_string (buffer);
			setNotes (buffer);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0000);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0001 (Iff & iff)
{
	iff.enterForm (TAG_0001);

		iff.enterChunk (TAG_DATA);

			std::string buffer;

			setConditionFamilyId (iff.read_int32 ());

			setNegateCondition (iff.read_bool8 ());

			setActionFamilyId (iff.read_int32 ());

			iff.read_string (buffer);
			setText (buffer);

			iff.read_string (buffer);
			setNotes (buffer);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0001);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0002 (Iff & iff)
{
	iff.enterForm (TAG_0002);

		iff.enterChunk (TAG_DATA);

			std::string buffer;

			setConditionFamilyId (iff.read_int32 ());

			setNegateCondition (iff.read_bool8 ());

			setActionFamilyId (iff.read_int32 ());

			setDebug (iff.read_bool8 ());

			iff.read_string (buffer);
			setText (buffer);

			iff.read_string (buffer);
			setNotes (buffer);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0002);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0003 (Iff & iff)
{
	iff.enterForm (TAG_0003);

		iff.enterChunk (TAG_DATA);

			std::string buffer;

			setConditionFamilyId (iff.read_int32 ());

			setNegateCondition (iff.read_bool8 ());

			setActionFamilyId (iff.read_int32 ());

			setLabelFamilyId (iff.read_int32 ());

			setLinkFamilyId (iff.read_int32 ());

			setDebug (iff.read_bool8 ());

			iff.read_string (buffer);
			setText (buffer);

			iff.read_string (buffer);
			setNotes (buffer);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0003);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0004 (Iff & iff)
{
	iff.enterForm (TAG_0004);

		iff.enterChunk (TAG_DATA);

			std::string buffer;

			setConditionFamilyId (iff.read_int32 ());

			setNegateCondition (iff.read_bool8 ());

			setActionFamilyId (iff.read_int32 ());

			setLabelFamilyId (iff.read_int32 ());

			setLinkFamilyId (iff.read_int32 ());

			setDebug (iff.read_bool8 ());

			iff.read_string (buffer);
			setPlayerAnimation (buffer);

			iff.read_string (buffer);
			setNpcAnimation (buffer);

			iff.read_string (buffer);
			setText (buffer);

			iff.read_string (buffer);
			setNotes (buffer);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0004);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0005 (Iff & iff)
{
	iff.enterForm (TAG_0005);

		iff.enterChunk (TAG_DATA);

			std::string buffer;

			setConditionFamilyId (iff.read_int32 ());

			setNegateCondition (iff.read_bool8 ());

			setActionFamilyId (iff.read_int32 ());

			setLabelFamilyId (iff.read_int32 ());

			setLinkFamilyId (iff.read_int32 ());

			setDebug (iff.read_bool8 ());

			setUseProsePackage (iff.read_bool8 ());

			setTokenTOFamilyId (iff.read_int32 ());

			setTokenDIFamilyId (iff.read_int32 ());

			setTokenDFFamilyId (iff.read_int32 ());

			iff.read_string (buffer);
			setPlayerAnimation (buffer);

			iff.read_string (buffer);
			setNpcAnimation (buffer);

			iff.read_string (buffer);
			setText (buffer);

			iff.read_string (buffer);
			setNotes (buffer);

		iff.exitChunk (TAG_DATA);

	iff.exitForm (TAG_0005);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0006(Iff & iff)
{
	iff.enterForm(TAG_0006);

		iff.enterChunk(TAG_DATA);

			std::string buffer;

			setDebug(iff.read_bool8());

			setConditionFamilyId(iff.read_int32());

			setNegateCondition(iff.read_bool8());

			setActionFamilyId(iff.read_int32());

			setLabelFamilyId(iff.read_int32());

			setLinkFamilyId(iff.read_int32());

			setUseProsePackage(iff.read_bool8());

			setTokenTOFamilyId(iff.read_int32());

			setTokenDIFamilyId(iff.read_int32());

			setTokenDFFamilyId(iff.read_int32());

			iff.read_string(buffer);
			setPlayerAnimation(buffer);

			iff.read_string(buffer);
			setNpcAnimation(buffer);

			iff.read_string(buffer);
			if (getStringIdTracker()->isUniqueStringId(buffer))
				setStringId(buffer);
			else
				setStringId(getStringIdTracker()->getUniqueStringId());

			iff.read_string(buffer);
			setText(buffer);

			iff.read_string(buffer);
			setNotes(buffer);

		iff.exitChunk(TAG_DATA);

	iff.exitForm(TAG_0006);
}

// ----------------------------------------------------------------------

void ConversationItem::load_0007(Iff & iff)
{
	iff.enterForm(TAG_0007);

		iff.enterChunk(TAG_DATA);

			std::string buffer;

			setDebug(iff.read_bool8());

			setConditionFamilyId(iff.read_int32());

			setNegateCondition(iff.read_bool8());

			setGroupEcho(iff.read_bool8());

			setActionFamilyId(iff.read_int32());

			setLabelFamilyId(iff.read_int32());

			setLinkFamilyId(iff.read_int32());

			setUseProsePackage(iff.read_bool8());

			setTokenTOFamilyId(iff.read_int32());

			setTokenDIFamilyId(iff.read_int32());

			setTokenDFFamilyId(iff.read_int32());

			iff.read_string(buffer);
			setPlayerAnimation(buffer);

			iff.read_string(buffer);
			setNpcAnimation(buffer);

			iff.read_string(buffer);
			if (getStringIdTracker()->isUniqueStringId(buffer))
				setStringId(buffer);
			else
				setStringId(getStringIdTracker()->getUniqueStringId());

			iff.read_string(buffer);
			setText(buffer);

			iff.read_string(buffer);
			setNotes(buffer);

		iff.exitChunk(TAG_DATA);

	iff.exitForm(TAG_0007);
}

// ======================================================================
// PUBLIC ConversationBranch
// ======================================================================

ConversationBranch::ConversationBranch (StringIdTracker * stringIdTracker) :
	ConversationItem (stringIdTracker),
	m_responseList (),
	m_branchId (0)
{
}

// ----------------------------------------------------------------------

ConversationBranch::~ConversationBranch ()
{
	std::for_each (m_responseList.begin (), m_responseList.end (), PointerDeleter ());
	m_responseList.clear ();
}

// ----------------------------------------------------------------------

void ConversationBranch::reset ()
{
	std::for_each (m_responseList.begin (), m_responseList.end (), PointerDeleter ());
	m_responseList.clear ();

	ConversationItem::reset ();
}

// ----------------------------------------------------------------------

void ConversationBranch::load (Iff & iff)
{
	iff.enterForm (TAG_BRAN);

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		case TAG_0001:
			load_0001 (iff);
			break;

		default:
			{
				char tagBuffer [5];
				ConvertTagToString (iff.getCurrentName (), tagBuffer);

				char buffer [128];
				iff.formatLocation (buffer, sizeof (buffer));
				DEBUG_FATAL (true, ("unknown ConversationBranch version type %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff.exitForm (TAG_BRAN);
}

// ----------------------------------------------------------------------

void ConversationBranch::save (Iff & iff) const
{
	iff.insertForm (TAG_BRAN);

		iff.insertForm (TAG_0001);

			ConversationItem::save (iff);

			for (int i = 0; i < getNumberOfResponses (); ++i)
			{
				ConversationResponse const * const response = getResponse (i);
				response->save (iff);
			}

		iff.exitForm (TAG_0001);

	iff.exitForm (TAG_BRAN);
}

// ----------------------------------------------------------------------

int ConversationBranch::getNumberOfResponses () const
{
	return static_cast<int> (m_responseList.size ());
}

// ----------------------------------------------------------------------

ConversationResponse const * ConversationBranch::getResponse (int const index) const
{
	return m_responseList [static_cast<size_t> (index)];
}

// ----------------------------------------------------------------------

ConversationResponse * ConversationBranch::getResponse (int const index)
{
	return m_responseList [static_cast<size_t> (index)];
}

// ----------------------------------------------------------------------

void ConversationBranch::addResponse (ConversationResponse * const response)
{
	response->setParent (this);
	m_responseList.push_back (response);
}

// ----------------------------------------------------------------------

void ConversationBranch::removeResponse (int const index)
{
	delete m_responseList [static_cast<size_t> (index)];

	IGNORE_RETURN (m_responseList.erase (m_responseList.begin () + index));
}

// ----------------------------------------------------------------------

void ConversationBranch::removeResponse (ConversationResponse const * const response, bool const doDelete)
{
	ResponseList::iterator iter = std::find (m_responseList.begin (), m_responseList.end (), response);
	if (iter != m_responseList.end ())
	{
		if (doDelete)
			delete *iter;

		IGNORE_RETURN (m_responseList.erase (iter));
	}
}

// ----------------------------------------------------------------------

void ConversationBranch::promoteResponse (ConversationResponse const * const response)
{
	size_t const size = m_responseList.size ();

	//-- can't promote with less than 2 items
	if (size < 2)
		return;

	//-- find response
	size_t i = 0;
	for (i = 0; i < size; ++i)
		if (m_responseList [i] == response)
			break;

	//-- if found and promote-able
	if (i < size - 1)
		std::swap (m_responseList [i], m_responseList [i + 1]);
}

// ----------------------------------------------------------------------

void ConversationBranch::demoteResponse (ConversationResponse const * const response)
{
	size_t const size = m_responseList.size ();

	//-- can't demote with less than 2 items
	if (size < 2)
		return;

	//-- find response
	size_t i = 0;
	for (i = 0; i < size; ++i)
		if (m_responseList [i] == response)
			break;

	//-- if found and demote-able
	if (i > 0)
		std::swap (m_responseList [i], m_responseList [i - 1]);
}

// ----------------------------------------------------------------------

void ConversationBranch::setBranchId (int const branchId) const
{
	m_branchId = branchId;
}

// ----------------------------------------------------------------------

int ConversationBranch::getBranchId () const
{
	return m_branchId;
}

// ----------------------------------------------------------------------

void ConversationBranch::setGroupEchoRecursive(const bool groupEcho)
{
	setGroupEcho(groupEcho);

	int const numBranches = getNumberOfResponses ();
	for(int i = 0; i < numBranches; ++i)
	{
		getResponse (i)->setGroupEchoRecursive(groupEcho);
	}
}

// ----------------------------------------------------------------------

void ConversationBranch::setDebug (bool const debug)
{
	ConversationItem::setDebug (debug);

	for (int i = 0; i < getNumberOfResponses (); ++i)
		getResponse (i)->setDebug (debug);
}

// ----------------------------------------------------------------------

bool ConversationBranch::hasChildren () const
{
	return getNumberOfResponses () != 0;
}

// ----------------------------------------------------------------------

ConversationItem const * ConversationBranch::getLink (int const linkFamilyId) const
{
	if (ConversationItem::getLink (linkFamilyId))
		return this;

	for (int i = 0; i < getNumberOfResponses (); ++i)
	{
		ConversationItem const * const conversationItem = getResponse (i)->getLink (linkFamilyId);
		if (conversationItem)
			return conversationItem;
	}

	return 0;
}

// ======================================================================
// PRIVATE ConversationBranch
// ======================================================================

void ConversationBranch::load_0000 (Iff & iff)
{
	iff.enterForm (TAG_0000);

		ConversationItem::load (iff);

		iff.enterForm (TAG_RLST);

			while (iff.getNumberOfBlocksLeft ())
			{
				ConversationResponse * const response = new ConversationResponse(getStringIdTracker());
				response->load (iff);
				addResponse (response);
			}

		iff.exitForm (TAG_RLST);

	iff.exitForm (TAG_0000);
}

// ----------------------------------------------------------------------

void ConversationBranch::load_0001 (Iff & iff)
{
	iff.enterForm (TAG_0001);

		ConversationItem::load (iff);

		while (iff.getNumberOfBlocksLeft ())
		{
			ConversationResponse * const response = new ConversationResponse(getStringIdTracker());
			response->load (iff);
			addResponse (response);
		}

	iff.exitForm (TAG_0001);
}

// ======================================================================
// PUBLIC ConversationResponse
// ======================================================================

ConversationResponse::ConversationResponse (StringIdTracker * stringIdTracker) :
	ConversationItem (stringIdTracker),
	m_branchList ()
{
}

// ----------------------------------------------------------------------

ConversationResponse::~ConversationResponse ()
{
	std::for_each (m_branchList.begin (), m_branchList.end (), PointerDeleter ());
	m_branchList.clear ();
}

// ----------------------------------------------------------------------

void ConversationResponse::reset ()
{
	std::for_each (m_branchList.begin (), m_branchList.end (), PointerDeleter ());
	m_branchList.clear ();

	ConversationItem::reset ();
}

// ----------------------------------------------------------------------

void ConversationResponse::load (Iff & iff)
{
	iff.enterForm (TAG_RESP);

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		case TAG_0001:
			load_0001 (iff);
			break;

		default:
			{
				char tagBuffer [5];
				ConvertTagToString (iff.getCurrentName (), tagBuffer);

				char buffer [128];
				iff.formatLocation (buffer, sizeof (buffer));
				DEBUG_FATAL (true, ("unknown ConversationResponse version %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff.exitForm (TAG_RESP);
}

// ----------------------------------------------------------------------

void ConversationResponse::save (Iff & iff) const
{
	iff.insertForm (TAG_RESP);

		iff.insertForm (TAG_0001);

			ConversationItem::save (iff);

			for (int i = 0; i < getNumberOfBranches (); ++i)
			{
				ConversationBranch const * const branch = getBranch (i);
				branch->save (iff);
			}

		iff.exitForm (TAG_0001);

	iff.exitForm (TAG_RESP);
}

// ----------------------------------------------------------------------

int ConversationResponse::getNumberOfBranches () const
{
	return static_cast<int> (m_branchList.size ());
}

// ----------------------------------------------------------------------

ConversationBranch const * ConversationResponse::getBranch (int const index) const
{
	return m_branchList [static_cast<size_t> (index)];
}

// ----------------------------------------------------------------------

ConversationBranch * ConversationResponse::getBranch (int const index)
{
	return m_branchList [static_cast<size_t> (index)];
}

// ----------------------------------------------------------------------

void ConversationResponse::addBranch (ConversationBranch * const branch)
{
	branch->setParent (this);
	m_branchList.push_back (branch);
}

// ----------------------------------------------------------------------

void ConversationResponse::removeBranch (int const index)
{
	delete m_branchList [static_cast<size_t> (index)];

	IGNORE_RETURN (m_branchList.erase (m_branchList.begin () + index));
}

// ----------------------------------------------------------------------

void ConversationResponse::removeBranch (ConversationBranch const * const branch, bool const doDelete)
{
	BranchList::iterator iter = std::find (m_branchList.begin (), m_branchList.end (), branch);
	if (iter != m_branchList.end ())
	{
		if (doDelete)
			delete *iter;

		IGNORE_RETURN (m_branchList.erase (iter));
	}
}

// ----------------------------------------------------------------------

void ConversationResponse::promoteBranch (ConversationBranch const * const branch)
{
	size_t const size = m_branchList.size ();

	//-- can't promote with less than 2 items
	if (size < 2)
		return;

	//-- find branch
	size_t i = 0;
	for (i = 0; i < size; ++i)
		if (m_branchList [i] == branch)
			break;

	//-- if found and promote-able
	if (i < size - 1)
		std::swap (m_branchList [i], m_branchList [i + 1]);
}

// ----------------------------------------------------------------------

void ConversationResponse::demoteBranch (ConversationBranch const * const branch)
{
	size_t const size = m_branchList.size ();

	//-- can't demote with less than 2 items
	if (size < 2)
		return;

	//-- find branch
	size_t i = 0;
	for (i = 0; i < size; ++i)
		if (m_branchList [i] == branch)
			break;

	//-- if found and demote-able
	if (i > 0)
		std::swap (m_branchList [i], m_branchList [i - 1]);
}

// ----------------------------------------------------------------------

void ConversationResponse::setGroupEchoRecursive(const bool groupEcho)
{
	setGroupEcho(groupEcho);
	
	int const numBranches = getNumberOfBranches ();
	for(int i = 0; i < numBranches; ++i)
	{
		getBranch (i)->setGroupEchoRecursive(groupEcho);
	}
}

// ----------------------------------------------------------------------

void ConversationResponse::setDebug (bool const debug)
{
	ConversationItem::setDebug (debug);

	for (int i = 0; i < getNumberOfBranches (); ++i)
		getBranch (i)->setDebug (debug);
}

// ----------------------------------------------------------------------

bool ConversationResponse::hasChildren () const
{
	return getNumberOfBranches () != 0;
}

// ----------------------------------------------------------------------

ConversationItem const * ConversationResponse::getLink (int const linkFamilyId) const
{
	if (ConversationItem::getLink (linkFamilyId))
		return this;

	for (int i = 0; i < getNumberOfBranches (); ++i)
	{
		ConversationItem const * const conversationItem = getBranch (i)->getLink (linkFamilyId);
		if (conversationItem)
			return conversationItem;
	}

	return 0;
}

// ======================================================================
// PRIVATE ConversationResponse
// ======================================================================

void ConversationResponse::load_0000 (Iff & iff)
{
	iff.enterForm (TAG_0000);

		ConversationItem::load (iff);

		iff.enterForm (TAG_BLST);

			while (iff.getNumberOfBlocksLeft ())
			{
				ConversationBranch * const branch = new ConversationBranch(getStringIdTracker());
				branch->load (iff);
				addBranch (branch);
			}

		iff.exitForm (TAG_BLST);

	iff.exitForm (TAG_0000);
}

// ----------------------------------------------------------------------

void ConversationResponse::load_0001 (Iff & iff)
{
	iff.enterForm (TAG_0001);

		ConversationItem::load (iff);

		while (iff.getNumberOfBlocksLeft ())
		{
			ConversationBranch * const branch = new ConversationBranch(getStringIdTracker());
			branch->load (iff);
			addBranch (branch);
		}

	iff.exitForm (TAG_0001);
}

// ======================================================================
// PUBLIC Conversation
// ======================================================================

Conversation::Conversation () :
	ConversationResponse (new StringIdTracker),
	m_conditionGroup (new ScriptGroup),
	m_actionGroup (new ScriptGroup),
	m_labelGroup (new ScriptGroup),
	m_tokenTOGroup (new ScriptGroup),
	m_tokenDIGroup (new ScriptGroup),
	m_tokenDFGroup (new ScriptGroup),
	m_triggerText (),
	m_librarySet ()
{
}

// ----------------------------------------------------------------------

Conversation::~Conversation ()
{
	delete m_conditionGroup;
	delete m_actionGroup;
	delete m_labelGroup;
	delete m_tokenTOGroup;
	delete m_tokenDIGroup;
	delete m_tokenDFGroup;

	deleteStringIdTracker();
}

// ----------------------------------------------------------------------

void Conversation::reset ()
{
	ConversationResponse::reset ();
	m_conditionGroup->reset ();
	m_actionGroup->reset ();
	m_labelGroup->reset ();
	m_tokenTOGroup->reset ();
	m_tokenDIGroup->reset ();
	m_tokenDFGroup->reset ();
	m_triggerText.clear ();
	m_librarySet.clear ();
}

// ----------------------------------------------------------------------

bool Conversation::load (char const * const fileName)
{
	Iff iff;
	if (iff.open (fileName, true))
	{
		if (iff.getCurrentName () == TAG_CNV)
		{
			iff.enterForm (TAG_CNV);

				switch (iff.getCurrentName ())
				{
				case TAG_0000:
					load_0000 (iff);
					break;

				case TAG_0001:
					load_0001 (iff);
					break;

				case TAG_0002:
					load_0002 (iff);
					break;

				default:
					{
						char tagBuffer [5];
						ConvertTagToString (iff.getCurrentName (), tagBuffer);

						char buffer [128];
						iff.formatLocation (buffer, sizeof (buffer));
						DEBUG_FATAL (true, ("unknown Conversation version %s/%s", buffer, tagBuffer));
					}
					break;
				}

			iff.exitForm ();
		}
		else
		{
			load_old (iff);
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool Conversation::save (char const * const fileName) const
{
	Iff iff (1024);
	iff.insertForm (TAG_CNV);
		iff.insertForm (TAG_0002);

			m_conditionGroup->save (iff);
			m_actionGroup->save (iff);
			m_labelGroup->save (iff);
			m_tokenTOGroup->save (iff);
			m_tokenDIGroup->save (iff);
			m_tokenDFGroup->save (iff);

			iff.insertChunk (TAG_DATA);
				iff.insertChunkString (m_triggerText.c_str ());
			iff.exitChunk ();

			ConversationResponse::save (iff);
			
			iff.insertForm (TAG_LSET);
				iff.insertForm (TAG_0000);
					iff.insertChunk (TAG_DATA);

					{
						iff.insertChunkData (static_cast<int32> (m_librarySet.size ()));

						LibrarySet::iterator end = m_librarySet.end ();
						for (LibrarySet::iterator iterator = m_librarySet.begin (); iterator != end; ++iterator)
							iff.insertChunkString (iterator->c_str ());
					}

					iff.exitChunk ();
				iff.exitForm ();
			iff.exitForm ();

		iff.exitForm ();
	iff.exitForm ();

	return iff.write (fileName, true);
}

// ----------------------------------------------------------------------

void Conversation::load_old (Iff & iff)
{
	m_conditionGroup->load (iff);
	m_actionGroup->load (iff);
	ConversationResponse::load (iff);
	if (iff.enterForm (TAG_LSET, true))
	{
		iff.enterForm (TAG_0000);
			iff.enterChunk (TAG_DATA);

			{
				int numberOfLibraries = iff.read_int32 ();
				while (numberOfLibraries)
				{
					std::string libraryName;
					iff.read_string (libraryName);
					IGNORE_RETURN (m_librarySet.insert (libraryName));

					--numberOfLibraries;
				}
			}

			iff.exitChunk (true);
		iff.exitForm (true);
	}
	else
	{
		IGNORE_RETURN (m_librarySet.insert (std::string ("ai_lib")));
		IGNORE_RETURN (m_librarySet.insert (std::string ("chat")));
	}
}

// ----------------------------------------------------------------------

void Conversation::load_0000 (Iff & iff)
{
	iff.enterForm (TAG_0000);

		m_conditionGroup->load (iff);
		m_actionGroup->load (iff);
		m_labelGroup->load (iff);
		ConversationResponse::load (iff);

		iff.enterForm (TAG_LSET);
			iff.enterForm (TAG_0000);
				iff.enterChunk (TAG_DATA);

				{
					int numberOfLibraries = iff.read_int32 ();
					while (numberOfLibraries)
					{
						std::string libraryName;
						iff.read_string (libraryName);
						IGNORE_RETURN (m_librarySet.insert (libraryName));

						--numberOfLibraries;
					}
				}

				iff.exitChunk ();
			iff.exitForm ();
		iff.exitForm ();

	iff.exitForm ();
}

// ----------------------------------------------------------------------

void Conversation::load_0001 (Iff & iff)
{
	iff.enterForm (TAG_0001);

		m_conditionGroup->load (iff);
		m_actionGroup->load (iff);
		m_labelGroup->load (iff);

		iff.enterChunk (TAG_DATA);
			iff.read_string (m_triggerText);
		iff.exitChunk ();

		ConversationResponse::load (iff);

		iff.enterForm (TAG_LSET);
			iff.enterForm (TAG_0000);
				iff.enterChunk (TAG_DATA);

				{
					int numberOfLibraries = iff.read_int32 ();
					while (numberOfLibraries)
					{
						std::string libraryName;
						iff.read_string (libraryName);
						IGNORE_RETURN (m_librarySet.insert (libraryName));

						--numberOfLibraries;
					}
				}

				iff.exitChunk ();
			iff.exitForm ();
		iff.exitForm ();

	iff.exitForm ();
}

// ----------------------------------------------------------------------

void Conversation::load_0002 (Iff & iff)
{
	iff.enterForm (TAG_0002);

		m_conditionGroup->load (iff);
		m_actionGroup->load (iff);
		m_labelGroup->load (iff);
		m_tokenTOGroup->load (iff);
		m_tokenDIGroup->load (iff);
		m_tokenDFGroup->load (iff);

		iff.enterChunk (TAG_DATA);
			iff.read_string (m_triggerText);
		iff.exitChunk ();

		ConversationResponse::load (iff);

		iff.enterForm (TAG_LSET);
			iff.enterForm (TAG_0000);
				iff.enterChunk (TAG_DATA);

				{
					int numberOfLibraries = iff.read_int32 ();
					while (numberOfLibraries)
					{
						std::string libraryName;
						iff.read_string (libraryName);
						IGNORE_RETURN (m_librarySet.insert (libraryName));

						--numberOfLibraries;
					}
				}

				iff.exitChunk ();
			iff.exitForm ();
		iff.exitForm ();

	iff.exitForm ();
}

// ----------------------------------------------------------------------

ScriptGroup * Conversation::getConditionGroup ()
{
	return m_conditionGroup;
}

// ----------------------------------------------------------------------

ScriptGroup const * Conversation::getConditionGroup () const
{
	return m_conditionGroup;
}

// ----------------------------------------------------------------------

ScriptGroup * Conversation::getActionGroup ()
{
	return m_actionGroup;
}

// ----------------------------------------------------------------------

ScriptGroup const * Conversation::getActionGroup () const
{
	return m_actionGroup;
}

// ----------------------------------------------------------------------

ScriptGroup * Conversation::getLabelGroup ()
{
	return m_labelGroup;
}

// ----------------------------------------------------------------------

ScriptGroup const * Conversation::getLabelGroup () const
{
	return m_labelGroup;
}

// ----------------------------------------------------------------------

ScriptGroup * Conversation::getTokenTOGroup ()
{
	return m_tokenTOGroup;
}

// ----------------------------------------------------------------------

ScriptGroup const * Conversation::getTokenTOGroup () const
{
	return m_tokenTOGroup;
}

// ----------------------------------------------------------------------

ScriptGroup * Conversation::getTokenDIGroup ()
{
	return m_tokenDIGroup;
}

// ----------------------------------------------------------------------

ScriptGroup const * Conversation::getTokenDIGroup () const
{
	return m_tokenDIGroup;
}

// ----------------------------------------------------------------------

ScriptGroup * Conversation::getTokenDFGroup ()
{
	return m_tokenDFGroup;
}

// ----------------------------------------------------------------------

ScriptGroup const * Conversation::getTokenDFGroup () const
{
	return m_tokenDFGroup;
}

// ----------------------------------------------------------------------

std::string const & Conversation::getTriggerText () const
{
	return m_triggerText;
}

// ----------------------------------------------------------------------

void Conversation::setTriggerText (std::string const & triggerText)
{
	m_triggerText = triggerText;
}

// ----------------------------------------------------------------------

Conversation::LibrarySet & Conversation::getLibrarySet ()
{
	return m_librarySet;
}

// ----------------------------------------------------------------------

Conversation::LibrarySet const & Conversation::getLibrarySet () const
{
	return m_librarySet;
}

// ======================================================================

