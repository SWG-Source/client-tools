// \addtodoc

#ifndef INC_NXN_BROWSEFILTER_H
#define INC_NXN_BROWSEFILTER_H

/*	\class		CNxNBrowseFilter
 *	
 *	\file		NxNBrowseFilter.h
 *	
 *	\brief		This is a class which encapsulates the quick generation of browse filter.
 *				Browse filters are used in conjunction with BrowseForObject().
 *	
 *	\author		Axel Pfeuffer, Helmut Klinger
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *		[ap]-05-26-2000 file created.
 *	\endmod
 */

//---------------------------------------------------------------------------
//	hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNBrowseFilterData*	tNxNBrowseFilterData;

class NXNINTEGRATORSDK_API CNxNBrowseFilter : public CNxNObject {
	public:
		//---------------------------------------------------------------------------
		// enumerated filter commands
		//---------------------------------------------------------------------------
		enum eFilterCommand {
			FC_NONE = 0,
			FC_HIDE,
			FC_SHOW,
			FC_GLOBALSEL,
			FC_LOCALSEL,
			FC_TRACKSEL,
			FC_TRACKSELROOT,
			FC_SETROOT,
			FC_NOEXPAND,
			FC_NODRAG,
			FC_NODROP,
			FC_NOCONTEXTMENU,
			FC_NOPROPERTYPAGE,
			FC_NOLABELEDIT,
			FC_ACCEPT,
			FC_EXTERNALDBLCLICK,
			FC_DISABLECOMMAND,
			FC_ENABLECOMMAND
		};

		//---------------------------------------------------------------------------
		// construction/destruction
		//---------------------------------------------------------------------------
		CNxNBrowseFilter();
		CNxNBrowseFilter(const CNxNBrowseFilter& bfFilterToCopy);
		virtual ~CNxNBrowseFilter();

		//---------------------------------------------------------------------------
		// operators
		//---------------------------------------------------------------------------
		const CNxNBrowseFilter& operator =(const CNxNBrowseFilter& bfFilterToCopy);

		//---------------------------------------------------------------------------
		// filter commands
		//---------------------------------------------------------------------------
		void Reset();

		/*!	\fn			inline bool CNxNBrowseFilter::Hide(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Hides objects of specified type.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool Hide(const CNxNString& strPath = _STR("\\")) { return Command(FC_HIDE, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::Show(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Shows objects of specified type.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool Show(const CNxNString& strPath = _STR("\\")) { return Command(FC_SHOW, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::TrackGlobalSelection(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				User wants this control to track the global selection!
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool TrackGlobalSelection(const CNxNString& strPath = _STR("\\")) { return Command(FC_GLOBALSEL, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::TrackGlobalSelection(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				User wants this control to track its own local selection!
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool TrackLocalSelection(const CNxNString& strPath = _STR("\\")) { return Command(FC_LOCALSEL, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::TrackSelection(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				??? Currently unkown function ??? 
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool TrackSelection(const CNxNString& strPath = _STR("\\")) { return Command(FC_TRACKSEL, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::TrackSelectionRoot(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				User specifies a selection tracking formula.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool TrackSelectionRoot(const CNxNString& strPath = _STR("\\")) { return Command(FC_TRACKSELROOT, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::SetRoot(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Set the specified namespace path as the new root. All parent objects above
		 *				this node are unvisible afterwards.
		 *	
		 *	\param		strPath is a NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool SetRoot(const CNxNString& strPath = _STR("\\")) { return Command(FC_SETROOT, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::NoExpand(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Disables tree expansion for specified type of objects.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool NoExpand(const CNxNString& strPath = _STR("\\")) { return Command(FC_NOEXPAND, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::NoDrag(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Disables drag operations for specified type of objects.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool NoDrag(const CNxNString& strPath = _STR("\\")) { return Command(FC_NODRAG, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::NoDrop(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Disables drop operations for specified type of objects.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool NoDrop(const CNxNString& strPath = _STR("\\")) { return Command(FC_NODROP, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::NoContextMenu(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Disables context menu for specified type of objects.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool NoContextMenu(const CNxNString& strPath = _STR("\\")) { return Command(FC_NOCONTEXTMENU, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::NoPropertyPage(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Disables property page for specified type of objects.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool NoPropertyPage(const CNxNString& strPath = _STR("\\")) { return Command(FC_NOPROPERTYPAGE, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::NoLabelEdit(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Disables label editing for specified type of objects.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool NoLabelEdit(const CNxNString& strPath = _STR("\\")) { return Command(FC_NOLABELEDIT, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::Accept(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				User can only select objects of specified type.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool Accept(const CNxNString& strPath = _STR("\\")) { return Command(FC_ACCEPT, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::ExternalDblClick(const CNxNString& strPath = _STR("\\"))
		 *	
		 *				Routes double click event to external client of the namespace control, not to the namespace.
		 *	
		 *	\param		strPath is a NamespaceType or NamespacePath.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool ExternalDblClick(const CNxNString& strPath = _STR("\\")) { return Command(FC_EXTERNALDBLCLICK, strPath); }

		/*!	\fn			inline bool CNxNBrowseFilter::DisableCommand(const CNxNString& strCommand = _STR("\\"))
		 *	
		 *				Disables a specific command.
		 *	
		 *	\param		strCommand is the name of the command.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool DisableCommand(const CNxNString& strCommand = _STR("")) { return Command(FC_DISABLECOMMAND, strCommand); }

		/*!	\fn			inline bool CNxNBrowseFilter::EnableCommand(const CNxNString& strCommand = _STR("\\"))
		 *	
		 *				Enables a specific command.
		 *	
		 *	\param		strCommand is the name of the command.
		 *	
		 *	\return		boolean expression which indicates if the command could be successfully added
		 *				to the internal list.
		 *	
		 *	\note		
		 */
		inline bool EnableCommand(const CNxNString& strCommand = _STR("")) { return Command(FC_ENABLECOMMAND, strCommand); }

		bool Command(eFilterCommand eCommand, const CNxNString& strPath = _STR(""));

		//---------------------------------------------------------------------------
		// complete filter string access
		//---------------------------------------------------------------------------
		CNxNString GetFilterString() const;

		//---------------------------------------------------------------------------
		// filter list direct access
		//---------------------------------------------------------------------------
		bool AddFilter(eFilterCommand eCommand, const CNxNString& strPath);

		eFilterCommand GetFilterAt(long lIndex) const;
		CNxNString GetFilterPathAt(long lIndex) const;
		CNxNString GetFilterStringAt(long lIndex) const;

		bool RemoveFilterAt(long lIndex);
		void RemoveFilters();

		long GetFilterCount() const;
		bool HasFilters() const;

	private:
		CNxNString GetCommandString(eFilterCommand eCommand) const;

		void ResetAndCopyFrom(const CNxNBrowseFilter& bfFilterToCopy);

		tNxNBrowseFilterData	m_pBrowseFilterData;
};


#endif // INC_NXN_BROWSEFILTER_H
