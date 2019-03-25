#include "svrsock.h"
#include "socketmessage.h"
#include <string.h>

#define SELECT_TIMEOUT  1000
#define MSG_MAX_BUFF    2048


SvrSock::SvrSock()
{
    m_log.SetOption( LOG_LEVEL_DETAIL, "./", "ServerSocket.txt");
    m_sock.SetSocketEx( "0.0.0.0", 6788, 100, 100 );
    m_log.WriteLog( LOG_LEVEL_NOTICE, "==========SERVER START============");
}


SvrSock::SvrSock( char* szIP, int nPort, int nRcvTimeOut, int nSndTimeOut)
{
    m_log.SetOption( LOG_LEVEL_DETAIL, "./", "ServerSocket.txt");
    m_sock.SetSocketEx( szIP, nPort, nRcvTimeOut, nSndTimeOut );
    m_log.WriteLog( LOG_LEVEL_NOTICE, "==========SERVER START============");
}

SvrSock::~SvrSock()
{

}

int SvrSock::Start()
{
    int nRet = m_sock.StartServerSocket();
    if( nRet <= 0 )
    {
        m_log.WriteLog(LOG_LEVEL_ERROR, "Start() : Socket start fail. Error[%d]", GetLastError());
    }
    else
    {
        m_log.WriteLog(LOG_LEVEL_NORMAL, "Start() : Socket start success");
    }

    return nRet;
}

int SvrSock::Wait()
{
    int nRet = m_sock.SelectSocket(SELECT_TIMEOUT);

    if ( nRet == CSOCKET_CONTINUE )
    {
        return CSOCKET_CONTINUE;
    }
    else if( nRet == CSOCKET_FAIL )
    {
        return CSOCKET_FAIL;
    }
    else
    {
        return nRet;
    }
}


int SvrSock::Accept()
{
    int nRet = m_sock.AcceptSocket();

    if( nRet == CSOCKET_CONTINUE )
    {
        return CSOCKET_CONTINUE;
    }
    else if ( nRet == CSOCKET_FAIL )
    {
        m_log.WriteLog( LOG_LEVEL_ERROR, "Accept fail  error[%d]", GetLastError() );
        return CSOCKET_FAIL;
    }
    else
    {
        m_log.WriteLog( LOG_LEVEL_NORMAL, "Accept Success cltsock[%d]", nRet);
        return CSOCKET_SUCC;
    }
}

void SvrSock::DisconnectSock( SOCKET cltsock )
{
    std::map<SOCKET, std::string>::iterator iter = m_cltMap.find( cltsock );
    if( iter == m_cltMap.end() )
    {
        m_log.WriteLog( LOG_LEVEL_NORMAL, "nonreg clt disconnect cltsock[%d]", cltsock);
        m_sock.DisconnectSocket( cltsock );
        return;
    }

    std::map<SOCKET, std::string>::iterator it = m_cltMap.begin();
    std::map<SOCKET, std::string>::iterator end = m_cltMap.end();

    //create msg
    ComMsg msg = { 0 };
    strcpy( msg.src, SERVER_NAME );
    strcpy( msg.msgkind, MSGKIND_EVT );
    strcpy( msg.msgid, MSGID_DISCON );

    EVT_DICON* body = (EVT_DICON*)msg.body;
    strcpy( body->username, m_cltMap[cltsock].c_str() );

    int nMsgLen = HEADER_SIZE;
    nMsgLen += strlen( body->username );

    //memcpy( msg.body, (char*)body, strlen( body.username ));

    int nRet = 0;


    for ( ; it != end ; it++)
    {
        //SEND ALL CLIENT
        if (it->first == cltsock)   continue;
        nRet = SendMsg( it->first, (char*)&msg, nMsgLen );
        if( nRet == CSOCKET_FAIL)
        {
            //실패처리
            m_log.WriteLog(LOG_LEVEL_ERROR, "DisconnectSock : Send Error [%d] clt[%d] username[%s]", GetLastError(), it->first, it->second.c_str());
            m_log.WriteHEX(LOG_LEVEL_DETAIL, (char*)&msg, nMsgLen);
        }

    }

    m_log.WriteLog( LOG_LEVEL_NORMAL, "client disconnect cltsock[%d] username[%s]", cltsock, body->username);
    m_cltMap.erase( cltsock );
    m_sock.DisconnectSocket( cltsock );
}

int SvrSock::SendMsg(SOCKET cltsock, char* szMsg, int nMsgLen)
{
    // set len
    char szSendMsg[MSG_MAX_BUFF] = { 0 };
    SetLength( szSendMsg, szMsg, nMsgLen);

    int nRet = m_sock.SendMsg( cltsock, szSendMsg, nMsgLen + MSG_LENGTH );

    if ( nRet == CSOCKET_FAIL )
    {
        m_log.WriteLog(LOG_LEVEL_ERROR, "SendMsg : send fail error[%d] cltsock[%d]", GetLastError(), cltsock );
        m_log.WriteHEX(LOG_LEVEL_DETAIL, szSendMsg, nMsgLen + MSG_LENGTH );
        return CSOCKET_FAIL;
    }
    else
    {
        m_log.WriteLog(LOG_LEVEL_NORMAL, "SendMsg : send success cltsock[%d]", cltsock );
        m_log.WriteHEX(LOG_LEVEL_DETAIL, szSendMsg, nMsgLen + MSG_LENGTH );
        return CSOCKET_SUCC;
    }

}

int SvrSock::RecvMsg( char* szMsg )
{
    int nRet = m_sock.RecvMsg( szMsg, MSG_LENGTH );

    if( nRet > 0 )
    {
        //길이처리
        int nSize = atoi( szMsg );

        if( nSize >  0 )
        {
            m_log.WriteLog(LOG_LEVEL_DETAIL, "RecvMsg : recv length cltsock[%d] size[%d]", m_sock.GetSelectSock(), nSize );

            if( nSize > MSG_FULL_LENGTH )
            {
                m_log.WriteLog( LOG_LEVEL_WARN, "RecvMsg : too long length cltsock[%d] size[%d] limit[%d]", m_sock.GetSelectSock(), nSize, MSG_FULL_LENGTH);
                return CSOCKET_FAIL;
            }

            nRet = m_sock.RecvMsg( szMsg, nSize );
            if( nRet > 0)
            {
                // 성공
                m_log.WriteLog(LOG_LEVEL_NORMAL, "RecvMsg : recv success cltsock[%d]", m_sock.GetSelectSock() );
                m_log.WriteHEX(LOG_LEVEL_DETAIL, szMsg, nSize );
                return CSOCKET_SUCC;

            }
            else
            {
                // 실패처리
                m_log.WriteLog(LOG_LEVEL_ERROR, "RecvMsg : recv fail error[%d] cltsock[%d]", GetLastError(), m_sock.GetSelectSock() );
                return CSOCKET_FAIL;
            }

        }        

        else
        {
            //실패처리
            m_log.WriteLog(LOG_LEVEL_ERROR, "RecvMsg : wrong length cltsock[%d] Size[%d]", m_sock.GetSelectSock(), nSize );
            //DisconnectSock( m_sock.GetSelectSock() );
            return CSOCKET_FAIL;
        }
    }
    else if( nRet == CSOCKET_CONTINUE)
    {
        return CSOCKET_CONTINUE;

    }
    else
    {
        //실패처리
        m_log.WriteLog(LOG_LEVEL_ERROR, "RecvMsg : recv length fail error[%d] cltsock[%d]", GetLastError(), m_sock.GetSelectSock() );
        //DisconnectSock( m_sock.GetSelectSock() );
        return CSOCKET_FAIL;
    }
}


int SvrSock::ProcMsg( char* szMsg )
{
    ComMsg* msg = (ComMsg*)szMsg;

    if( strcmp( msg->msgkind, MSGKIND_RQST ) == 0)
    {
        ProcRqst( msg );
    }
    else
    {
        //오류처리
        m_log.WriteLog(LOG_LEVEL_WARN, "ProcMsg : unknown msgkind[%s]", msg->msgkind );
        DisconnectSock( m_sock.GetSelectSock());
    }

}

int SvrSock::ProcRqst( ComMsg* szMsg )
{
    if( strcmp( szMsg->msgid, MSGID_CONN ) == 0)
    {
        ConnectClient( szMsg );

    }
    else if( strcmp( szMsg->msgid, MSGID_SENDMSG ) == 0)
    {
        SendChat( szMsg );
    }
    else
    {
        //실패처리
        m_log.WriteLog(LOG_LEVEL_WARN, "ProcMsg : unknown msgid[%s]", szMsg->msgid );
    }

}

void SvrSock::ConnectClient( ComMsg* szMsg )
{
    std::string strUserName = szMsg->src;

    std::map<SOCKET,std::string>::iterator it =  m_cltMap.begin();
    std::map<SOCKET,std::string>::iterator end =  m_cltMap.end();


    ComMsg msg;
    MakeHeader( &msg, SERVER_NAME, szMsg->src, MSGKIND_RESP, MSGID_CONN );

    BODY_RESP_CONN body = {0};

    for( ; it != end ; it++ )
    {
        if( it->second.compare( strUserName ) == 0 )
        {
            // 겹치는 아이디있음

            strcpy( body.result, MSG_RESULT_FAIL );
            strcpy( body.reason, MSG_REASON_SN);

            memcpy( msg.body, &body, sizeof( body.result ) + strlen( body.reason ) );

            if( SendMsg( m_sock.GetSelectSock(), (char*)&msg, HEADER_SIZE+sizeof (body.result)+strlen(body.reason)) == CSOCKET_FAIL)
            {
                //실패처리
                m_log.WriteLog(LOG_LEVEL_ERROR, "ConnectClient : Send error [%d] clt[%d]", GetLastError(), m_sock.GetSelectSock() );
                m_log.WriteHEX(LOG_LEVEL_DETAIL, (char*)&msg, HEADER_SIZE+sizeof (body.result)+strlen(body.reason));
            }
            DisconnectSock( m_sock.GetSelectSock());
            return;
        }
    }

    strcpy( body.result, MSG_RESULT_SUCC );
    memcpy( msg.body, &body, sizeof(body));
    SOCKET cltSock = m_sock.GetSelectSock();
    if( SendMsg( cltSock, (char*)&msg, HEADER_SIZE+sizeof(body.result)) == CSOCKET_FAIL)
    {
        //실패처리
        m_log.WriteLog(LOG_LEVEL_ERROR, "ConnectClient : Send Error error[%d] clt[%d]", GetLastError(),it->first);
        m_log.WriteHEX(LOG_LEVEL_DETAIL, (char*)&msg, HEADER_SIZE+sizeof (body.result));
    }
    else
    {
        it =  m_cltMap.begin();
        for ( ; it != end ; it++ )
        {
            ComMsg evtMsg;
            MakeHeader( &evtMsg, SERVER_NAME, it->second.c_str(), MSGKIND_EVT, MSGID_CONN );
            EVT_CONN body = { 0 };
            strcpy( body.username, strUserName.c_str() );
            memcpy( evtMsg.body, &body, strUserName.length() );

            if( SendMsg( it->first, (char*)&evtMsg, HEADER_SIZE + strUserName.length() ) == CSOCKET_FAIL)
            {
                // 실패처리
                m_log.WriteLog(LOG_LEVEL_ERROR, "ConnectClient : Send error errorcode[%d] clt[%d]", GetLastError(), it->first );
                m_log.WriteHEX(LOG_LEVEL_DETAIL, (char*)&evtMsg, HEADER_SIZE + strUserName.length());
            }
        }

        //추가
        m_log.WriteLog(LOG_LEVEL_NORMAL, "ConnectClient : Add Client clt[%d] unsername[%s]", cltSock, strUserName.c_str() );
        m_cltMap[cltSock] = strUserName;
    }




}

void SvrSock::SendChat( ComMsg* szMsg )
{
    std::map<SOCKET,std::string>::iterator findCltName;



    ComMsg msg;
    MakeHeader( &msg, SERVER_NAME, szMsg->src, MSGKIND_RESP, MSGID_SENDMSG );

    RESP_SENDMSG body = {0};
    SOCKET cltsock = m_sock.GetSelectSock();
    findCltName = m_cltMap.find( cltsock );
    if ( findCltName == m_cltMap.end() )
    {
        strcpy( body.result, MSG_RESULT_FAIL );
        strcpy( body.reason, MSG_REASON_NR);

        memcpy( msg.body, &body, sizeof( body.result ) + strlen( body.reason ) );

        if( SendMsg( m_sock.GetSelectSock(), (char*)&msg, HEADER_SIZE+sizeof (body.result)+strlen(body.reason)) == CSOCKET_FAIL)
        {
            //실패처리
            m_log.WriteLog(LOG_LEVEL_ERROR, "SendChat : Send error [%d] clt[%d]", GetLastError(), m_sock.GetSelectSock() );
            m_log.WriteHEX(LOG_LEVEL_DETAIL, (char*)&msg, HEADER_SIZE + HEADER_SIZE+sizeof (body.result)+strlen(body.reason) );
        }
    }
    else
    {
        RQST_SENDMSG* rcvMsg = (RQST_SENDMSG*)szMsg->body;

        strcpy( body.result, MSG_RESULT_SUCC );
        strcpy( body.reason, rcvMsg->msg );

        memcpy( msg.body, &body, sizeof( body.result ) + strlen( body.reason ) );

        m_log.WriteLog( LOG_LEVEL_DETAIL, "RESP SEND MSG : result : [%s] reason[%s]", body.result, body.reason);

        if( SendMsg( m_sock.GetSelectSock(), (char*)&msg, HEADER_SIZE+sizeof (body.result)+strlen(body.reason)) == CSOCKET_FAIL)
        {
            //실패처리
            m_log.WriteLog(LOG_LEVEL_ERROR, "SendChat : Send error [%d] clt[%d]", GetLastError(), m_sock.GetSelectSock() );
            m_log.WriteHEX(LOG_LEVEL_DETAIL, (char*)&msg, HEADER_SIZE + sizeof (body.result)+strlen(body.reason) );
        }
        else
        {
            std::map<SOCKET,std::string>::iterator it =  m_cltMap.begin();
            std::map<SOCKET,std::string>::iterator end =  m_cltMap.end();
            ComMsg evtMsg;
            MakeHeader( &evtMsg, SERVER_NAME, it->second.c_str(), MSGKIND_EVT, MSGID_SENDMSG );
            EVT_SENDMSG body = { 0 };

            for ( ; it != end ; it++ )
            {
                if( it->first == m_sock.GetSelectSock() )
                    continue;

                strcpy( body.username, szMsg->src );
                strcpy( body.msg, rcvMsg->msg );

                memcpy( evtMsg.body, &body, sizeof(body.username) + strlen(rcvMsg->msg));

                if( SendMsg( it->first, (char*)&evtMsg, HEADER_SIZE + sizeof(body.username) + strlen(rcvMsg->msg) ) == CSOCKET_FAIL)
                {
                    // 실패처리
                    m_log.WriteLog(LOG_LEVEL_ERROR, "SendChat : Send error [%d] clt[%d]", GetLastError(), it->first );
                    m_log.WriteHEX(LOG_LEVEL_DETAIL, (char*)&evtMsg, HEADER_SIZE + sizeof(body.username) + strlen(rcvMsg->msg) );
                }
            }
        }

    }
}


void SvrSock::SetLength(char* szSendMsg, char* szMsg, int nLen)
{
    char szSize[MSG_LENGTH+1] = { 0 };
    sprintf( szSize, "%04d", nLen);
    memcpy( szSendMsg, szSize, MSG_LENGTH );
    memcpy( szSendMsg + MSG_LENGTH, szMsg, nLen );

}


void SvrSock::MakeHeader( ComMsg* msg, const char* szSrc, const char* szDest, const char* szKind, const char* szId)
{
    memset( msg, 0x00, sizeof( ComMsg ));
    strcpy( msg->src, szSrc );
    strcpy( msg->dest, szDest );
    strcpy( msg->msgkind, szKind);
    strcpy( msg->msgid, szId );
}


SOCKET SvrSock::GetSock()
{
    return m_sock.GetSock();
}

SOCKET SvrSock::GetSelectSock()
{
    return m_sock.GetSelectSock();

}








