// ping.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "ping1.h"

int main(int argc, char* argv[])
{
	WORD wVersionRequested;
    WSADATA wsaData;
    int err;
 
    wVersionRequested = MAKEWORD( 1, 1 );
 
    err = WSAStartup( wVersionRequested, &wsaData );
 
    CPing ping;
    bool bResult = false;
    if (argc == 2)
    {
        bResult = ping.Ping(argv[1]);
    }
    else
    {
        bResult = ping.Ping("www.baidu.com");
    }
     
    std::cout << "result : " << bResult << std::endl;
    if (bResult)
    {
        int nTime;
        u_char nTTL;
        float fMiss;
        ping.Result(&nTime, &fMiss, &nTTL);
 
        std::cout << "time : " << nTime << " TTL : " << (int)nTTL << " miss : " << fMiss*100 << "% " << std::endl;
    }
    return 0;
}


