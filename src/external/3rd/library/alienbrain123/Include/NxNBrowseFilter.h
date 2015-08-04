// \addtodoc

#ifndef INC_NXN_BROWSEFILTER_H
#define INC_NXN_BROWSEFILTER_H

/*  \class      CNxNBrowseFilter
 *  
 *  \file       NxNBrowseFilter.h
 *  
 *  \brief      This is a class that is used for the generation of browse filters.
 *  
 *  \author     Axel Pfeuffer, Helmut Klinger
 *  
 *  \version    1.00
 *  
 *  \date       2000
 *  
 *  \mod
 *      [ap]-26-May-2000 file created.
 *      [ap]-20-Apr-2001 New filter command FC_SETBGCOLOR added, concerning function added and implemented.
 *  \endmod
 */


class NXNINTEGRATORSDK_API CNxNBrowseFilter : public CNxNObject
{
    public:
        //---------------------------------------------------------------------------
        // enumerated filter commands
        //---------------------------------------------------------------------------
        enum eFilterCommand
        {
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
            FC_ENABLECOMMAND,
            FC_SETBGCOLOR
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

        /*! \fn         inline bool CNxNBrowseFilter::Hide(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Hides objects of specified type.
         *  
         *  \param      strPath   string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the hide command was added successfully
         *  
         *  \note       
         */
        inline bool Hide(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_HIDE, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::Show(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Shows objects of specified type.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the show command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool Show(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_SHOW, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::TrackGlobalSelection(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Instructs the control to track the global selection.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool TrackGlobalSelection(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_GLOBALSEL, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::TrackLocalSelection(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Instructs the control to track only its own local selection! This turns
         *              off the TrackGlobalSelection functionality.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool TrackLocalSelection(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_LOCALSEL, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::TrackSelection(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Use this filter command in conjunction with the ActiveX Namespace List control.
         *              You can specifiy a special Namespace Type for objects that can be shown
         *              in the list view.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list.
         *  
         *  \note       
         */
        inline bool TrackSelection(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_TRACKSEL, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::TrackSelectionRoot(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Instructs the control the use the given selection tracking formula.
         *              This formula can be generated from the global selection path.
         *              Such a formula can look like this:<br>
         *              "\Workspace\%2\System\"<br>
         *              If the global selection has a selected file like 
         *              "\Workspace\MyProject\Folder\File.txt," the second part of that namespace 
         *              path ("MyProject") will be substituted into the formula so that the result 
         *              is "\Workspace\MyProject\System\". The general usage of the percent 
         *              symbol (%) is "%<namespace part number>," where 0 means the complete 
         *              namespace path of the selection. This filter command can be used when 
         *              working with the ActiveX Namespace Tree control.  Please keep in mind 
         *              that this filter expression does not affect on the ActiveX Namespace List 
         *              control.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool TrackSelectionRoot(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_TRACKSELROOT, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::SetRoot(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Set the specified namespace path as the new root.  
         *              All parent objects above this node are not shown afterwards.
         *  
         *  \param      strPath     string containg the namespace path being the new root
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool SetRoot(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_SETROOT, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::NoExpand(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Disables tree expansion for the specified type of objects.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool NoExpand(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_NOEXPAND, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::NoDrag(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Disables drag operations for the specified type of objects.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool NoDrag(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_NODRAG, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::NoDrop(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Disables drop operations for the specified type of objects.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool NoDrop(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_NODROP, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::NoContextMenu(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Disables context menu for the specified type of objects.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command was successfully added to the internal list
         *  
         *  \note       
         */
        inline bool NoContextMenu(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_NOCONTEXTMENU, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::NoPropertyPage(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Disables property page for the specified type of objects
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool NoPropertyPage(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_NOPROPERTYPAGE, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::NoLabelEdit(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Disables label editing for the specified type of objects.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool NoLabelEdit(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_NOLABELEDIT, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::Accept(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Instructs the control to allow only objects of the specified type for selection.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command could be successfully added to the internal list
         *  
         *  \note       
         */
        inline bool Accept(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_ACCEPT, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::ExternalDblClick(const CNxNString& strPath = _STR(L"\\"))
         *  
         *              Prevents the ActiveX namespace control from processing the double click.  
         *              An external client can now take over the processing of the double click.  
         *              The ExternalDblClick command should be used in conjunction with the 
         *              NxNNamespaceTree ActiveX control, otherwise the double click will be 
         *              disabled.
         *  
         *  \param      strPath     string containg a namespace type or namespace path
         *  
         *  \return     boolean value indicating if the command was successfully added to the internal list
         *  
         *  \note       
         */
        inline bool ExternalDblClick(const CNxNString& strPath = _STR(L"\\"))
        {
            return Command(FC_EXTERNALDBLCLICK, strPath);
        }

        /*! \fn         inline bool CNxNBrowseFilter::DisableCommand(const CNxNString& strCommand = _STR(L""))
         *  
         *              Disables a specific command (mainly context menu commands).
         *  
         *  \param      strCommand  string containing the name of the command
         *  
         *  \return     boolean value indicating if the disable command was successfully added to the internal list
         *  
         *  \note       
         */
        inline bool DisableCommand(const CNxNString& strCommand = _STR(L""))
        {
            return Command(FC_DISABLECOMMAND, strCommand);
        }

        /*! \fn         inline bool CNxNBrowseFilter::EnableCommand(const CNxNString& strCommand = _STR(L""))
         *  
         *              Enables a specific command (mainly context menu commands).
         *  
         *  \param      strCommand  string containing the name of the command
         *  
         *  \return     boolean value indicating if the command could be successfully added
         *              to the internal list
         *  
         *  \note       
         */
        inline bool EnableCommand(const CNxNString& strCommand = _STR(L""))
        {
            return Command(FC_ENABLECOMMAND, strCommand);
        }

        bool SetBackgroundColor(int nRed, int nGreen, int nBlue);

        bool Command(eFilterCommand eCommand, const CNxNString& strPath = _STR(L""));

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


    private:
        typedef struct
        {
            int         eCommand;
            CNxNString  strPath;
        } sFilterEntry;

        typedef class NXNINTEGRATORSDK_API CNxNArray<sFilterEntry> tFilterEntryList;

        tFilterEntryList    m_aFilterList;
};


#endif // INC_NXN_BROWSEFILTER_H
