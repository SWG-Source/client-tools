// \addtodoc

/*!  \class		CNxNMutex NxNMutex.h
 *
 *  \brief		This is a wrapper class for mutex objects used for synchronizing threads
 *				and processes sharing the same resources.
 *
 *  \author		Axel Pfeuffer
 *
 *  \version	1.0
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-09-Aug-2000 file created.
 *      [jr]-18-Dec-2000 Internationalization
 *	\endmod
 */

//
// This file was changed during internationalization on 12/18/2000 by Jens.


#ifndef INC_NXN_MUTEX_HPP
#define INC_NXN_MUTEX_HPP

#ifndef _INC_WINDOWS
#	include <windows.h>
#endif
#ifndef _INC_PROCESS
#	include <process.h>
#endif


class CNxNMutex
{
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNMutex();
		virtual ~CNxNMutex();

		//---------------------------------------------------------------------------
		//	mutex access methods
		//---------------------------------------------------------------------------
		void Lock();
		void Unlock();
		bool IsLocked();

		HANDLE GetHandle() { return m_hMutex; };

	private:
		HANDLE	m_hMutex;
		bool	m_bIsLocked;
};


/*!	\fn			inline CNxNMutex::CNxNMutex()
 *	
 *				Creates a mutex object. This is an basic object and must be created 
 *				to make any further application processes possible. 
 *	
 *	\param		none
 *	
 *	\return		none
 *	
 *	\note		
 */
inline CNxNMutex::CNxNMutex()
{
    m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
}


/*!	\fn			inline CNxNMutex::~CNxNMutex()
 *	
 *				Destructs the mutex object.
 *	
 *	\param		none
 *	
 *	\return		none
 *	
 *	\note		
 */
inline CNxNMutex::~CNxNMutex()
{
    ::CloseHandle(m_hMutex);
}


/*!	\fn			inline void CNxNMutex::Lock()
 *	
 *				This method locks a mutex.  If another thread tries to lock a locked mutex, it waits until the mutex is unlocked. 
 *	
 *	\param		none
 *	
 *	\return		none
 *	
 *	\note		
 */
inline void CNxNMutex::Lock()
{
    ::WaitForSingleObject(m_hMutex, INFINITE);
}

/*!	\fn			inline void CNxNMutex::Unlock()
 *	
 *				This method unlocks a previously locked mutex object.
 *	
 *	\param		none
 *	
 *	\return		none
 *	
 *	\note		
 */
inline void CNxNMutex::Unlock()
{
    ::ReleaseMutex(m_hMutex);
}


/*!	\fn			inline bool CNxNMutex::IsLocked()
 *	
 *				This method provides the callee with the information if the mutex
 *				object is currently locked or not.
 *	
 *	\param		none
 *	
 *	\return		boolean expression indicating if the mutex object is locked.
 *	
 *	\note		
 */
inline bool CNxNMutex::IsLocked()
{
    DWORD dwResult = ::WaitForSingleObject(m_hMutex, 0);

	return (dwResult != WAIT_OBJECT_0);
}

#endif // INC_NXN_MUTEX_HPP
