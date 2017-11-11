#include "clientGame/FirstClientGame.h"
#include "HTTPPoster.h"
#include "TCPConnection.h"
#include <time.h>
#include <stdio.h>

#if defined(WIN32) && !defined(snprintf)
# define snprintf _snprintf
#endif

HTTPPoster::HTTPPoster()
: m_state( STATE_IDLE ),
  m_uTCPConnectionHandle( 0 ),
  m_pTCPConnection( NULL ),
  m_iLastPostTime( 0 )
{
    m_cleanup();
}

HTTPPoster::~HTTPPoster()
{
    m_cleanup();
}

void HTTPPoster::m_cleanup( const bool kbClearLastResult )
{
    m_state = STATE_IDLE;

    m_uTCPConnectionHandle = 0;
    if ( m_pTCPConnection )
    {
        m_pTCPConnection->close( true );
        delete m_pTCPConnection;
        m_pTCPConnection = NULL;
    }

    m_aHTTPReqData.clear();
    if ( kbClearLastResult )
    {
        m_sLastPostResult = "";
    }
    m_iLastPostTime = 0;
}

bool HTTPPoster::post( const std::string &ksHost, const uint16 ku16Port, const std::string &ksURI, const PostedText kaTextFields[], const uint32 kuNumTextFields, const PostedFile kaFiles[], const uint32 kuNumFiles)
{
    // fail if in the middle of another post
    if ( isPosting() )
        return false;

    // cleanup any stale state from last post attempt
    m_cleanup();

    // build output data
    m_buildHTTPReqData( ksURI, kaTextFields, kuNumTextFields, kaFiles, kuNumFiles );

    // start establishing a connection
    bool bRetval = m_TCPManager.connect( ksHost.c_str(), ku16Port, &m_uTCPConnectionHandle);
    if ( !bRetval )
    {
        m_cleanup();
        return false;
    }

    // update state
    m_state = STATE_CONNECTING;
    m_iLastPostTime = static_cast<int32>(time(NULL));
    return true;
}

bool HTTPPoster::isPosting() const
{
    return ( m_state != STATE_IDLE );
}



void HTTPPoster::cancelPost()
{
    m_cleanup();
}



int HTTPPoster::finishPost( const int32 kiTimeout )
{
    if ( !isPosting() )
        return POST_GENERAL_ERROR;

    // check timeout
    if ( 0 <= kiTimeout && ( m_iLastPostTime + kiTimeout < time(NULL) ) )
    {
        m_cleanup();
        return POST_TIMEOUT;
    }


    int result = POST_GENERAL_ERROR;

    switch ( m_state )
    {
    case STATE_CONNECTING:
        {
            //finish connecting
            if ( m_TCPManager.getOutgoingConnection( &m_pTCPConnection, m_uTCPConnectionHandle, 0 ) )
            {
                if ( !m_pTCPConnection )
                {
                    m_cleanup();
                    result = POST_GENERAL_ERROR;
                }
                else
                {
                    m_TCPManager.setBufferedWrites( m_pTCPConnection, true );

                    // when got connection, return more work needed, set state to submitting
                    result = POST_NEEDS_TIME;
                    m_state = STATE_SUBMITTING;
                }
            }
            else
            {
                //couldn't finish yet
                result = POST_NEEDS_TIME;
            }
        }
        break;

    case STATE_SUBMITTING:
        {
            //write data

            if ( !m_pTCPConnection || !m_pTCPConnection->isConnected() )
            {
                m_cleanup();
                result = POST_GENERAL_ERROR;
            }
            else
            {
                if ( 0 < m_aHTTPReqData.size() )
                {
                    const int kiWriteLen = m_pTCPConnection->write( &(m_aHTTPReqData[0]), m_aHTTPReqData.size(), 0 );
                    if ( m_aHTTPReqData.size() != (size_t)kiWriteLen )
                    {
                        m_cleanup();
                        result = POST_GENERAL_ERROR;
                    }
                    else
                    {
                        m_aHTTPReqData.clear();
                    }
                }

                //data has been written if we got here, just need to pump it through

                //make sure there wasn't already an error
                if ( STATE_SUBMITTING == m_state )
                {
                    //if there's nothing left to send, go to next state
                    if ( 0 >= m_pTCPConnection->getWriteQueueLength() )
                    {
                        result = POST_NEEDS_TIME;
                        m_state = STATE_RECEIVING;
                    }
                    else
                    {
                        //try to send the rest
                        const int kiBytesSent = m_pTCPConnection->pumpWrites();

                        if ( 0 == kiBytesSent )
                        {
                            //if error, fail
                            m_cleanup();
                            result = POST_GENERAL_ERROR;
                        }
                        else if ( 0 >= m_pTCPConnection->getWriteQueueLength() )
                        {
                            //if there's nothing left to send, go to next state
                            result = POST_NEEDS_TIME;
                            m_state = STATE_RECEIVING;
                        }
                        else
                        {
                            //more to send, and no error - stay in same state, but need more time
                            result = POST_NEEDS_TIME;
                        }
                    }
                }
            }
        }
        break;

    case STATE_RECEIVING:
        {
            //recv data

            // we don't care if the connection is closed at this point - we sent our payload

            result = POST_NEEDS_TIME;

            m_pTCPConnection->queueIncoming();

            if ( 0 < m_pTCPConnection->getReadQueueLength() )
            {
                //got some data

                std::vector< uint8 > aTmpBuf;
                aTmpBuf.resize( m_pTCPConnection->getReadQueueLength() );
                
                m_pTCPConnection->read( &(aTmpBuf[0]), m_pTCPConnection->getReadQueueLength(), 0 );

                m_sLastPostResult.append( (const char *)&(aTmpBuf[0]), aTmpBuf.size() );
            }


            if ( !m_pTCPConnection->isConnected() )
            {
                //if we got to this point, there must have been data read already, or the connection was closed before we read all data
                // either way, treat it as a success, since we got our data sent
                m_cleanup( false );//keep result, since we succeeded
                result = POST_FINISH;
            }
        }
        break;
    default:
        result = POST_GENERAL_ERROR;
    }

    return result;
}


void HTTPPoster::m_buildHTTPReqData( const std::string &ksURI, const PostedText kaTextFields[], const uint32 kuNumTextFields, const PostedFile kaFiles[], const uint32 kuNumFiles )
{
    std::string entireMsg = "POST ";
    entireMsg += ksURI;
    entireMsg += " HTTP/1.0\r\n";

    if ( 0 < kuNumTextFields || 0 < kuNumFiles )
    {
        entireMsg += "Content-Type: multipart/form-data; boundary=AaB03x\r\n";

        std::string payload;

        uint32 i = 0;
        for ( i = 0; kuNumTextFields > i; ++i )
        {
            payload += "--AaB03x\r\n";
            payload += "Content-Disposition: form-data; name=\"";
            payload += kaTextFields[ i ].m_sName;
            payload += "\"\r\n\r\n";
            payload += kaTextFields[ i ].m_sValue;
            payload += "\r\n";

        }
        for ( i = 0; kuNumFiles > i; ++i )
        {
            if ( 0 >= kaFiles[ i ].m_aFileData.size() )
                continue;

            payload += "--AaB03x\r\n";
            payload += "Content-Disposition: form-data; name=\"";
            payload += kaFiles[ i ].m_sFieldName;
            payload += "\"; filename=\"";
            payload += kaFiles[ i ].m_sFileName;
            payload += "\"\r\nContent-Type: ";
            payload += kaFiles[ i ].m_sContentType;
            payload += "\r\n\r\n";
            payload.append( (const char *)&(kaFiles[ i ].m_aFileData[0]), kaFiles[ i ].m_aFileData.size() );
            payload += "\r\n";
        }
        payload += "--AaB03x--";


        char tmp[ 1024 ];
        snprintf( tmp, sizeof( tmp ), "Content-Length: %d\r\n\r\n", payload.length() );
        tmp[ sizeof( tmp ) - 1 ] = '\0';
        entireMsg += ( tmp );

        entireMsg += payload;
    }
    else
    {
        entireMsg += "\r\n";
    }

    m_aHTTPReqData.resize( entireMsg.length() );
    memcpy( (void*)&(m_aHTTPReqData[0]), entireMsg.data(), entireMsg.length() );
}



const std::string &HTTPPoster::getLastPostResult()
{
    //TODO:  RLS:  strip any http headers (up to and including first \n\n, or \r\n\r\n)
    uint32 uCharsToStripCount = 0;//0==strip nothing

    for ( unsigned i = 0; m_sLastPostResult.length() > i; ++i )
    {
        const uint32 kuRemainingLen = m_sLastPostResult.length() - i;

        //check for \n\n first

        //make sure there are at least as many characters left as we need to check
        if ( 2 <= kuRemainingLen )
        {
            if ( '\n' == m_sLastPostResult[ i ]
                 && '\n' == m_sLastPostResult[ i+1 ] )
            {
                //found end of headers
                uCharsToStripCount = i+2;
                break;
            }
        }

        //then check for \r\n\r\n

        //make sure there are at least as many characters left as we need to check
        if ( 4 <= kuRemainingLen )
        {
            if ( '\r' == m_sLastPostResult[ i ]
                 && '\n' == m_sLastPostResult[ i+1 ] 
                 && '\r' == m_sLastPostResult[ i+2 ]
                 && '\n' == m_sLastPostResult[ i+3 ] )
            {
                //found end of headers
                uCharsToStripCount = i+4;
                break;
            }
        }
    }

    if ( 0 < uCharsToStripCount )
    {
        //found end of headers, now strip it
        m_sLastPostResult.erase( 0 , uCharsToStripCount );
    }

    return m_sLastPostResult;
}
