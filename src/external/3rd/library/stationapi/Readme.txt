02/28/2001

* All UID references are now 32-bit values, rather than 64-bit values.
* RequestLogin() now expects a StationAPISession pointer for a parent session argument.  This should be NULL if there is no parent.
* RequestSession() now expects a StationAPISession pointer for the session argument.  NULL is not legal.

11/16/2000

* Pulled all the globals into the api object.  Haven't actually tried multiple instances of api yet.
* Created a object wrapper for crypt.  Now using CCrypt objects instead of crypt functions using globals.
* Fixes to PackClass from Dave.  Now using PackClass instead of pack functions using globals.
* Destroying and recreating api object no longer resets local tracking number to 0.

11/06/2000

* Bug fixes.

10/30/2000

* API is now returning requests with timeout if server fails to reply.
* IsConnected() returns true only if api is connected and the server is connected.

10/26/2000

* rdp_fast_malloc_init/destroy is now done with a reference system, so you can now do your own beforehand if you need to (Dave!).

10/24/2000

* Default connection is now sdinf3.station.sony.com port 5997 by default.  Server should be running there most of the the time now.
  The RDP library is now the communications between the api and server.  rdp.lib is now used actively.
  Login packets have an extra encryption layer between api and server.  Demo now links in 989crypt.lib.
  tsip.lib is now handled on the server side and should no longer be linked in with the api.

* RequestLogin now distinguishes between validation and login.  Server sessions are maintained if you log in.  Exclusive login still isn't.
  RequestSession now allows you to logout, touch or validate sessions actively. For Validate, it also returns the UID on success.
  Server does not yet time out sessions, but it does track session time and identity.

* User pointer added to all RequestX() and OnX() calls.  Also ability to cancel a request and never get a callback from it.  
  Pointer value is strictly passthrough, validity is up to the caller.  User pointer has not beeen actively tested yet, but should work.

* The demo has more test items.  Not very useful, but enough to do basic function testing and provide code snippets.

* StationAPIApplication values added.  Many other minor additions.

* StationAPISessionID is now typed as a class rather than a uint.  A string, for all intents and purposes.


10/04/2000 esh

* version string added to stationapi.h

* Most arguments are now typed, rather than using generic types.

* I had RequestLogon() and OnLogin().  Renamed RequestLogon to RequestLogin for consistancy.

* RequestLogin has more arguments now -- ip, product, and parentsession.  
  It's intended that the clients IP address be passed for tracking purposes.  If there is no client, pass the servers IP instead.
  Product is the product associated with the session, since it's become apparent that a single server may deal with multiple products.
  Parent session is used when related sessions are created without closing the original one.  Pass 0 if not needed.

* Many minor changes to items not yet in use.

9/28/2000 esh

* linux Makefile added for stationapi.  move tsip directory up two levels for linux

* minor changes for linux portability in demo

* memory free bug fixed

* internal declarations in stationapi.h have been moved to another H file.

* Activated SAPI_REQUEST_LOGIN_EXCLUSIVE and SAPI_REQUEST_LOGIN_MULTIPLE.  
Currently same function as SAPI_REQUEST_LOGIN_VALIDATE except that they return a random sessionid value.
Exclusive is not, but can be coded.

*SAPI_REQUEST_SESSION_LOGOUT, SAPI_REQUEST_SESSION_TOUCH, SAPI_REQUEST_SESSION_VALIDATE return success when called.
This way the function can be coded, even if the functionality isn't present yet.


9/26/2000 esh

Requirements:
	Visual C++ 6.0

	install RDP directory as C:/RDP  (or modify project include and library paths appropriately)
	project directory can go anywhere.

	Permissions for server to connect to sip.station.sony.com port 7268 must be arranged with operations.
	Private routing is preferred, as SIP has no other security.  Telnet (or chkaccess below) can be used to validate.

	Currently Windows, but linux port soon.

Projects in stationapi workspace:

	stationapidemo:
	Demo project using station API.  Look at stationapidemo.cpp and stationapi.h.

	stationapi:
	API library to talk to the station tracking server via RDP.  
	First pass, it talks to the older tsip api instead.  
	Only the ability to validate station logins is implemented.

	tsip:
	Older API library to talk to station SIP servers.

	chkaccess:
	tsip demo program -- command line utility to check name/password against a SIP server

	testaccess:
	another tsip demo program -- ignore

Questions?:

	Eric Hagstrom
	ehagstrom@verant.com
	858-577-3123
