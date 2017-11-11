//======================================================================
//
// SwgCuiServerData.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiServerData_H
#define INCLUDED_SwgCuiServerData_H

//----------------------------------------------------------------------

#include "Singleton/Singleton.h"
#include "sharedMessageDispatch/Receiver.h"

namespace MessageDispatch
{
	class Emitter;
}

//======================================================================

class SwgCuiServerData : public Singleton<SwgCuiServerData>, public MessageDispatch::Receiver
{
public:
	
	struct Messages
	{
		static const char * const WHO_LIST_UPDATED;
	};
	
	typedef stdvector<Unicode::String>::fwd StringVector;
	const StringVector & getWhoList () const;
	
	virtual void        receiveMessage (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	                   SwgCuiServerData ();
	                   ~SwgCuiServerData ();
	
private:
	                   SwgCuiServerData (const SwgCuiServerData & rhs);
	SwgCuiServerData & operator= (const SwgCuiServerData & rhs);

	StringVector *                      m_whoList;
	MessageDispatch::Emitter *          m_emitter;
};

//----------------------------------------------------------------------

inline const SwgCuiServerData::StringVector & SwgCuiServerData::getWhoList () const
{
	NOT_NULL (m_whoList);
	return *m_whoList;
}

//======================================================================

#endif
