// ======================================================================
//
// CuiWidget3dPaperdoll.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWidget3dPaperdoll.h"

#include "UICanvas.h"
#include "UICursor.h"
#include "UIMessage.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/ObjectListCamera.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/SlottedContainmentProperty.h"

// ======================================================================

const char * const CuiWidget3dPaperdoll::TypeName = "CuiWidget3dPaperdoll";

CuiWidget3dPaperdoll::CuiWidget3dPaperdoll () :
CuiWidget3dObjectViewer (),
m_equipCursor           (0),
m_lastCursorPos         (),
m_selectedBoxExtent     (),
m_selectedWearableIndex (-1),
m_selectedAttachableTransform (),
m_selectedAttachableIndex (-1),
m_dragInProgress        (false),
m_draggingBoxExtent     (),
m_dragTargetInfo        (new DragTargetInfoVector),
m_hairSlot              (new SlotId (SlotIdManager::findSlotId (ConstCharCrcLowerString ("hair")))),
m_transceiverSelection  (new CuiContainerSelectionChanged::TransceiverType),
m_allowManipulation     (false)
{
	DEBUG_FATAL (*m_hairSlot == SlotId::invalid, ("no hair."));
	SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
}

//-----------------------------------------------------------------

CuiWidget3dPaperdoll::~CuiWidget3dPaperdoll ()
{
	delete m_transceiverSelection;
	m_transceiverSelection  = 0;

	delete m_dragTargetInfo;
	m_dragTargetInfo = 0;

	m_equipCursor    = 0;
	delete m_hairSlot;
	m_hairSlot       = 0;
}

//----------------------------------------------------------------------

void CuiWidget3dPaperdoll::Link ()
{
	m_equipCursor = dynamic_cast<UICursor*>(GetObjectFromPath ("/styles.cursors.equip", TUICursor));
	CuiWidget3dObjectViewer::Link ();
}

//-----------------------------------------------------------------

void CuiWidget3dPaperdoll::Render( UICanvas & canvas ) const
{
	CuiWidget3dObjectViewer::RenderStart (canvas);
	
	if (m_allowManipulation)
	{
		const UIPoint worldPoint (canvas.GetTranslation ());
	
		Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());

		if (m_dragInProgress && !m_dragTargetInfo->empty ())
		{
			for (DragTargetInfoVector::const_iterator it = m_dragTargetInfo->begin (); it != m_dragTargetInfo->end (); ++it)
			{
				const Transform & transform = (*it).transform;
				const bool selected = (*it).selected;
				
				Transform fullTransform (Transform::IF_none);
				fullTransform.multiply (m_objectTransform, transform);
				Graphics::setObjectToWorldTransformAndScale (fullTransform, Vector::xyz111);
				Graphics::drawExtent                        (&m_draggingBoxExtent, selected ? VectorArgb::solidGreen : VectorArgb::solidYellow);
			}
		}
		
		if (m_selectedWearableIndex >= 0)
		{
			Graphics::setObjectToWorldTransformAndScale (m_objectTransform, Vector::xyz111);
			Graphics::drawExtent                        (&m_selectedBoxExtent, VectorArgb::solidGreen);
		}
		
		if (m_selectedAttachableIndex >= 0)
		{		
			Graphics::setObjectToWorldTransformAndScale (m_selectedAttachableTransform, Vector::xyz111);
			Graphics::drawExtent                        (&m_selectedBoxExtent, VectorArgb::solidGreen);
		}
	}

	CuiWidget3dObjectViewer::RenderStop ();
}

//-----------------------------------------------------------------

bool CuiWidget3dPaperdoll::OnMessage( UIWidget *context, const UIMessage & msg )
{
	if (!m_renderObject)
		return true;
	
	if (msg.Type == UIMessage::LeftMouseDoubleClick && m_allowManipulation)
	{
		TangibleObject * const obj    = findSelectedObject ();		
		if (obj)
		{
			TangibleObject * const target = NON_NULL (dynamic_cast<TangibleObject *>(m_renderObject.getPointer ()));
			IGNORE_RETURN (CuiInventoryManager::unequipItem (*obj, *target));
			return false;
		}
	}
	
	//----------------------------------------------------------------------
	
	if (msg.Type == UIMessage::LeftMouseDown && m_allowManipulation)
	{
		ClientObject * const obj = findSelectedObject ();
		
		if (obj)
			m_transceiverSelection->emitMessage (CuiContainerSelectionChanged::Payload (0, obj));
		else
			m_transceiverSelection->emitMessage (CuiContainerSelectionChanged::Payload (0, 0));
	}
	
	//-----------------------------------------------------------------


	if (msg.Type == UIMessage::MouseMove)
	{
		m_lastCursorPos = msg.MouseCoords + GetWorldLocation ();
		//-- if we are not mouse draggable, return.  otherwise let the superclass drag the mouse around
	//	if (!getMouseDraggable ())
	//		return true;
	}

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::DragOver)
	{
		if (m_dragInProgress || !msg.DragObject)
			return false;

		SetMouseCursor (0);

		m_dragInProgress = true;
		m_dragTargetInfo->clear ();

		CuiDragInfo dragInfo (*msg.DragObject);

		ClientObject * const moving_obj  = dragInfo.getClientObject ();
		if (moving_obj)
		{
			TangibleObject * const tangible_obj = dynamic_cast<TangibleObject *>(moving_obj);

			if (tangible_obj)
			{	
				Appearance * const app = NON_NULL (moving_obj->getAppearance ());
				SkeletalAppearance2 * const moving_skelApp = dynamic_cast<SkeletalAppearance2 *>(app);
				SlottedContainer * const slottedContainer = ContainerInterface::getSlottedContainer (*m_renderObject);
				const SlottedContainmentProperty * const slottedProperty = ContainerInterface::getSlottedContainmentProperty(*tangible_obj);
	
				CreatureObject * const creatureObject = dynamic_cast<CreatureObject *>(m_renderObject.getPointer());
				if (creatureObject)
				{
					// See if this item can be worn by the creature object
				
					bool const wearable = creatureObject->isAppearanceWearable(*moving_obj);
				
					if (!wearable)
					{
						return false;
					}
				}

				if (slottedContainer && slottedProperty)
				{
					if (moving_skelApp)
					{
						std::vector<int> validArrangements;
						Container::ContainerErrorCode tmp = Container::CEC_Success;

						slottedContainer->getValidArrangements(*moving_obj, validArrangements, tmp, false, false);
						for (std::vector<int>::iterator i = validArrangements.begin(); i != validArrangements.end(); ++i)
						{
							const SlottedContainmentProperty::SlotArrangement & slotIds = slottedProperty->getSlotArrangement (*i);
							if (slotIds.empty ())
								continue;

							m_dragTargetInfo->push_back (DragTargetInfo (*i, slotIds[0].getSlotId(), Transform::identity));
						}


	//					m_dragTargetInfo->push_back (DragTargetInfo (0, 0, Transform::identity));

						const BoxExtent & box = moving_skelApp->getExactMeshExtent ();
						const Sphere & sphere = box.getSphere ();

						m_draggingBoxExtent.set (box.getMin (), box.getMax (), sphere.getCenter (), sphere.getRadius ());
					}
					else
					{
						const Extent * const extent = app->getExtent ();
						const Sphere & sphere       = extent->getSphere ();
						const BoxExtent * box       = dynamic_cast<const BoxExtent *> (extent);

						if (box)
							m_draggingBoxExtent.set (box->getMin (), box->getMax (), sphere.getCenter (), sphere.getRadius ());
						else
						{
							const Vector corner (sphere.getRadius () * Vector::xyz111);
							m_draggingBoxExtent.set (sphere.getCenter () - corner, sphere.getCenter () + corner, sphere.getCenter (), sphere.getRadius ());
						}
						
						SkeletalAppearance2 * const skelApp = dynamic_cast<SkeletalAppearance2 *>(m_renderObject->getAppearance ());
						
						if (!skelApp)
							return false;

						// Choose to use the Skeleton instance for the currently displayed LOD.
						// This is important, as the jointToRoot transform is used and may be
						// slightly different from LOD to LOD.
						const Skeleton * const skeleton = skelApp->getDisplayLodSkeleton ();
						if (skeleton)
						{
							const Transform * const jointToRootTransformArray = NON_NULL (skeleton->getJointToRootTransformArray());
							
							typedef std::vector<int> SizeTVector;
							SizeTVector returnList;
							Container::ContainerErrorCode tmp = Container::CEC_Success;
							slottedContainer->getValidArrangements (*moving_obj, returnList, tmp);
							
							for (SizeTVector::const_iterator it = returnList.begin (); it != returnList.end (); ++it)
							{
								const int arrangementId = *it;
								
								const SlottedContainmentProperty::SlotArrangement & slotIds = slottedProperty->getSlotArrangement (static_cast<int>(arrangementId));
								
								if (slotIds.empty ())
									continue;

								// use the appearance hardpoint from the first slot that has an associated appearance hardpoint
								ArrangementDescriptor::SlotIdVector::const_iterator svEndIt = slotIds.end ();
								for (ArrangementDescriptor::SlotIdVector::const_iterator svIt = slotIds.begin (); svIt != svEndIt; ++svIt)
								{
									const SlotId &slotId = *svIt;

									if (!SlotIdManager::isSlotAppearanceRelated (slotId))
										continue;

									const CrcString &hardpointName = SlotIdManager::getSlotHardpointName (slotId);
									if (hardpointName.isEmpty())
										continue;

									int transformIndex = 0;
									bool found = false;

									skeleton->findTransformIndex (hardpointName, &transformIndex, &found);

									if (!found || transformIndex < 0)
										continue;

									m_dragTargetInfo->push_back (DragTargetInfo (static_cast<size_t>(arrangementId), slotIds [0].getSlotId (), jointToRootTransformArray [transformIndex]));
								}
							}
						}
					}
				}
			}
		}
		
		return false;
	}

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::DragStart)
	{
		return false;
	}
	
	//-----------------------------------------------------------------
	//-- drop an item onto the player
	//-----------------------------------------------------------------

	if (msg.Type == UIMessage::DragEnd)
	{
		if (!msg.DragObject)
			return false;

		CuiDragInfo dragInfo (*msg.DragObject);
		ClientObject * const moving_obj              = dragInfo.getClientObject ();
		TangibleObject * const target          = NON_NULL (dynamic_cast<TangibleObject *>(m_renderObject.getPointer ()));
		
		if (moving_obj && m_dragInProgress && !m_dragTargetInfo->empty ())
		{			
			if (m_allowManipulation)
			{
				//- @todo it might be necessary to force an update like Notify usually does
				const DragTargetInfo * dti = 0;
				
				for (DragTargetInfoVector::const_iterator it = m_dragTargetInfo->begin (); it != m_dragTargetInfo->end (); ++it)
				{
					if ((*it).selected)
					{
						dti = &(*it);
						break;
					}
				}
				
				if (dti)
				{			
					//-- @todo: we don't want to make a copy
					TangibleObject * const tangible_obj = NON_NULL (dynamic_cast<TangibleObject *>(moving_obj));
					
					if (!CuiInventoryManager::equipItem (*tangible_obj, *target, dti->arrangement))
					{
						WARNING (true, ("somehow could not add the item in the specified arrangement.\n"));
					}
				}
			}
			else
			{
				TangibleObject * const tangible_obj = NON_NULL (dynamic_cast<TangibleObject *>(moving_obj));
					
				if (!CuiInventoryManager::equipItem (*tangible_obj, *target))
				{
					WARNING (true, ("somehow could not add the item in default arrangement.\n"));
				}
			}
		}
		
		m_dragInProgress = false;
		
		if (msg.DragSource == this)
			msg.DragObject->Detach (0);
		
		return false;
	}

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::DragCancel)
	{
		m_dragInProgress = false;
		
		if (msg.DragSource == this)
		{
			msg.DragObject->Detach (0);
			return false;
		}

		return true;
	}

	//----------------------------------------------------------------------

	if (msg.Type == UIMessage::MouseExit)
	{
		m_dragInProgress = false;
		return true;
	}

	//----------------------------------------------------------------------

	return CuiWidget3dObjectViewer::OnMessage (context, msg);
}

//-----------------------------------------------------------------

void CuiWidget3dPaperdoll::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{
	if (!m_renderObject)
		return;
	
	m_selectedAttachableIndex = -1;
	m_selectedWearableIndex   = -1;

	if (IsUnderMouse ())
	{
		SetLocalTooltip (Unicode::String ());

		const Vector worldStart (m_camera->getPosition_w ());
		const Vector worldEnd   (worldStart + 10000.0f * m_camera->rotate_o2w (m_camera->reverseProjectInScreenSpace (m_lastCursorPos.x, m_lastCursorPos.y))); //lint !e1702 // bug				

		//----------------------------------------------------------------------
		//-- we are dragging an object, see if there is an appropraite place to drop it

		if (m_dragInProgress)
		{
			if (m_allowManipulation)
			{				
				float            min_intersection  = 10000.0f;
				DragTargetInfo * min_dti           = 0;
				
				for (DragTargetInfoVector::iterator it = m_dragTargetInfo->begin (); it != m_dragTargetInfo->end (); ++it)
				{
					DragTargetInfo & dti = *it;
					dti.selected = false;
					Transform fullTransform (Transform::IF_none);
					fullTransform.multiply (m_objectTransform, dti.transform); 
					
					const Vector objectStart = fullTransform.rotateTranslate_p2l (worldStart);
					const Vector objectEnd   = fullTransform.rotateTranslate_p2l (worldEnd);
					
					float t = 0.0f;
					if (m_draggingBoxExtent.intersect (objectStart, objectEnd, &t))
					{
						if (t < min_intersection)
						{
							min_intersection        = t;
							min_dti                 = &dti;
						}
					}
				}

				if (min_dti)
				{
					min_dti->selected = true;
					SetLocalTooltip (Unicode::narrowToWide (SlotIdManager::getSlotName (SlotId (min_dti->firstSlotIdNumber)).getString ()));
				}
			}
			else
				SetLocalTooltip (Unicode::String ());
		}
		

		//----------------------------------------------------------------------
		//-- we are hovering over the figure, see if there is something underneath the cursor to pick up

		else if (m_allowManipulation)
		{
			const Vector objectStart = m_objectTransform.rotateTranslate_p2l (worldStart);
			const Vector objectEnd   = m_objectTransform.rotateTranslate_p2l (worldEnd);
			
			SkeletalAppearance2 * const skelApp = NON_NULL (dynamic_cast<SkeletalAppearance2 *>(m_renderObject->getAppearance ()));
			
			float              min_intersection  = 10000.0f;
			const Appearance * min_appearance    = 0;
			const Extent *     min_extent        = 0;
			
			//----------------------------------------------------------------------
			
			{
				const int numAttachables = skelApp->getAttachedAppearanceCount ();
				const Skeleton * const skeleton = skelApp->getDisplayLodSkeleton ();
				if (skeleton)
				{
					const Transform * const jointToRootTransformArray = skeleton->getJointToRootTransformArray ();
					
					for (int i = 0; i < numAttachables; ++i)
					{
						const Appearance * const attachedApp = NON_NULL (skelApp->getAttachedAppearance (i));
						const Extent * const extent          = NON_NULL (attachedApp->getExtent ());
						
						const CrcString & transformName = skelApp->getAttachedAppearanceTransformName (i);
						const int transformIndex             = skeleton->getTransformIndex (transformName);
						DEBUG_FATAL (transformIndex < 0, ("out of range\n"));
						const Transform & transform = jointToRootTransformArray [transformIndex];
						
						const Vector attachableObjectStart (transform.rotateTranslate_p2l (objectStart));
						const Vector attachableObjectEnd   (transform.rotateTranslate_p2l (objectEnd));
						
						float t;
						if (extent->intersect (attachableObjectStart, attachableObjectEnd, &t))
						{
							if (t < min_intersection)
							{
								min_intersection        = t;
								min_appearance          = attachedApp;
								min_extent              = extent;
								m_selectedAttachableIndex = i;
								m_selectedAttachableTransform.multiply (m_objectTransform, transform);
							}
						}
					}	
				}
			}
			
			//----------------------------------------------------------------------
			
			{
				const int numWearables = skelApp->getWearableCount ();
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				
				SlottedContainer * const slotted = ContainerInterface::getSlottedContainer (*m_renderObject);
				ClientObject * const hair        = slotted ? safe_cast<ClientObject *>(slotted->getObjectInSlot (*m_hairSlot, tmp).getObject ()): 0;

				for (int i = 0; i < numWearables; ++i)
				{
					SkeletalAppearance2 * const wearableSkelApp = NON_NULL (dynamic_cast<SkeletalAppearance2 *>(skelApp->getWearableAppearance (i)));

					if (wearableSkelApp->getOwner () == hair)
						continue;

					const ContainedByProperty * const containedBy = wearableSkelApp->getOwner ()->getContainedByProperty ();

					if (!containedBy || (containedBy->getContainedBy () != m_renderObject))
						continue;

					const BoxExtent & extent = wearableSkelApp->getExactMeshExtent ();
					
					float t;
					if (extent.intersect (objectStart, objectEnd, &t))
					{
						if (t < min_intersection)
						{
							min_intersection        = t;
							min_appearance          = wearableSkelApp;
							min_extent              = &extent;
							m_selectedWearableIndex = i;
							m_selectedAttachableIndex = -1;
						}
					}
				}			
			}
			
			//----------------------------------------------------------------------
			
			if (min_appearance && min_extent)
			{
				SetMouseCursor (m_equipCursor);
				setMouseDraggable (false);
				m_allowMouseWheelHACK = true;
				SetDragable (true);
				
				const Sphere & sphere = min_extent->getSphere ();
				const BoxExtent * box = dynamic_cast<const BoxExtent *> (min_extent);
				if (box)
				{
					m_selectedBoxExtent.set (box->getMin (), box->getMax (), sphere.getCenter (), sphere.getRadius ());
				}
				else
				{
					const Vector corner (sphere.getRadius () * Vector::xyz111);
					m_selectedBoxExtent.set (sphere.getCenter () - corner, sphere.getCenter () + corner, sphere.getCenter (), sphere.getRadius ());
				}

				SetLocalTooltip (safe_cast<const ClientObject *>(min_appearance->getOwner ())->getLocalizedName ());
			}
			else
			{
				SetMouseCursor (0);
				setMouseDraggable (true);
				SetDragable (false);
			}
		}
	}
	
	CuiWidget3dObjectViewer::Notify (NotifyingObject, ContextObject, NotificationCode);
}

//-----------------------------------------------------------------

bool CuiWidget3dPaperdoll::SetProperty( const UILowerString & Name, const UIString &Value )
{
	return CuiWidget3dObjectViewer::SetProperty (Name, Value);
}

//-----------------------------------------------------------------

bool CuiWidget3dPaperdoll::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	return CuiWidget3dObjectViewer::GetProperty (Name, Value);
}

//-----------------------------------------------------------------

void CuiWidget3dPaperdoll::GetPropertyNames( UIPropertyNameVector & In, bool forCopy) const
{
	CuiWidget3dObjectViewer::GetPropertyNames (In, forCopy);
}

//-----------------------------------------------------------------

bool CuiWidget3dPaperdoll::IsDropOk (const UIWidget & , const UIString & , const UIPoint & )
{
	//- @todo it might be necessary to force an update like Notify usually does
	if (m_dragInProgress && !m_dragTargetInfo->empty ())
	{
		if (m_allowManipulation)
		{
			const DragTargetInfo * dti = 0;
			
			for (DragTargetInfoVector::const_iterator it = m_dragTargetInfo->begin (); it != m_dragTargetInfo->end (); ++it)
			{
				if ((*it).selected)
				{
					dti = &(*it);
					break;
				}
			}
			
			return dti != 0;
		}
		else
			return !m_dragTargetInfo->empty ();
	}

	return false;
}

//-----------------------------------------------------------------

UIWidget * CuiWidget3dPaperdoll::GetCustomDragWidget (const UIPoint & point, UIPoint & )
{	
	if (!m_renderObject)
		return 0;

	TangibleObject * const target = NON_NULL (dynamic_cast<TangibleObject *>(m_renderObject.getPointer ()));
	TangibleObject * const objectToRemove = findSelectedObject ();

	//----------------------------------------------------------------------
	
	if (objectToRemove)
	{
		if (!CuiInventoryManager::unequipItem (*objectToRemove, *target))
			return 0;
				
		CuiWidget3dObjectViewer * const viewer = new CuiWidget3dObjectViewer;
		IGNORE_RETURN (viewer->setObject (objectToRemove, Vector (), Vector ()));
		viewer->setAutoComputeMinimumVectorFromExtent (true);
		viewer->setRotateSpeed (1.0f);
		viewer->setMouseDraggable (false);
		viewer->SetName ("IconViewerTransferWearableFromPlayer");
		viewer->setPaused (false);
		IGNORE_RETURN (viewer->SetProperty (UIWidget::PropertyName::DragType, Unicode::narrowToWide ("GameObject")));
		viewer->SetSize (UISize (64L, 64L));
		viewer->SetLocation (point);
		//				viewer->setZoom (getZoom  ());
		viewer->setObjectYaw (getObjectYaw ());
		
		viewer->Attach (0);

		//-- this is sortof a hack to get the lighting right
		UIBaseObject * const parent = GetParent ();
		if (parent && parent->IsA (TUIWidget))
		{
			UIWidget * const parentWidget = NON_NULL (safe_cast<UIWidget *>(parent));

			if (parentWidget->AddChild (viewer))
			{
				viewer->Link ();
				IGNORE_RETURN (parentWidget->RemoveChild (viewer));
			}
		}
		
		return viewer;
	}
	
	return 0;
	
}

//----------------------------------------------------------------------

TangibleObject * CuiWidget3dPaperdoll::findSelectedObject ()
{
	if (!m_renderObject)
		return 0;

	SkeletalAppearance2 * const skelApp = NON_NULL (dynamic_cast<SkeletalAppearance2 *>(m_renderObject->getAppearance ()));

	if (m_selectedWearableIndex >= 0)
	{
		SkeletalAppearance2 * const wornSkelApp = skelApp->getWearableAppearance (m_selectedWearableIndex);
		return wornSkelApp ? dynamic_cast<TangibleObject *>(wornSkelApp->getOwner ()) : 0;
	}

	if (m_selectedAttachableIndex >= 0)
	{
		Appearance * const attachedApp = const_cast<Appearance *>(skelApp->getAttachedAppearance (m_selectedAttachableIndex));
		return attachedApp ? dynamic_cast<TangibleObject *>(attachedApp->getOwner ()) : 0;
	}

	return 0;
}

//----------------------------------------------------------------------

void CuiWidget3dPaperdoll::setAllowManipulation   (bool b)
{
	m_allowManipulation = b;
}

// ======================================================================
