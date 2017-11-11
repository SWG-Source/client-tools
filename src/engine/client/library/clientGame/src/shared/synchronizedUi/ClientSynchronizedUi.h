// ======================================================================
//
// ClientSynchronizedUi.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ClientSynchronizedUi_H
#define	INCLUDED_ClientSynchronizedUi_H


#include "Archive/AutoDeltaByteStream.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class BaselinesMessage;
class DeltasMessage;
class ClientObject;

class ClientSynchronizedUi
{
public:
	explicit ClientSynchronizedUi(ClientObject& owner);
	virtual ~ClientSynchronizedUi() = 0;

	void applyBaselines(const BaselinesMessage& source);
	void applyDeltas(const DeltasMessage& source);
	void clearDeltas();

	const ClientObject*           getOwner () const;
	ClientObject*                 getOwner ();

protected:
	void                          addToUiPackage(Archive::AutoDeltaVariableBase & source);

private:
	ClientSynchronizedUi();
	ClientSynchronizedUi(const ClientSynchronizedUi &);
	ClientSynchronizedUi& operator= (const ClientSynchronizedUi &);

	ClientObject*                 m_owner;
	Archive::AutoDeltaByteStream  m_uiPackage;

	virtual void onBaselinesRecieved();
};

//----------------------------------------------------------------------

inline const ClientObject* ClientSynchronizedUi::getOwner () const
{
	return m_owner;
}

//----------------------------------------------------------------------

inline ClientObject* ClientSynchronizedUi::getOwner ()
{
	return m_owner;
}

//----------------------------------------------------------------------


#endif
