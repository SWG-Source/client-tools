// ======================================================================
//
// TriggerWindow.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "TriggerWindow.h"
#include "TriggerWindow.moc"

#include "UnicodeUtils.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"

#include <qtable.h>

// ======================================================================

TriggerWindow::TriggerWindow(QWidget *parent, const char *name)
: BaseTriggerWindow (parent, name),
  MessageDispatch::Receiver(),
  m_networkId()
{
	connectToMessage("ConGenericMessage");
}

//-----------------------------------------------------------------

TriggerWindow::~TriggerWindow()
{
}

//-----------------------------------------------------------------

void TriggerWindow::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType("ConGenericMessage"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ConGenericMessage cg(ri);
		const std::string msg = cg.getMsg();
		std::string line;
		std::vector<std::string> lines;
		size_t startpos = 0;
		size_t endpos;

		while(Unicode::getFirstToken(msg, startpos, endpos, line, "\n"))
		{
			lines.push_back(line);
			startpos = endpos + 1;
		}

		unsigned int numTriggers;
		uint32 oid;
		int result = sscanf(lines[0].c_str(), "Listing %d trigger volumes for object %d", &numTriggers, &oid);

		//validate that this is the trigger listing for the object we care about
		if(oid != m_networkId.getValue())
			return;
		
		if(result > 0)
		{
			//clear out current table entries
			int numRows = m_triggerTable->numRows();
			for(int i =0; i < numRows; ++i)
				m_triggerTable->removeRow(i);

			m_triggerTable->insertRows(0, static_cast<int>(numTriggers));

			//the first line must list the number of triggers following, and the number of additional lines we read should be this big
			//subtract 2, one for the header line and one for the footer line
			DEBUG_FATAL((result != 2) || (numTriggers != lines.size()-2), ("bad data sent in getTriggerVolumes"));

			char name[1024];
			real radius;
			int row = 0;
			for(unsigned int j = 1; j <= numTriggers; ++j, ++row)
			{
				result = sscanf(lines[j].c_str(), "trigger volume for object %d %s %f\n", &oid, name, &radius);
				//put the new entry in the table
				m_triggerTable->setText(row, 0, name);
				QString radiusString;
				IGNORE_RETURN(radiusString.setNum(radius));
				m_triggerTable->setText(row, 1, radiusString);
			}
		}
	}
}

//-----------------------------------------------------------------

void TriggerWindow::setNetworkId(const NetworkId& nid)
{
	m_networkId = nid;
}

//-----------------------------------------------------------------
