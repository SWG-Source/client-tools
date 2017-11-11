#include <stdio.h>
#include <malloc.h>
#include "stationapi.h"

// NULL if master, else pointer back to master
StationRequest::StationRequest(StationRequest * M)
{
// use global so that tracking numbers dont reset to 0 on a soft restart
// use global so multiple copies of api will not both have requests with same tracking number
static StationAPITrack nMasterTracking = 0;

    pMaster = M;
    tRetry = time(NULL);
    tTimeout = time(NULL) + SAPI_REQUEST_TIMEOUT_SUBMIT;

    nSubmit = 0;
    nPending = 0;
    nResults = 0;
    nStatus = SR_STATUS_NONE;

    nRequestType = SAPI_CALL_NONE;
    nUIDin = 0;
    nRequest = SAPI_REQUEST_LOGIN_EXCLUSIVE;
    nProduct = SAPI_PRODUCT_DEMO;
    sName = NULL;
    sOldPass = NULL;
    sNewPass = NULL;
    nQuantity = 0;
    sapiList = NULL;
    nIP = 0;
    CParentSession = "";

    connection = NULL;
    pUserPointer = NULL;

    sapiResult = SAPI_RESULT_SUCCESS;
    sText = NULL;
    nUID = 0;
    nAccountStatus = 0;
    nGameStatus = 0;
    CSessionid = "";

    if (pMaster)
    {   // slave node, normal case
        pSubmitNext = NULL;
        pSubmitPrev = NULL;
        pNext = NULL;
        pPrev = NULL;
        pResultNext = NULL;
        pResultPrev = NULL;
//        nTrack = ++pMaster->nTrack; // next tracking number
        nTrack = ++nMasterTracking; // next tracking number
    }
    else
    {   // master node
        pSubmitNext = this;
        pSubmitPrev = this;
        pNext = this;
        pPrev = this;
        pResultNext = this;
        pResultPrev = this;
        nTrack = 0;                 // Initial tracking number

        umutex_create( &uProcessLock );
    }

}

StationRequest::~StationRequest()
{
    if (pMaster)
    {   // slave node, normal case
        // remove ourselves from queues before deletion
        Unlink(this);
        if (sName)
            free(sName);
        if (sOldPass)
            free(sOldPass);
        if (sNewPass)
            free(sNewPass);
        if (sapiList)
            delete sapiList;
        if (sText)
            free(sText);
    }
    else
    {   // master node -- clean up any undeleted nodes!
        while (pSubmitNext != this)
            delete pSubmitNext;
        while (pNext != this)
            delete pNext;
        while (pResultNext != this)
            delete pResultNext;
        umutex_destroy( &uProcessLock );
    }
}

// remove node from whichever queue its in, if any
void StationRequest::Unlink(StationRequest * R)
{
    switch (R->nStatus)
    {
    case SR_STATUS_NONE:
        break;
    case SR_STATUS_SUBMIT:
        umutex_lock( &R->pMaster->uProcessLock );
        --(R->pMaster->nSubmit);
        R->nStatus = SR_STATUS_NONE;
        R->pSubmitNext->pSubmitPrev = R->pSubmitPrev;
        R->pSubmitPrev->pSubmitNext = R->pSubmitNext;
        R->pSubmitNext = NULL;
        R->pSubmitPrev = NULL;
        umutex_unlock( &R->pMaster->uProcessLock );
        break;
    case SR_STATUS_PENDING:
        umutex_lock( &R->pMaster->uProcessLock );
        --(R->pMaster->nPending);
        R->nStatus = SR_STATUS_NONE;
        R->pNext->pPrev = R->pPrev;
        R->pPrev->pNext = R->pNext;
        R->pNext = NULL;
        R->pPrev = NULL;
        umutex_unlock( &R->pMaster->uProcessLock );
        break;
    case SR_STATUS_RESULT:
    case SR_STATUS_URGENT_RESULT:
        umutex_lock( &R->pMaster->uProcessLock );
        --(R->pMaster->nResults);
        R->nStatus = SR_STATUS_NONE;
        R->pResultNext->pResultPrev = R->pResultPrev;
        R->pResultPrev->pResultNext = R->pResultNext;
        R->pResultNext = NULL;
        R->pResultPrev = NULL;
        umutex_unlock( &R->pMaster->uProcessLock );
        break;
    default:
        printf("%d: Unlink bad status %d\n", R->nTrack, R->nStatus);
        break;
    }
}

// insert linked node on specified queue
// typical sequence of events is:
//    Link(R, SR_STATUS_SUBMIT);        // add to queue to be submitted
//    Link(R,SR_STATUS_PENDING);        // move from submit queue to pend queue
//    Link(R,SR_STATUS_RESULT);         // move from pend queue to result queue
void StationRequest::Link(StationRequest * R, StationRequestStatus status)
{
    // remove node from previous queue
    Unlink(R);

    R->nStatus = status;

    umutex_lock( &R->pMaster->uProcessLock );
    StationRequest * X;
    switch (R->nStatus)
    {
    case SR_STATUS_SUBMIT:
        X = R->pMaster->pSubmitPrev;
        R->pSubmitNext = X;            
        R->pSubmitPrev = X->pSubmitPrev;
        R->pSubmitPrev->pSubmitNext = R;
        X->pSubmitPrev = R;
        ++(R->pMaster->nSubmit);
        break;
    case SR_STATUS_PENDING:
        X = R->pMaster->pPrev;
        R->pNext = X;            
        R->pPrev = X->pPrev;
        R->pPrev->pNext = R;
        X->pPrev = R;
        ++(R->pMaster->nPending);
        break;
    case SR_STATUS_RESULT:
        X = R->pMaster->pResultPrev;
        R->pResultNext = X;            
        R->pResultPrev = X->pResultPrev;
        R->pResultPrev->pResultNext = R;
        X->pResultPrev = R;
        ++(R->pMaster->nResults);
        break;
    case SR_STATUS_URGENT_RESULT:  // this is different, insert at Front of queue so it's processed next!
        X = R->pMaster->pResultNext;
        R->pResultPrev = X;            
        R->pResultNext = X->pResultNext;
        R->pResultNext->pResultPrev = R;
        X->pResultNext = R;
        ++(R->pMaster->nResults);
        break;
    default:    // case shouldnt happen
        printf("%d: Link bad status %d\n", R->nTrack, R-nStatus);
        break;
    }
    umutex_unlock( &R->pMaster->uProcessLock );
}

// these functions are for master level use only!
void StationRequest::CancelRequest(StationRequest * R)
{
    Link(R,SR_STATUS_URGENT_RESULT);
}

uint32 StationRequest::SubmitRequest(StationRequest * R)
{
    if (R->sapiResult != SAPI_RESULT_SUCCESS)
    {   // it's already failed for some reason, just deliver the result
        Link(R,SR_STATUS_RESULT);
        return R->nTrack;
    }
    else
    {
        Link(R, SR_STATUS_SUBMIT);
        return R->nTrack;
    }
}

uint32 StationRequest::PendRequest(StationRequest * R)
{
    Link(R,SR_STATUS_PENDING);
    return R->nTrack;
}

uint32 StationRequest::ResultRequest(StationRequest * R)
{
    Link(R,SR_STATUS_RESULT);
    return R->nTrack;
}


