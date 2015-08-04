//======================================================================
//
// SwgCuiContainerProvider.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiContainerProvider.h"

//======================================================================

SwgCuiContainerProvider::SwgCuiContainerProvider () :
m_contentDirty  (true),
m_providerDirty (true),
m_filter        (0)
{
}

//----------------------------------------------------------------------

SwgCuiContainerProvider::~SwgCuiContainerProvider ()
{
	m_filter = 0;
}

//----------------------------------------------------------------------


void SwgCuiContainerProvider::setContentDirty (bool b)
{
	m_contentDirty = b;
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::setProviderDirty (bool b)
{
	m_providerDirty = b;
}

//----------------------------------------------------------------------

const ClientObject * SwgCuiContainerProvider::getContainerObject     () const
{
	return 0;
}

//----------------------------------------------------------------------

ClientObject * SwgCuiContainerProvider::getContainerObject     ()
{
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::setFilter  (Filter * filter)
{
	m_filter = filter;
}

//----------------------------------------------------------------------

const std::string & SwgCuiContainerProvider::getSlotName            () const
{
	static const std::string emptyStr;
	return emptyStr;
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::setContainerObject     (ClientObject * , const std::string & )
{
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::setObjectSorting       (const IntVector &)
{
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::onOpeningAndClosing    (bool )
{
}

//----------------------------------------------------------------------

bool SwgCuiContainerProvider::handleDragOver          (const CuiDragInfo & , bool & )
{
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProvider::handleDrop              (const CuiDragInfo & )
{
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProvider::handleDragStart         (const CuiDragInfo & )
{
	return false;
}

//----------------------------------------------------------------------

int SwgCuiContainerProvider::getTotalVolume          () const
{
	return 0;
}

//----------------------------------------------------------------------

int SwgCuiContainerProvider::getCurrentVolume        () const
{
	return 0;
}

//----------------------------------------------------------------------

int SwgCuiContainerProvider::getTotalVolumeLimitedByParents () const
{
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::applyStates             ()
{
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::tryToApplyOrdering(stdvector<NetworkId>::fwd const &)
{
}

//----------------------------------------------------------------------

void SwgCuiContainerProvider::tryToPlaceItemBefore(const NetworkId & , const NetworkId & )
{
}
//======================================================================
