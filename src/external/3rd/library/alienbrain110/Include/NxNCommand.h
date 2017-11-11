// \addtodoc

#ifndef INC_NXN_PARAM_H
#pragma message( "     Include of 'NxNParam.h' is missing before!" )
#endif

#ifndef INC_NXN_COMMAND_H
#define INC_NXN_COMMAND_H

/* \class		CNxNCommand
 *
 *	\file		NxNCommand.h
 *
 *  \brief		CNxNCommand is a parameterized class used for calling alienbrain functions via
 *				RunCommand(). This class contains the command name plus the concerning parameters.
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
typedef struct _tNxNCommandData*	tNxNCommandData;


class NXNINTEGRATORSDK_API CNxNCommand : public CNxNObject {

	public:

		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
//		CNxNCommand();
		CNxNCommand(const CNxNString& strCommandOpCode, const CNxNString& strPath, const CNxNParam& aParams, long lFlags = 0, CNxNResponse* pResponse = NULL);
		CNxNCommand(const CNxNString& strCommandOpCode, const CNxNString& strPath, long lMaxParams, long* pActualParams, const CNxNString& strParam, long lFlags = 0, CNxNResponse* pResponse = NULL);
		CNxNCommand(const CNxNString& strCommandOpCode, const CNxNString& strPath, long lMaxParams, long lFlags = 0, CNxNResponse* pResponse = NULL);

		virtual ~CNxNCommand();

		//----------------------------------
		//	command modification methods
		//----------------------------------
		void SetOpCode(const CNxNString& strCommandOpCode);
		CNxNString GetOpCode() const;

		void SetPath(const CNxNString& strPath);
		CNxNString GetPath() const;

		void SetFlags(long lFlags);
		void AddFlags(long lFlags);
		void RemoveFlags(long lFlags);
		long GetFlags() const;

		void SetResponse(CNxNResponse* pResponse);
		CNxNResponse* GetResponse() const;

		void SetHwnd(long hWnd);
		long GetHwnd() const;

		void Reset(bool bClearContents = true);

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
//		BSTR* GetParams(void);
//		const BSTR* GetParamArray(void) const;

		//----------------------------------
		//	get direct access to command data
		//----------------------------------
		tNxNCommandData GetCommandData() const { return m_pCommandData; };

	private:
		tNxNCommandData	m_pCommandData;
};

#endif // INC_NXN_COMMAND_H