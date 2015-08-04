// \addtodoc

#ifndef INC_NXN_FILE_H
#define INC_NXN_FILE_H

/* \class		CNxNFile
 *
 *	\file		NxNFile.h
 *
 *  \brief		CNxNFile is a class which can handle all kind of file types. All proper methods
 *				are inherited from the CNxNDbNode interface. To get more detailed informations
 *				about the useage, see there.
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

class NXNINTEGRATORSDK_API CNxNFile : public CNxNDbNode {
	NXN_DECLARE_DYNCREATE(CNxNFile);

	public:
		CNxNFile(CNxNFolder* pParent = NULL);
		CNxNFile(CNxNProject* pParent);
		virtual ~CNxNFile();

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_FILE); };

	private:
};

#endif // INC_NXN_FILE_H