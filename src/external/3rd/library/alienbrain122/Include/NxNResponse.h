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

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNResponseData*	tNxNResponseData;

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
/*		void SetWaitTitle(CNxNString& strWaitTitle);
		void SetWaitMessage(CNxNString& strWaitMessage);

*/		
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

/*		//---------------------------------------------------------------------------
		//	for non modal wait dialogs
		//---------------------------------------------------------------------------
		void WaitEnter(long lChildCount);
		void WaitLeave();

		//---------------------------------------------------------------------------
		//	displaying
		//---------------------------------------------------------------------------
		void ShowWaitDialog(bool bShow);
*/
		//---------------------------------------------------------------------------
		//	response properties
		//---------------------------------------------------------------------------
		bool SetProperty(CNxNProperty& propProperty);
		bool GetProperty(CNxNProperty& propProperty);

		//---------------------------------------------------------------------------
		//	direct access to response object data
		//---------------------------------------------------------------------------
		inline tNxNResponseData GetResponseData() const { return m_pResponseData; };



	private:

		tNxNResponseData	m_pResponseData;
};

#endif // INC_NXN_RESPONSE_H
