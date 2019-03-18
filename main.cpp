#include <iostream>
#include "svrsock.h"
#include <string.h>
#ifdef __LINUX
#include <unistd.h>
#endif

using namespace std;

int main()
{
#ifndef __LINUX
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    char szBuff[BODY_LENGTH] = { 0 };
    SvrSock sock;

    int nRet;
    while(1)
    {
        nRet = sock.Wait();
        if( nRet == CSOCKET_CONTINUE )
        {
#ifndef __LINUX
            Sleep(1);
#endif
            continue;
        }
        else if ( nRet > 0)
        {

            if( sock.GetSock() == (SOCKET)nRet )
            {
                nRet = sock.Accept();
                if( nRet == CSOCKET_SUCC)   continue;
            }
            else
            {
                memset( szBuff, 0x00, sizeof( szBuff ));
                nRet = sock.RecvMsg( szBuff );
                if( nRet == CSOCKET_CONTINUE)
                {
                    cout << "CSOCKET_CONTINUE" << endl;
#ifndef __LINUX
                    Sleep(1);
#endif
                    continue;
                }
                else if ( nRet == CSOCKET_SUCC)
                {
                    cout << "CSOCKET_SUCC" << szBuff << endl;
                    sock.ProcMsg( szBuff );

                }
            }

        }
        else
        {
            cout << "select fail" << GetLastError() << endl;
#ifndef __LINUX
            Sleep(1000);
#else
            sleep(1);
#endif
        }
#ifndef __LINUX
        Sleep(1);
#endif
    }

    cout << "11111111111111111111" << endl;
    return 0;
}
