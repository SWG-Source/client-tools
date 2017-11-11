// \addtodoc

#ifndef INC_NXN_WORKSPACE_H
#define INC_NXN_WORKSPACE_H

/* \class		CNxNWorkspace
 *
 *	\file		NxNWorkspace.h
 *
 *  \brief		CNxNWorkspace encapsulates a workspace node. Each opened or created workspace
 *				has got a local workspace file having an '.nwk' extension. Usually they contain
 *				project object and thus are used to create, insert, load, unload and remove projects.
 *				A workspace object implicitly needs a CNxNIntegraotr object as a parent.
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
typedef struct _tNxNWorkspaceData*	tNxNWorkspaceData;

class NXNINTEGRATORSDK_API CNxNWorkspace : public CNxNVirtualNode {
	NXN_DECLARE_DYNCREATE(CNxNWorkspace);

	public:
		//--------------------------------------------------------------------
		// construction / destruction
		//--------------------------------------------------------------------
		CNxNWorkspace(CNxNIntegrator* pNxNIntegrator = NULL);
		virtual ~CNxNWorkspace();

		//--------------------------------------------------------------------
		// project specific methods
		//--------------------------------------------------------------------
		CNxNProject* NewProject();

		CNxNProject* NewProjectEx(	const CNxNString& strProjectName, const CNxNString& strPath, 
									const CNxNString& strUserName, const CNxNString& strPassword, 
									const CNxNString& strHostName, const CNxNString& strMountPath,
									const CNxNString& strBuildPath, const CNxNString& strWorkingPath);

		CNxNProject* InsertProject(	const CNxNString& strFileName = _STR(""), const CNxNString& strUserName = _STR(""), 
									const CNxNString& strPassword = _STR(""), const	CNxNString& strHostName = _STR(""));

		CNxNProject* InsertProjectEx(	const CNxNString& strFileName, const CNxNString& strUserName, const CNxNString& strPassword, 
										const CNxNString& strHostName);

		CNxNProject* LoadProject(const CNxNString& strProjectName);

		CNxNProject* LoadProjectAs(const CNxNString& strProjectName);

		CNxNProject* LoadProjectEx(	const CNxNString& strProjectName, const CNxNString& strUserName, const CNxNString& strPassword, 
									const CNxNString& strHostName);

		bool UnloadProject(CNxNProject*& pProject);	


		bool RemoveProject(CNxNProject*& pProject, bool bShowDialog = true);

		CNxNProject* GetProject(const CNxNString& strProjectName);

		//--------------------------------------------------------------------
		// workspace information access methods
		//--------------------------------------------------------------------
		CNxNString GetFileName() const;

		//---------------------------------------------------------------------------
		//	project browsing
		//---------------------------------------------------------------------------
		/*!	\fn			inline CNxNProject* CNxNWorkspace::BrowseForProject()
		 *	
		 *				Brings up a browser window which allows browsing for projects below this
		 *				workspace. If you clicked OK after selecting a project, a pointer
		 *				to a CNxNProject object will be returned. A click to CANCEL will result
		 *				in a NULL pointer. 
		 *	
		 *	\param		none
		 *	
		 *	\return		A pointer to a CNxNProject object. If the return value indicates NULL
		 *				either the user clicked the CANCEL button in the browser window or
		 *				an error occured during browsing. Use GetLastError() to get more
		 *				details about the occured error.
		 *	
		 *	\note		
		 */
		inline CNxNProject* BrowseForProject()
		{
			// generate a browse filter
			CNxNBrowseFilter bfFilter;

			bfFilter.Hide(); // hide all
			bfFilter.Show(_STR("\\Workspace")); // show all workspace items
			bfFilter.Show(CNxNProject::GetType()); // show projects
			bfFilter.Hide(_STR("\\Workspace\\DbItem\\FileFolder\\System")); // hide all system items
			bfFilter.Hide(_STR("\\Workspace\\DbItem\\Property")); // hide all system items
			bfFilter.NoExpand(CNxNProject::GetType()); // project node are not allowed to expand
			bfFilter.NoDrag(); // no drag
			bfFilter.NoDrop(); // no drop
			bfFilter.NoLabelEdit(); // no label edit
			bfFilter.DisableCommand(); // disable all commands
			bfFilter.Accept(CNxNProject::GetType()); // user can only select projects
			bfFilter.SetRoot(GetNamespacePath()); // new root

			return (CNxNProject*)BrowseForObject(bfFilter, _STR("Select a project"));
		}

		//--------------------------------------------------------------------
		// data retrieval
		//--------------------------------------------------------------------
		bool IsOpened() const;

		//---------------------------------------------------------------------------
		//	static object type information
		//---------------------------------------------------------------------------
		static CNxNType NXNINTEGRATORSDK_API_CALL GetType() { return CNxNType(NXN_TYPE_WORKSPACE); };

		//--------------------------------------------------------------------
		// direct access to workspace data
		//--------------------------------------------------------------------
		inline tNxNWorkspaceData GetWorkspaceData() { return m_pWorkspaceData; };

	protected:
		//--------------------------------------------------------------------
		// overloaded methods
		//--------------------------------------------------------------------
		void SetParent(CNxNNode* pNewParent);


	private:
		tNxNWorkspaceData	m_pWorkspaceData;
};

#endif // INC_NXN_WORKSPACE_H
