//// documentation includes
//@Include: StationAPISession.h

#ifndef _STATION_API_H
#define _STATION_API_H

#define SAPI_VERSION    "2000.10.24"    // last change date of this file

// these are the default arguments for StationAPI:Connect()
#define SAPI_DEFAULT_SERVER "sdlogin1.station.sony.com"
#define SAPI_DEFAULT_PORT 5997

#include "order.h"

#include "allsys.h"
#include "utime.h"
#include "uerror.h"
#include "utypes.h"
extern "C" {
#include "uthread.h"
#include "umutex.h"
}
//#include "stationrequest.h"   // moved down past enums and structs
#include "crypt.h"
#include "StationAPISession.h"  // Declares StationAPISession class
/**type
*/
//@{
/// numeric IP
typedef uint32  StationAPIIP;
/// Tracking number
typedef uint32  StationAPITrack;
/// User ID value
typedef uint32  StationAPIUID;
/// account status
typedef int     StationAPIAccountStatus;
/// game status
typedef int     StationAPIGameStatus;
/// application instance
typedef int     StationAPIInstance;

/// Transmission protocol codes
enum StationAPIProtocol
{
    ///
    SAPI_PROTOCOL_RDP = 0,
    //
    SAPI_PROTOCOL_END
};

/// Application codes -- list to be expanded as applications added
enum StationAPIApplication
{
    /// for demos/testing only
    SAPI_APPLICATION_DEMO = 0,
    ///
    SAPI_APP_STATION_CLIENT_SERVER,
    ///
    SAPI_APP_TANARUS_GAME_SERVER,
    ///
    SAPI_APP_INFANTRY_GAME_SERVER,
    ///
    SAPI_APP_PLANETSIDE_GAME_SERVER,
    ///
    SAPI_APP_SOVEREIGN_GAME_SERVER,
    ///
    SAPI_APP_STARWARS_GAME_SERVER,
    // extend application list here
    SAPI_APPLICATION_END
};

/// Product codes -- list to be expanded as products added
enum StationAPIProduct  // applications may have multiple products
{
    /// unspecified product, for demos/testing only
    SAPI_PRODUCT_DEMO = 0,
    ///
    SAPI_PRODUCT_STATION_CLIENT,
    ///
    SAPI_PRODUCT_TANARUS = 100,
    ///
    SAPI_PRODUCT_INFANTRY = 200,
    ///
    SAPI_PRODUCT_PLANETSIDE = 300,
    ///
    SAPI_PRODUCT_SOVEREIGN = 400,
    ///
    SAPI_PRODUCT_STARWARS = 500,
    // extend product list here
    SAPI_PRODUCT_END
};

/// Request code values
enum StationAPIRequest
{
    // RequestLogin
    ///
    SAPI_REQUEST_LOGIN_EXCLUSIVE = 0,
    ///
    SAPI_REQUEST_LOGIN_MULTIPLE,
    ///
    SAPI_REQUEST_LOGIN_VALIDATE,
    // RequestSession
    ///
    SAPI_REQUEST_SESSION_LOGOUT = 100,
    ///
    SAPI_REQUEST_SESSION_TOUCH,
    ///
    SAPI_REQUEST_SESSION_VALIDATE,
    // RequestCreate
    SAPI_REQUEST_CREATE_UID     = 200,
    // RequestPurchase
    SAPI_REQUEST_PURCHASE_QUOTE = 300,
    SAPI_REQUEST_PURCHASE_SUBSCRIPTION, // credit field required
    SAPI_REQUEST_PURCHASE_HARDGOOD,     // credit field required
    SAPI_REQUEST_PURCHASE_SOFTGOOD,     // credit field required
    SAPI_REQUEST_PURCHASE_CANCEL,
    // RequestConsume
    SAPI_REQUEST_CONSUME_TOKENS = 400,
    // RequestHistory
    SAPI_REQUEST_HISTORY_RECENT = 500,
    SAPI_REQUEST_HISTORY_PREVIOUS,
    SAPI_REQUEST_END
};

/// Return code values
enum StationAPIResult
{
    /// operation suceeded
    SAPI_RESULT_SUCCESS = 0,
    /// operation timed out waiting for available thread
    SAPI_RESULT_BUSY,
    /// operation timed out waiting for a server response
    SAPI_RESULT_TIMEOUT,
    /// result was garbage
    SAPI_RESULT_COMMERROR,
    /// not connected to server
    SAPI_RESULT_NOT_CONNECTED, 
    /// illegal input, detected clientside
    SAPI_RESULT_BAD_ARGUMENT,      
    /// specified operation is not supported yet
    SAPI_RESULT_NOSUPPORT,       
    // expand local results here -- write code to treat unknown result codes as a general failure
    /// operation failed at server -- general case
    SAPI_RESULT_FAILED = 100,     
    /// server database problem
    SAPI_RESULT_DATABASE_ERROR,
    /// you are not allowed to do this operation
    SAPI_RESULT_ILLEGAL_OPERATION,
    /// illegal or invalid input
    SAPI_RESULT_BAD_INPUT,        
    /// name not in database, or name/password mismatch
    SAPI_RESULT_NAME_NOT_FOUND,   
    /// password mismatch 
    SAPI_RESULT_PASSWORD_FAILED,
    /// station UID not found in database
    SAPI_RESULT_UID_NOT_FOUND,    
    /// station account on hold
    SAPI_RESULT_ON_HOLD,       
    /// station account cancelled by user
    SAPI_RESULT_CANCEL_USER,     
    /// station account cancelled for standards violations
    SAPI_RESULT_CANCEL_STANDARDS,
    /// station account cancelled for other reason
    SAPI_RESULT_CANCEL_OTHER, 
    /// server session number not active
    SAPI_RESULT_SESSION_NOT_FOUND, 
    /// cannot create name that already exists
    SAPI_RESULT_NAME_EXISTS,
    // expand server results here -- write code to treat unknown result codes as a general failure
    SAPI_RESULT_END
};
//@}

#define SMALL_LENGTH            16
#define MEDIUM_LENGTH           32
#define LARGE_LENGTH            64
#define HUGE_LENGTH             128

//
struct StationAPIRegistration
{
	char		sStationName[SMALL_LENGTH];
	char		sStationPassword[SMALL_LENGTH];
	char		sHandle[SMALL_LENGTH];
	char		sEmail[HUGE_LENGTH];
	char		sBirthdate[SMALL_LENGTH];
	char		sLast[MEDIUM_LENGTH];
	char		sFirst[MEDIUM_LENGTH];
	char		sMiddle[SMALL_LENGTH];
	char		sAddress[HUGE_LENGTH];
	char		sCity[LARGE_LENGTH];
	char		sState[LARGE_LENGTH];
	char		sZip[MEDIUM_LENGTH];
	char		sCountry[MEDIUM_LENGTH];
	char		sDayphone[LARGE_LENGTH];
	char		sEvephone[LARGE_LENGTH];
	char		sWorkplace[LARGE_LENGTH];
	char		sHearabout[LARGE_LENGTH];
	char		sIsp[LARGE_LENGTH];
	char		sKey[LARGE_LENGTH];
	char		sGender[SMALL_LENGTH];
	char		sPress[SMALL_LENGTH];
	char		sDeveloper[SMALL_LENGTH];
	char		sPromo[SMALL_LENGTH];
	char		sProcessorSpeed[LARGE_LENGTH];
	char		sProcessorType[LARGE_LENGTH];
	char		sSystemRam[LARGE_LENGTH];
	char		sTwoDVideoCard[LARGE_LENGTH];
	char		sThreeDVideoCard[LARGE_LENGTH];
	char		sTextureRam[LARGE_LENGTH];
	char		sSoundRard[LARGE_LENGTH];
	char		sConnectionSpeed[LARGE_LENGTH];
	char		sSystemsOwned[LARGE_LENGTH];
	char		sFavoriteGame[LARGE_LENGTH];
};

// true means field is present
struct StationAPIRegistrationFields
{
	bool		bStationName;
	bool		bStationPassword;
	bool		bHandle;
	bool		bEmail;
	bool		bBirthdate;
	bool		bLast;
	bool		bFirst;
	bool		bMiddle;
	bool		bAddress;
	bool		bCity;
	bool		bState;
	bool		bZip;
	bool		bCountry;
	bool		bDayPhone;
	bool		bEvePhone;
	bool		bWorkplace;
	bool		bHearabout;
	bool		bIsp;
	bool		bKey;
	bool		bGender;
	bool		bPress;
	bool		bDeveloper;
	bool		bPromo;
	bool		bProcessorSpeed;
	bool		bProcessorType;
	bool		bSystemRam;
	bool		bTwoDVideoCard;
	bool		bThreeDVideoCard;
	bool		bTextureRam;
	bool		bSoundCard;
	bool		bConnectionSpeed;
	bool		bSystemsOwned;
	bool		bFavoriteGame;
};

struct StationAPIHistory
{
    // history format TBD
};

struct StationAPICredit
{
    char        sCreditNumber[MEDIUM_LENGTH];
    char        sExpiration[SMALL_LENGTH];
	char		sLast[MEDIUM_LENGTH];
	char		sFirst[MEDIUM_LENGTH];
	char		sMiddle[SMALL_LENGTH];
	char		sAddress[HUGE_LENGTH];
	char		sCity[LARGE_LENGTH];
	char		sState[LARGE_LENGTH];
	char		sZip[MEDIUM_LENGTH];
	char		sCountry[MEDIUM_LENGTH];
    char        sProduct[LARGE_LENGTH];
};

#include "stationrequest.h"
#define SAPI_LIST_MAX  8096

class StationAPIList
{
public:
     // constructor
    StationAPIList(); 
    // destructor
    ~StationAPIList();

    // list creation, using initialized input structures
    void BuildRegistrationList(StationAPIRegistration * content, StationAPIRegistrationFields * fieldsused = NULL); // NULL means all fields
    void BuildGetRegistrationList(StationAPIRegistrationFields * fieldsused = NULL); // NULL means all fields
    void BuildCreditList(StationAPICredit * content);
    /* add more cases as needed */

    // list extraction, setting output structures
    void ExtractRegistrationList(StationAPIRegistration * content, StationAPIRegistrationFields * fieldsused);
    void ExtractHistoryList(StationAPIHistory * content);
    /* add more cases as needed */

    // manual methods for special cases -- do not use special cases for mainline code!
    void InitList(void);                // call before first PutField to initialize list
    void BuildGetDataField(char * fieldname);    // IN: name of database field desired
    void BuildPutDataField(char * fieldname, char * content);    // IN: name of database field to set, IN: content of field
    char * GetField(char * fieldname);  // IN: name of database field, OUT: pointer to string returned, or NULL if not in data set

private:
    int nByteCount;
    char listBuffer[SAPI_LIST_MAX];
};

/**
    Client Station API -- connects to the API Server.
    \begin{verbatim}
// This class should be inherited so that only the virtual callbacks you wish to implement need be declared:
class MyStationAPI : public StationAPI
{
    void OnSession (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, void * user);
    void OnLogin    (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, 
                StationAPIAccountStatus account_status, StationAPIGameStatus game_status, StationAPISession sessionid, void * user);
};
    \end{verbatim}
*/
class StationAPI
{
public:  
    rdp_t *g_rdp;	            // rdp struct
    connection_t* g_c;	        // connection to server
    bool g_logged_into_server;  // status flag
    CCrypt * pEncryption;
    bool bInCallback;           // true if actively in callback function
    bool bConnected;            // true if connected to server
    bool bTimingOut;            // true if last result was a timeout
    bool bProcessFlag;          // true if async callbacks, false for polled
    char * sDNS;                // DNS name of server
    int nPort;                  // port of server
    int nProtocol;              // protocol settings
    int nServerID;              // game ID
    int nLocationID;            // instance ID
    StationRequest * srList;    // Station Request master node

    bool bProcessRunning;       // true if api thread is running
    bool bProcessStop;          // true signals thread to exit
    uthread_t apiUthread;

    bool StartProcess();
    void StopProcess();

    void CheckArguments (StationRequest * R);
public:
    StationAPI();   
    ~StationAPI();  

    /**@name Control Requests
    */
    //@{

    /** Set identity -- call before connecting.
    \begin{itemize}
    \item applicationid: ID assigned to application
    \item instanceid: instance can be used to distinguish between multiple copies of applications
    \end{itemize}
    */
    void SetIdentity(StationAPIApplication applicationid, StationAPIInstance instanceid = 0);

    /** Specify communications protocol (optional) -- call before connecting.
    \begin{itemize}
    \item protocol: change protocol assigned to application -- default is RDP
    \end{itemize}
    */
    void SetProtocol(StationAPIProtocol protocol);

    /** start server connection -- return 0 for success, else protocol specific error value.
    \begin{itemize}
    \item dns   url of api server
    \item port  port of api server
    \end{itemize}
    */
    int32 Connect       (char * dns = SAPI_DEFAULT_SERVER, int port = SAPI_DEFAULT_PORT);

    /** stop server connection.
    \begin{itemize}
    \item linger_time: max time to wait (optional)
    \end{itemize}
    */
    void Disconnect    (int linger_time = 10000);

    /// returns true if connection is live.
    bool IsConnected    (void);

    /// returns number of submitted requests -- use to monitor request load.
    int GetRequestsQueued (void);

    /// returns number of requests waiting for processing -- use to monitor request load.
    int GetRequestsWaiting (void);

    /** Set to true to enable async callbacks, set to false to queue results.
       Can use interactively for critical sections, setting flag to false will block if a callback is in progress 
    */
    void SetProcess (bool flag);

    /** Poll for syncronous processing -- blocking for up to X milliseconds to process.  Returns immediately if no work.
    */
    void ProcessWait (int duration = 1000);

    //@}

    /**@name Request Functions
        All requests will return a tracking number.
        All requests will result in a callback.
        Sensitive requests are encrypted before transmission
    */
    //@{

    // Create new station identity with anywhere from minimum to complete registration and demographic data
    StationAPITrack RequestCreate    (StationAPIRequest request, char * name, char * password, StationAPIList * reginfo, void * user = NULL);

    /** Login -- Create a new session on server.
    \begin{itemize}
    \item request: request code
    \item name: user name, zero terminated
    \item password: password, zero terminated
    \item ip: IP of originating client, for reporting purposes
    \item product: product code
    \item parentsession: associated session, if any
    \item user: user pointer passed through to callback function
    \end{itemize}
    */
    StationAPITrack RequestLogin     (StationAPIRequest request, char * name, char * password, 
        StationAPIIP ip, StationAPIProduct product, StationAPISession * parentsession = NULL, void * user = NULL);
//    StationAPITrack RequestLogin     (StationAPIRequest request, char * name, char * password, 
//        StationAPIIP ip, StationAPIProduct product, StationAPISession parentsession = StationAPISession(""), void * user = NULL);

    /** Session -- Modify session on server (logout, touch, verify).
    \begin{itemize}
    \item [request:] request code
    \item [sessionid:] session to modify
    \item [user:] user pointer passed through to callback function
    \end{itemize}
    */
    StationAPITrack RequestSession   (StationAPIRequest request, StationAPISession * sessionid, void * user = NULL);
//    StationAPITrack RequestSession   (StationAPIRequest request, StationAPISession * sessionid, void * user = NULL);

    StationAPITrack RequestPassword  (StationAPIUID uid, char * oldpassword, char* newpassword, void * user = NULL);
    StationAPITrack RequestGetData   (StationAPIUID uid, StationAPIList * list, void * user = NULL);
    StationAPITrack RequestPutData   (StationAPIUID uid, StationAPIList * list, void * user = NULL);
    StationAPITrack RequestPurchase  (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product, StationAPIList * credit = NULL, void * user = NULL);
    StationAPITrack RequestConsume   (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product, int quantity = 1, void * user = NULL);
    StationAPITrack RequestHistory   (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product, void * user = NULL);

    /// Cancel request in progress by tracking number -- no callback will occur.
    void CancelRequest(StationAPITrack track);

    //@}

    /**@name Callback Functions.
    Result routines are called when responses from requests occur. All requests will produce a response.
    OnResult is always called first. If it does not handle the request, the specific callback is called.
    */
    //@{

    /** Always called first. Return true if handled, else false to call secondary callback.
    \begin{itemize}
    \item track: tracking number returned by request
    \item result: result code
    \item text: displayable text string associated with result code
    \item user: user pointer passed through from Request Function
    \end{itemize}
    */
    virtual bool OnResult   (StationAPITrack track, StationAPIResult result, char * text, void * user);

    virtual void OnCreate   (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, void * user);

    /** Callback for RequestLogin.
    \begin{itemize}
    \item track: tracking number returned by request
    \item result: result code
    \item text: displayable text string associated with result code
    \item uid: UID of user on success
    \item account_status: account status on success
    \item game_status: game status on success
    \item sessionid: new session id on success
    \item user: user pointer passed through from Request Function
    \end{itemize}
    */
    virtual void OnLogin    (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, 
        StationAPIAccountStatus account_status, StationAPIGameStatus game_status, StationAPISession sessionid, void * user);

    /** Callback for RequestSession.
    \begin{itemize}
    \item track: tracking number returned by request
    \item result: result code
    \item text: displayable text string associated with result code
    \item uid: UID of user on touch or verify
    \item user: user pointer passed through from Request Function
    \end{itemize}
    */
    virtual void OnSession (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, void * user);

    virtual void OnPassword (StationAPITrack track, StationAPIResult result, char * text, void * user);

    virtual void OnGetData  (StationAPITrack track, StationAPIResult result, char * text, StationAPIList * list, void * user);

    virtual void OnPutData  (StationAPITrack track, StationAPIResult result, char * text, void * user);

    virtual void OnPurchase (StationAPITrack track, StationAPIResult result, char * text, void * user);

    virtual void OnConsume  (StationAPITrack track, StationAPIResult result, char * text, void * user);

    virtual void OnHistory  (StationAPITrack track, StationAPIResult result, char * text, StationAPIList * list, void * user);

    //@}
};

#endif