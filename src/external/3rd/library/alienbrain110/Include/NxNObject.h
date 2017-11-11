// \addtodoc

#ifndef INC_NXN_OBJECT_H
#define INC_NXN_OBJECT_H

/* \class		CNxNObject
 *
 *	\file		NxNObject.h
 *
 *  \brief		CNxNObject is the base class for all NxN specific object used in this SDK.
 *				By default this object has no functionality except the error handling.
 *				As you can see in the interface description there are two data members that
 *				are relevant for error handling.
 *				<ul>
 *					<li><b>m_ulLastError:</b> This data member contains the encoded error code.
 *					<li><b>m_strLastErrorMessage:</b> This data member contains the error code as a string.
 *				</ul>
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
typedef struct _tNxNObjectData*	tNxNObjectData;

class NXNINTEGRATORSDK_API CNxNObject {
	NXN_DECLARE_DYNCREATE(CNxNObject);

	public:
		CNxNObject();
		virtual ~CNxNObject();

		unsigned long GetLastError() const;
		CNxNString GetLastErrorMessage() const;

//		unsigned long AddRef();
//		unsigned long Release();

	protected:
		void SetLastError(unsigned long ulLastError) const;

	private:
		tNxNObjectData	m_pObjectData;
		unsigned long	m_ulLastError;
		CNxNString		m_strLastErrorMessage;
		unsigned long	m_ulRef;
};

#endif // INC_NXN_OBJECT_H
