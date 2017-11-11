// This file is NOT part of the external API
#ifndef _STATION_REQUEST_H
#define _STATION_REQUEST_H

#include "allsys.h"
#include "utime.h"
#include "uerror.h"
#include "utypes.h"
extern "C" {
#include "uthread.h"
#include "umutex.h"
}
#include "rdp_api.h"

// external prototypes in stationapi.h
#include "StationAPISession.h"  // Declares StationAPISession class
typedef uint32  StationAPIIP;
typedef uint32  StationAPITrack;
typedef uint32  StationAPIUID;
typedef int     StationAPIAccountStatus;
typedef int     StationAPIGameStatus;

class StationAPI;
class StationAPIList;
enum StationAPIRequest;
enum StationAPIProduct;
enum StationAPIResult;
// end prototypes

// if the submission does not succeed in this time, fail
#define SAPI_REQUEST_TIMEOUT_SUBMIT    60
// if the server does not respond in this time, fail
#define SAPI_REQUEST_TIMEOUT_PENDING   60

enum StationRequestStatus
{
    SR_STATUS_NONE,
    SR_STATUS_SUBMIT, 
    SR_STATUS_PENDING, 
    SR_STATUS_RESULT,
    SR_STATUS_URGENT_RESULT
};

enum StationCallType
{
    SAPI_CALL_NONE,
    SAPI_CALL_CREATE,
    SAPI_CALL_LOGIN,
    SAPI_CALL_SESSION,
    SAPI_CALL_PASSWORD,
    SAPI_CALL_GETDATA,
    SAPI_CALL_PUTDATA,
    SAPI_CALL_PURCHASE,
    SAPI_CALL_CONSUME,
    SAPI_CALL_HISTORY,
    SAPI_CALL_CANCEL_REQUEST,
    SAPI_CALL_END
};

class StationRequest
{
public:
    StationRequest(StationRequest * M);            // master node, or NULL if it is the master
    ~StationRequest();

    umutex_t uProcessLock;
    void StationRequest::Link(StationRequest * R, StationRequestStatus status);
    void StationRequest::Unlink(StationRequest * R);

    void StationRequest::CancelRequest(StationRequest * R); // request to cancel another request
    // return tracking number of request
    StationAPITrack SubmitRequest(StationRequest * R);      // add request to submission queue
    StationAPITrack PendRequest(StationRequest * R);        // add request to pending queue, remove from submit
    StationAPITrack ResultRequest(StationRequest * R);      // add request to result queue, result is known

    StationAPITrack nTrack;                          // tracking number of this request, or next request number for master node
    StationRequestStatus nStatus;
    StationRequest * pMaster;               // pointer back to master node
    time_t tRetry;                          // when to check request
    time_t tTimeout;                        // when this request times out

    // INPUT items
    uint32 nSipRequestNum;
    StationCallType nRequestType;
    StationAPIUID nUIDin;
    StationAPIRequest nRequest;
    StationAPIProduct nProduct;
    char * sName;
    char * sOldPass;
    char * sNewPass;
    int nQuantity;
    StationAPIList * sapiList;
    StationAPIIP nIP;
    StationAPISession CParentSession;
    connection_t * connection;
    void * pUserPointer;        // client side point for the user to use


    // OUTPUT items
    StationAPIResult sapiResult;
    char * sText;
    StationAPIUID nUID;
    StationAPIAccountStatus nAccountStatus;
    StationAPIGameStatus nGameStatus;
    StationAPISession CSessionid;

    // master request items only, not static so that multiple masters are possible
    int nSubmit;                            // number of requests submitted
    int nPending;                           // number of requests pending
    int nResults;                           // number of results waiting

    // requests waiting to be processed
    StationRequest * pSubmitNext;           // timeout at this end
    StationRequest * pSubmitPrev;           // add at this end

    // requests waiting for replys from server, or for slave, general link
    StationRequest * pNext;                 // timeout at this end
    StationRequest * pPrev;                 // add at this end

    // requests that are done and have results
    StationRequest * pResultNext;           // remove results at this end
    StationRequest * pResultPrev;           // add results at this end

};


#endif