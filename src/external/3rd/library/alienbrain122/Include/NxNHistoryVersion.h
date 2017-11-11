// \addtodoc

#ifndef INC_NXN_HISTORYVERSION_H
#define INC_NXN_HISTORYVERSION_H

/*	\class		CNxNHistoryVersion NxNHistoryVersion.h
 *
 *  \brief		This class represents a specific version of a file.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-13-Feb-2000 file created.
 *	\endmod
 */

class NXNINTEGRATORSDK_API CNxNHistoryVersion : public CNxNVersionControlItem
{
	NXN_DECLARE_DYNCREATE(CNxNHistoryVersion);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNHistoryVersion(CNxNHistory* pParent = NULL);
		virtual ~CNxNHistoryVersion();

		//---------------------------------------------------------------------------
		//	command methods
		//---------------------------------------------------------------------------
        bool View();
        bool Get(bool bShowDialog = true,
                 const CNxNString& sDestDir = _STR(""),
                 eNxNVersionOverwritePolicy eOverwritePolicy = NXN_VC_OVERWRITE_POLICY_ASK,
                 eNxNVersionOverwritePolicy eOverwritePolicyWriteable = NXN_VC_OVERWRITE_POLICY_ASK,
                 bool bRunSilent = false);
        bool Rollback(bool bShowDialog = true,
                      const CNxNString& sComment = _STR(L""),
                      bool bGetLocalCopy = true,
                      bool bRestoreProps = true,
                      bool bRunSilent = false);

        //---------------------------------------------------------------------------
		//	information retrieval
		//---------------------------------------------------------------------------
		long GetVersionNumber() const;
		long GetHandle() const;
		long GetChecksum() const;

		long GetBucketHandle() const;
		CNxNString GetBucketFileName() const;

		TNxNHugeInt GetFileSize() const;
		CNxNTime GetFileTime() const;

		bool IsCompressed() const;
		bool IsOnline() const;
		bool IsAccessible() const;

        CNxNString GetCreator() const;
        CNxNString GetComment() const;
        CNxNTime GetCreationTime() const;

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_HISTORYVERSION);
        }

	private:
};

#endif // INC_NXN_HISTORYVERSION_H
