/*---------------------------------------------------------------------------*/
/*   Program Name = server_548.cpp                                           */
/*                                                                           */
/*   Name           : Oluwaferanmi Odedairo                                  */
/*   SIUE Student ID : 800743548                                             */
/*   Course         : CS 447 - Networks and Data Communication               */
/*   Project        : Project I - Multi-client TCP socket server             */
/*                                                                           */
/*   TCP server that talks to the provided client.cpp over Winsock.          */
/*   Serves up to 3 clients at once (one worker thread per connection).      */
/*   Client is never modified.                                               */
/*---------------------------------------------------------------------------*/
/*   Build: Visual Studio, Console App. Winsock is linked via the #pragma.   */
/*---------------------------------------------------------------------------*/

/* Keep these two above the includes. Without them VS errors out on          */
/* inet_ntoa / strcpy / sprintf (C4996, SDL checks treat them as errors).    */
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>        /* atoi */
#include <string.h>
#include <time.h>
#include <winsock2.h>      /* has to come before windows.h */
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")   /* link Winsock, no Project setting needed */

#define  SERVER_PORT        1050      /* port I listen on */
#define  MAX_BUFFER         100
#define  BACKLOG            5         /* pending queue, only need >= 3 */

/* (b) prohibited client, declared by IP via #define. Swap in the IP to block. */
#define  PROHIBITED_CLIENT  "192.168.1.99"

/* what each worker thread needs to know about its client */
struct Conn {
    SOCKET sock;
    char   ip[64];     /* grabbed from accept() */
    int    port;       /* client's source port */
};

DWORD WINAPI serveClient(LPVOID arg);

/*===========================================================================*/
int main(int argc, char* argv[])
{
    WSADATA wsa;
    int port = SERVER_PORT;

    /* let me override the port from the command line if I want */
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port <= 0) port = SERVER_PORT;
    }

    /* fire up Winsock */
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    /* socket -> bind -> listen */
    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("BIND Error ...\n");
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    if (listen(serverSock, BACKLOG) < 0) {
        printf("LISTEN Error ...\n");
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    printf("Server is up and listening on port %d ...\n", port);
    printf("Prohibited client IP: %s\n\n", PROHIBITED_CLIENT);

    /* main loop - always comes back here to accept the next client */
    while (1) {
        struct sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);

        SOCKET connSock = accept(serverSock, (struct sockaddr*)&clientAddr, &addrLen);
        if (connSock == INVALID_SOCKET) {
            continue;   /* bad accept, just keep going */
        }

        /* pull the client's IP + source port off the accepted connection */
        Conn* conn = new Conn;
        conn->sock = connSock;
        strcpy(conn->ip, inet_ntoa(clientAddr.sin_addr));
        conn->port = ntohs(clientAddr.sin_port);

        printf("Connection from %s arrived (Port No = %d)\n",
            conn->ip, conn->port);

        /* hand it off to a worker so up to 3 run at the same time */
        HANDLE t = CreateThread(NULL, 0, serveClient, conn, 0, NULL);
        if (t == NULL) {
            printf("Thread creation failed.\n");
            closesocket(connSock);
            delete conn;
        }
        else {
            CloseHandle(t);   /* not joining; the thread frees its own stuff */
        }
    }

    closesocket(serverSock);
    WSACleanup();
    return 0;
}

/*===========================================================================*/
/* one of these runs per client connection */
DWORD WINAPI serveClient(LPVOID arg)
{
    Conn* conn = (Conn*)arg;
    SOCKET s = conn->sock;

    char inBuf[MAX_BUFFER];
    char outBuf[MAX_BUFFER];
    memset(inBuf, 0, MAX_BUFFER);
    memset(outBuf, 0, MAX_BUFFER);

    /* read the client's second message = its ID */
    int numBytes = recv(s, inBuf, MAX_BUFFER, 0);
    if (numBytes > 0) {
        char clientID = inBuf[0];      /* client sends "<id>\n" */

        /* timestamp = when the ID showed up */
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_s(&tm_now, &now);
        char timeStr[32];
        sprintf(timeStr, "%02d:%02d:%02d",
            tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);

        printf("Client ID = %c at %s\n\n", clientID, timeStr);

        if (strcmp(conn->ip, PROHIBITED_CLIENT) == 0) {
            /* blocked client gets the rejection line instead of a time */
            strcpy(outBuf, "Your access is denied by this server!");
        }
        else {
            /* normal: send back "<id> <time>", e.g. "1 12:04:00" */
            sprintf(outBuf, "%c %s", clientID, timeStr);
        }

        /* send the whole 100-byte buffer so client prints "100 bytes received." */
        send(s, outBuf, MAX_BUFFER, 0);
    }

    /* hold one second after replying, then drop the connection */
    Sleep(1000);
    closesocket(s);

    delete conn;
    return 0;
}

/* === end === */