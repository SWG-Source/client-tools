// \addtodoc

#ifndef INC_NXN_OBJECT_H
#define INC_NXN_OBJECT_H

/* \class		CNxNObject
 *
 *	\file		NxNObject.h
 *
 *  \brief		CNxNObject is a very basic SDK class from which most other SDK classes are derived.
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

class NXNINTEGRATORSDK_API CNxNObject
{
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
        unsigned long	m_ulLastError;
		CNxNString		m_strLastErrorMessage;

        unsigned long	m_ulRef;
};

#endif // INC_NXN_OBJECT_H
