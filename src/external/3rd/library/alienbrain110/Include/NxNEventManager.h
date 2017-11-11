// \addtodoc

#ifndef INC_NXN_EVENTMANAGER_H
#define INC_NXN_EVENTMANAGER_H

/*	\class		CNxNEventManager NxNEventManager.h
 *
 *  \brief		This class provides access to object specific menus (context menus).
 *				Please keep in mind, that this class is only meant for data retrieval
 *				and not for modification things. Thus you can not modify the menu structure.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-07-17-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNEventManagerData*	tNxNEventManagerData;

class NXNINTEGRATORSDK_API CNxNEventManager : public CNxNObject {
	NXN_DECLARE_DYNCREATE(CNxNEventManager);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNEventManager();
		virtual ~CNxNEventManager();

		//---------------------------------------------------------------------------
		//	initialization/deinitialization
		//---------------------------------------------------------------------------
		bool InitInstance();
		bool ExitInstance();

		//---------------------------------------------------------------------------
		//	event target registration/unregistration
		//---------------------------------------------------------------------------
		bool RegisterEventTarget(CNxNEventTarget* pEventTarget);
		bool UnregisterEventTarget(CNxNEventTarget* pEventTarget);

		//---------------------------------------------------------------------------
		//	event dispatcher
		//---------------------------------------------------------------------------
		bool Dispatch(CNxNEventMsg& eEvent);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_EVENTMANAGER); };

	private:

		tNxNEventManagerData	m_pEventManagerData;
};

#endif // INC_NXN_EVENTMANAGER_H
