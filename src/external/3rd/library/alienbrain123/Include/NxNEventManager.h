// \addtodoc

#ifndef INC_NXN_EVENTMANAGER_H
#define INC_NXN_EVENTMANAGER_H

/*  \class      CNxNEventManager NxNEventManager.h
 *
 *  \brief      This class is responsible for the event message routing mechanism introduced
 *              with alienbrain 4.0.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-17-Jul-2000 file created.
 *  \endmod
 */

class CNxNEventListener_Impl;

class NXNINTEGRATORSDK_API CNxNEventManager : public CNxNObject
{
    NXN_DECLARE_DYNCREATE(CNxNEventManager);

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNEventManager();
        virtual ~CNxNEventManager();

        //---------------------------------------------------------------------------
        //  initialization/deinitialization
        //---------------------------------------------------------------------------
        bool InitInstance();
        bool ExitInstance();

        //---------------------------------------------------------------------------
        //  event target registration/unregistration
        //---------------------------------------------------------------------------
        bool RegisterEventTarget(CNxNEventTarget* pEventTarget);
        bool UnregisterEventTarget(CNxNEventTarget* pEventTarget);

        //---------------------------------------------------------------------------
        //  event dispatcher
        //---------------------------------------------------------------------------
        bool Dispatch(CNxNEventMsg& eEvent);

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_EVENTMANAGER);
        }


    private:
        typedef class NXNINTEGRATORSDK_API CNxNArray<CNxNEventTarget*> tEventTargetList;

        CNxNEventListener_Impl*     m_pEventListener;
        long                        m_lEventListenerID;
        tEventTargetList            m_aEventTargets;
};

#endif // INC_NXN_EVENTMANAGER_H
