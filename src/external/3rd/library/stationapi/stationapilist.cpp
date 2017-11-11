#include <string.h>
#include "stationapi.h"

StationAPIList::StationAPIList()
{
}

StationAPIList::~StationAPIList()
{
}


// list creation, using initialized input structures
void StationAPIList::BuildRegistrationList(StationAPIRegistration * content, StationAPIRegistrationFields * fieldsused)
{
    InitList();
    // TODO
}
 // NULL means all fields
void StationAPIList::BuildGetRegistrationList(StationAPIRegistrationFields * fieldsused)
{
    InitList();
    // TODO
}
 // NULL means all fields
void StationAPIList::BuildCreditList(StationAPICredit * content)
{
    InitList();
    // TODO
}

/* add more cases as needed */

// list extraction, setting output structures
void StationAPIList::ExtractRegistrationList(StationAPIRegistration * content, StationAPIRegistrationFields * fieldsused)
{
    memset(content,0,sizeof(StationAPIRegistration));
    memset(fieldsused,0,sizeof(StationAPIRegistrationFields));
    // TODO!
}

void StationAPIList::ExtractHistoryList(StationAPIHistory * content)
{
    memset(content,0,sizeof(StationAPIHistory));
    // TODO!
}

/* add more cases as needed */

// manual methods for special cases -- do not use special cases for mainline code!
// call before first PutField to initialize list
void StationAPIList::InitList(void)
{
    nByteCount = 0;
}

// IN: name of database field desired
void StationAPIList::BuildGetDataField(char * fieldname)
{
    // TODO!
}

// IN: name of database field to set, IN: content of field
void StationAPIList::BuildPutDataField(char * fieldname, char * content)
{
    // TODO!
}

// IN: name of database field, OUT: pointer to string returned, or NULL if not in data set
char * StationAPIList::GetField(char * fieldname)
{
    // TODO!
    return NULL;
}
