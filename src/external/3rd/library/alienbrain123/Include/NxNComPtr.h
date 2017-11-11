#ifndef INC_NXN_COMSTLPTR_H
#define INC_NXN_COMSTLPTR_H

template <class Interface>
class CComSTLPtr 
{
public:
    // Construction
	CComSTLPtr()
	{
        // Standard construction. Reseting everything to unused.
		m_pInterface = NULL;
        m_pStreamToInterface = NULL;
        m_dwThreadIdOfStreamWriter = NULL;
        m_hStreamMutex = ::CreateMutex( NULL, FALSE, NULL );
	}


	CComSTLPtr( Interface* pInterface )
	{
        // Construction with a COM pointer.
        m_pInterface = pInterface;
        m_pStreamToInterface = NULL;
        m_dwThreadIdOfStreamWriter = NULL;
        m_hStreamMutex = ::CreateMutex( NULL, FALSE, NULL );

        if ( m_pInterface == NULL )
            return;

        // Reference for the cache.
        m_pInterface->AddRef();

        // Stream the interface.
        PrepareStream();
    }


	CComSTLPtr( const CComSTLPtr< Interface >& pSTLPtr )
	{
        // Construction with an encapsulated pointer.
        m_pStreamToInterface = NULL;
        m_dwThreadIdOfStreamWriter = NULL;
        m_hStreamMutex = ::CreateMutex( NULL, FALSE, NULL );

        m_pInterface = pSTLPtr.m_pInterface;

        if ( m_pInterface == NULL )
            return;

        // Reference for the cache.
        m_pInterface->AddRef();

        // Stream the interface.
        PrepareStream();
	}


    // Destruction
	~CComSTLPtr()
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        // Release the stream.
        ReleaseStream();

        if ( m_hStreamMutex != NULL )
        {
            HANDLE hMutex = m_hStreamMutex;

            m_hStreamMutex = NULL;

            ::ReleaseMutex( hMutex );
            ::CloseHandle( hMutex );
        }
    }


    // Create instance
    HRESULT CreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL )
    {
        HRESULT     hRes;
        Interface*  pInterface;

        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        hRes = ::CoCreateInstance( rclsid, pUnkOuter, dwClsContext, __uuidof( Interface ), (void**)&pInterface );  // Addrefs the object.
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLPtr: CreateInstance failed!" );
            ::ReleaseMutex( m_hStreamMutex );
            return hRes;
        }

        // Maybe we're already assigned.
        hRes = ReleaseStream();
        if ( FAILED( hRes ) )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return hRes;
        }

        // Copy the interface to the cache.
        m_pInterface = pInterface;

        // Stream the interface.
        hRes = PrepareStream();
        if ( FAILED( hRes ) )
        {
            m_pInterface->Release();
            ::ReleaseMutex( m_hStreamMutex );
            return hRes;
        }

        // We keep a reference in m_pInterface for the cache.

        ::ReleaseMutex( m_hStreamMutex );
        return hRes;
    }



    // Casts
	operator Interface*()
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        if ( m_pInterface == NULL )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return NULL;
        }

        // Prepare the pointer for this thread.
        if ( FAILED( PrepareForThread() ) )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return NULL;
        }

        // Caller must release this reference.
        Interface* pInterface = m_pInterface;
//        int iCount = pInterface->AddRef();

        ::ReleaseMutex( m_hStreamMutex );

        return pInterface;
	}


    // Indirections
/*	Interface& operator*()
	{
        Interface* pInterface;

        // Prepare the pointer for this thread.
        pInterface = PrepareForThread();        

		NXN_ASSERT( pInterface != NULL );
        return *pInterface;
	}*/


	Interface* operator->()
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        m_pInterface->AddRef();
        int nRefCount = m_pInterface->Release();

        // Prepare the pointer for this thread.
        if ( FAILED( PrepareForThread() ) )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return NULL;
        }

        Interface* pInterface = m_pInterface;

        m_pInterface->AddRef();
        nRefCount = m_pInterface->Release();

        ::ReleaseMutex( m_hStreamMutex );

        return pInterface;
	}


	const CComSTLPtr< Interface >& operator=( Interface* pInterface )
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        // Maybe we're already assigned.
        ReleaseStream();

        // Assign.
        m_pInterface = pInterface;
        m_pStreamToInterface = NULL;
        m_dwThreadIdOfStreamWriter = NULL;

        if ( m_pInterface == NULL )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return *this;
        }

        // Reference for the cache.
        m_pInterface->AddRef();

        // Stream the interface.
        PrepareStream();

        ::ReleaseMutex( m_hStreamMutex );

        return *this;
	}


	const CComSTLPtr< Interface >& operator=( const CComSTLPtr< Interface >& pSTLPtr )
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        // Maybe we're already assigned.
        ReleaseStream();

        // Assign.
        m_pInterface = pSTLPtr.m_pInterface;
        m_pStreamToInterface = NULL;
        m_dwThreadIdOfStreamWriter = NULL;

        if ( m_pInterface == NULL )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return *this;
        }

        // Reference for the cache.
        m_pInterface->AddRef();

        // Stream the interface.
        PrepareStream();

        ::ReleaseMutex( m_hStreamMutex );

        return *this;
	}


	bool operator!() const
	{
        // This can be done without preparing the pointer for the current thread.
		return ( m_pInterface == NULL );
	}


protected:
    HRESULT PrepareStream()
    {
        HRESULT hRes;

        // Preparing the stream must stream the interface pointer.

#if defined(DEBUG) || defined(_DEBUG)
        // In debug mode we check the interface pointer if it
        // supports the interface we need.
        Interface* pInterface;
        hRes = m_pInterface->QueryInterface( __uuidof( Interface ), (void**)&pInterface );
        if ( FAILED( hRes ) )
        {
            return hRes;
        }
        int iCount = m_pInterface->Release();
#endif
        
        // Create stream and put interface pointer into it.
        hRes = ::CoMarshalInterThreadInterfaceInStream( __uuidof( Interface ), (IUnknown*)m_pInterface, &m_pStreamToInterface );
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLPtr: Prepare stream failed. Streaming of pointer was not possible." );
            m_pInterface = NULL;
            return hRes;
        }

        // What's the thread's id?
        m_dwThreadIdOfStreamWriter = ::GetCurrentThreadId();

        return hRes;
    }


    HRESULT ReleaseStream()
    {
        HRESULT hRes = S_OK;

        if ( m_pStreamToInterface != NULL )
        {
            // Releasing cache.
            m_pInterface->Release();
            m_pInterface = NULL;

            hRes = ::CoGetInterfaceAndReleaseStream( m_pStreamToInterface, __uuidof( Interface ), (void**)&m_pInterface );
            m_pStreamToInterface = NULL;
            if ( FAILED( hRes ) )
            {
                NXN_ASSERT( !"CComSTLPtr: ReleaseStream failed! Could not read from the stream!" );
            } 

            // Releasing reference, because we don't need it anymore.
            int iCount = m_pInterface->Release();
            m_pInterface = NULL;

            m_dwThreadIdOfStreamWriter = NULL;
        }

        return hRes;
    }


    HRESULT PrepareForThread()
    {
        HRESULT hRes;

        if ( m_dwThreadIdOfStreamWriter == ::GetCurrentThreadId() )
        {
            // Cache hit.
            // The object stored can be accessed directly by this thread.
            return S_OK;
        }

        if ( m_pInterface == NULL )
            return S_OK;

        // We need to the stream.
        if ( m_pStreamToInterface == NULL )
        {
            NXN_ASSERT( !"CComSTLPtr: PrepareForThread failed! There's no stream!" );
            return E_FAIL;
        }

        // Releasing cache.
        m_pInterface->Release();
        m_pInterface = NULL;

        // The interface pointer must be marshaled.
        hRes = ::CoGetInterfaceAndReleaseStream( m_pStreamToInterface, __uuidof( Interface ), (void**)&m_pInterface );
        m_pStreamToInterface = NULL;
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLPtr: PrepareForThread failed! Could not read pointer from the stream!" );
            return NULL;
        }

        // Since there might be somebody else needing this pointer later on, we have to write it to
        // a new stream object.
        hRes = ::CoMarshalInterThreadInterfaceInStream( __uuidof( Interface ), (IUnknown*)m_pInterface, &m_pStreamToInterface );
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLPtr: PrepareForThread failed! Could not write the pointer back to a stream!" );
            m_pInterface = NULL;
            return NULL;
        }

        // What's the thread's id?
        m_dwThreadIdOfStreamWriter = ::GetCurrentThreadId();

        // Everything's fine.
        return hRes;
    }




private:
	Interface* m_pInterface;
    IStream*   m_pStreamToInterface;
    DWORD      m_dwThreadIdOfStreamWriter;
    HANDLE     m_hStreamMutex;
};








template <class Interface>
class CComSTLSingletonPtr
{
public:
    // Construction
	CComSTLSingletonPtr()
	{
        // Standard construction. Reseting everything to unused.
		m_pInterface = NULL;
        m_pStreamToInterface = NULL;
        m_dwThreadIdOfStreamWriter = NULL;
        m_hStreamMutex = ::CreateMutex( NULL, FALSE, NULL );
        m_hMapObject = NULL;
	}


    // Destruction
	~CComSTLSingletonPtr()
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        if ( m_hMapObject != NULL )
        {
            RetrieveGlobalStream();

            ::CloseHandle( m_hMapObject );
            m_hMapObject = NULL;

            // Try to retrieve it again. If it fails we need to release
            // the stream, because we are the last one using it.
            if ( RetrieveGlobalStream() == S_FALSE )
            {
                // Last one turns out the light.
                ReleaseStream();
            }
            else
            {
                // Only release cache.
                m_pInterface->Release();
                m_pInterface = NULL;
            }

            if ( m_hMapObject != NULL )
            {
                ::CloseHandle( m_hMapObject );
                m_hMapObject = NULL;
            }
        }

        if ( m_hStreamMutex != NULL )
        {
            HANDLE hMutex = m_hStreamMutex;

            m_hStreamMutex = NULL;

            ::ReleaseMutex( hMutex );
            ::CloseHandle( hMutex );
        }
    }


    // Singleton create.
    HRESULT GetOrCreateSingleton( REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL )
    {
        HRESULT hRes;

        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        // Maybe we're already assigned.
        if ( m_hMapObject != NULL )
        {
            ::CloseHandle( m_hMapObject );
            m_hMapObject = NULL;

            // Try to retrieve it again. If it fails we need to release
            // the stream, because we are the last one using it.
            if ( RetrieveGlobalStream() == S_FALSE )
            {
                // Last one turns out the light.
                hRes = ReleaseStream();
                if ( FAILED( hRes ) )
                {
                    ::ReleaseMutex( m_hStreamMutex );
                    return hRes;
                }
            }

            if ( m_hMapObject != NULL )
            {
                ::CloseHandle( m_hMapObject );
                m_hMapObject = NULL;
            }
        }

        m_pInterface = NULL;
        m_pStreamToInterface = NULL;
        m_dwThreadIdOfStreamWriter = NULL;

        // Retrieve the global stream object.
        hRes = RetrieveGlobalStream();
        if ( hRes == S_FALSE )
        {        
            // Wasn't available, so create new instance.
            hRes = CreateInstance( rclsid, pUnkOuter, dwClsContext );
            if ( !FAILED( hRes ) )
            {
                // Write to global stream.
                hRes = PutGlobalStream();
            }
        }
        else if ( !FAILED( hRes ) )
        {
            // Global stream was available. Now prepare for the current thread.
            hRes = PrepareForThread();
        }

        ::ReleaseMutex( m_hStreamMutex );

        return hRes;
    }



    // Casts
	operator Interface*() const
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        if ( m_hMapObject == NULL )
            return NULL;

        // Retrieve global stream.
        if ( FAILED( RetrieveGlobalStream() ) )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return NULL;
        }

        // Prepare the pointer for this thread.
        if ( FAILED( PrepareForThread() ) )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return NULL;
        }

        // Caller must release this reference.
        Interface* pInterface = m_pInterface;
        int iCount = pInterface->AddRef();

        ::ReleaseMutex( m_hStreamMutex );

        return pInterface;
	}


    // Indirection
	Interface* operator->()
	{
        ::WaitForSingleObject( m_hStreamMutex, INFINITE );

        if ( m_hMapObject == NULL )
            return NULL;

        // Retrieve global stream.
        if ( FAILED( RetrieveGlobalStream() ) )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return NULL;
        }

        // Prepare the pointer for this thread.
        if ( FAILED( PrepareForThread() ) )
        {
            ::ReleaseMutex( m_hStreamMutex );
            return NULL;
        }

        Interface* pInterface = m_pInterface;

        ::ReleaseMutex( m_hStreamMutex );

        return pInterface;
	}


protected:
    HRESULT PrepareStream()
    {
        HRESULT hRes;

        // Preparing the stream must stream the interface pointer.

#if defined(DEBUG) || defined(_DEBUG)
        // In debug mode we check the interface pointer if it
        // supports the interface we need.
        Interface* pInterface;
        hRes = m_pInterface->QueryInterface( __uuidof( Interface ), (void**)&pInterface );
        if ( FAILED( hRes ) )
        {
            return hRes;
        }
        int iCount = m_pInterface->Release();
#endif
        
        // Create stream and put interface pointer into it.
        hRes = ::CoMarshalInterThreadInterfaceInStream( __uuidof( Interface ), (IUnknown*)m_pInterface, &m_pStreamToInterface );
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLSingletonPtr: Prepare stream failed. Streaming of pointer was not possible." );
            m_pInterface = NULL;
            return hRes;
        }

        // What's the thread's id?
        m_dwThreadIdOfStreamWriter = ::GetCurrentThreadId();

        return hRes;
    }


    HRESULT ReleaseStream()
    {
        HRESULT hRes = S_OK;

        if ( m_pStreamToInterface != NULL )
        {
            // Releasing cache.
            m_pInterface->Release();
            m_pInterface = NULL;

            hRes = ::CoGetInterfaceAndReleaseStream( m_pStreamToInterface, __uuidof( Interface ), (void**)&m_pInterface );
            m_pStreamToInterface = NULL;
            if ( FAILED( hRes ) )
            {
                NXN_ASSERT( !"CComSTLSingletonPtr: ReleaseStream failed! Could not read from the stream!" );
            } 

            // Releasing reference, because we don't need it anymore.
            int iCount = m_pInterface->Release();
            m_pInterface = NULL;

            m_dwThreadIdOfStreamWriter = NULL;
        }

        return hRes;
    }


    HRESULT PrepareForThread()
    {
        HRESULT hRes;

        // We need to the stream.
        if ( m_pStreamToInterface == NULL )
        {
            NXN_ASSERT( !"CComSTLSingletonPtr: PrepareForThread failed! There's no stream!" );
            return E_FAIL;
        }

        // Releasing cache.
        if ( m_pInterface != NULL )
        {
            m_pInterface->Release();
            m_pInterface = NULL;
        }

        // The interface pointer must be marshaled.
        hRes = ::CoGetInterfaceAndReleaseStream( m_pStreamToInterface, __uuidof( Interface ), (void**)&m_pInterface );
        m_pStreamToInterface = NULL;
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLSingletonPtr: PrepareForThread failed! Could not read pointer from the stream!" );
            return NULL;
        }

        // Since there might be somebody else needing this pointer later on, we have to write it to
        // a new stream object.
        hRes = ::CoMarshalInterThreadInterfaceInStream( __uuidof( Interface ), (IUnknown*)m_pInterface, &m_pStreamToInterface );
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLSingletonPtr: PrepareForThread failed! Could not write the pointer back to a stream!" );
            m_pInterface = NULL;
            return NULL;
        }

        // Stream changed. Put it back into the global stream.
        hRes = PutGlobalStream();
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLSingletonPtr: PrepareForThread failed! Could not write the stream to shared memory!" );
            m_pInterface = NULL;
            return NULL;
        }

        // What's the thread's id?
        m_dwThreadIdOfStreamWriter = ::GetCurrentThreadId();

        // Everything's fine.
        return hRes;
    }

    
    HRESULT RetrieveGlobalStream()
    {
        IStream**   pGlobalStream;

        if ( m_hMapObject == NULL )
        {
            m_hMapObject = ::CreateFileMapping( INVALID_HANDLE_VALUE,           // use paging file
                                                NULL,                           // no security attributes
                                                PAGE_READWRITE,                 // read/write access
                                                0,                              // size: high 32-bits
                                                sizeof( m_pStreamToInterface ), // size: low 32-bits
                                                "dllmemfilemap" );              // name of map object

            if ( m_hMapObject == NULL ) 
                return E_OUTOFMEMORY; 

            if ( GetLastError() != ERROR_ALREADY_EXISTS ) 
            {
                return S_FALSE;
            }
        }

        // Get a pointer to the file-mapped shared memory.

        pGlobalStream = (IStream** )::MapViewOfFile( m_hMapObject,   // object to map view of
                                                     FILE_MAP_WRITE, // read/write access
                                                     0,              // high offset:  map from
                                                     0,              // low offset:   beginning
                                                     0 );            // default: map entire file
        if ( pGlobalStream == NULL )
        {
            ::CloseHandle( m_hMapObject ); 
            m_hMapObject = NULL;
            m_bSingleton = false;
            return E_FAIL;
        }

        m_pStreamToInterface = *pGlobalStream;

        ::UnmapViewOfFile( pGlobalStream );

        return S_OK;
    }


    HRESULT PutGlobalStream()
    {
        IStream**   pGlobalStream;

        if ( m_hMapObject == NULL )
        {
            m_hMapObject = ::CreateFileMapping( INVALID_HANDLE_VALUE,           // use paging file
                                                NULL,                           // no security attributes
                                                PAGE_READWRITE,                 // read/write access
                                                0,                              // size: high 32-bits
                                                sizeof( m_pStreamToInterface ), // size: low 32-bits
                                               "dllmemfilemap" );              // name of map object

            if ( m_hMapObject == NULL )
                return E_OUTOFMEMORY; 
        }

        // Get a pointer to the file-mapped shared memory.

        pGlobalStream = (IStream** )::MapViewOfFile( m_hMapObject,   // object to map view of
                                                     FILE_MAP_WRITE, // read/write access
                                                     0,              // high offset:  map from
                                                     0,              // low offset:   beginning
                                                     0 );            // default: map entire file
        if ( pGlobalStream == NULL )
        {
            ::CloseHandle( m_hMapObject ); 
            m_hMapObject = NULL;
            m_bSingleton = false;
            return E_FAIL;
        }

        *pGlobalStream = m_pStreamToInterface;

        ::UnmapViewOfFile( pGlobalStream );

        return S_OK;
    }



    // Create instance
    HRESULT CreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL )
    {
        HRESULT     hRes;
        Interface*  pInterface;

        hRes = ::CoCreateInstance( rclsid, pUnkOuter, dwClsContext, __uuidof( Interface ), (void**)&pInterface );  // Addrefs the object.
        if ( FAILED( hRes ) )
        {
            NXN_ASSERT( !"CComSTLSingletonPtr: CreateInstance failed!" );
            ::ReleaseMutex( m_hStreamMutex );
            return hRes;
        }

        // Copy the interface to the cache.
        m_pInterface = pInterface;

        // Stream the interface.
        hRes = PrepareStream();
        if ( FAILED( hRes ) )
        {
            m_pInterface->Release();
            ::ReleaseMutex( m_hStreamMutex );
            return hRes;
        }

        // We keep a reference in m_pInterface for the cache.

        return hRes;
    }


private:
	Interface* m_pInterface;
    IStream*   m_pStreamToInterface;
    DWORD      m_dwThreadIdOfStreamWriter;
    HANDLE     m_hStreamMutex;
    bool       m_bSingleton;
    HANDLE     m_hMapObject;
};



#endif // INC_NXN_COMSTLPTR_H