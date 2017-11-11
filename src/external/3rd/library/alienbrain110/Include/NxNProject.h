// \addtodoc

#ifndef INC_NXN_PROJECT_H
#define INC_NXN_PROJECT_H

/* \class		CNxNProject
 *
 *	\file		NxNProject.h
 *
 *  \brief		The class CNxNProject encapsulates projects that rely on servers in databases.
 *				Usually projects contain folders and files which can be created, checked in/out,
 *				removed, etc. It is possible to share the same projects over a large scale of persons
 *				gaining them access to the same data.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
 *	\mod
 *		[ap]-05-02-2000 file created.
 *		[ap]-06-13-2000 GetFile() implemented.
 *		[ap]-06-13-2000 GetFolder() implemented.
 *		[ap]-06-13-2000 CreateFile() implemented.
 *		[ap]-06-13-2000 CreateFolder() implemented.
 *		[ap]-09-08-2000 BUGFIX: CreateFolder() now works.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNProjectData*	tNxNProjectData;

class NXNINTEGRATORSDK_API CNxNProject : public CNxNDbNode {
	NXN_DECLARE_DYNCREATE(CNxNProject);

	public:
		//---------------------------------------------------------------------------
		//	construction/destruction
		//---------------------------------------------------------------------------
		CNxNProject(CNxNWorkspace* pParent = NULL);
		virtual ~CNxNProject();

		//---------------------------------------------------------------------------
		//	project information retrieval
		//---------------------------------------------------------------------------
		bool SetUserName(const CNxNString& strUserName);
		bool SetPassword(const CNxNString& strPassword);
		bool SetHostName(const CNxNString& strHostName);

		CNxNString GetUserName() const;
		CNxNString GetHostName() const;
		bool IsConnected() const;

		bool Connect(	const CNxNString& strUserName = _STR(""), 
						const CNxNString& strPassword = _STR(""), 
						const CNxNString& strHostName = _STR(""));

		bool Disconnect();

		//---------------------------------------------------------------------------
		//	folder & file browsing
		//---------------------------------------------------------------------------
		/*!	\fn			inline CNxNFolder* CNxNProject::BrowseForFolder()
		 *	
		 *				Brings up a browser window which allows browsing for folders below this
		 *				project (root). If you clicked OK after selecting a subfolder, a pointer
		 *				to a CNxNFolder object will be returned. A click to CANCEL will result
		 *				in a NULL pointer. 
		 *	
		 *	\param		none
		 *	
		 *	\return		A pointer to a CNxNFolder object. If the return value indicates NULL
		 *				either the user clicked the CANCEL button in the brwoser window or
		 *				an error occured during browsing. Use GetLastError() to get more
		 *				details about the occured error.
		 *	
		 *	\note		
		 */
		inline CNxNFolder* BrowseForFolder()
		{	
			// generate a browse filter
			CNxNBrowseFilter bfFilter;

			bfFilter.Hide(); // hide all
			bfFilter.Show(_STR("\\Workspace")); // show all workspace items
			bfFilter.Show(CNxNFolder::GetType()); // show projects
			bfFilter.Hide(CNxNFile::GetType()); // hide files
			bfFilter.Hide(_STR("\\Workspace\\DbItem\\FileFolder\\System")); // hide all system items
			bfFilter.Hide(_STR("\\Workspace\\DbItem\\Property")); // hide all system items
			bfFilter.Hide(_STR("\\Workspace\\DiskItem")); // hide all disk items (local files)
			bfFilter.NoDrag(); // no drag
			bfFilter.NoDrop(); // no drop
			bfFilter.NoLabelEdit(); // no label edit
			bfFilter.DisableCommand(); // disable all commands
			bfFilter.Accept(CNxNFolder::GetType()); // user can only select projects
			bfFilter.SetRoot(GetNamespacePath()); // new root

			return (CNxNFolder*)BrowseForObject(bfFilter, _STR("Select a subfolder"));
		}

		/*!	\fn			inline CNxNFile* CNxNProject::BrowseForFile()
		 *	
		 *				Brings up a browser window which allows browsing for files below this
		 *				project (root). If you clicked OK after selecting a file below the root, 
		 *				a pointer to a CNxNFile object will be returned. A click to CANCEL will result
		 *				in a NULL pointer.
		 *	
		 *	\param		none
		 *	
		 *	\return		A pointer to a CNxNFile object. If the return value indicates NULL
		 *				either the user clicked on CANCEL in the browser window or
		 *				an error occured during browsing. Use GetLastError() to get more
		 *				details about occured errors.
		 *	
		 *	\note		
		 */
		inline CNxNFile* BrowseForFile()
		{	
			// generate a browse filter
			CNxNBrowseFilter bfFilter;

			bfFilter.Hide(); // hide all
			bfFilter.Show(_STR("\\Workspace")); // show all workspace items
			bfFilter.Show(CNxNFolder::GetType()); // show folders
			bfFilter.Hide(_STR("\\Workspace\\DbItem\\FileFolder\\System")); // hide all system items
			bfFilter.Hide(_STR("\\Workspace\\DbItem\\Property")); // hide all system items
			bfFilter.Hide(_STR("\\Workspace\\DiskItem")); // hide all disk items (local files)
			bfFilter.NoExpand(CNxNFile::GetType()); // files are not allowed to expand
			bfFilter.NoDrag(); // no drag
			bfFilter.NoDrop(); // no drop
			bfFilter.NoLabelEdit(); // no label edit
			bfFilter.DisableCommand(); // disable all commands
			bfFilter.Accept(CNxNFile::GetType()); // user can only select projects
			bfFilter.SetRoot(GetNamespacePath()); // new root

			return (CNxNFile*)BrowseForObject(bfFilter, _STR("Select a file"));
		}

		//---------------------------------------------------------------------------
		//	folder & file access
		//---------------------------------------------------------------------------
		/*!	\fn			inline CNxNFolder* CNxNProject::GetFolder(const CNxNString& strName)
		 *	
		 *				Returns a CNxNFolder pointer to the specified child object.
		 *				The specified name is used to find the concerning object directly below
		 *				this object. 
		 *	
		 *	\param		strName is a reference to the object's name to retrieve.
		 *	
		 *	\return		A pointer to a CNxNFolder object. If the return value indicates NULL
		 *				that specified CNxNFolder object is not one of this objects childs or
		 *				not of folder type. Use GetLastError() to get more details about the
		 *				error.
		 *	
		 *	\note		
		 */
		inline CNxNFolder* GetFolder(const CNxNString& strName)
		{
			SetLastError(S_OK);

			CNxNNode* pFolder = GetNode(strName); // get folder node

			// folder object found?
			if (!pFolder) {
				// => not found => return NULL;
				SetLastError(NXN_ERR_PROJECT_SUBFOLDER_NOT_FOUND);

				return NULL;
			}
	
			// node is not of type folder?
			if (!pFolder->IsOfType(CNxNFolder::GetType())) {
				// => not of type folder => return NULL;
				SetLastError(NXN_ERR_PROJECT_NO_FOLDER);

				return NULL;
			}
			
			return (CNxNFolder*)pFolder;
		}

		/*!	\fn			inline CNxNFile* CNxNProject::GetFile(const CNxNString& strName)
		 *	
		 *				Returns a CNxNFile pointer to the specified child object.
		 *				The specified name is used to find the concerning object directly below
		 *				this object. 
		 *	
		 *	\param		strName is a reference to the object's name to retrieve.
		 *	
		 *	\return		A pointer to a CNxNFile object. If the return value indicates NULL
		 *				that specified CNxNFile object is either not one of this objects childs or
		 *				not of file type. Use GetLastError() to get more details about the
		 *				error.
		 *	
		 *	\note		
		 */
		inline CNxNFile* GetFile(const CNxNString& strName)
		{
			SetLastError(S_OK);

			CNxNNode* pFile = GetNode(strName); // get file node

			// file object found?
			if (!pFile) {
				// => not found => return NULL;
				SetLastError(NXN_ERR_PROJECT_FILE_NOT_FOUND);

				return NULL;
			}
	
			// node is not of type file?
			if (!pFile->IsOfType(CNxNFile::GetType())) {
				// => not of type file => return NULL;
				SetLastError(NXN_ERR_PROJECT_NO_FILE);

				return NULL;
			}
			
			return (CNxNFile*)pFile;
		}


		CNxNFolder* CreateFolder(	const CNxNString& strFolderName, 
									bool bShowDialog = true,
									const CNxNString& strComment = _STR(""),
									const CNxNString& strKeywords = _STR(""),
									bool bFailIfExists = true);

		/*!	\fn			inline CNxNFile* CNxNProject::CreateFile(	const CNxNString& strFileName,
		  															const CNxNString& strComment = _STR(""),
		  															const CNxNString& strKeywords = _STR(""),
		  															const CNxNString& strLocalPath = _STR(""))
		 *	
		 *				Returns a CNxNFolder pointer to the specified child object.
		 *				The specified name is used to find the concerning object directly below
		 *				this object. 
		 *	
		 *	\param		strFileName is a reference to the file's name to create.
		 *	\param		strComment is a reference to a creation comment.
		 *	\param		strKeywords is a reference to creation keywords.
		 *	\param		strLocalPath is a reference to the local path of the file to create.
		 *	
		 *	\return		A pointer to a CNxNFile object. If the return value indicates NULL
		 *				that specified CNxNFile object could not be created. 
		 *				Use GetLastError() to get more details about the
		 *				error.
		 *	
		 *	\note		
		 */
		inline CNxNFile* CreateFile(	const CNxNString& strFileName, 
										const CNxNString& strComment = _STR(""),
										const CNxNString& strKeywords = _STR(""),
										const CNxNString& strLocalPath = _STR(""))
		{
			return (CNxNFile*)CreateNode(strFileName, CNxNFile::GetType(), true, strComment, strKeywords,
											strLocalPath, false, false);
		}

		//---------------------------------------------------------------------------
		//	file & folder retrieval using the concerning project specific handle
		//---------------------------------------------------------------------------
		CNxNFile* GetFileFromHandle(long lHandle);
		CNxNFolder* GetFolderFromHandle(long lHandle);

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_PROJECT); };

		//---------------------------------------------------------------------------
		//	direct access to node object data
		//---------------------------------------------------------------------------
		inline tNxNProjectData GetProjectData() const { return m_pProjectData; };

	protected:
//		void Reset();

	private:
		tNxNProjectData	m_pProjectData;
};


#endif // INC_NXN_PROJECT_H
