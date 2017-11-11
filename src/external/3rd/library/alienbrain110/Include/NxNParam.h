// \addtodoc

#ifndef INC_NXN_PARAM_H
#define INC_NXN_PARAM_H

/* \class		CNxNParam
 *
 *	\file		NxNParam.h
 *
 *  \brief		This class encapsulates the complete functionality of the already existing
 *				CNxNParamArray.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-02-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNParamData*	tNxNParamData;

class NXNINTEGRATORSDK_API CNxNParam {
	//---------------------------------------------------------------------------
	//	friend classes
	//---------------------------------------------------------------------------
	friend CNxNParam;

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
		bool SetAt(long lIndex, CNxNString& strValue);
		bool SetAt(long lIndex, long lValue);
		bool SetBoolAt(long lIndex, bool bValue);

#ifdef _MFC_VER
		bool SetAt(long lIndex, CString& strValue);
		bool SetAt(long lIndex, LPCSTR pValue);
#endif _MFC_VER

		//----------------------------------
		//	get other information 
		//----------------------------------
		long GetMaxParams(void)	const;
		long* GetActualParams(void) const;
		CNxNString GetParams(void);
//		BSTR* GetParams(void);
//		const BSTR* GetParamArray(void) const;

		//----------------------------------
		//	get direct access to parameter data
		//----------------------------------
		tNxNParamData GetParamData() const { return m_pParamData; };

	private:
		tNxNParamData	m_pParamData;
};

#endif // INC_NXN_PARAM_H
