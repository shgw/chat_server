#include <iostream>
#include "svrsock.h"
#include <string.h>
#ifdef __LINUX
#include <unistd.h>
#define Sleep usleep
#define SLEEP_TIME  1000
#else
#define SLEEP_TIME  1
#endif

using namespace std;

int main()
{
#ifndef __LINUX
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    char szBuff[MSG_FULL_LENGTH] = { 0 };
    SvrSock sock;
    int nRet;

    nRet = sock.Start();
    if( nRet <= 0 )
    {
        cout << "Socket start fail." << endl;
        return -1;
    }
    while(1)
    {
        nRet = sock.Wait();
        switch (nRet )
        {
        case CSERVER_ACCEPT:
            nRet = sock.Accept();
            if( nRet != CSOCKET_SUCC)
            {
                cout << "SOCKET ACCEPT ERROR [" << GetLastError() << "]" << endl;
            }
            break;

        case CSERVER_RECV:
            memset( szBuff, 0x00, sizeof( szBuff ));
            nRet = sock.RecvMsg( szBuff );
            if( nRet == CSOCKET_CONTINUE)
            {
                break;
            }
            else if ( nRet == CSOCKET_SUCC)
            {
                sock.ProcMsg( szBuff );

            }
            else
            {
                sock.DisconnectSock(sock.GetSelectSock());
            }
            break;

        case CSOCKET_FAIL:
            cout << "SOCKET ERROR [" << GetLastError() << "]" << endl;
            return -1;

        case CSOCKET_CONTINUE:
            break;

        default:
            break;

        }

        Sleep(SLEEP_TIME);
    }

    return 0;
}
