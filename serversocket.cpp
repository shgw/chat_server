#include "serversocket.h"

#ifdef __LINUX
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif


#define CSOCKET_BACKLOG 5

int CServerSocket::StartServerSocket()
{
#ifdef __LINUX
    m_epfd = epoll_create( MAX_EVENTS );
    if( m_epfd < 0 )
    {
        return CSOCKET_FAIL;
    }
#endif
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

#ifdef __LINUX
    struct epoll_event ev = { 0 };

    ev.events = EPOLLIN;
    ev.data.fd = m_sock;
    epoll_ctl( m_epfd, EPOLL_CTL_ADD, m_sock, &ev );
#else
    FD_ZERO(&m_cltfds);
    FD_SET(m_sock, &m_cltfds);
#endif

    return m_sock;
}

SOCKET CServerSocket::SelectSocket(int usec)
{
    int nRet;

#ifndef __LINUX
    m_oldfds = m_cltfds;

    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = usec;
    nRet = select( NULL, &m_oldfds, NULL, NULL, &t);
#else    
    nRet = epoll_wait( m_epfd, m_epEvent, MAX_EVENTS, usec );
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

#ifndef __LINUX        
    m_selectsock =  m_oldfds.fd_array[0];

    if(FD_ISSET(m_selectsock, &m_oldfds))
    {
        return m_selectsock;
    }
#else
    m_selectsock = m_epEvent[0].data.fd;
    return m_selectsock;
#endif

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

#ifdef __LINUX
    struct epoll_event ev = { 0 };
    ev.events = EPOLLIN;
    ev.data.fd = cltsock;
    epoll_ctl( m_epfd, EPOLL_CTL_ADD, cltsock, &ev );
#else
    FD_SET(cltsock, &m_cltfds);
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
#ifdef __LINUX
    epoll_ctl( m_epfd, EPOLL_CTL_DEL, cltsock, m_epEvent );
#else
    FD_CLR( cltsock, &m_cltfds );
#endif
}

SOCKET CServerSocket::GetSelectSock()
{
    return m_selectsock;
}
