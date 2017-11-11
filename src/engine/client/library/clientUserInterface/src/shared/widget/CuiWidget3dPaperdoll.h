// ======================================================================
//
// CuiWidget3dPaperdoll.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiWidget3dPaperdoll_H
#define INCLUDED_CuiWidget3dPaperdoll_H

// ======================================================================

#include "clientUserInterface/CuiWidget3dObjectViewer.h"
#include "sharedCollision/BoxExtent.h"
#include "clientUserInterface/CuiContainerSelectionChanged.h"

namespace MessageDispatch
{
	class Emitter;
};

class TangibleObject;
class SlotId;

//-----------------------------------------------------------------

class CuiWidget3dPaperdoll : public CuiWidget3dObjectViewer
{
public:

	static const char * const TypeName;

	struct PropertyName
	{

	};
	                          CuiWidget3dPaperdoll   (); 
	virtual                  ~CuiWidget3dPaperdoll   ();
	virtual UIBaseObject *    Clone                  () const { return new CuiWidget3dPaperdoll; }
	virtual void              Link                   ();

	virtual void              Render                 (UICanvas &) const;
	virtual bool              OnMessage              (UIWidget *context, const UIMessage & msg);
	
	virtual const char       *GetTypeName            () const { return TypeName; }

	virtual void              Notify                 (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );
	virtual bool              SetProperty            (const UILowerString & Name, const UIString &Value );
	virtual bool              GetProperty            (const UILowerString & Name, UIString &Value ) const;

	virtual void              GetPropertyNames       (UIPropertyNameVector &, bool forCopy ) const;

	virtual bool              IsDropOk               (const UIWidget & widget, const UIString & DragType, const UIPoint & point);
	virtual UIWidget *        GetCustomDragWidget    (const UIPoint & point, UIPoint & offset);

	CuiContainerSelectionChanged::TransceiverType &     getTransceiverSelection ();
	
	TangibleObject *          findSelectedObject     ();

	void                      setAllowManipulation   (bool b);

private:
	                          CuiWidget3dPaperdoll (const CuiWidget3dPaperdoll & rhs);
	                          CuiWidget3dPaperdoll & operator= (const CuiWidget3dPaperdoll & rhs);

	UICursor *                m_equipCursor;
	UIPoint                   m_lastCursorPos;
	BoxExtent                 m_selectedBoxExtent;
	int                       m_selectedWearableIndex;

	Transform                 m_selectedAttachableTransform;
	int                       m_selectedAttachableIndex;

	bool                      m_dragInProgress;
	BoxExtent                 m_draggingBoxExtent;

	struct DragTargetInfo
	{
		size_t    arrangement;
		Transform transform;
		bool      selected;
		int       firstSlotIdNumber;
		DragTargetInfo (size_t a, int slot, const Transform & t) : arrangement (a), transform (t), selected (false), firstSlotIdNumber (slot) {}
	};

	typedef stdvector<DragTargetInfo>::fwd  DragTargetInfoVector;
	DragTargetInfoVector *                  m_dragTargetInfo;

	SlotId *                   m_hairSlot;

	CuiContainerSelectionChanged::TransceiverType * m_transceiverSelection;

	bool                       m_allowManipulation;
};

//----------------------------------------------------------------------

inline CuiContainerSelectionChanged::TransceiverType &     CuiWidget3dPaperdoll::getTransceiverSelection ()
{
	return *NON_NULL (m_transceiverSelection);
}

// ======================================================================

#endif
