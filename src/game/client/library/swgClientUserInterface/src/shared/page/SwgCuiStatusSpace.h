//======================================================================
//
// SwgCuiStatusSpace.h
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiStatusSpace_H
#define INCLUDED_SwgCuiStatusSpace_H

//======================================================================

#include "swgClientUserInterface/SwgCuiMfdStatus.h"
#include "sharedObject/CachedNetworkId.h"

//----------------------------------------------------------------------

class CachedNetworkId;
class CuiWidget3dObjectListViewer;
class ShipObject;
class UIComposite;
class UIImage;
class UIPage;
class UIText;
class UIVolumePage;

//----------------------------------------------------------------------

class SwgCuiStatusSpace :
public SwgCuiMfdStatus
{
public:
	explicit SwgCuiStatusSpace(UIPage & page);

	void setTarget(TangibleObject * tangible);
	void setTarget(const NetworkId & id);

	void setTargetNamePrefix(const Unicode::String & prefix);
	void setTargetName(const Unicode::String & name);
	void setShowRange(bool b);
	void update(float deltaTimeSecs);
	
	void setDisplayStates(bool stats, bool states, bool posture);

protected:

	void performActivate();
	void performDeactivate();

private:
	virtual ~SwgCuiStatusSpace();
	SwgCuiStatusSpace();
	SwgCuiStatusSpace(const SwgCuiStatusSpace &);
	SwgCuiStatusSpace & operator=(const SwgCuiStatusSpace &);

private:
	void updateTargetName(const ClientObject & obj);
	void setShipTarget(ShipObject const * ship);

	void updateTangible(TangibleObject const & tangible, float deltaTimeSecs);
	void updateTangible(TangibleObject const & tangible, int current, int maxHit, float deltaTimeSecs);
	void updateColors();
	bool updateIconChild(const std::string & statename, bool active, UIBaseObject *& lastChild);
	void updateShip(ShipObject const & ship);
	void updateTargetDamage(ShipObject const & ship);
	void set3DObject(Object * object);
	void updateButtonBar(TangibleObject const * const tangible);

private:
	bool m_displayStats;

	Unicode::String m_targetNamePrefix;
	float m_lastNameUpdateSecs;

	bool m_showRange;

	UIImage * m_iconDifficulty;
	UIText * m_textName;
	UIText * m_textType;
	UIText * m_textDifficulty;
	UIText * m_shieldsPercent;
	UIText * m_armorPercent;
	UIText * m_systemPercent;
	UIText * m_chassisPercent;
	UIText * m_textComponent;
	UIPage * m_imperialFaction;
	UIPage * m_rebelFaction;

	CuiWidget3dObjectListViewer * m_objectViewer;
	CachedNetworkId m_lastRenderObject;

	UIWidget * m_canFollow;
	UIWidget * m_canMatchSpeed;
	UIComposite * m_buttonBar;

	// Keep track of target condition widgets.
	typedef stdmap<int /*TangibleObject::Conditions*/, UIWidget *>::fwd ConditionMap;
	ConditionMap * m_targetConditionWidgets;
};

//======================================================================

#endif
