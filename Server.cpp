#define WIN32_LEAN_AND_MEAN //exclued unneeded header files

#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<ws2tcpip.h>
#include<winsock2.h>
#include<string.h>
#include<cstdlib>

#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 512
#define PORT "9999"

struct ARGS {
    LPVOID lpParameter;
    SOCKET ClientSocket;
    int iResult;
    int id;
};

DWORD WINAPI handle_client(void* funcArgs_) {
    ARGS* funcArgs = (ARGS*)funcArgs_;
    
    printf_s("Connecting Client %d\n", funcArgs->id);

    //recv and send
    char welcome[BUF_SIZE];
    sprintf_s(welcome, "Client %d, thank your for connecting!", funcArgs->id);
    send(funcArgs->ClientSocket, welcome, BUF_SIZE, 0);

    char buf[BUF_SIZE];
    while (1) {
        recv(funcArgs->ClientSocket, buf, BUF_SIZE, 0);
        if (funcArgs->iResult > 0) {
            if (!strcmp(buf, "quit")) {
                printf_s("Disconnecting Client %d\n", funcArgs->id);
                return 0;
                //break;
            }
            printf_s("Recieved \"%s\" from Client %d\n", buf, funcArgs->id);
        }
        else if (funcArgs->iResult == 0) {
            if (!strcmp(buf, "quit")) {
                printf_s("Disconnecting Client %d\n", funcArgs->id);
                return 0;
                //break;
            }
            printf_s("Recieved \"%s\" from Client %d\n", buf, funcArgs->id);
        }
        else if (funcArgs->iResult == SOCKET_ERROR) {
            printf_s("bind failed with error %d\n", WSAGetLastError());
            closesocket(funcArgs->ClientSocket);
            WSACleanup();
            return 0;
        }

        //Send client number of times we will send
        int n = rand() % 8 + 2;
        char n_str[12];
        sprintf_s(n_str, "%d", n);
        send(funcArgs->ClientSocket, n_str, 12, 0);
        //printf_s("Sent %d to Client %d\n", n, funcArgs->id);

        for (int i = 0; i < n; i++) {
            send(funcArgs->ClientSocket, buf, BUF_SIZE, 0);
        }

        printf_s("Sent %dx %s to Client %d\n", n, buf, funcArgs->id);
    }

    //cleanup
    closesocket(funcArgs->ClientSocket);
    WSACleanup(); //might cause issues
    return 0;
}


int main() {
    //initiate use of WinSock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error %d\n", iResult);
        return 1;
    }

    //create listening socket
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    iResult = getaddrinfo(NULL, PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error %d\n", iResult);
        WSACleanup();
        return 1;
    }

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);//maybe more
    if (ListenSocket == INVALID_SOCKET) {
        printf("sokcet failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //bind
    bind(ListenSocket, result->ai_addr, result->ai_addrlen);
    if (iResult != 0) {
        printf("bind failed with error %d\n", iResult);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);

    //listen
    listen(ListenSocket, SOMAXCONN);
    printf("Server is listening...\n");

    int id = 0; //creates id
    SOCKET ClientSockets[10];
    ARGS funcs[10];
    int i = 0;
    while (1) {
        //accept
        ClientSockets[i] = accept(ListenSocket, NULL, NULL);
        
        //thread creation
        funcs[i] = { NULL, ClientSockets[i], iResult, id };
        id++;
        
        CreateThread(NULL, 0, handle_client, &funcs[i], 0, NULL);
        i++;
    }
    
    //cleanup
    WSACleanup();
    return 0;
}