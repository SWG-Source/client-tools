//======================================================================
//
// SwgCuiMfdStatusBar.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiMfdStatusBar.h"

#include "StringId.h"
#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIRectangleStyle.h"
#include "UIText.h"
#include "UIWidgetRectangleStyles.h"
#include "UnicodeUtils.h"
#include "clientGame/ConfigClientGame.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"

//@todo: this include path is invalid
#include "swgSharedUtility/Attributes.h"
#include "swgSharedUtility/Attributes.def"

//======================================================================

const UILowerString SwgCuiMfdStatusBar::Properties::Orientation = UILowerString ("Orientation");
const UILowerString SwgCuiMfdStatusBar::Properties::RechargeUpdateMultiplier = UILowerString ("RechargeUpdateMultiplier");
const UILowerString SwgCuiMfdStatusBar::Properties::CurrentUpdateMultiplier = UILowerString ("CurrentUpdateMultiplier");

//----------------------------------------------------------------------

SwgCuiMfdStatusBar::SwgCuiMfdStatusBar (UIPage & page, bool useVerboseTooltip) :
CuiMediator              ("SwgCuiMfdStatusBar", page),
UIEventCallback           (),
m_valueText              (0),
m_rechargePage            (0),
m_currentPage            (0),
m_currentMaxPage         (0),
m_currentMax             (0),
m_normalMax              (0),
m_orientation            (O_horizontal),
m_currentMargin          (),
m_currentMaxMargin       (),
m_lastSize               (),
m_useVerboseTooltip       (useVerboseTooltip),
m_attributeIndex          (0),
m_currentTickPage         (0),
m_currentDesired          (0),
m_currentInterpolated     (0.0f),
m_currentUpdateMultiplier (2.0f),
m_rechargeDesired         (0),
m_rechargeInterpolated    (0.0f),
m_rechargeUpdateMultiplier(3.0f)
{
	getCodeDataObject (TUIText, m_valueText,      "ValueText", true);
	getCodeDataObject (TUIPage, m_currentPage,    "juice");
	getCodeDataObject (TUIPage, m_rechargePage ,    "moves");
	getCodeDataObject (TUIPage, m_currentMaxPage, "cap");

	m_currentPage->SetMinimumSize( UISize( 0, 0 ) );

	const UIData * const codeData = getCodeData ();
	if (codeData)
	{
		codeData->GetPropertyPoint (UILowerString ("currentMargin"),      m_currentMargin);
		codeData->GetPropertyPoint (UILowerString ("currentMaxMargin"), m_currentMaxMargin);
	}

	std::string orientationStr;
	if (getPage ().GetPropertyNarrow (Properties::Orientation, orientationStr))
	{
		if (!_stricmp (orientationStr.c_str (), "vertical"))
			m_orientation = O_vertical;
		else if (!_stricmp (orientationStr.c_str (), "horizontal"))
			m_orientation = O_horizontal;
		else
		{
			WARNING (true, ("Invalid orientation property for SwgCuiMfdStatusBar: '%s'", orientationStr.c_str ()));
		}
	}

	//-- currentTick is optional.
	getCodeDataObject (TUIPage, m_currentTickPage, "currentTick", true);

	// Get the update multiplier values.
	if (getPage().HasProperty(Properties::CurrentUpdateMultiplier)) 
		{
		getPage().GetPropertyFloat(Properties::CurrentUpdateMultiplier, m_currentUpdateMultiplier);
	}

	if (getPage().HasProperty(Properties::RechargeUpdateMultiplier)) 
	{
		getPage().GetPropertyFloat(Properties::RechargeUpdateMultiplier, m_rechargeUpdateMultiplier);
	}
}

//----------------------------------------------------------------------

SwgCuiMfdStatusBar::~SwgCuiMfdStatusBar ()
{	
	deactivate ();
}

//-----------------------------------------------------------------

void SwgCuiMfdStatusBar::performActivate()
{
	m_lastSize = UISize::zero;
}

//-----------------------------------------------------------------

void SwgCuiMfdStatusBar::performDeactivate()
{

}

//----------------------------------------------------------------------

void SwgCuiMfdStatusBar::updateBar(int scaleMax, int normalMax, int currentMax, int current, bool showRecharge, int recharge, float deltaTimeSecs)
{
	int hamBarType = ConfigClientGame::getHamBarType();

	const UISize & curSize = getPage ().GetSize ();
	m_lastSize = curSize;

	static const UIRect zeroRect;

	//-----------------------------------------------------------------
	//-- update the text display

	if (m_valueText && current != m_currentDesired)
	{
		char buf [32];
		snprintf (buf, sizeof (buf), "%d", current);
		m_valueText->SetLocalText (Unicode::narrowToWide (buf));
	}

	m_currentDesired    = current;
	m_currentMax = currentMax;
	m_normalMax  = normalMax;
	m_rechargeDesired   = recharge;

	// Interpolate the current value.
	float const deltaTimeSecsClampedCurrent = clamp(0.0f, deltaTimeSecs * m_currentUpdateMultiplier, 1.0f);	
	m_currentInterpolated = (deltaTimeSecsClampedCurrent != deltaTimeSecs) ? current : linearInterpolate(m_currentInterpolated, static_cast<float>(current), deltaTimeSecsClampedCurrent);
	int const currentInterpolated = static_cast<int>(m_currentInterpolated);

	float const deltaTimeSecsClampedRecharge = clamp(0.0f, deltaTimeSecs * m_rechargeUpdateMultiplier, 1.0f);	
	m_rechargeInterpolated = (deltaTimeSecsClampedRecharge != deltaTimeSecs) ? recharge : linearInterpolate(m_rechargeInterpolated, static_cast<float>(recharge), deltaTimeSecsClampedRecharge);
	int const rechargeInterpolated = static_cast<int>(m_rechargeInterpolated);

	//----------------------------------------------------------------------
	//-- Black Bar

	if (m_currentMaxPage)
	{
		UIPage * const currentMaxParent = (m_currentMaxPage->GetParent ()->IsA (TUIPage)) ? safe_cast<UIPage *>(m_currentMaxPage->GetParent ()) : NULL;

		if (currentMaxParent)
		{
			const long front = m_currentMaxMargin.x;
			const long back  = m_currentMaxMargin.y;

			const long minLength                    = 0;
			const long currentMaxParentLength       = getOrientedLength (*currentMaxParent);

			if (normalMax > 0)
			{
				const long currentMaxParentLengthUsable = ((currentMaxParentLength - minLength) - front) - back;

				const long loc = (currentMaxParentLengthUsable * currentMax / normalMax) + front;
				setOrientedLocation (*m_currentMaxPage, currentMaxParentLength, loc);
				setOrientedLength   (*m_currentMaxPage, currentMaxParentLength - loc);
			}
			else
			{
				setOrientedLength   (*m_currentMaxPage, minLength);
				setOrientedLocation (*m_currentMaxPage, currentMaxParentLength, currentMaxParentLength - minLength);
			}
		}


		//-----------------------------------------------------------------
		
		if (m_currentTickPage)
		{
			UIPage * const currentTickParent = dynamic_cast<UIPage * const>(m_currentTickPage->GetParent ());
			if (currentTickParent)
			{
				long const currentTickParentLength = getOrientedLength (*currentTickParent);
				if (scaleMax > 0)
				{
					setOrientedLocation (*m_currentTickPage, currentTickParentLength, (currentTickParentLength * current / scaleMax) - (getOrientedLength(*m_currentTickPage)/2) );
				}
				else
				{
					setOrientedLocation (*m_currentTickPage, currentTickParentLength, 0L);
				}
			}
		}
	}

	
	UIPage * const currentParent = (m_currentPage->GetParent ()->IsA (TUIPage)) ? safe_cast<UIPage *>(m_currentPage->GetParent ()) : NULL;
	
	if (currentParent)
	{
		const long front = m_currentMargin.x;
		const long back  = m_currentMargin.y;
	
		const long currentParentLength       = getOrientedLength   (*currentParent);
		const long currentMaxLocation        = m_currentMaxPage ? getOrientedLocation (*m_currentMaxPage, currentParentLength) : currentParentLength;
		const long currentParentLengthUsable = (currentMaxLocation - front) - back;
		const long currentOrientedLength     = currentMax > 0 ? (currentParentLengthUsable * currentInterpolated / currentMax) : 0;
		
		//----------------------------------------------------------------------
		//-- Colored bar

		if (currentMax > 0)
		{
			setOrientedLength   (*m_currentPage, currentOrientedLength);
			setOrientedLocation (*m_currentPage, currentParentLength, front);
		}
		else
		{
			setOrientedLength     (*m_currentPage, 0);
			setOrientedLocation   (*m_currentPage, currentParentLength, front);
		}
				
		//----------------------------------------------------------------------
		//-- Recharge bar

		long const rechargeOrientedLength = currentMax > 0 ? (currentParentLengthUsable * rechargeInterpolated / currentMax) : 0;
		long const diffCurrentRecharge = std::min(currentOrientedLength - rechargeOrientedLength, static_cast<long>(currentParentLengthUsable));
		
		if (showRecharge && ((hamBarType == 1) || (hamBarType == 2)) )
		{
			long const rechargeLength = currentMax > 0 ? (m_rechargeDesired * currentParentLengthUsable / currentMax) : 0;
			setOrientedLength(*m_rechargePage, rechargeLength);
			setOrientedLocation(*m_rechargePage, currentParentLength, 0);
		}
		else if (showRecharge && diffCurrentRecharge > std::max(front, back))
			{
			setOrientedLength(*m_rechargePage, diffCurrentRecharge);
			setOrientedLocation(*m_rechargePage, currentParentLength, rechargeOrientedLength);
			m_rechargePage->SetVisible(true);
			}
			else
			{
			m_rechargePage->SetVisible(false);
		}
	}
}
	
//----------------------------------------------------------------------

long SwgCuiMfdStatusBar::getOrientedScalar   (const UIPoint & pt) const
{
	if (m_orientation == O_vertical)
		return pt.y;
	else
		return pt.x;
}

//----------------------------------------------------------------------

long  SwgCuiMfdStatusBar::getOrientedLength   (const UIWidget & widget) const
{
	return getOrientedScalar (widget.GetSize ());
}

//----------------------------------------------------------------------

long  SwgCuiMfdStatusBar::getOrientedLocation (const UIWidget & widget, long const parentLength) const
{
	if (m_orientation == O_vertical)
		return parentLength - getOrientedScalar (widget.GetLocation ());

	else
	return getOrientedScalar (widget.GetLocation ());
}

//----------------------------------------------------------------------

void  SwgCuiMfdStatusBar::setOrientedLength   (UIWidget & widget, long length) const
{
	if (m_orientation == O_vertical)
		widget.SetHeight (length);
	else
		widget.SetWidth (length);
}

//----------------------------------------------------------------------

void  SwgCuiMfdStatusBar::setOrientedLocation (UIWidget & widget, long parentLength, long length) const
{
	if (m_orientation == O_vertical)
	{
		widget.SetLocation (widget.GetLocation ().x, parentLength - ((getOrientedLength (widget) + length)));
	}
	else
	{
		widget.SetLocation (length, widget.GetLocation ().y);
	}
}

//----------------------------------------------------------------------

void SwgCuiMfdStatusBar::setAttributeIndex (int attributeIndex)
{
	m_attributeIndex = attributeIndex;
}

//----------------------------------------------------------------------

void SwgCuiMfdStatusBar::setUseVerboseTooltip (bool useVerboseTooltip)
{
	m_useVerboseTooltip = useVerboseTooltip;
}

//======================================================================
