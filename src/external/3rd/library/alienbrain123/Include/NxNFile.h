// \addtodoc

#ifndef INC_NXN_FILE_H
#define INC_NXN_FILE_H

/* \class		CNxNFile
 *
 *	\file		NxNFile.h
 *
 *  \brief		CNxNFile is a class that represents all kind of files stored in the database. All proper methods
 *				are inherited from the CNxNDbNode interface. To get more detailed information
 *				about the usage, see there.
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

class CNxNDbFolder;

class NXNINTEGRATORSDK_API CNxNFile : public CNxNDbNode
{
	NXN_DECLARE_DYNCREATE(CNxNFile);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNFile(CNxNDbFolder* pParent = NULL);
		virtual ~CNxNFile();

        //---------------------------------------------------------------------------
		//	File / namespace type
		//---------------------------------------------------------------------------
        bool IsShare() const;
        virtual CNxNType GetNamespaceType() const;

        //---------------------------------------------------------------------------
		//	methods concerning file history
		//---------------------------------------------------------------------------
        bool HasHistory();
        CNxNHistory* GetHistory();
        bool ShowHistoryDialog();

        //---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_FILE);
        }

	private:
};

#endif // INC_NXN_FILE_H