#ifndef HTTPPOSTER_H
#define HTTPPOSTER_H

#include "TCPManager.h"
#include <vector>

/*
USAGE: 
{
    // setup a new post if not already posting
    if ( "have something to post" && !m_HTTPPoster.isPosting() )
    {
        if ( m_HTTPPoster.post( ...... whatever .... ) )
        {
            // it's started
        }
    }

    if ( m_HTTPPoster.isPosting() )
    {
        // pump it until done
        switch( m_HTTPPoster.finishPost( cl_http_post_timeout ) )
        {
        case POST_FINISH:
            {
                std::string sOutput = m_HTTPPoster.getLastPostResult();
                //handle response here
            }
            break;
        case POST_NEEDS_TIME:
            //still might complete later
            break;
        default:
            {
                //handle fatal error
            }
            break;
        }
    }
}
*/
class HTTPPoster
{
public:
    enum PostResult
    {
        POST_FINISH = 1,
        POST_NEEDS_TIME = 0,
        POST_GENERAL_ERROR = -1,
        POST_TIMEOUT = -2
    };

    struct PostedFile
    {
		std::vector< uint8 > m_aFileData; 
        std::string m_sFieldName;//form field name
		std::string m_sFileName;//name of file
		std::string m_sContentType;
    };

    struct PostedText
    {
        std::string m_sName;
        std::string m_sValue;
    };

    HTTPPoster();
    virtual ~HTTPPoster();

    // returns false if already posting something - but can be reused once the previous post finishes (or if you cancelPost() yourself)
    bool post( const std::string &ksHost, const uint16 ku16Port, const std::string &ksURI, const PostedText kaTextFields[], const uint32 kuNumTextFields, const PostedFile kaFiles[], const uint32 kuNumFiles);

    // returns true if already trying to post something, and it hasn't timed out yet
    bool isPosting() const;

    // reset, as if post was never called
    void cancelPost();


    /* tries to finish last post started, 
            if timeout == negative, never test for timeout
            else if timeout seconds has passed since post was began then it cleans itself up and returns a failure code
       return: PostResult enum value

       note:  kiTimeout is counted from teh time the post is submitted, not just finishPost
    */
    int finishPost( const int32 kiTimeout );

    // returns last text returned by post - minus any html headers.  empty if no results.
    const std::string &getLastPostResult();


private:
    enum eState
    {
        STATE_IDLE,
        STATE_CONNECTING,
        STATE_SUBMITTING,
        STATE_RECEIVING
    };

    eState m_state;

    TCPManager m_TCPManager;
    uint32 m_uTCPConnectionHandle;
    TCPConnection  *m_pTCPConnection;

    std::vector< uint8 > m_aHTTPReqData;
    std::string m_sLastPostResult;
    int32 m_iLastPostTime;


    void m_cleanup( const bool kbClearLastResult = true );
    void m_buildHTTPReqData( const std::string &ksURI, const PostedText kaTextFields[], const uint32 kuNumTextFields, const PostedFile kaFiles[], const uint32 kuNumFiles );
};

#endif //HTTPPOSTER_H
