// \addtodoc

#ifndef INC_NXN_USERMANAGEMENTITEM_H
#define INC_NXN_USERMANAGEMENTITEM_H

/*  \class      CNxNUserManagementItem NxNUserManagementItem.h
 *
 *  \brief      This is the base class for all item types related to the
 *              user management system.
 *
 *  \author     Dietmar Püttmann
 *
 *  \version    1.00
 *
 *  \date       2001
 *
 */

class NXNINTEGRATORSDK_API CNxNUserManagementItem : public CNxNDbNode
{
    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNUserManagementItem(CNxNNode* pParent = NULL);
        virtual ~CNxNUserManagementItem();

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_USERMANAGEMENTITEM);
        }

    private:
};

#endif // INC_NXN_USERMANAGEMENTITEM_H
