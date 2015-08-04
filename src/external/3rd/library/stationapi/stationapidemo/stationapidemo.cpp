/*****************************************************************************************************************
The purpose of the StationAPI is to provide a single point of contact between a game server and support services.
Appropriate commuications encryption is to be provided by the API.  The server also will enforce security by refusing
to return sensitive information such as passwords or credit information.

General request form: 
All requests are submitted and return immediately with a tracking code which the application needs to connect with the originating request.
Tracking codes will always be returned...even local api errors will be reported via the callbacks.
        StationAPITrack RequestItem (arguments);
When a request has a result (pass, fail, or timeout), it will result in a call to:
        bool OnResult   (StationAPITrack track, StationAPIResult result, char * text);
        // the tracking number is returned so you can connect the result with the orignal request
        // result, hopefully SAPI_RESULT_SUCCESS
        // text, a displayable string explaining the result code for logging or user display if not specifically handled
If you choose to handle the result in OnResult, return true, else return false in which case the specific callback is called:
        void OnItemResult (StationAPITrack track, StationAPIResult result, char * text, additional arguments);
        // if there is an error situation, any additional arguments are not valid
        // copy the results, as the results will be discarded once you return from this callback
 
The core functions of the API are designed to provide:

    Account Validatation and Authentication
    User Tracking
    Account Creation and Modification
    Purchasing, Subscriptions, and Consumption

Account Validatation and Authentication functions are rolled into a single request:
        StationAPITrack RequestLogin     (StationAPIRequest request, char * name, char * password);
        // request SAPI_REQUEST_LOGIN_EXCLUSIVE (unique login required for this product)
        // request SAPI_REQUEST_LOGIN_MULTIPLE (multiple logins ok for this product)
        // request SAPI_REQUEST_LOGIN_VALIDATE(validate only, do not log in, return sessionid if user active, or 0 if not.)
    which has the callback:
        void OnLogin (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, StationAPIAccountStatus account_status, StationAPIGameStatus game_status, StationAPISession sessionid );
        // uid is the station UID
        // account_status applies to the user overall, and could indicate total banning from station products
        // game_status applies to the user with respect to this game only
        // sessionid uniquely identifies this session to the server, to distinguish multiple logins of the same user

User Tracking will be handled by the server, which will track who is currently active, in which games.
        StationAPITrack RequestLogin     (StationAPIRequest request, char * name, char * password); // initiates a session // see example code below
        StationAPITrack RequestSession   (StationAPIRequest request, StationAPISession sessionid);
        // request SAPI_REQUEST_SESSION_LOGOUT (terminate server session specified by sessionid for this game)
        // request SAPI_REQUEST_SESSION_TOUCH (way for game to actively tell API server that this client is still active)
        // request SAPI_REQUEST_SESSION_VALIDATE (verify if server still has this sessionid logged in);

Account Creation and Modification.  Creation is requested via:
        StationAPITrack RequestCreate    (StationAPIRequest request, char * name, char * password, StationAPIList * reginfo); // see example code below
        // request SAPI_REQUEST_CREATE_UID
        // name, station name (case insensitive)
        // password, station password (case sensitive)
        // pointer to registration information, which much contain a minimum set for the function to succeed.
    Modification is done via:
        StationAPITrack RequestGetData   (StationAPIUID uid, StationAPIList * list);    // fetch information
    and:
        StationAPITrack RequestPutData   (StationAPIUID uid, StationAPIList * list);    // change information or add new fields

Purchasing, Subscriptions, and Consumption.  All types of purchases are made with:
        StationAPITrack RequestPurchase  (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product, StationAPIList * credit = NULL);
        // request SAPI_REQUEST_PURCHASE_QUOTE (a way to retrive a price quote about a product, format TBD)
        // request SAPI_REQUEST_PURCHASE_SUBSCRIPTION // credit field required (purchase a subscription)
        // request SAPI_REQUEST_PURCHASE_HARDGOOD     // credit field required (purchase something physical)
        // request SAPI_REQUEST_PURCHASE_SOFTGOOD     // credit field required (purchase permissions, i.e. tokens)
        // request SAPI_REQUEST_PURCHASE_CANCEL (cancel subscription)
    The "token" softgood, which may be used by some games can be consumed with this request:
        StationAPITrack RequestConsume   (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product = 0, int quantity = 1);
        // request SAPI_REQUEST_CONSUME_TOKENS
        // product indicates a product to be consumed, with 0 indicating the default consumable for this game
        // quantity indicates the number of tokens to consume
    Lastly, a way to get account history information is planned:
        StationAPITrack RequestHistory   (StationAPIUID uid, StationAPIRequest request, StationAPIProduct product = 0);
        // request SAPI_REQUEST_HISTORY_RECENT      // current billing period
        // request SAPI_REQUEST_HISTORY_PREVIOUS    // previous billing period
        // product indicates return results about a specific product, with 0 being a default case

*****************************************************************************************************************/

#include <stdio.h>

#include "stationapi.h"

#define MY_ID       SAPI_APPLICATION_DEMO   // the ID assigned to me
#define MY_PRODUCT  SAPI_PRODUCT_DEMO       // the product I'm managing

StationAPIRegistration          sapiRegister;
StationAPIRegistrationFields    sapiRegisterFlag;
StationAPISession lastsession;

/// Sample general callback function, in this case used to catch a few errors and let the rest through

// Declase my API interface, and which of the virtual callback functions we choose to implement.
class MyStationAPI : public StationAPI
{
    bool OnResult   (StationAPITrack track, StationAPIResult result, char * text, void * user);
    void OnSession (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, void * user);
    void OnLogin    (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, 
                StationAPIAccountStatus account_status, StationAPIGameStatus game_status, StationAPISession sessionid, void * user);
};

//MyStationAPI sapiServer;    // declare an instance
MyStationAPI * pSapi = NULL;

bool MyStationAPI::OnResult (StationAPITrack track, StationAPIResult result, char * text, void * user)
{
    switch (result)
    {
    case SAPI_RESULT_BUSY:              // operation timed out waiting for available thread
    case SAPI_RESULT_TIMEOUT:           // operation timed out waiting for a server response
    case SAPI_RESULT_NOT_CONNECTED:     // not connected to server
        printf ("OnResult Track %x: Unable to connect to server -- %s\n", track, text);
        // TODO discard request from your local queue
        return true;    // result handled, do not call specific callback
    default:
        return false;   // result not handled, call specific callback
    };
}

void MyStationAPI::OnSession (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, void * user)
{
    switch (result)
    {
    case SAPI_RESULT_SUCCESS:
        printf ("OnSession Track #%x: Login UID %08x (%d)\n", track, uid, uid);    // for this demo, all we care about is the UID and success case
        // TODO handle session success
        break;
    default:
        printf ("OnSession Track $%x: Login failed (%d) %s\n", track, result, text);
        // TODO handle session failure
        break;
    };
    // TODO discard request from your local queue
}

void MyStationAPI::OnLogin (StationAPITrack track, StationAPIResult result, char * text, StationAPIUID uid, StationAPIAccountStatus account_status, StationAPIGameStatus game_status, StationAPISession sessionid, void * user) 
{ 
    switch (result)
    {
    case SAPI_RESULT_SUCCESS:
        printf ("OnLogin Track #%x: Login UID %08x (%d), session (%s)\n", track, uid, uid, sessionid.GetString());    // for this demo, all we care about is the UID and success case
        lastsession = sessionid;
        // TODO handle login success
        break;
    default:
        printf ("OnLogin Track $%x: Login failed (%d) %s\n", track, result, text);
        // TODO handle login failure
        break;
    };
    // TODO discard request from your local queue
};

//------------------------------------------
// threaded console input function, with hardcoded Q exit
void KeyCheck( void * result)
//------------------------------------------
{
      static int c = 0;
      static int consolekey = 0;
      if (result == NULL)
      {       // active thread
              while(true)
              {
                      c = getchar();
                      if (c > ' ')
                      {
                              consolekey = c;
                              if (c == 'Q' || c == 'q')
                                  break; // stop input thread
                      }
              }
      }
      else
      {       // checking result
              *((int *)result) = consolekey;
              consolekey = 0;
      }
}

//------------------------------------------
bool ProcessConsoleKey()
//------------------------------------------
{
	time_t t;
	int c;
    int nn;
    static bool isConnected = false;
    static bool isProcess = true;

    KeyCheck(&c);
 	if (c)
	{
		t = time(NULL);
		c = toupper(c);
		switch (c)
		{
		case '?':	
		case 'H':	
            printf("\nCOMMAND SUMMARY AT %s\n"
                    "H = this page\n"
                    "C = connect \n"
                    "D = disconnect \n"
                    "I = invalid login test\n"
                    "L = valid login test\n"
                    "P = toggle process flag\n"
                    "S = status report\n"
                    "V = validate last session\n"
                    "X = logout last session\n"
                    "Q = quit \n"
					,asctime(localtime(&t)));
			break;

        case 'C':
            if (isConnected) 
            {
                printf("Already Connected.\n");
                break;
            }
            else
            {
                pSapi->SetIdentity(MY_ID);                          // identify myself
                pSapi->SetProcess(isProcess);                       // set for syncronous use
                nn = pSapi->Connect();                              // connect to address and port specified by a config file
//nn = pSapi->Connect("mib.station.sony.com"); 
//nn = pSapi->Connect("127.0.0.1"); 
                if (nn)
                    printf("Connect failed %d\n",nn);
                else
                {
                    printf("Connected, requests may be made.\n");
                    isConnected = true;
                }
            }
            break;

        case 'D':
            if (isConnected) 
            {
                pSapi->Disconnect();
                printf("Disconnected, unresolved requests discarded.");
                isConnected = false;
            }
            else
            {
                printf("Not Connected.\n");
                break;
            }
            break;

        case 'I':
            if (isConnected)   // request calls are illegal if not connected!
            {
                StationAPIIP clientip = 0x12345678; // original client IP number, for tracking purposes
                nn = pSapi->RequestLogin(SAPI_REQUEST_LOGIN_MULTIPLE,"invalidtest", "invalidtest", clientip, MY_PRODUCT);
                printf("Track# %x: Invalid Login Request submitted\n", nn);
            }
            else
                printf("Must connect to issue requests!\n");
            break;

        case 'L':
            if (isConnected)   // request calls are illegal if not connected!
            {
                StationAPIIP clientip = 0x12345678; // original client IP number, for tracking purposes
                nn = pSapi->RequestLogin(SAPI_REQUEST_LOGIN_MULTIPLE,"verantdemo", "terman", clientip, MY_PRODUCT); 
                printf("Track# %x: Valid Login Request submitted\n", nn);
            }
            else
                printf("Must connect to issue requests!\n");
            break;

        case 'P':
            isProcess = !isProcess;
            pSapi->SetProcess(isProcess);
            printf("Process flag toggled to %d\n",isProcess);
            break;

        case 'S':
            printf("Process flag = %d\n",isProcess);
            printf("IsConnected = %d, IsConnected() = %d\n",isConnected, pSapi->IsConnected()); // 2nd is whether api is connected to server
            printf("Requests Queued = %d\n",pSapi->GetRequestsQueued());
            printf("Requests Waiting = %d\n",pSapi->GetRequestsWaiting());
            break;

        case 'V':
            if (isConnected)   // request calls are illegal if not connected!
            {
                StationAPIIP clientip = 0x12345678; // client IP number
                nn = pSapi->RequestSession(SAPI_REQUEST_SESSION_VALIDATE,&lastsession);
                printf("Track# %x: Validating last sessionid (%s) submitted\n", nn, lastsession.GetString());
            }
            else
                printf("Must connect to issue requests!\n");
            break;

        case 'X':
            if (isConnected)   // request calls are illegal if not connected!
            {
                StationAPIIP clientip = 0x12345678; // client IP number
                nn = pSapi->RequestSession(SAPI_REQUEST_SESSION_LOGOUT, &lastsession);
                printf("Track# %x: Logging out last sessionid (%s) submitted\n", nn, lastsession.GetString());
            }
            else
                printf("Must connect to issue requests!\n");
            break;

		case 'Q':	
            return false;
		}
	}
    else
    {
        sleep_ms(50);
    }

	return true;
}

void main(void)
{
    uthread_t inputthread;

    uthread_init(&inputthread);
    uthread_create(&inputthread, KeyCheck, NULL);

    pSapi = new MyStationAPI(); // declare an instance

    printf("Ready -- H for help\n");
    while(ProcessConsoleKey())
    {
        // nothing here
    }
    // cleanup
    pSapi->Disconnect();

    delete pSapi;

    uthread_destroy(&inputthread);
}
