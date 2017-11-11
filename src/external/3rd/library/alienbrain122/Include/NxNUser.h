// \addtodoc

#ifndef INC_NXN_USER_H
#define INC_NXN_USER_H

/*  \class      CNxNUser NxNUser.h
 *
 *  \brief      This class represents a single user.
 *
 *  \author     Dietmar Püttmann
 *
 *  \version    1.00
 *
 *  \date       2001
 *
 */

class NXNINTEGRATORSDK_API CNxNUser: public CNxNUserManagementItem
{
    NXN_DECLARE_DYNCREATE(CNxNUser);

    public:
        //---------------------------------------------------------------------------
        //  construction/destruction
        //---------------------------------------------------------------------------
        CNxNUser(CNxNNode* pParent = NULL);
        virtual ~CNxNUser();

        //---------------------------------------------------------------------------
        //  static object type information
        //---------------------------------------------------------------------------
        static CNxNType NXNINTEGRATORSDK_API_CALL GetType()
        {
            return CNxNType(NXN_TYPE_USER);
        }

    private:
};

#endif // INC_NXN_USER_H
