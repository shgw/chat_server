#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include "socket.h"

class CServerSocket : public CSocket
{
private:
    fd_set m_cltfds;
    fd_set m_oldfds;
    SOCKET m_selectsock;
#ifdef __LINUX
    int m_maxfd;
#endif

public:
    int StartServerSocket();
    SOCKET SelectSocket(int usec);
    SOCKET AcceptSocket();
    int RecvMsg( char* szMsg, int nLen);
    int SendMsg(SOCKET cltsock, char* szMsg, int nLen);
    void ShutdownSocket( SOCKET cltsock);
    void DisconnectSocket(SOCKET cltsock);
    SOCKET GetSelectSock();
};

#endif // SERVERSOCKET_H
