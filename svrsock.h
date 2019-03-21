#ifndef SVRSOCK_H
#define SVRSOCK_H

#include <map>
#include <serversocket.h>
#include <log.h>
#include <string>
#include "socketmessage.h"


class SvrSock
{
private:
    CLog m_log;
    CServerSocket m_sock;
    std::map <SOCKET, std::string> m_cltMap;

    void SetLength( char* szSendMsg, char* szMsg, int nLen);

public:
    SvrSock();
    SvrSock( char* szIP, int nPort, int nRcvTimeOut, int nSndTimeOut);
    ~SvrSock();

    int Start();
    int Wait();
    int Accept();
    void DisconnectSock(SOCKET cltsock );
    int SendMsg(SOCKET cltsock, char* szMsg, int nMsgLen);
    int RecvMsg( char* szMsg );
    int ProcMsg( char* szMsg );
    int ProcRqst( ComMsg* szMsg );
    void ConnectClient( ComMsg* szMsg );
    void SendChat( ComMsg* szMsg );
    void MakeHeader( ComMsg* msg, const char* szSrc, const char* szDest, const char* szKind, const char* szId);
    SOCKET GetSock();
    SOCKET GetSelectSock();

};

#endif // SVRSOCK_H
