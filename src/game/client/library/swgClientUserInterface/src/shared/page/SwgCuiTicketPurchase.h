//===================================================================
//
// SwgCuiTicketPurchase.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiTicketPurchase_H
#define INCLUDED_SwgCuiTicketPurchase_H

//===================================================================

class PlanetTravelPointListResponse;
class UIButton;
class UICheckbox;
class UIComboBox;
class UIText;

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"

//===================================================================

namespace MessageDispatch
{
	class Callback;
}


class SwgCuiTicketPurchase : 
public UIEventCallback, 
public CuiMediator, 
public MessageDispatch::Receiver
{
public:

	enum TravelType
	{
		TT_buyTicket,
		TT_personalShip,
		TT_instantTravel
	};

	explicit     SwgCuiTicketPurchase (UIPage& page);

	virtual void OnButtonPressed           (UIWidget* context);
	virtual void OnCheckboxSet             (UIWidget* context);
	virtual void OnCheckboxUnset           (UIWidget* context);
	virtual void OnGenericSelectionChanged (UIWidget* context);
	virtual bool OnMessage                 (UIWidget* context, const UIMessage& msg);

	void         processMessage            (const PlanetTravelPointListResponse& message);

	virtual void receiveMessage            (const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

	bool         setData                   (const std::string& planetName, const std::string& travelPointName);
	void         update                    (float deltaTimeSecs);
	void         setTravelType             (TravelType type);
	void         setTerminalId             (NetworkId const & terminalId);
	void         setShipControlDeviceId    (NetworkId const & shipControlDeviceId);

	void         onGCWValuesUpdated        (bool);

protected:
	virtual void performActivate           ();
	virtual void performDeactivate         ();

private:

	struct TravelPoint;
	typedef stdvector<TravelPoint>::fwd TravelPointList;
	typedef std::pair<UIButton*, TravelPointList> ButtonPointPair;
	typedef stdmap<std::string, ButtonPointPair>::fwd PlanetTravelPointListMap;
	typedef stdvector<UIButton *>::fwd ButtonVector;

private:

	virtual ~SwgCuiTicketPurchase ();
	
	void               update                 ();
	void               updateArriveLocationNames ();
	void               updateArriveLocationNames (bool skipTravelPoint, const std::string& skipTravelPointName);
	void               addArrivePlanetUiText  ();
	void               addArriveLocationUiText(const std::string& planetName, bool skipTravelPoint=false, const std::string* skipTravelPointName=0);
	void               selectArrivePlanet     (const std::string& selectionName);
	void               selectArriveLocation   (const std::string& selectionName);
	int                getTravelPointCost     (const std::string& planetName, const std::string& travelPointName) const;
	const std::string& getDepartPlanet        () const;
	const std::string& getDepartLocation      () const;
	std::string        getSelectedArrivePlanet() const;
	std::string        getSelectedArriveLocation() const;
	void               showGalacticMap        ();
	void               showPlanet             (const std::string& planetName);
	void               removeButtons          (UIPage* page) const;
	void               clearPointButtons      ();
	void               updateTravelType       ();
	bool               isInterplanetary       () const;
	void			   updateGCWIcons         ();

private:

	SwgCuiTicketPurchase ();
	SwgCuiTicketPurchase (const SwgCuiTicketPurchase&);
	SwgCuiTicketPurchase& operator= (const SwgCuiTicketPurchase&);

private:

	UIComboBox* m_arrivePlanetComboBox;
	UIComboBox* m_arriveLocationComboBox;
	UICheckbox* m_roundTripCheckBox;
	UICheckbox* m_nameToggleCheckBox;
	UIText*     m_costText;
	UIButton*   m_purchaseButton;
	UIButton*   m_cancelButton;
	UIButton*   m_sampleButton;
	UIPage*     m_galacticMapPage;
	UIPage*     m_planetsPage;
	UIPage*     m_planetNamesPage;
	UIText *    m_textSample;
	UIButton *  m_buttonShowGalaxy;
	UIButton *  m_buttonShowPlanet;
	UIText *    m_textLoadingStatus;
	UIPage *    m_pageCost;
	UICheckbox * m_buttonRoundTrip;
	UIButton *  m_buttonTravel;

	PlanetTravelPointListMap* m_planetTravelPointListMap;
	ButtonVector *            m_pointButtons;

	std::string m_startingPlanetName;
	std::string m_startingTravelPointName;
	int         m_requestsOutstanding;
	int         m_currentSequenceId;
	float       m_timer;
	TravelType  m_travelType;
	NetworkId   m_terminalId;
	NetworkId   m_shipControlDeviceId;

	MessageDispatch::Callback *  m_callback;
};

//===================================================================

#endif
