#ifndef __MONITOR_DATA__
#define __MONITOR_DATA__

// **********************************************************************************************
// **********************************************************************************************
// **********************************  DO NOT EDIT THIS FILE  ***********************************
// **********************************************************************************************
// **********************************************************************************************
//#include <UdpHandler.hpp>
#include "../../udplibrary/UdpLibrary.hpp"

#define CURRENT_API_VERSION	 3
#define CURRENT_API_1		 1
#define CURRENT_API_2		 2
#define CURRENT_API_3		 3

#define MON_HISTORY 		4		// This will ping every 30 sec. and report max of 5 min.
#define MON_ONLY_SHOW		5		// This will not store data. WILL NOT GRAPH

#define STATUS_LOCKED		-1		// Server is locking out players
#define STATUS_DOWN			-2		// Server is down.
#define STATUS_NOCONNECT	-3		// Server is not connected
#define STATUS_LOADING		-4		// Server is  in  a loading state

#define ELEMENT_MAX_START	5000


/* 
	Provide binary data packing and unpacking functions

Usage
	Format strings are as follows:
	'c' - char (1)
	'l' - long int (4)
	'L' - long long int (8)
	'i' - int (4)
	's' - short int (2)
	'S' - C-style, null-terminated string (n + null)
	'Bn' - buffer, of size n.  Used for non-terminated strings, or
	       other binary data.  
*/

extern int packString(char *buffer, int & len, char * value);
extern int packByte(char *buffer, int & len, char value);
extern int packShort(char *buffer, int & len, short value);
extern int unpackShort(char *buffer, int & len, short & value);
extern int unpackShort(char *buffer, int & len, unsigned short & value);
extern int unpackByte( char *buffer, int & len, char & value);
 

//----------------------------------------------------------------
typedef unsigned char byte;

//----------------------------------------------------------------
// Define the message types that are understood via the Monitoring API
enum MON_MESSAGES
{
	MON_MSG_AUTH = 1,				// client sends auth info during connect request
	MON_MSG_AUTHREPLY,			// server responds to auth request
	MON_MSG_QUERY_ELEMENTS,		// client asking for element data
	MON_MSG_QUERY_HIERARCHY,	// client asking for hierarchy data
	MON_MSG_REPLY_ELEMENTS,		// server returns element data
	MON_MSG_REPLY_HIERARCHY,	// server returns element data
	MON_MSG_QUERY_DESCRIPTION, // client asking for Description of element
	MON_MSG_REPLY_DESCRIPTION, // server returns element Description of element
    MON_MSG_QUERY_HIERARCHY_BLOCK,	// client asking for hierarchy data
    MON_MSG_REPLY_HIERARCHY_BLOCK_BEGIN,
	MON_MSG_REPLY_HIERARCHY_BLOCK,	// server returns element data
    MON_MSG_REPLY_HIERARCHY_BLOCK_END,
	MON_MSG_ERROR = 99					// one of many errors, defined elsewhere
};

// ----------------------------------------------------------------
// define error codes that can be sent and received via MON_MSG_ERROR types 
enum MON_ERRORS 
{
	INVALID_HEADER = 10,					// could not read the header data reliably
	INVALID_SEQUENCE = 20,				// out of range or unknown sequence number
	INVALID_MSG_LENGTH = 30,			// message length field is incorrect
	INVALID_AUTH_REQUEST = 101,		// could not read the entire request
	INVALID_ELEMENT_REQUEST = 102,	// could not read the entire request
	INVALID_HIERARCHY_REQUEST = 103,	// could not read the entire request
	INVALID_AUTH_REPLY = 105,			// the data reply was incorrectly formatted
	INVALID_ELEMENT_REPLY = 106,		// the data reply was incorrectly formatted
	INVALID_HIERARCHY_REPLY = 107,	// the data reply was incorrectly formatted
	INVALID_ERROR_CODE = 108,			// an error was sent, but it isn't recognized
	INVALID_MSG_TYPE = 109,				// an invalid message type was sent
	INVALID_CONFIG_FILE = 110			// Error in reading config file
};

// ----------------------------------------------------------------
// monMessage
// The base message class for the Monitor API
// This message should not be used explicitly.  Use simpleMessage for basic tasks.
// monMessage does not contain the etx field as required by the monitoring protocol.

class monMessage
{
public:
	monMessage(short  command, short sequence, short size);
	monMessage(const unsigned char * source);
	monMessage(const monMessage &copy);
	monMessage();

	inline const unsigned short getCommand() const {return command;}
	inline const unsigned short getSequence() const {return sequence;}
	inline const unsigned short getSize() const {return size;}
    inline void setSize(unsigned short _size ) {size=_size;}

private:
	short command;
	short sequence;
	unsigned short size;
};

//----------------------------------------------------------------
// stringMessage
// This message type is used for passing messages that can consist of a NULL terminated string
class stringMessage : public monMessage
{
public:
	stringMessage(const unsigned short command, const unsigned short sequence, const unsigned short size, char * data);
	stringMessage(const unsigned char * source);

	virtual ~stringMessage();

	inline const char * getData() const {return data;};

private:
	char * data;
};

//----------------------------------------------------------------
// authReplyMessage
// This message is used exclusively to tell the client whether or not they were successfully 
// authenticated.  This message is sent in reply to a MON_MSG_AUTH request.
class authReplyMessage : public monMessage
{
public:
	authReplyMessage(const unsigned short command, const unsigned short sequence, const unsigned short size, byte data);
	authReplyMessage(const unsigned char * source);

	inline const unsigned char getData() const {return data;};
	inline const short		 getVersion() const { return version; };
	inline void setData(const unsigned char newData) {data = newData;}

private:
	char data;
	short version;
};

//----------------------------------------------------------------
// dataReplyMessage
// This message type is sent for Element and Hierarchy data requests
// The data is stored and retrieved as a byte array, internally represented as a std::vector<byte>
class dataReplyMessage : public monMessage
{
public:
	dataReplyMessage(const unsigned short command, const unsigned short sequence, const unsigned short size, unsigned char *data, int dataLen);
	dataReplyMessage(const unsigned char * source);

	virtual ~dataReplyMessage();

	inline const unsigned char * getData() const {return data;};
	
private:
	unsigned char * data;
};


//----------------------------------------------------------------
// dataReplyMessage
// This message type is sent for Element and Hierarchy data requests
// The data is stored and retrieved as a byte array, internally represented as a std::vector<byte>
class dataBlockReplyMessage : public monMessage
{
public:
	dataBlockReplyMessage(const unsigned char * source);
	virtual ~dataBlockReplyMessage();
	inline const unsigned char * getData() const {return data;};
	
private:
	unsigned char * data;
};


//----------------------------------------------------------------
// simpleMessage
// This is the most basic monitor API message.  It can be created from an 
// Archive::ByteStream in order to discover the message type for more complex messages,
// or it could be sent explicitly for query message types.
class simpleMessage : public monMessage
{
public:
	simpleMessage(const unsigned short command, const unsigned short sequence, const unsigned short size);
	simpleMessage(const unsigned char * source);

	virtual ~simpleMessage();

private:
};

// ***************  Internal Struct  ***************************
typedef struct {

	int		value;		// Value of the element
	int		id;			// Id for users, user control, could be any value, must be unique
	int		ping;		// Ping time, 	use one of the API defined values, MON_PING_15,...
	char	*label;		// Label of element, use periods to make sub labels, last name sperated 
						// by periods is the name of the element.
						// 
						// Example:   "Universe.Planet.Zone1.count"
						//  Universe.Planet.Zone1 = Tree name 
						//  count = element name
						// 
						// NOTE: This label is the name of the data element, change the label name
						//       and a new history of data is started. You will not be able to change
						//			the name at run time.
						//    
	char *discription;  // Discription of element. This can be changed at will.
	

	long ChangedTime;

}  MON_ELEMENT;

/*
class GenericNotifier
{
public:
        GenericNotifier(){}
        virtual ~GenericNotifier(){}
	virtual void dataReported(int  id ){}
};

*/
class CMonitorData {
	
	MON_ELEMENT			*m_data;			// Pointer to MON_DATA elements
	int					m_ndata;
	int					m_count;
	int					m_max;
	char				*m_buffer;
	int					m_nbuffer;
	short				m_sequence;
//	GenericNotifier     *m_notifier;

	int parseList( char **list, char *data, char tok , int max );
	void resize_buffer(int new_size);
	void send( UdpConnection *mConnection, short & sequence, short msg, char *data );
	void send( UdpConnection *mConnection, short & sequence, short msg, char *data , int size);

public:

//	CMonitorData(GenericNotifier *notifier=NULL );
	CMonitorData();
	virtual ~CMonitorData();

	int getCount(){ return m_ndata; }
	int DataMax(){ return m_max; }
    void setMax(int _max);
								 
	// bool processHierarchyRequest(UdpConnection *con, short & sequence);
    bool processHierarchyRequestBlock(UdpConnection *con, short & sequence);
	bool processElementsRequest( UdpConnection *con, short & sequence, char * userData, int dataLen , long lastUpdateTime);
	bool processDescriptionRequest(UdpConnection *con, short & sequence, char * userData, int dataLen,unsigned char *mark);
	int add(const char *label, int id, int ping, const char *des );
	int  setDescription( int Id, const char *Description , int & mode);
	char *getDescription(int x){ if( m_ndata >= x ) return NULL; return m_data[x].discription;}
	int  pingValue(int p);
	void set(int Id, int value);
	void remove(int Id);
	void dump();

    // int compress(char *dest,unsigned long *destLen,const char *source,long sourceLen,int level);

};

#define AUTHADDRESS_MAX			30
#define CONNECTION_MAX			8


class MonitorObject : public UdpConnectionHandler
{
	CMonitorData  *mMonitorData;
	short mSequence;
	char *mPasswd;
	char **mAddressList;
	unsigned char *mMark;
	int  mbAuthenticated;
	bool mHierarchySent;
	bool mbprint;
	long mlastUpdateTime;
	bool checkAddress(const char * address);
	bool checkPasswd( const char *passwd);

	public:

	UdpConnection *mConnection;
	MonitorObject(UdpConnection *con, CMonitorData *, char *passwd, char **addressList ,bool _bprint );
	virtual ~MonitorObject();
	virtual void OnRoutePacket( UdpConnection *con, const unsigned char *data, int dataLen);
	virtual void OnTerminated( UdpConnection *con);
	bool processAuthRequest(const unsigned char * data, int dataLen);
	bool processError(const unsigned char * data);
	void HeirarchyChanged(){ mHierarchySent = false; }
	void DescriptionMark(int x,int mode);
};


class MonitorManager : public UdpManagerHandler
{
	bool	mbprint;
	CMonitorData *mMonitorData;
	char *passString;
	char *allowedAddressList[AUTHADDRESS_MAX];
	bool loadAuthData(const char * filename);

public:

	MonitorManager(const char *configFile, CMonitorData *gamedata, UdpManager *manager , bool _bprint = false);
	virtual ~MonitorManager();

	void AddObject(MonitorObject *player);
	void GiveTime();
	void HierarchyChanged();
	void DescriptionMark(int x,int mode);
	virtual void OnConnectRequest(UdpConnection *con);

protected:

	MonitorObject *mObject[CONNECTION_MAX];
	int mObjectCount;
	UdpManager *mManager;
};



#endif



