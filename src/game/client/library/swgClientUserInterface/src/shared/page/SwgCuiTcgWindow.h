// ======================================================================
//
// SwgCuiTcgWindow.h
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#ifndef INCLUDED_SwgCuiTcgWindow_H
#define INCLUDED_SwgCuiTcgWindow_H

#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMessageBox.h"

#include "libEverQuestTCG/libEverQuestTCG.h"

#include <vector>

class UIPage;
class SwgCuiTcgControl;

namespace MessageDispatch {	class Callback; };
namespace libEverQuestTCG { class Window; }; 
// ======================================================================

class SwgCuiTcgWindow : public CuiMediator
{
public:
	SwgCuiTcgWindow(UIPage & page);
	virtual ~SwgCuiTcgWindow();

protected:
	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);
	virtual bool close();

	void createTcgControl();

private: // disabled

	struct Window
	{
		Window() : pPage( 0 ), pTCGControl( 0 ){}

		UIPage           *pPage;
		SwgCuiTcgControl *pTCGControl;
	};

	SwgCuiTcgWindow();
	SwgCuiTcgWindow(SwgCuiTcgWindow const & rhs);
	SwgCuiTcgWindow& operator= (SwgCuiTcgWindow const & rhs);

	void        onMessageBoxClose(const CuiMessageBox& box);

private:
	UIPage*            m_tcgPage;
	UIBaseObject *     m_tcgParent;
	SwgCuiTcgControl * m_tcgControl;

	std::vector<Window> m_windows;

	MessageDispatch::Callback* m_callbacks;
};

// ======================================================================

#endif
