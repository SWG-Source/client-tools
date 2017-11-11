//======================================================================
//
// SwgCuiGroundRadar_WaypointPane.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiGroundRadar_WaypointPane.h"

#include "UIButton.h"
#include "UICanvas.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIText.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/GameCamera.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiWidgetGroundRadar.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedTerrain/TerrainObject.h"

#include <vector>

//======================================================================


SwgCuiGroundRadar::WaypointPane::WaypointPane (const CuiWidgetGroundRadar & groundRadar, UIImage & arrow, UIImage & arrowWaypoint, UIImage & arrowGroup, UIImage & bracket) :
UIWidget        (),
m_groundRadar   (groundRadar),
m_arrow         (&arrow),
m_arrowWaypoint (&arrowWaypoint),
m_arrowGroup    (&arrowGroup),
m_bracket       (&bracket),
m_infoVector    (new InfoVector)
{
	SetName ("WaypointPane");
	SetAbsorbsInput (false);

	m_arrow->Attach         (0);
	m_arrowWaypoint->Attach (0);
	m_arrowGroup->Attach    (0);
	m_bracket->Attach       (0);
}

//----------------------------------------------------------------------

SwgCuiGroundRadar::WaypointPane::~WaypointPane ()
{
	delete m_infoVector;
	m_infoVector = 0;

	m_arrow->Detach         (0);
	m_arrowWaypoint->Detach (0);
	m_arrowGroup->Detach    (0);
	m_bracket->Detach       (0);

	m_arrow         = 0;
	m_arrowWaypoint = 0;
	m_arrowGroup    = 0;
	m_bracket       = 0;
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::WaypointPane::Render (UICanvas & canvas) const
{	
	if (!Game::getScene ())
		return;

	canvas.SetOpacity (1.0f);

	for (InfoVector::const_iterator it = m_infoVector->begin (); it != m_infoVector->end (); ++it)
	{
		const Info & info = *it;
		canvas.PushState ();

		UIImage * const arrow = info.isWaypoint ? m_arrowWaypoint : (info.isBracket ? m_bracket : (info.isGroup ? m_arrowGroup : m_arrow));
		NOT_NULL (arrow);

		UIPoint CenteringTranslation = (GetSize() / 2L);		

		canvas.SetColor (info.color);
		canvas.ModifyColor (arrow->GetBackgroundTint ());

		canvas.Translate   (CenteringTranslation);
		canvas.Rotate      (info.angle );
		canvas.Translate   (-CenteringTranslation);

		canvas.Translate   (CenteringTranslation.x - 16, 0);

		const UIPoint & arrowCenteringTranslation = arrow->GetSize();
		canvas.Translate   (arrowCenteringTranslation);
		canvas.Rotate      (arrow->GetRotation());
		canvas.Translate   (-arrowCenteringTranslation);

		arrow->Render      (canvas);

		canvas.PopState    ();
	}
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::WaypointPane::clearWaypoints ()
{
	m_infoVector->clear ();
}

//----------------------------------------------------------------------

void SwgCuiGroundRadar::WaypointPane::addWaypoint (const Info & info)
{
	m_infoVector->push_back (info);
}

//----------------------------------------------------------------------
