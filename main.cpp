#include <iostream>
#include "svrsock.h"

using namespace std;

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    char szBuff[BODY_LENGTH] = { 0 };
    SvrSock sock;

    int nRet;
    while(1)
    {
        nRet = sock.Wait();
        if( nRet == CSOCKET_CONTINUE )
        {
            Sleep(1);
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
                    Sleep(1);
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
            Sleep(1000);
        }




        Sleep(1);
    }

    cout << "11111111111111111111" << endl;
    return 0;
}
