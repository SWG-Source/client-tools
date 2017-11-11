// \addtodoc

#ifndef INC_NXN_FOLDER_H
#define INC_NXN_FOLDER_H

/* \class		CNxNFolder NxNFolder.h
 *
 *  \brief		CNxNFile is a class which can handle folders that are contained in the project database. 
 *				All proper methods are inherited from the CNxNDbFolder interface. To get more detailed 
 *				informations about the usage, see there.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-02-May-2000 file created.
 *		[ap]-13-Jun-2000 GetFile() implemented.
 *		[ap]-13-Jun-2000 GetFolder() implemented.
 *		[ap]-13-Jun-2000 CreateFolder() implemented.
 *		[ap]-13-Jun-2000 CreateFile() implemented.
 *      [jr]-18-Dec-2000 Internationalization
 *		[ap]-19-Apr-2001 MODIFICATION: Base class changed to new CNxNDbFolder class.
 *		[ap]-20-Apr-2001 CreateFile() moved to CNxNDbFolder.
 *		[ap]-20-Apr-2001 CreateFolder() moved to CNxNDbFolder.
 *		[ap]-20-Apr-2001 BrowseForFolder() moved to CNxNDbFolder.
 *		[ap]-20-Apr-2001 BrowseForFile() moved to CNxNDbFolder.
 *		[ap]-20-Apr-2001 GetForFolder() moved to CNxNDbFolder.
 *		[ap]-20-Apr-2001 GetForFile() moved to CNxNDbFolder.
 *	\endmod
 */

//
// This file was changed during internationalization on 12/18/2000 by Jens.



class NXNINTEGRATORSDK_API CNxNFolder : public CNxNDbFolder
{
	NXN_DECLARE_DYNCREATE(CNxNFolder);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNFolder(CNxNDbFolder* pParent = NULL);
		virtual ~CNxNFolder();

		//---------------------------------------------------------------------------
		//	overloaded methods
		//---------------------------------------------------------------------------
		virtual CNxNString GetNamespacePath() const;

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_FOLDER);
        }

	private:
};


#endif // INC_NXN_FOLDER_H
