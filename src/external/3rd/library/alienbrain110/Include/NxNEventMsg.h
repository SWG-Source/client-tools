// \addtodoc

#ifndef INC_NXN_EVENTMSG_H
#define INC_NXN_EVENTMSG_H

/*	\class		CNxNEventMsg NxNEvent.h
 *
 *  \brief		This class provides an interface for receiving events from the alienbrain/medializer
 *				kernel. Just implement this interface (using the provided macros), to be able
 *				to receive and process events. 
 *
 *  \author		Josef Göbel, Axel Pfeuffer
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-10-27-2000 file created and implementation copied from internal class.
 *	\endmod
 */

extern const CNxNString& NXN_PROPID_EVENTID;
extern const CNxNString& NXN_PROPID_EVENTHANDLE;
extern const CNxNString& NXN_PROPID_EVENTRESULT;
extern const CNxNString& NXN_PROPID_PRIORITY;
extern const CNxNString& NXN_PROPID_ABORTEVENT;
extern const CNxNString& NXN_PROPID_CONTEXTID;
extern const CNxNString& NXN_PROPID_CONTEXTBEGIN;
extern const CNxNString& NXN_PROPID_CONTEXTEND;


// NxN Namespace specific stuff
extern const CNxNString& NXN_PROPID_NAMESPACEPATH;
extern const CNxNString& NXN_PROPID_ABORTRECURSION;
extern const CNxNString& NXN_PROPID_ABORTCOMMAND;
extern const CNxNString& NXN_PROPID_OPCODE;
extern const CNxNString& NXN_PROPID_HIDEDIALOG;

extern const CNxNString& NXN_EVENTID_RECURSION;

//---------------------------------------------------------------------------
//	CNxNEventMsg class interface
//---------------------------------------------------------------------------
class NXNINTEGRATORSDK_API CNxNEventMsg : public CNxNString {
	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNEventMsg();
		CNxNEventMsg(const CNxNString& strEventMsg);
		virtual ~CNxNEventMsg();

		//---------------------------------------------------------------------------
		//	for event cleanup
		//---------------------------------------------------------------------------
		bool Clear();

		//---------------------------------------------------------------------------
		//	TODO! - operators
		//---------------------------------------------------------------------------
	
		//---------------------------------------------------------------------------
		//	property methods
		//---------------------------------------------------------------------------
		bool AddProperty(const CNxNString& strPropID, const CNxNString& strPropValue);
		bool RemoveProperty(const CNxNString& strPropID);

		bool SetProperty(const CNxNString& strPropID, const CNxNString& strPropValue);
		bool SetProperty(const CNxNString& strPropID, long lPropValueInt);

		bool GetProperty(const CNxNString& strPropID, CNxNString& strPropValue);
		CNxNString GetProperty(const CNxNString& strPropID);

		bool GetPropertyInt(const CNxNString& strPropID, long& lPropValueInt);
		long GetPropertyInt(const CNxNString& strPropID);

		//---------------------------------------------------------------------------
		//	event information retrieval concerning the properties
		//---------------------------------------------------------------------------
		int GetNumProperties();
		bool GetNthProperty(int nIndex, CNxNString& strPropID, CNxNString& strPropValue);
		CNxNString GetNthProperty(int nIndex, CNxNString& strPropID);
		CNxNString GetNthProperty(int nIndex);

	private:
};

#endif // INC_NXN_EVENTMSG_H
