#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include "socket.h"
#ifdef __LINUX
#include <sys/epoll.h>
#endif

#define MAX_EVENTS  10

#define CSERVER_ACCEPT  1
#define CSERVER_RECV    2

class CServerSocket : public CSocket
{
private:
    fd_set m_cltfds;
    fd_set m_oldfds;
    SOCKET m_selectsock;
#ifdef __LINUX    
    struct epoll_event m_epEvent[MAX_EVENTS];
    int m_epfd;
    int m_maxfd;
#endif

public:
    int StartServerSocket();
    int SelectSocket(int usec);
    SOCKET AcceptSocket();
    int RecvMsg( char* szMsg, int nLen);
    int SendMsg(SOCKET cltsock, char* szMsg, int nLen);
    void ShutdownSocket( SOCKET cltsock);
    void DisconnectSocket(SOCKET cltsock);
    SOCKET GetSelectSock();
};

#endif // SERVERSOCKET_H
