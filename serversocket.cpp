#include "serversocket.h"

#ifdef __LINUX
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif


#define CSOCKET_BACKLOG 5

int CServerSocket::StartServerSocket()
{
    int nRet = CSocket::CreateSocket();
    if( nRet == CSOCKET_FAIL ) return CSOCKET_FAIL;

    nRet = bind( m_sock, (sockaddr*)&m_sockaddr, sizeof(m_sockaddr));
    if( nRet == SOCKET_ERROR )
    {
        return CSOCKET_FAIL;
    }

    nRet = listen( m_sock, CSOCKET_BACKLOG );

    if( nRet == SOCKET_ERROR )
    {
        return CSOCKET_FAIL;
    }

    FD_ZERO(&m_cltfds);
    FD_SET(m_sock, &m_cltfds);

#ifdef __LINUX
    m_maxfd = m_sock;
#endif

    return m_sock;
}

SOCKET CServerSocket::SelectSocket(int usec)
{
    m_oldfds = m_cltfds;

    int nRet;
    int nCount;
    SOCKET rcvsock;
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = usec;

#ifndef __LINUX
    nRet = select( NULL, &m_oldfds, NULL, NULL, &t);
    nCount = m_oldfds.fd_count;
#else
    nCount = m_maxfd+1;
    nRet = select( m_maxfd+1, &m_oldfds, NULL, NULL, &t);
#endif

    if( nRet < 0 )
    {
        return CSOCKET_FAIL;
    }
    else if ( nRet == 0 )
    {
        // timeout
        return CSOCKET_CONTINUE;
    }

    for( int i = 0 ; i < nCount ; i++ )
    {
#ifndef __LINUX        
        rcvsock =  m_oldfds.fd_array[i];        
#else
        rcvsock = i;        
#endif
        m_selectsock = rcvsock;

        if(FD_ISSET(rcvsock, &m_oldfds))
        {
            return rcvsock;
        }
    }

    return CSOCKET_CONTINUE;
}


SOCKET CServerSocket::AcceptSocket()
{    
    if( m_selectsock != m_sock) return CSOCKET_CONTINUE;
    if( m_selectsock <= 0 )     return CSOCKET_FAIL;

#ifndef __LINUX
    int nLen;
#else
    socklen_t nLen;
#endif
    SOCKET cltsock;
    sockaddr_in cltaddr;
    nLen = sizeof(sockaddr_in);
    cltsock = accept( m_sock, (sockaddr*)&cltaddr, &nLen);
    if( cltsock == INVALID_SOCKET )
    {
        return CSOCKET_FAIL;
    }
    FD_SET(cltsock, &m_cltfds);
#ifdef __LINUX
    if( cltsock > m_maxfd)  m_maxfd = cltsock;
#endif

    return cltsock;
}

int CServerSocket::RecvMsg( char* szMsg, int nLen )
{
    if ( m_selectsock == m_sock)    return CSOCKET_CONTINUE;
    if ( m_selectsock <= 0)         return CSOCKET_FAIL;

    int nMsgLen = 0;
    int nTmp = 0;

    memset( szMsg, 0x00, nLen );

    for( ;nMsgLen < nLen; )
    {
        nMsgLen = rcv( m_selectsock, szMsg+nTmp, nLen);
        if( nMsgLen <= 0)
        {
            #ifndef __LINUX
            if( GetLastError() == WSAETIMEDOUT )
#else
            if( errno == ETIMEDOUT )
#endif
            {
                return  CSOCKET_CONTINUE;

            }
            else
            {
                DisconnectSocket( m_selectsock );
                return CSOCKET_FAIL;
            }
        }
        else
        {
            nLen -= nMsgLen;
            nTmp += nMsgLen;
            nMsgLen = 0;
        }
    }
    return nTmp;

}

int CServerSocket::SendMsg(SOCKET cltsock, char* szMsg, int nLen)
{
    if ( cltsock == m_sock)
        return CSOCKET_CONTINUE;
    if ( cltsock <= 0)
        return CSOCKET_FAIL;

    int nMsgLen = 0;
    int nTmp = 0;

    for( ; nMsgLen < nLen ; )
    {
        nMsgLen = snd( cltsock, szMsg+nTmp, nLen );
        if( nMsgLen == SOCKET_ERROR )
        {
            return CSOCKET_FAIL;
        }
        else
        {
            nLen -= nMsgLen;
            nTmp += nMsgLen;
            nMsgLen = 0;
        }
    }
    return nTmp;

}
void CServerSocket::ShutdownSocket( SOCKET cltsock)
{
   shutdown( cltsock, SD_BOTH );
   FD_CLR( cltsock, &m_cltfds );
}
void CServerSocket::DisconnectSocket(SOCKET cltsock)
{
    closesocket( cltsock );
    FD_CLR( cltsock, &m_cltfds );
}

SOCKET CServerSocket::GetSelectSock()
{
    return m_selectsock;
}
