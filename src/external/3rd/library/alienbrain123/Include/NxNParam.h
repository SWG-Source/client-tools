// \addtodoc

#ifndef INC_NXN_PARAM_H
#define INC_NXN_PARAM_H

/* \class		CNxNParam
 *
 *	\file		NxNParam.h
 *
 *  \brief      This class offers some functionality to assemble parameter arrays used for
 *              the various commands that can be issued to the namespace.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-02-May-2000 file created.
 *	\endmod
 */

class CNxNParamArray;

class NXNINTEGRATORSDK_API CNxNParam
{
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNParam(const CNxNParam& Other);
		CNxNParam(long lMaxParams, long* pActualParams, const CNxNString& strParam);
		CNxNParam(long lMaxParams);

		virtual ~CNxNParam();

		void Reset(bool bClearContents = true);
		void ResetAndCopyFrom(const CNxNParam& Other);

		//----------------------------------
		//	parameter retrieval methods
		//----------------------------------
		CNxNString GetString(long lIndex) const;
		long GetInt(long lIndex) const;
		long GetLong(long lIndex) const; 
		DWORD GetDWORD(long lIndex) const;
		bool GetBool(long lIndex, bool bDefault) const;

		//----------------------------------
		//	set an entry in the return array
		//----------------------------------
		bool SetAt(long lIndex, const CNxNString& strValue);
		bool SetAt(long lIndex, long lValue);
		bool SetBoolAt(long lIndex, bool bValue);

#ifdef _MFC_VER
		bool SetAt(long lIndex, const CString& strValue);
		bool SetAt(long lIndex, LPCSTR pValue);
#endif _MFC_VER

		//----------------------------------
		//	get other information 
		//----------------------------------
		long GetMaxParams(void)	const;
		long* GetActualParams(void) const;
		CNxNString GetParams(void);

        CNxNParamArray* GetParamArray()
        {
            return m_pParamArray;
        }


    private:
        CNxNParamArray* m_pParamArray;
};

#endif // INC_NXN_PARAM_H
