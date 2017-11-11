//======================================================================
//
// SwgCuiAttachmentList.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAttachmentList.h"

#include "clientGame/AttachmentData.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/Waypoint.h"

#include "UIPage.h"

//======================================================================

namespace SwgCuiAttachmentListNamespace
{
	class MyIconCallback : 
	public CuiIconManagerCallback
	{
	public:
		virtual void       overrideTooltip (const ClientObject & , Unicode::String & ) const
		{
		}
		
		virtual void       overrideCursor  (const ClientObject & obj, UICursor *& cursor) const
		{
			UNREF (obj);
			cursor = 0;
		}
	};
}

using namespace SwgCuiAttachmentListNamespace;

//----------------------------------------------------------------------

SwgCuiAttachmentList::Renderable::Renderable () : 
object         (0),
attachmentData (new AttachmentData)
{
}

//----------------------------------------------------------------------

SwgCuiAttachmentList::Renderable::~Renderable ()
{
	delete attachmentData;
	delete object;

	attachmentData = 0;
	object = 0;
}

//----------------------------------------------------------------------

UIWidget * SwgCuiAttachmentList::Renderable::fetchIcon (bool doRegister, CuiIconManagerCallback * callback) const
{
	if (object)
	{
		CuiWidget3dObjectListViewer * const viewer = CuiIconManager::createObjectIcon (*object, CuiDragInfo::DragTypes::AttachmentData);
		viewer->setDrawName       (true);
		viewer->SetContextCapable (false, false);

		CuiDragInfo cdi;
		cdi.set       (*attachmentData);
		cdi.setWidget (*viewer);
		UIPoint dummy;
		UIWidget * const dragWidget = viewer->GetCustomDragWidget (UIPoint::zero, dummy);
		if (dragWidget)
			cdi.setWidget (*dragWidget);

		if (doRegister)
			CuiIconManager::registerObjectIcon (*viewer, callback, false);

		return viewer; 
	}
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiAttachmentList::Renderable::releaseIcon (UIWidget & widget)
{
	CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(&widget);
	if (viewer)
	{
		CuiIconManager::unregisterObjectIcon (*viewer);
	}
}

//----------------------------------------------------------------------

void SwgCuiAttachmentList::Renderable::releaseIconsForPage (UIPage & page)
{
	CuiIconManager::unregisterIconsForPage (page);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


CuiIconManagerCallback  * SwgCuiAttachmentList::cms_defaultIconCallback = 0;
int                       SwgCuiAttachmentList::ms_instanceCount        = 0;

//----------------------------------------------------------------------

SwgCuiAttachmentList::SwgCuiAttachmentList () :
m_rv (new RenderableVector)
{
	++ms_instanceCount;
	if (!cms_defaultIconCallback)
		cms_defaultIconCallback = new MyIconCallback;
}

//----------------------------------------------------------------------

SwgCuiAttachmentList::~SwgCuiAttachmentList ()
{
	clear ();
	delete m_rv;
	m_rv = 0;

	--ms_instanceCount;

	if (ms_instanceCount <= 0)
	{
		delete cms_defaultIconCallback;
		cms_defaultIconCallback = 0;
	}
}

//----------------------------------------------------------------------

void SwgCuiAttachmentList::set (const Attachments & attachments)
{
	clear ();

	for (Attachments::const_iterator it = attachments.begin (); it != attachments.end (); ++it)
	{
		AttachmentData * const ad = *it;
		push_back (*ad);
	}
}

//----------------------------------------------------------------------

SwgCuiAttachmentList::Renderable * SwgCuiAttachmentList::push_back (const AttachmentData & ad)
{
	Renderable * const r = new Renderable;
	*(r->attachmentData) = ad;
	
	const int type = ad.getType ();
	switch (type)
	{
	case AttachmentData::T_waypoint:
		{
			const WaypointDataBase * const wd = ad.getWaypointData ();
			NOT_NULL (wd);
			Waypoint wp(ClientObject::getNextFakeNetworkId());
			wp.set(*wd);
			ClientWaypointObject * const waypoint = ClientWaypointObject::createClientWaypoint (wp);
			NOT_NULL (waypoint);
			r->object = waypoint;
		}
		break;
	default:
		break;
	}
	
	m_rv->push_back (r);

	return r;
}

//----------------------------------------------------------------------

void SwgCuiAttachmentList::clear ()
{
	for (RenderableVector::iterator it = m_rv->begin (); it != m_rv->end (); ++it)
	{
		Renderable * renderable = *it;
		delete renderable;
	}

	m_rv->clear ();
}

//----------------------------------------------------------------------

const SwgCuiAttachmentList::Renderable * SwgCuiAttachmentList::get (int index) const
{
	if (index < 0 || index >= static_cast<int>(m_rv->size ()))
		return 0;

	return (*m_rv) [index];
}

//----------------------------------------------------------------------

SwgCuiAttachmentList::Renderable * SwgCuiAttachmentList::get (int index)
{
	if (index < 0 || index >= static_cast<int>(m_rv->size ()))
		return 0;

	return (*m_rv) [index];
}

//----------------------------------------------------------------------

bool SwgCuiAttachmentList::empty () const
{
	return m_rv->empty ();
}

//----------------------------------------------------------------------

SwgCuiAttachmentList::Renderable * SwgCuiAttachmentList::back                 ()
{
	if (m_rv->empty ())
		return 0;
	
	return m_rv->back ();
}

//----------------------------------------------------------------------

void SwgCuiAttachmentList::erase  (int index)
{
	if (index < 0 || index >= static_cast<int>(m_rv->size ()))
		return;

	RenderableVector::iterator it = m_rv->begin ();
	std::advance (it, index);
	DEBUG_FATAL (it == m_rv->end (), ("bad"));
	Renderable * const renderable = *it;
	delete renderable;
	m_rv->erase (it);
}

//----------------------------------------------------------------------

int SwgCuiAttachmentList::size () const
{
	return static_cast<int>(m_rv->size ());
}

//----------------------------------------------------------------------

void SwgCuiAttachmentList::constructOob         (Unicode::String & oob) const
{
	for (RenderableVector::const_iterator it = m_rv->begin (); it != m_rv->end (); ++it)
	{
		const Renderable * const renderable = *it;
		NOT_NULL (renderable->attachmentData);
		AttachmentData & ad = *renderable->attachmentData;
		ad.packToOob (oob, -3);	
	}
}

//======================================================================
