// \addtodoc

#ifndef INC_NXN_RESPONSE_H
#define INC_NXN_RESPONSE_H

/* \class		CNxNResponse NxNReponse.h
 *
 *  \brief		This class is currently not used!
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	0.10
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-02-May-2000 file created.
 *	\endmod
 */


class CNxNResponseObjectProxy;

class NXNINTEGRATORSDK_API CNxNResponse : public CNxNObject
{
	NXN_DECLARE_DYNCREATE(CNxNResponse);

	friend CNxNIntegrator;

	public:
		//--------------------------------------------------------------------
		// construction / destruction
		//--------------------------------------------------------------------
		CNxNResponse();
		virtual ~CNxNResponse();

		//--------------------------------------------------------------------
		// initialization
		//--------------------------------------------------------------------
		bool Initialize(CNxNIntegrator* pIntegrator);
		bool Uninitialize();

		//--------------------------------------------------------------------
		// standard dialog information retrieval
		//--------------------------------------------------------------------
		void SetParentWindow(long hWnd);

        void SetDisplayed(bool bDisplayed);
		bool WasDisplayed();

		void SetCanceled(bool bCancelled)
        {
            SetCancelled(bCancelled);
        }

        bool WasCanceled()
        {
            return WasCancelled();
        }

		bool WasCancelled();
        void SetCancelled(bool bCancelled);

		//---------------------------------------------------------------------------
		//	response properties
		//---------------------------------------------------------------------------
		bool SetProperty(CNxNProperty& propProperty);
		bool GetProperty(CNxNProperty& propProperty);

        CNxNResponseObjectProxy* GetResponseObjectProxy()
        {
            return m_pResponseObjectProxy;
        }


	private:
        CNxNResponseObjectProxy* m_pResponseObjectProxy;
	    CNxNIntegrator*			 m_pIntegrator;
};

#endif // INC_NXN_RESPONSE_H
