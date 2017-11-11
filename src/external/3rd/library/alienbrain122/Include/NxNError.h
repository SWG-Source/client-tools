#ifndef INC_NXN_XDK_ERROR_H
#define INC_NXN_XDK_ERROR_H

//---------------------------------------------------------------------------
// abstract macro for error throwing
//---------------------------------------------------------------------------
#define     NXN_ERROR(Type, ErrorCode, ErrorString)  { throw CNxNError(Type, ErrorCode, ErrorString); }

//---------------------------------------------------------------------------
// enumerated error constants
//---------------------------------------------------------------------------
enum eErrorType
{
	ET_GENERAL				= -1,       // general failure

    ET_WARNING			    = -2,

    ET_FATAL_ERROR          = -10,
};

//---------------------------------------------------------------------------
// definition of the CNxNError sructure
//---------------------------------------------------------------------------
class CNxNError
{
    public:
        CNxNError (eErrorType eType, unsigned long ulErrorCode, const CNxNString& strErrorMessage)
	    {
		    m_eType                 = eType;
            m_ulLastError           = ulErrorCode;
		    m_strLastErrorMessage   = strErrorMessage;
	    }

	    CNxNString GetErrorString() const;

    // private data members
    private:
        eErrorType      m_eType;                // error type
        unsigned long   m_ulLastError;          // error code
	    CNxNString      m_strLastErrorMessage;      // error string
};


//---------------------------------------------------------------------------
// Global functions for enabling/disabling exception handling
//---------------------------------------------------------------------------
void NxNError_UseExceptions(bool bUseExceptions = true);
bool NxNError_AreExceptionsEnabled();

#endif // INC_NXN_XDK_ERROR_H