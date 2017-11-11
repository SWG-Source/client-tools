// \addtodoc

#ifndef INC_NXN_MENU_H
#define INC_NXN_MENU_H

/*  \class      CNxNMenu NxNMenu.h
 *
 *  \brief      This class provides access to object specific menus (context menus).
 *              Please keep in mind that this class is only meant for data retrieval
 *              and not for modification access. You can not modify the menu structure via this class.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
 *  \mod
 *      [ap]-17-Jul-2000 file created.
 *  \endmod
 */

//---------------------------------------------------------------------------
//  hidden structure for data storage
//---------------------------------------------------------------------------
typedef struct _tNxNMenuData*   tNxNMenuData;

class NXNINTEGRATORSDK_API CNxNMenu : public CNxNObject
{
    NXN_DECLARE_DYNCREATE(CNxNMenu);

    friend CNxNMenu;

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNMenu(CNxNMenu* pParent);
        CNxNMenu(CNxNNode* pAttachedToNode = NULL);
        virtual ~CNxNMenu();

        //---------------------------------------------------------------------------
        //  menu hierarchy information access
        //---------------------------------------------------------------------------
        CNxNMenu* GetParent() const;

        inline bool HasParent() const
        {
            return (GetParent() != NULL);
        }

        CNxNMenu* GetFirst();
        CNxNMenu* GetNext();

        //---------------------------------------------------------------------------
        //  property access
        //---------------------------------------------------------------------------
        bool SetProperty(const CNxNProperty& propProperty);
        bool GetProperty(CNxNProperty& propProperty);

        //---------------------------------------------------------------------------
        //  object information access
        //---------------------------------------------------------------------------
        CNxNString GetParameter();
        long GetFlags() const;
        bool GetInfo(long& lMenuItemFlags, CNxNString& strOpCode, CNxNString& strDisplayString, CNxNString& strHelpString) const;
        bool IsGrayed() const;
        bool IsDefault() const;

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_MENU);
        }

        //---------------------------------------------------------------------------
        //  direct access to menu object data
        //---------------------------------------------------------------------------
        inline tNxNMenuData GetMenuData() const
        {
            return m_pMenuData;
        }

    private:
        bool AddInternalChild(CNxNMenu* pChild);
        bool RemoveInternalChild(CNxNMenu* pChild);
        void DeleteInternalChilds();

        long GetInternalChildCount() const;
        bool HasInternalChilds() const;

        CNxNMenu* GetInternalChild(int i) const;

    private:
        tNxNMenuData    m_pMenuData;
};

#endif // INC_NXN_MENU_H
