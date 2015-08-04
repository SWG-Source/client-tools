// See Documentation in stationapidemo.cpp

#include "stationapi.h"
#include "stationmsg.h"
#include "PackClass.h"
#include "crypt.h"

#include "rdp_api.h"
#include "extend_rdp.h"

void ProcessThread( void * );

//--------------------------
void RDPServerDisconnect(StationAPI * api)
//
// Log out of server and close connection
//--------------------------
{
	if (api->g_c != NULL)
	{
	    if (api->g_logged_into_server)
        {
            PackClass Pack(0);
            Pack.pack_begin(CLIENT_DISCONNECT);
            Pack.send(api->g_c);
        	//connection_send(api->g_c, (char*)&pack_data, pack_offset, 1, RDP_SEND_GUARANTEED );
        }
		connection_close(api->g_c, 10000, NULL, NULL );
	}

	api->g_c = NULL;
	api->g_logged_into_server = false;
}

//-------------------------------
int RDPServerConnect(char *hostname,int port,StationAPI * api)
//
// Connect to server by sending and receiving a dummy message
// return 0 = none, 1 = comms error, 2 = need to patch
//-------------------------------
{
	int result;

    if (!api->g_rdp)
        return 1;

    //----- CREATE CONNECTION -----
    result = rdp_connect(api->g_rdp, &api->g_c, hostname, port, RDP_CONNECTION_KEEPALIVE );
    if ( result != 0 )
    {
        //printf("rdp_connect failed %d", result);
        return 1;
    }

    // ----- SEND CONNECT MESSAGE -----
    PackClass Pack(100);
    Pack.pack_begin(CLIENT_CONNECT);
    Pack.pack_string(SAPI_VERSION);  // tell server client version, let it decide compatibility
    Pack.send(api->g_c);
//	result = connection_send(api->g_c, (char*)&pack_data, pack_offset, 1, RDP_SEND_GUARANTEED );
    //printf("Send CONNECT %d\n", result);

    //----- WAIT FOR REPLY FROM SERVER -----
    msg_arrival_t *msg=NULL;
    time_t now = time(NULL);
    while (time(NULL) < (now + 30))						// 30 seconds
    {
   	    msg = rdp_receive(api->g_rdp,10);
        if (msg != NULL)
	        break;											// got a msg
    }
    if (msg == NULL)
    {
        RDPServerDisconnect(api);
        return 1;
    }

    //	----- got a message -----
    connection_t *c = msg_arrival_get_sender(msg);
    if (connection_connected(c) == FALSE)
    {
        fast_free(msg);
        RDPServerDisconnect(api);
        return 1;
    }
    msg_t *packet = (msg_t*)msg_arrival_get_data(msg);
    if (packet == NULL)
    {
        fast_free(msg);
        RDPServerDisconnect(api);
        return 1;
    }
    packet->msg = htog16( packet->msg );		// message ID
    if (packet->msg != SERVER_VERSION)
    {
        fast_free(msg);
        RDPServerDisconnect(api);
        return 2;
    }
	api->g_logged_into_server = true;

    // ----- Connected ok! -----
    fast_free(msg);
    return 0;
}

//-------------------------------
// constructor
StationAPI::StationAPI()
//-------------------------------
{
    g_rdp = NULL;	    // rdp struct
    g_c = NULL;	        // connection to server
    g_logged_into_server = false;
    pEncryption = NULL;
    bInCallback = false;
    bConnected = false;
    bTimingOut = false;
    bProcessFlag = false;    // processed callbacks by default
    sDNS = NULL;
    nPort = -1;
    nProtocol = 0;          // rdp by default
    nServerID = 0;          // test id
    nLocationID = 0;        // default first location
    srList = NULL;
    bProcessStop = false;
    bProcessRunning = false;
}

//-------------------------------
// destructor
StationAPI::~StationAPI()
//-------------------------------
{
    if (bConnected)
        Disconnect();
}

//-------------------------------
void StationAPI::SetIdentity (StationAPIApplication applicationid, StationAPIInstance instanceid)
//-------------------------------
{
    nServerID = applicationid;
    nLocationID = instanceid;
}

//-------------------------------
void StationAPI::SetProtocol (StationAPIProtocol protocol)
//-------------------------------
{
    nProtocol = protocol;
}

//-------------------------------
// start and stop connection -- return 0 for success, else error value
int32 StationAPI::Connect (char * dns, int port)
//-------------------------------
{
    if (bConnected)
        return 0;

    if (sDNS)
        free(sDNS);
    sDNS = strdup(dns);
    nPort = port;

    // connect
    // INIT RDP
	ref_fast_malloc_init(10*1024);		// used by rdp

#ifdef ENCRYPT_ON
	uint32 result = rdp_create(&g_rdp,0,1,RDP_TRANSPORT_UDP | RDP_ENCRYPT | RDP_CRC);
#else
	uint32 result = rdp_create(&g_rdp,0,1,RDP_TRANSPORT_UDP | RDP_CRC);
#endif
	if (result != 0)
	{
		//fprintf(stderr, "rdp_create failed %x, Failed to start API.\n", result);
		ref_fast_malloc_destroy();
		return 1;
	}
    result = RDPServerConnect(sDNS, nPort, this);  // 0 = ok, 1 = comms, 2 = need to patch

    if (result != 0) 
    {
		//fprintf(stderr, "connect failed %x, Failed to start API.\n", result);
		rdp_destroy(g_rdp,1);
        g_rdp = NULL;
		ref_fast_malloc_destroy();
        return result;
    }

//	SetEcryptionKey( SAPI_ENCRYPT_KEY );		// if we never change keys, only has to be done once
    pEncryption = new CCrypt( SAPI_ENCRYPT_KEY );		// if we never change keys, only has to be done once

    if (!StartProcess())
    {
		fprintf(stderr, "Failed to start API process.\n");
        RDPServerDisconnect(this);
		rdp_destroy(g_rdp,1);
        g_rdp = NULL;
		ref_fast_malloc_destroy();
        return -1;
    }

    bConnected = true;

    return 0;   // 0 = success
}

//-------------------------------
// disconnect connection -- blocking for up to X milliseconds for clean disconnect, then terminate regardless
void StationAPI::Disconnect (int linger_time)
//-------------------------------
{
    if (bConnected)
    {
        // disconnect
        StopProcess();
        if (pEncryption)
        {
            delete pEncryption;
            pEncryption = NULL;
        }
        RDPServerDisconnect(this);
		rdp_destroy(g_rdp,1);
        g_rdp = NULL;
		ref_fast_malloc_destroy();
        bConnected = false;
    }
    if (sDNS)
    {
        free(sDNS);
        sDNS = NULL;
    }
}

//-------------------------------
bool StationAPI::StartProcess()
//-------------------------------
{
    if (bProcessRunning)
        return true;

    srList = new StationRequest(NULL);  // create master node
    uthread_init( &apiUthread );
	void * ptr = this;
    uint32 result = uthread_create( &apiUthread, ProcessThread, ptr );
    if ( result != URESULT_OK )
	{
		fprintf(stderr,"Process Thread failed to start! errno=%d\n",errno);
		return false;
	}
	return true;
}

//-------------------------------
void StationAPI::StopProcess()
//-------------------------------
{
    if (bProcessRunning)
    {
        bProcessStop = true;    // signal thread to stop
        // up to 5 seconds for thread to stop itself
        time_t tQuit = time(NULL) + 5;
        while (bProcessRunning && time(NULL) < tQuit)
            sleep_ms(200); 
        uthread_destroy( &apiUthread );
        bProcessStop = false;
        bProcessRunning = false;
        delete srList;  // remove master node
        srList = NULL;
    }
}

//-------------------------------
// true if connection is live
bool StationAPI::IsConnected (void) 
//-------------------------------
{ 
    if (!bConnected)
        return false;   // api is not connected
    if (!g_logged_into_server)
        return false;   // server is not connected
    return true;
}

//-------------------------------
// number of submitted requests -- use to monitor request load
int StationAPI::GetRequestsQueued (void) 
//-------------------------------
{ 
    if (srList)
        return srList->nSubmit+srList->nPending+srList->nResults; 
    else
        return 0;
}

//-------------------------------
// number of requests waiting for processing -- use to monitor request load
int StationAPI::GetRequestsWaiting (void) 
//-------------------------------
{ 
    if (srList)
        return srList->nResults; 
    else
        return 0;
}

//-------------------------------
// Set to true to enable async callbacks, set to false to queue results
// can use interactively for critical sections, setting flag to false will block if a callback is in progress
void StationAPI::SetProcess (bool flag) 
//-------------------------------
{ 
    bProcessFlag = flag; 
    // block if necessary until no callback is in progress
    while (bInCallback && bProcessFlag == false)
    {
        sleep_ms(50);	// waiting, give up timeslice
    }
}

//-------------------------------
// Use for syncronous processing -- blocking for up to X milliseconds to process
void StationAPI::ProcessWait (int duration)
//-------------------------------
{
    // do nothing if callbacks are in async mode or nothing to do.
    if (srList && bProcessFlag == false && srList->nResults > 0)
    {   
        // process waiting requests until we get them all or the time has elapsed
        time_t tQuit = time(NULL) + duration;
        SetProcess(true);    // enable callbacks
        do  // at least one request
        {
			sleep_ms(50);	// waiting, give up timeslice
        } while ( (srList->nResults > 0) && (duration > time(NULL)));
        SetProcess(false);    // disable callbacks, block if call in progress
    }
}

//-------------------------------
void StationAPI::CheckArguments (StationRequest * R)
//-------------------------------
{
    switch(R->nRequestType)
    {
    case SAPI_CALL_CREATE:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    case SAPI_CALL_LOGIN:
        // TODO check input args
        break;
    case SAPI_CALL_SESSION:
        // TODO check input args
        break;
    case SAPI_CALL_PASSWORD:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    case SAPI_CALL_GETDATA:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    case SAPI_CALL_PUTDATA:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    case SAPI_CALL_PURCHASE:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    case SAPI_CALL_CONSUME:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    case SAPI_CALL_HISTORY:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    default:
        R->sapiResult = SAPI_RESULT_NOSUPPORT;
        break;
    }
}

//-------------------------------
void SetErrorText(StationRequest * R)
//-------------------------------
{
    switch(R->sapiResult)
    {
    case SAPI_RESULT_SUCCESS:           // operation suceeded
        R->sText = "Success!";
        break;
    case SAPI_RESULT_BUSY:              // operation timed out waiting for available thread
        R->sText = "Server is not responding.";
        break;
    case SAPI_RESULT_TIMEOUT:           // operation timed out waiting for a server response
        R->sText = "Operation timed out.";
        break;
    case SAPI_RESULT_COMMERROR:         // result was garbage
        R->sText = "Communications error.";
        break;
    case SAPI_RESULT_NOT_CONNECTED:     // not connected to server
        R->sText = "Not connected to server.";
        break;
    case SAPI_RESULT_BAD_ARGUMENT:      // illegal input, detected clientside
        R->sText = "Bad argument for this operation.";
        break;
    case SAPI_RESULT_NOSUPPORT:         // specified operation is not supported yet
        R->sText = "This operation is not currently supported.";
        break;
    case SAPI_RESULT_FAILED:            // operation failed at server -- general case
        R->sText = "Operation failed.";
        break;
    case SAPI_RESULT_DATABASE_ERROR:    // server database problem
        R->sText = "Database error.";
        break;
    case SAPI_RESULT_ILLEGAL_OPERATION: // you are not allowed to do this operation
        R->sText = "Illegal operation.";
        break;
    case SAPI_RESULT_BAD_INPUT:         // illegal or invalid input
        R->sText = "Bad input.";
        break;
    case SAPI_RESULT_NAME_NOT_FOUND:    // name not in database, or name/password mismatch
        R->sText = "Your name or password do not match.";
        break;
    case SAPI_RESULT_PASSWORD_FAILED:   // password mismatch 
        R->sText = "Your password does not match.";
        break;
    case SAPI_RESULT_UID_NOT_FOUND:     // station UID not found in database
        R->sText = "This account was not found in the database.";
        break;
    case SAPI_RESULT_ON_HOLD:           // station account on hold
        R->sText = "Your station account is on hold.";
        break;
    case SAPI_RESULT_CANCEL_USER:       // station account cancelled by user
        R->sText = "Your station account was cancelled at your request.";
        break;
    case SAPI_RESULT_CANCEL_STANDARDS:  // station account cancelled for standards violations
        R->sText = "Your station account was cancelled for a standards violation.";
        break;
    case SAPI_RESULT_CANCEL_OTHER:      // station account cancelled for other reason
        R->sText = "Your station account is cancelled.";
        break;
    case SAPI_RESULT_SESSION_NOT_FOUND: // server session number not active
        R->sText = "Failed -- Session not found.";
        break;
    case SAPI_RESULT_NAME_EXISTS:       // cannot create name that already exists
        R->sText = "Failed -- This name already exists.";
        break;
    default:
        R->sText = "??? Add message for this operation!";
        break;
    }
}

//-------------------------------
// Create new station identity with anywhere from minimum to complete registration and demographic data
StationAPITrack StationAPI::RequestCreate    (StationAPIRequest request, char * name, char * password, StationAPIList * reginfo, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_CREATE;
    R->nRequest = request;
    R->sName = strdup(name);
    R->sOldPass = strdup(password);
    R->sapiList = new StationAPIList;
    memcpy(R->sapiList, reginfo, sizeof(StationAPIList));
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
// Create session on server, name/password or uid/password
StationAPITrack StationAPI::RequestLogin     (StationAPIRequest request, char * name, char * password,
                                        StationAPIIP ip, StationAPIProduct product, StationAPISession * parentsession, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_LOGIN;
    R->nRequest = request;
    R->sName = strdup(name);
    R->sOldPass = strdup(password);
    R->nIP = ip;
    R->nProduct = product;
    if (parentsession)
        R->CParentSession = *parentsession;
    else
        R->CParentSession = "";
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
// Modify session on server (logout, touch, verify)
StationAPITrack StationAPI::RequestSession   (StationAPIRequest request, StationAPISession * sessionid, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_SESSION;
    R->nRequest = request;
    R->CSessionid = *sessionid;
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
StationAPITrack StationAPI::RequestPassword  (StationAPIUID uid, char * oldpassword, char* newpassword, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_PASSWORD;
    R->nUIDin = uid;
    R->sOldPass = strdup(oldpassword);
    R->sNewPass = strdup(newpassword);
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
StationAPITrack StationAPI::RequestGetData   (StationAPIUID uid, StationAPIList * list, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_GETDATA;
    R->nUIDin = uid;
    R->sapiList = list;
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
StationAPITrack StationAPI::RequestPutData   (StationAPIUID uid, StationAPIList * list, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_PUTDATA;
    R->nUIDin = uid;
    R->sapiList = new StationAPIList;
    memcpy(R->sapiList, list, sizeof(StationAPIList));
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
StationAPITrack StationAPI::RequestPurchase  (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product, StationAPIList * credit, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_PURCHASE;
    R->nRequest = request;
    R->nProduct = product;
    R->sapiList = new StationAPIList;
    memcpy(R->sapiList, credit, sizeof(StationAPIList));
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
StationAPITrack StationAPI::RequestConsume   (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product, int quantity, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_CONSUME;
    R->nRequest = request;
    R->nProduct = product;
    R->nQuantity = quantity;
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
StationAPITrack StationAPI::RequestHistory   (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product, void * user)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->pUserPointer = user;
    R->nRequestType = SAPI_CALL_HISTORY;
    R->nRequest = request;
    R->nProduct = product;
    CheckArguments(R);
    return srList->SubmitRequest(R);      // add request to submission queue, get tracking number
}

//-------------------------------
void StationAPI::CancelRequest(StationAPITrack track)
//-------------------------------
{
    StationRequest * R = new StationRequest(srList);
    R->nTrack = track;
    R->nRequestType = SAPI_CALL_CANCEL_REQUEST;
    srList->CancelRequest(R);
    return;
}

// Default implementations of virtual functions

//-------------------------------
bool StationAPI::OnResult   (StationAPITrack track, StationAPIResult result, char * text, void * user)
//-------------------------------
{
    return false;
}

// level 1 -- called if OnResult() returned false
//-------------------------------
void StationAPI::OnCreate   (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnLogin    (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, 
                             StationAPIAccountStatus account_status, StationAPIGameStatus game_status, StationAPISession sessionid, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnSession (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnPassword (StationAPITrack track, StationAPIResult result, char * text, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnGetData  (StationAPITrack track, StationAPIResult result, char * text, StationAPIList * list, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnPutData  (StationAPITrack track, StationAPIResult result, char * text, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnPurchase (StationAPITrack track, StationAPIResult result, char * text, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnConsume  (StationAPITrack track, StationAPIResult result, char * text, void * user)
//-------------------------------
{
}

//-------------------------------
void StationAPI::OnHistory  (StationAPITrack track, StationAPIResult result, char * text, StationAPIList * list, void * user)
//-------------------------------
{
}

//------------------------------------------
void SendRDPSession(StationRequest * R, StationAPI * api)
//------------------------------------------
{
    client_session session_details;

    // create request
    session_details.nTrack = R->nTrack;
    session_details.nRequest = R->nRequest;
    strcpy(session_details.sSession, R->CSessionid.GetString() );

    // convert from local format
    session_details.nTrack = (StationAPITrack)htog32(session_details.nTrack);
    session_details.nRequest = (StationAPIRequest)htog32(session_details.nRequest);

    // pack message
    PackClass Pack(sizeof(session_details));
    Pack.pack_begin(CLIENT_REQUEST_SESSION);
	Pack.pack_struct(&session_details, sizeof(session_details));
    // send it
    Pack.send(api->g_c);
//	connection_send(api->g_c, (char*)&pack_data, pack_offset, 1, RDP_SEND_GUARANTEED );
}

//------------------------------------------
void SendRDPLogin(StationRequest * R, StationAPI * api)
//------------------------------------------
{
    union {
    client_login login_details;
    encrypt_client_login in;
    };
    encrypt_client_login out;

    // create request
    login_details.nTrack = R->nTrack;
    login_details.nRequest = R->nRequest;
    strcpy(login_details.sStationName, R->sName );
    strcpy(login_details.sStationPassword, R->sOldPass );
    login_details.nIP = R->nIP;
    login_details.nProduct = R->nProduct;
    strcpy(login_details.sParentSession, R->CParentSession.GetString() );

    // convert from local format
    login_details.nTrack = (StationAPITrack)htog32(login_details.nTrack);
    login_details.nRequest = (StationAPIRequest)htog32(login_details.nRequest);
    login_details.nIP = (StationAPIIP)htog32(login_details.nIP);
    login_details.nProduct = (StationAPIProduct)htog32(login_details.nProduct);

    // pack message and encrypt
    PackClass Pack(sizeof(out));
    Pack.pack_begin(CLIENT_REQUEST_LOGIN);
    api->pEncryption->EncryptBuffer(&in, &out, sizeof(in));
	Pack.pack_struct(&out, sizeof(out));

    // send it
    Pack.send(api->g_c);
//	connection_send(api->g_c, (char*)&pack_data, pack_offset, 1, RDP_SEND_GUARANTEED );
}

//------------------------------------------
void HandleRDPSessionContent(StationRequest * R, StationAPI * api, msg_t * packet)
//------------------------------------------
{
    server_session session;

    PackClass UnPack(0);
	UnPack.unpack_begin((void *)packet);
    UnPack.unpack_struct(&session,sizeof(session));

    // convert from local format
    session.nTrack = (StationAPITrack)htog32(session.nTrack);
    session.nResult = (StationAPIResult)htog32(session.nResult);
    session.nUid = (StationAPIUID)htog32(session.nUid);

    R->sapiResult = session.nResult;
    R->nUID = session.nUid;
    api->srList->Link(R,SR_STATUS_RESULT); // move request to results
}

//------------------------------------------
void HandleRDPLoginContent(StationRequest * R, StationAPI * api, msg_t * packet)
//------------------------------------------
{
    server_login login;

    PackClass UnPack(0);
	UnPack.unpack_begin((void *)packet);
    UnPack.unpack_struct(&login,sizeof(login));

    // convert from local format
    login.nTrack = (StationAPITrack)htog32(login.nTrack);
    login.nResult = (StationAPIResult)htog32(login.nResult);
    login.nUid = (StationAPIUID)htog32(login.nUid);
    login.nAccountStatus = (StationAPIAccountStatus)htog32(login.nAccountStatus);
    login.nGameStatus = (StationAPIGameStatus)htog32(login.nGameStatus);

    R->sapiResult = login.nResult;
    R->nUID = login.nUid;
    R->nAccountStatus = login.nAccountStatus;
    R->nGameStatus = login.nGameStatus;
    R->CSessionid = login.sSession;
    api->srList->Link(R,SR_STATUS_RESULT); // move request to results
}

//------------------------------------------
void HandleRDPMessage(StationAPI * api, msg_t * packet)
//------------------------------------------
{
    StationRequest * R;
    StationRequest * N;

    PackClass Pack(0);
	Pack.unpack_begin((void *)packet);
    StationAPITrack nTrack = (StationAPITrack)Pack.unpack_long();

    R = api->srList->pNext;
    // find request with this resource number attached
    while (R != api->srList)
    {
        N = R->pNext;
        if (R->nTrack == nTrack)
        {
        	int code = packet->msg; // message opcode

            switch(R->nRequestType)
            {
            case SAPI_CALL_CREATE:
                break;
            case SAPI_CALL_LOGIN:
                HandleRDPLoginContent(R, api, packet);
                break;
            case SAPI_CALL_SESSION:
                HandleRDPSessionContent(R, api, packet);
                break;
            case SAPI_CALL_PASSWORD:
                break;
            case SAPI_CALL_GETDATA:
                break;
            case SAPI_CALL_PUTDATA:
                break;
            case SAPI_CALL_PURCHASE:
                break;
            case SAPI_CALL_CONSUME:
                break;
            case SAPI_CALL_HISTORY:
                break;
            }
            return; // normal exit
        }
        R = N;
    }
    //printf("Did not find tracking number in queue! (%x)\n",nTrack);
}

//------------------------------------------
// return true if connection has failed
bool ProcessRDP(StationAPI * api)
//------------------------------------------
{
	msg_arrival_t * arrival;
    connection_t * c;
    msg_t * packet;
    bool isFailed = false;

    if (!api->g_logged_into_server)
    {   // not connected, no messages to get
        sleep_ms(50);
        return true;
    }

	for(;;)
	{
		arrival = rdp_receive(api->g_rdp, 10);			// No timeout!!
		if ( arrival == NULL )
			break;  // no messages incoming

		c = msg_arrival_get_sender(arrival);

        if (c == NULL)                                      // connectionless send
		{
			//HandleConnectionlessSends(arrival);
		}
		else if (connection_connected(c) == FALSE)
		{
			printf("Client lost connection to server\n");
            isFailed = true;
		}
		else if (msg_arrival_has_fin(arrival))
		{
			printf("Client received FIN from server\n");
            isFailed = true;
		}
		else 
		{
    	    packet = (msg_t*)msg_arrival_get_data(arrival);
			if ((packet != NULL))
			{
                packet->msg = htog16( packet->msg );		// message ID
                HandleRDPMessage(api, packet);
			}
            else
            {
    			printf("Client lost connection to server -- null packet\n");
                isFailed = true;
            }
        }

		fast_free(arrival);
	}
    if (isFailed)
    {
	    api->g_logged_into_server = false;   // dont send disconnect message
        RDPServerDisconnect(api);
    }

    return isFailed;
}

//------------------------------------------
void CheckForTimeout(StationAPI * api)
//------------------------------------------
{
    StationRequest * R;
    StationRequest * N;
    time_t  now = time(NULL);

    // search pending queue
    R = api->srList->pNext;
    while (R != api->srList)
    {
        N = R->pNext;
        if (R->tTimeout < now)
        {
            R->sapiResult = SAPI_RESULT_TIMEOUT;
            api->srList->Link(R,SR_STATUS_RESULT); // move request to results
            api->bTimingOut = true;
        }
        R = N;
    }
}

//------------------------------------------
void DoCallback(StationAPI * api, StationRequest * R)
//------------------------------------------
{
    api->bInCallback = true;

    SetErrorText(R);
    // try the generic callback
    if (!api->OnResult (R->nTrack, R->sapiResult, R->sText, R->pUserPointer))
    {
        // if the generic callback didn't handle it, call the specific callback
        switch(R->nRequestType)
        {
        case SAPI_CALL_CREATE:
            api->OnCreate   (R->nTrack, R->sapiResult, R->sText, R->nUID, R->pUserPointer);
            break;
        case SAPI_CALL_LOGIN:
            api->OnLogin    (R->nTrack, R->sapiResult, R->sText, R->nUID, R->nAccountStatus, R->nGameStatus, R->CSessionid, R->pUserPointer);
            break;
        case SAPI_CALL_SESSION:
            api->OnSession (R->nTrack, R->sapiResult, R->sText, R->nUID, R->pUserPointer);
            break;
        case SAPI_CALL_PASSWORD:
            api->OnPassword (R->nTrack, R->sapiResult, R->sText, R->pUserPointer);
            break;
        case SAPI_CALL_GETDATA:
            api->OnGetData  (R->nTrack, R->sapiResult, R->sText, R->sapiList, R->pUserPointer);
            break;
        case SAPI_CALL_PUTDATA:
            api->OnPutData  (R->nTrack, R->sapiResult, R->sText, R->pUserPointer);
            break;
        case SAPI_CALL_PURCHASE:
            api->OnPurchase (R->nTrack, R->sapiResult, R->sText, R->pUserPointer);
            break;
        case SAPI_CALL_CONSUME:
            api->OnConsume  (R->nTrack, R->sapiResult, R->sText, R->pUserPointer);
            break;
        case SAPI_CALL_HISTORY:
            api->OnHistory  (R->nTrack, R->sapiResult, R->sText, R->sapiList, R->pUserPointer);
            break;
        }
    }
    // result is now processed, discard it
    R->sText = NULL;

    api->bInCallback = false;
}

//------------------------------------------
void DeleteRequests(StationAPI * api, StationRequest * R)
//------------------------------------------
{
    StationAPITrack nTrack = R->nTrack;
    delete R;   // get rid of cancel request itself

    StationRequest * N;

    // search active queue
    N = api->srList->pNext;
    while (N != api->srList)
    {
        if (N->nTrack == nTrack)
        {
            delete N;
            return;
        }
        N = N->pNext;
    }

    // search result queue
    N = api->srList->pResultNext;
    while (N != api->srList)
    {
        if (N->nTrack == nTrack)
        {
            delete N;
            return;
        }
        N = N->pResultNext;
    }

    // search submit queue
    N = api->srList->pSubmitNext;
    while (N != api->srList)
    {
        if (N->nTrack == nTrack)
        {
            delete N;
            return;
        }
        N = N->pSubmitNext;
    }
}

/*======================================================================

   FUNCTION: ProcessThread

   DESCRIPTION:
      Independant service thread with error handling

======================================================================*/

#ifdef WIN32
//-------------------------------
// Exception handling for windows environments
int FilterFunction(LPEXCEPTION_POINTERS eptr, StationAPI * api)
//-------------------------------
{
	PVOID address = eptr->ExceptionRecord->ExceptionAddress;
	DWORD code = eptr->ExceptionRecord->ExceptionCode;
	DWORD flags = eptr->ExceptionRecord->ExceptionFlags;
	DWORD nparms = eptr->ExceptionRecord->NumberParameters;

	char fname[256];
	FILE *fh;
	
	sprintf(fname,"EXCEPTION_PROCESS_THREAD.txt");
	fh = fopen(fname,"at");
	if (fh)
	{
		long tme = time(0L);
		fprintf(fh,"%s\n", ctime((long *)&tme));
		fprintf(fh,"ProcessThread Exception info: Address %x, Code %x, Flags %x, Parameters:",
			address, code, flags);
		for (DWORD n=0; n < nparms; n++)
			fprintf(fh,"%x,", eptr->ExceptionRecord->ExceptionInformation[n]);
		fprintf(fh,"\n");
		fclose(fh);
	}

	fprintf(stderr, "ProcessThread Exception info: Address %x, Code %x, Flags %x, Parameters %d\n",
			address, code, flags, nparms);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

//-------------------------------
void ProcessThread( void* data )
//-------------------------------
{
	StationAPI * api = (StationAPI *) data;
    StationRequest * R;
    StationRequest * N;
    time_t  now = time(NULL);

#ifdef WIN32
__try {
#endif

	// we are up and ready
	fprintf(stderr,"Process Thread started.\n");

    api->bProcessRunning = true;
    api->bProcessStop = false;
	while(api->bProcessRunning && !api->bProcessStop)
	{
        now = time(NULL);

        // Process Result Queue
        if (api->bProcessFlag && api->srList->nResults > 0)  // any results ready?
        {   // handle the top one
            R = api->srList->pResultNext;
            if (R->nRequestType == SAPI_CALL_CANCEL_REQUEST)
            {
                DeleteRequests(api, R); // deletes R as well
            }
            else
            {
                DoCallback(api, R);
                delete R;
            }
        }

        // Process Submit Queue
        if (api->srList->nSubmit > 0)   // any requests to send?
        {   // handle the top one
            R = api->srList->pSubmitNext;
            while (R != api->srList)
            {
                N = R->pSubmitNext;
                if (R->tRetry < now)
                {
                    switch(R->nRequestType)
                    {
                    case SAPI_CALL_LOGIN:
                        switch (R->nRequest)
                        {
                        case SAPI_REQUEST_LOGIN_EXCLUSIVE:
                        case SAPI_REQUEST_LOGIN_MULTIPLE:
                            //R->nSessionid = itoa(time(NULL)); // fake sessionid
                            // for now, these do same thing as validate -- TODO, full implementation
                        case SAPI_REQUEST_LOGIN_VALIDATE:
                            SendRDPLogin(R, api);
                            R->tTimeout = now + SAPI_REQUEST_TIMEOUT_PENDING;
                            api->srList->Link(R,SR_STATUS_PENDING); // move request to pending
                            break;
                         // unhandled login request number
                        default:
                            R->sapiResult = SAPI_RESULT_NOSUPPORT;
                            api->srList->Link(R,SR_STATUS_RESULT); // move request to results since submission failed
                            break;
                        }
                        break;

                    case SAPI_CALL_SESSION:
                        switch (R->nRequest)
                        {
                        case SAPI_REQUEST_SESSION_LOGOUT:
                        case SAPI_REQUEST_SESSION_TOUCH:
                        case SAPI_REQUEST_SESSION_VALIDATE:
                            SendRDPSession(R, api);
                            R->tTimeout = now + SAPI_REQUEST_TIMEOUT_PENDING;
                            api->srList->Link(R,SR_STATUS_PENDING); // move request to pending
                            break;
                        // unhandled session request number
                        default:
                            R->sapiResult = SAPI_RESULT_NOSUPPORT;
                            api->srList->Link(R,SR_STATUS_RESULT); // move request to results since submission failed
                            break;
                        }
                        break;

                    // unhandled request type
                    default:
                        R->sapiResult = SAPI_RESULT_NOSUPPORT;
                        api->srList->Link(R,SR_STATUS_RESULT); // move request to results since submission failed
                        break;

                    } // R->nRequestType
                    break;    // abort loop, we submitted one
                }
                R = N;
            }
        }

        // Process RDP messages
        ProcessRDP(api);

        CheckForTimeout(api);

	}

	fprintf(stderr,"Process Thread stopped.\n");
    api->bProcessRunning = false;
    api->bProcessStop = false;

#ifdef WIN32
} // __try
__except(FilterFunction( GetExceptionInformation(), api))
{
    api->bProcessRunning = false;
    api->bProcessStop = false;
    return;
}
#endif

}   // ProcessThread
