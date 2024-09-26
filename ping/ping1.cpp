#include "stdafx.h"
#include "ping1.h"
 
#pragma comment(lib, "ws2_32.lib")
 
bool CPing::Ping(LPCSTR pstrHost, UINT nRetries)
{
    //����һ��Raw�׽���
    SOCKET rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (rawSocket == INVALID_SOCKET)
    {
        int err = WSAGetLastError();
        return false;
    }
    int nNetTimeout = 1000;//1��
    //����ʱ��
    setsockopt(rawSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout,sizeof(int));
    //����ʱ��
    setsockopt(rawSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int));
 
    //���������Ϣ
    LPHOSTENT lpHost = gethostbyname(pstrHost);
    if (lpHost == NULL)
    {
        return false;
    }
     
    //����Ŀ���׽��ֵ�ַ��Ϣ
    struct    sockaddr_in saDest;
    struct    sockaddr_in saSrc;
    saDest.sin_addr.s_addr = *((u_long FAR *) (lpHost->h_addr));
    saDest.sin_family = AF_INET;
    saDest.sin_port = 3077;//0;
 
    DWORD    dwTimeSent;
    u_char   cTTL;
    int     nRet;
    int     nRecvNum = 0;
    int     nTotalTime = 0;
 
    //���ping
    for (UINT nLoop = 0; nLoop < nRetries; ++nLoop)
    {
        //����ICMP��Ӧ����
        if ((nRet = SendEchoRequest(rawSocket, &saDest)) < 0)
        {
            break;
        }
 
        if ((nRet = WaitForEchoReply(rawSocket)) == SOCKET_ERROR)
        {
            break;
        }
        if (nRet)
        {
            //��û�Ӧ
            if ( (dwTimeSent = RecvEchoReply(rawSocket, &saSrc, &cTTL)) < 0)
            {
                nRet = dwTimeSent;
                break;
            }
            //����ʱ��
            nTotalTime += GetTickCount() - dwTimeSent;
            //Sleep(1000);
            ++nRecvNum;
        }
    }
    closesocket(rawSocket);
    if (nRecvNum > 0 && nRet >= 0)
    {
        m_Result.nElapseTime = nTotalTime/nRetries;
        m_Result.cTTL = cTTL;
        m_Result.fMissPack = (float)(nRetries - nRecvNum)/nRetries;
        return true;
    }
 
    return false;
}
 
//����ICMPECHO���ݰ�����
int CPing::SendEchoRequest(SOCKET s,LPSOCKADDR_IN lpstToAddr)
{
    static ECHOREQUEST echoReq;
    static int nId = 1;
    static int nSeq = 1;
    int nRet;
 
    //�����Ӧ����
    echoReq.icmpHdr.Type        = ICMP_ECHOREQ;
    echoReq.icmpHdr.Code        = 0;
    echoReq.icmpHdr.Checksum    = 0;
    echoReq.icmpHdr.ID          = nId++;
    echoReq.icmpHdr.Seq         = nSeq++;
 
    for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
        echoReq.cData[nRet] = ' '+nRet;
 
    //���淢��ʱ��
    echoReq.dwTime  = GetTickCount();
 
    echoReq.icmpHdr.Checksum = in_cksum((u_short *)&echoReq, sizeof(ECHOREQUEST));
 
    //��������
    nRet = sendto(s,                       
                 (LPSTR)&echoReq,          
                 sizeof(ECHOREQUEST),
                 0,                        
                 (LPSOCKADDR)lpstToAddr,
                 sizeof(SOCKADDR_IN));  
    //��鷵��ֵ
    if (nRet == SOCKET_ERROR)
    {
    }
 
    return (nRet);
}
 
//����ICMPECHO���ݰ���Ӧ
DWORD CPing::RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char *pTTL)
{
    ECHOREPLY echoReply;
    int nRet;
    int nAddrLen = sizeof(struct sockaddr_in);
 
    //���������Ӧ
    nRet = recvfrom(s,                 
                    (LPSTR)&echoReply, 
                    sizeof(ECHOREPLY), 
                    0,                 
                    (LPSOCKADDR)lpsaFrom,
                    &nAddrLen);        
 
    //��鷵��ֵ
    if (nRet == SOCKET_ERROR)
    {
        return nRet;
    }
 
    //���ط��͵�ʱ��
    *pTTL = echoReply.ipHdr.TTL;
 
    return(echoReply.echoRequest.dwTime);          
}
 
//�ȴ���Ӧ
int CPing::WaitForEchoReply(SOCKET s)
{
    struct timeval Timeout;
    fd_set readfds;
 
    readfds.fd_count = 1;
    readfds.fd_array[0] = s;
    Timeout.tv_sec = 1;
    Timeout.tv_usec = 0;
 
    return(select(1, &readfds, NULL, NULL, &Timeout));
}
 
//ת����ַ
u_short CPing::in_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register u_short answer;
    register int sum = 0;
 
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }
 
    if( nleft == 1 ) {
        u_short u = 0;
 
        *(u_char *)(&u) = *(u_char *)w ;
        sum += u;
    }
 
    sum = (sum >> 16) + (sum & 0xffff);  
    sum += (sum >> 16);          
    answer = ~sum;             
    return (answer);
}
 
void CPing::Result(int* nElapseTime, float* fMissPack, u_char* cTTL)
{
    if (nElapseTime)
    {
        *nElapseTime = m_Result.nElapseTime;
    }
    if (fMissPack)
    {
        *fMissPack = m_Result.fMissPack;
    }
    if (cTTL)
    {
        *cTTL = m_Result.cTTL;
    }
}
