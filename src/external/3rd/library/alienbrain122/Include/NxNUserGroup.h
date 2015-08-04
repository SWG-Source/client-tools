// \addtodoc

#ifndef INC_NXN_USERGROUP_H
#define INC_NXN_USERGROUP_H

/*  \class      CNxNUserGroup NxNUserGroup.h
 *
 *  \brief      This class represents a user group. It allows to iterate through all the users and
 *              user groups contained in this group.
 *
 *  \author     Dietmar Püttmann
 *
 *  \version    1.00
 *
 *  \date       2001
 *
 */

class NXNINTEGRATORSDK_API CNxNUserGroup: public CNxNUserManagementItem
{
    NXN_DECLARE_DYNCREATE(CNxNUserGroup);

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNUserGroup(CNxNNode* pParent = NULL);
        virtual ~CNxNUserGroup();

        CNxNUser* GetFirstUser(TNxNNodeIterator* pIterator);
        CNxNUser* GetNextUser(TNxNNodeIterator* pIterator);

        CNxNUserGroup* GetFirstGroup(TNxNNodeIterator* pIterator);
        CNxNUserGroup* GetNextGroup(TNxNNodeIterator* pIterator);

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_USERGROUP);
        }

    private:
};

#endif // INC_NXN_USERGROUP_H
