//======================================================================
//
// SwgCuiMfdStatus.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiMfdStatus.h"

#include "sharedMessageDispatch/Transceiver.h"
#include "UIImage.h"

//======================================================================

SwgCuiMfdStatus::SwgCuiMfdStatus(char const * const mediatorName, UIPage & page) :
SwgCuiLockableMediator(mediatorName, page),
m_objectId(),
m_callback(new MessageDispatch::Callback),
m_directionArrow(0)
{
	getCodeDataObject (TUIImage, m_directionArrow, "directionArrow", true);
}

//----------------------------------------------------------------------

SwgCuiMfdStatus::~SwgCuiMfdStatus ()
{
	delete m_callback;
	m_callback = 0;
	
	m_directionArrow = 0;
}

// ----------------------------------------------------------------------

void SwgCuiMfdStatus::setObjectName(Unicode::String const & /*name*/)
{
}

// ----------------------------------------------------------------------

void SwgCuiMfdStatus::update(ClientMfdStatusUpdateMessage const &)
{
}


//======================================================================
