// \addtodoc
#ifndef INC_NXN_CSTRING_H
#define INC_NXN_CSTRING_H

/* \class		CNxNCString NxNCString.h
 *
 *  \brief		This is a string conversion helper class.
 *
 *  \author		Axel Pfeuffer, (c) 1999-20001 by NxN Software AG
 *
 *  \version	1.00
 *
 *  \date		2001
 *
 *	\mod
 *		[ap]-25-Apr-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
// necessary includes
//---------------------------------------------------------------------------
#include <atlbase.h> // A2WBSTR
#include <functional> // binary_function
#include <macros.h>

//---------------------------------------------------------------------------
// internal data structure
//---------------------------------------------------------------------------
struct CNxNCStringData
{
	long    m_nRefs;              // reference count
	int     m_nDataLength;        // length of data (including terminator)
	int     m_nAllocLength;       // length of allocation
    char*   m_pszData;            // pointer to the string data itself.
};

// XDK string defined?
#ifdef INC_NXN_STRING_H 
    class CNxNString;
#endif

// CNxNBSTR string defined?
#ifdef INC_NXN_BSTR_H_
    class CNxNBSTR;
#endif

class NXNINTEGRATORSDK_API CNxNCString
{
    public:
        //---------------------------------------------------------------------------
        // construction/destruction
        //---------------------------------------------------------------------------
        CNxNCString();
        CNxNCString(LPCSTR  lpszString);
        CNxNCString(LPCWSTR lpszString);
// XDK string defined?
#ifdef INC_NXN_STRING_H 
        CNxNCString(const CNxNString& sSrc);
#endif
// CNxNBSTR string defined?
#ifdef INC_NXN_BSTR_H_
        CNxNCString(const CNxNBSTR& sSrc);
#endif

        virtual ~CNxNCString();

        //---------------------------------------------------------------------------
        // operators
        //---------------------------------------------------------------------------
        inline operator LPCSTR() const { return m_sdStringData.m_pszData; };

        const CNxNCString& operator=(const CNxNCString& sSrc);
// XDK string defined?
#ifdef INC_NXN_STRING_H 
        const CNxNCString& operator=(const CNxNString& sSrc);
#endif

// CNxNBSTR string defined?
#ifdef INC_NXN_BSTR_H_
        const CNxNCString& operator=(const CNxNBSTR& sSrc);
#endif
        const CNxNCString& operator=(LPCSTR lpszString);
        const CNxNCString& operator=(LPCWSTR lpszString);

        //---------------------------------------------------------------------------
        // information retrieval methods
        //---------------------------------------------------------------------------
        inline int GetLength() const { return m_sdStringData.m_nDataLength; };
        inline bool IsEmpty() const { return (GetLength() == 0); };

        inline CNxNCStringData* GetData() const { return (CNxNCStringData*)(&m_sdStringData); }

    // private helper methods
    private:
        void Init();

        void AllocBuffer(int nLen);
        void FreeBuffer();
        void TerminateBuffer();
        void CopyBuffer(int nSrcLen, LPCSTR lpszSrcData);
        void GrowBuffer(int nLen);

        inline int SafeStringLength(LPCSTR lpszString) { return (lpszString == NULL) ? 0 : strlen(lpszString); };

    private:
        CNxNCStringData         m_sdStringData;
};


#endif // INC_NXN_CSTRING_H
