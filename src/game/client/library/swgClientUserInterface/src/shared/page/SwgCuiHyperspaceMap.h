//===================================================================
//
// SwgCuiHyperspaceMap.h
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiHyperspaceMap_H
#define INCLUDED_SwgCuiHyperspaceMap_H

//===================================================================

class UIButton;
class UIText;

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

//===================================================================

class SwgCuiHyperspaceMap : public UIEventCallback, public CuiMediator
{
public:
	explicit SwgCuiHyperspaceMap(UIPage& page);
	virtual void OnButtonPressed(UIWidget* context);
	virtual void OnPopupMenuSelection(UIWidget * context);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	virtual ~SwgCuiHyperspaceMap();
	void createContextMenu(std::string const & system);

private:

	SwgCuiHyperspaceMap();
	SwgCuiHyperspaceMap(const SwgCuiHyperspaceMap&);
	SwgCuiHyperspaceMap& operator=(const SwgCuiHyperspaceMap&);

private:
	UIButton * m_hyperspaceButton;
	UIButton * m_cancelButton;
	UIButton * m_buttonCorellia;
	UIButton * m_buttonDantooine;
	UIButton * m_buttonDathomir;
	UIButton * m_buttonEndor;
	UIButton * m_buttonLok;
	UIButton * m_buttonNaboo;
	UIButton * m_buttonRori;
	UIButton * m_buttonTalus;
	UIButton * m_buttonTatooine;
	UIButton * m_buttonYavin4;
	UIButton * m_buttonKashyyyk;
	UIButton * m_buttonOrdMantell;
	UIText * m_hyperspacePointName;
	UIText * m_hyperspacePointDescription;
	UIText * m_hyperspacePointSystemLocation;
	std::string m_selectedHyperspacePoint;
};

//===================================================================

#endif
