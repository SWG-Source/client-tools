// \addtodoc

#ifndef INC_NXN_DISKFOLDER_H
#define INC_NXN_DISKFOLDER_H

/*	\class		CNxNDiskFolder NxNDiskFolder.h
 *
 *  \brief		This class represents folders on local disks.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-13-Feb-2000 Folder created.
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNDiskFolder : public CNxNDiskItem
{
	NXN_DECLARE_DYNCREATE(CNxNDiskFolder);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNDiskFolder(CNxNFolder* pParent = NULL);
		CNxNDiskFolder(CNxNDiskFolder* pParent);
		virtual ~CNxNDiskFolder();

		//---------------------------------------------------------------------------
		//	folder concerning methods
		//---------------------------------------------------------------------------
        CNxNDiskFolder* CreateFolder(const CNxNString& strFolderName, bool bShowDialog = false);
        bool Delete(CNxNDiskItem*& rpDiskItem);

		//---------------------------------------------------------------------------
		//	information retrieval
		//---------------------------------------------------------------------------
        CNxNDiskFolder* GetFolder(const CNxNString& strName);
        CNxNDiskFile* GetFile(const CNxNString& strName);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_DISKITEM_FOLDER);
        }

	private:
};

#endif // INC_NXN_DISKFOLDER_H
