// ======================================================================
//
// SwgCuiTcgWindow.cpp
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTcgWindow.h"

#include "clientAudio/Audio.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedFoundation/Os.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgClientUserInterface/SwgCuiTcgControl.h"

#include "UIImage.h"

// ======================================================================

SwgCuiTcgWindow::SwgCuiTcgWindow(UIPage & page)
: CuiMediator("SwgCuiTcgWindow", page)
, m_tcgControl(0)
, m_tcgPage(0)
, m_tcgParent(0)
, m_callbacks ( new MessageDispatch::Callback )
{
	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));
}

// ----------------------------------------------------------------------

SwgCuiTcgWindow::~SwgCuiTcgWindow()
{
	m_tcgControl = 0;
	m_tcgPage = 0;
	m_tcgParent = 0;

	delete m_callbacks;
	m_callbacks = 0;
}

// ----------------------------------------------------------------------

void SwgCuiTcgWindow::performActivate()
{
	CuiManager::requestPointer(true);
	CuiManager::requestKeyboard(true);

	if(!m_tcgPage || !m_tcgControl)
		createTcgControl();

	if (m_tcgControl)
	{
		m_tcgControl->initializeEqTcgWindow();

		if (m_windows.empty())
		{
			libEverQuestTCG::Window* mainWindow = m_tcgControl->getEqTcgWindow();

			if (mainWindow)
			{
				Window newWindow;
				newWindow.pPage = NULL;
				newWindow.pTCGControl = m_tcgControl;

				mainWindow->setUserData(m_tcgControl);

				m_windows.push_back(newWindow);
			}

		}
	}

	setIsUpdating(true);

	Audio::silenceNonBufferedMusic(true);
}

// ----------------------------------------------------------------------

void SwgCuiTcgWindow::performDeactivate()
{
	CuiManager::requestPointer(false);
	CuiManager::requestKeyboard(false);

	setIsUpdating(false);
}

// ----------------------------------------------------------------------

bool SwgCuiTcgWindow::close()
{

	if (m_tcgControl && !m_tcgControl->getEqTcgWindow()) // User clicked EXIT via TCG game. They've already gotten a "are you sure?" message.
	{
		// Window is already closed, no need to do anything.
	}
	else
	{
		CuiMessageBox * const box = CuiMessageBox::createYesNoBox (CuiStringIds::tcg_exit_confirmation.localize());
		m_callbacks->connect (box->getTransceiverClosed (), *this, &SwgCuiTcgWindow::onMessageBoxClose);
		return true; // This will prevent the Workspace from closing us. We'll handle it later.
	}

	Audio::silenceNonBufferedMusic(false);

	return true;
}

// ----------------------------------------------------------------------

void SwgCuiTcgWindow::createTcgControl()
{
	UIPage * const tcgControlParent = NON_NULL(safe_cast<UIPage *>(getPage().GetChild("tcgControlParent")));

	if (tcgControlParent)
	{
		m_tcgPage = tcgControlParent;
		UIBaseObject * const tcgPageParent = m_tcgPage->GetParent();
		m_tcgParent = tcgPageParent ? tcgPageParent->GetParent() : 0;

		FATAL(!m_tcgParent, ("m_tcgParent is null!"));

		if(m_tcgControl) // Our TCG Control was already created.
			return;

		m_tcgControl = new SwgCuiTcgControl;

		NOT_NULL(m_tcgControl);

		if (m_tcgControl)
		{
			m_tcgControl->SetGetsInput(true);
			m_tcgControl->SetSelectable(true);
			m_tcgControl->SetVisible(true);

			m_tcgControl->SetSize(tcgControlParent->GetSize());

#ifdef _DEBUG
			char buffer[256];
			memset(buffer, 0, 256);
			sprintf(buffer, "TCG Main Control");
			m_tcgControl->SetName(buffer);
#endif

			tcgControlParent->AddChild(m_tcgControl);
			m_tcgControl->SetProperty(UIWidget::PropertyName::PackSize, Unicode::narrowToWide("a,a"));

			UIImage * tcgImage = 0;
			getCodeDataObject(TUIImage, tcgImage, "tcgImage");

			m_tcgControl->setImage(tcgImage);

			m_tcgControl->initializeEqTcgWindow();

			libEverQuestTCG::Window* mainWindow = m_tcgControl->getEqTcgWindow();

			if (mainWindow)
			{
				Window newWindow;
				newWindow.pPage = NULL;
				newWindow.pTCGControl = m_tcgControl;

				mainWindow->setUserData(m_tcgControl);

				m_windows.push_back(newWindow);
			}
		}
	}
}

// ----------------------------------------------------------------------

void SwgCuiTcgWindow::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	if (!m_tcgControl)
		return;

	if (!m_tcgControl->getEqTcgWindow())
		closeNextFrame();
	else
		m_tcgControl->alter(deltaTimeSecs);

	if (!m_tcgPage)
		return;

	unsigned uWindows = libEverQuestTCG::getWindows(0,0);
	unsigned uWindowsRemoved = 0;

	libEverQuestTCG::Window **ppWindows = 0;

	if( uWindows > 0 )
	{
		ppWindows = (libEverQuestTCG::Window **)alloca( uWindows * sizeof( libEverQuestTCG::Window * ) );
		libEverQuestTCG::getWindows( ppWindows, uWindows );
	}

	libEverQuestTCG::Window **ppWindowsEnd = ppWindows + uWindows;

	libEverQuestTCG::Window * m_pMainTCGWindow = m_tcgControl->getEqTcgWindow();

	if( m_pMainTCGWindow ) // Still need to go in here if 0 == uWindows so m_pMainTCGWindow gets cleared
	{
		// Verify main window still exists
		if( ppWindowsEnd == std::find( ppWindows, ppWindowsEnd, m_pMainTCGWindow ) )
		{
			while( ppWindowsEnd != ppWindows )
			{
				--ppWindowsEnd;
				(*ppWindowsEnd)->close();
			}

			uWindows = 0;
			m_pMainTCGWindow = 0;
		}
	}

	bool forceFocus = false;

	// Look for deleted windows
	for( unsigned i = 0; i != m_windows.size(); /* increment in body */ )
	{
		Window &rWindow = m_windows[i];
		libEverQuestTCG::Window *pWindow = rWindow.pTCGControl->getEqTcgWindow();
		libEverQuestTCG::Window **ppWindow = std::find( ppWindows, ppWindowsEnd, pWindow );

		// Get the window page
		if( ppWindow == ppWindowsEnd )
		{
			// @TODO: The sliding menu pop-up windows can get into a state where they need to be closed by a left mouse button up message
			if (pWindow && pWindow != m_pMainTCGWindow && pWindow->canGetFocus())
				pWindow->onLeftMouseUp(-1, -1, -1, -1, 0);

			rWindow.pTCGControl->setEqTcgWindow( 0 );

			if(rWindow.pPage)
				m_tcgParent->RemoveChild( rWindow.pPage );

			m_windows.erase( m_windows.begin() + i );
			++uWindowsRemoved;

			forceFocus = true;
		}
		else
		{
			if( m_pMainTCGWindow != pWindow )
			{
				// Sync up the rects with anything that might have happened
				int x, y;
				unsigned w, h;
				pWindow->getRect( x, y, w, h );

				UISize deltaSize( UISize( w, h ) - rWindow.pPage->GetSize() );
				UIPoint deltaLoc( UIPoint( x, y ) - rWindow.pPage->GetLocation() );

#ifdef _DEBUG
				UISize pageSize = rWindow.pPage->GetSize();
				UIPoint pageLoc = rWindow.pPage->GetLocation();

				DEBUG_REPORT_PRINT(true, ("Wx = %d, Wy = %d, Ww = %d, Wh = %d\n", x, y, w, h));
				DEBUG_REPORT_PRINT(true, ("Px = %d, Py = %d, Pw = %d, Ph = %d\n", pageLoc.x, pageLoc.y, pageSize.x, pageSize.y));
				DEBUG_REPORT_PRINT(true, ("Dx = %d, Dy = %d\n", deltaLoc.x, deltaLoc.y));
#endif
				if( deltaSize.x | deltaSize.y )
				{
					UIPoint newSize( rWindow.pPage->GetSize() + deltaSize );
					DEBUG_REPORT_PRINT(true, ("Sizing page 0x%08X to %d, %d", rWindow.pPage, newSize.x, newSize.y));
					rWindow.pPage->SetSize( newSize );
					rWindow.pTCGControl->SetSize( newSize );
				}

				if( deltaLoc.x | deltaLoc.y )
				{
					UIPoint newLoc( rWindow.pPage->GetLocation() + deltaLoc );
					DEBUG_REPORT_PRINT(true, ("Moving page 0x%08X to %d, %d", rWindow.pPage, newLoc.x, newLoc.y));
					rWindow.pPage->SetLocation( newLoc );
				}
			}

			++i;
		}
	}

	// Look for new windows
	for( unsigned i = 0; i != uWindows; ++i )
	{
		libEverQuestTCG::Window *pWindow = ppWindows[ i ];

		if( SwgCuiTcgControl *pTCGControl = reinterpret_cast< SwgCuiTcgControl * >( pWindow->getUserData() ) )
		{
			// Existing window, push location into lib
			UIPoint pt( pTCGControl->GetWorldLocation() );
			pWindow->setLocation( pt.x, pt.y );
		}
		else
		{
			// don't let QT get mouse capture
			if (::GetCapture() != Os::getWindow())
				ReleaseCapture();

			// This is a new window

			int iX, iY;
			unsigned uWidth, uHeight;
			pWindow->getRect( iX, iY, uWidth, uHeight );

			Window newWindow;
			newWindow.pTCGControl = new SwgCuiTcgControl;

			DEBUG_REPORT_LOG(false, ("Create - CanGetFocus = %s\n", pWindow->canGetFocus() ? "true" : "false"));

			if (pWindow->canGetFocus())
			{
				newWindow.pTCGControl->SetGetsInput(true);
				newWindow.pTCGControl->SetSelectable(true);

				newWindow.pTCGControl->SetSelected(true);
				newWindow.pTCGControl->SetFocus();

				static_cast<UIPage *>(m_tcgParent)->GiveWidgetMouseLock(newWindow.pTCGControl);
			}
			else
			{
				newWindow.pTCGControl->SetGetsInput(false);
				newWindow.pTCGControl->SetSelectable(false);
			}

			newWindow.pTCGControl->setEqTcgWindow( pWindow );
			newWindow.pTCGControl->SetSize( UISize( uWidth, uHeight ) );
			newWindow.pTCGControl->SetVisible( true );

#ifdef _DEBUG
			char buffer[256];
			memset(buffer, 0, 256);
			sprintf(buffer, "TCG Child Control #%d", i);
			newWindow.pTCGControl->SetName(buffer);
#endif
			
			UIImage * tcgImage = 0;
			getCodeDataObject(TUIImage, tcgImage, "tcgImageContext");

			newWindow.pTCGControl->setImage(tcgImage);
			
			pWindow->setUserData( newWindow.pTCGControl );

			bool bNaked = true;

			if( bNaked )
			{
				newWindow.pPage = new UIPage;
				newWindow.pPage->SetVisible( true );
				newWindow.pPage->SetSize( UISize( uWidth, uHeight ) );
				
				newWindow.pPage->SetLocation( iX, iY );

				newWindow.pPage->AddChild( newWindow.pTCGControl );
			}

			m_tcgParent->AddChild(newWindow.pPage);
			m_tcgParent->MoveChild(newWindow.pPage, UIBaseObject::Top);

			/*
			static bool dofocus = false;
			if(dofocus)
			if( m_tcgPage->IsSelected() && pWindow->canGetFocus() )
				newWindow.pTCGControl->SetFocus();

			m_tcgControl->SetFocus();
			*/

			m_windows.push_back( newWindow );

			//if( pWindow == m_pMainTCGWindow )
			//{
			//	UISize rootSize = static_cast< UIWidget * >( m_tcgPage->GetRoot() )->GetSize();
			//	UISize controlSize = newWindow.pTCGControl->GetSize();
			//	if( controlSize.x >= rootSize.x && controlSize.y >= rootSize.y )
			//		m_maximize(true);

			//	// Main window always gets focus
			//	newWindow.pTCGControl->SetFocus();
			//}
		}
	}

	// Always give the first child window mouse lock when there are any child windows open as long as its not a tooltip
	if (uWindows > 1 && ppWindows)
	{
		libEverQuestTCG::Window * pWindow = ppWindows[1];

		if (pWindow && pWindow->canGetFocus() && pWindow->getUserData())
		{
			UIWidget * const widget = static_cast<UIWidget *>(pWindow->getUserData());

			if (widget)
			{
				widget->SetSelected(true);
				widget->SetFocus();
				static_cast<UIPage *>(m_tcgParent)->GiveWidgetMouseLock(static_cast<UIWidget *>(widget));
			}
		}
	}

	if (forceFocus && m_tcgControl && m_tcgControl->getEqTcgWindow())
	{
		static_cast<UIPage *>(m_tcgParent)->ReleaseMouseLock(UIPoint(0,0));

		m_tcgControl->SetSelected(true);
		m_tcgControl->SetFocus();
		m_tcgControl->getEqTcgWindow()->setFocus(true);
	}
}

void SwgCuiTcgWindow::onMessageBoxClose(const CuiMessageBox& box)
{
	if(box.completedAffirmative())
	{
		if(m_tcgControl)
		    m_tcgControl->getEqTcgWindow()->close();

	   //m_tcgPage = 0;

	   //Audio::silenceNonBufferedMusic(false);

	   //CuiMediator::close();

	   //activate();
	}
}

// ======================================================================
