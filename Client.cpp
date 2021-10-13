#define WIN32_LEAN_AND_MEAN //exclued unneeded header files

#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<ws2tcpip.h>
#include<winsock2.h>
#include<string.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 512
#define PORT "9999"

int main() {
    //initiate use of WinSock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf_s("WSAStartup failed with error %d\n", iResult);
        return 1;
    }

    //create  socket
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    iResult = getaddrinfo("localhost", PORT, &hints, &result);
    if (iResult != 0) {
        printf_s("getaddrinfo failed with error %d\n", iResult);
        WSACleanup();
        return 1;
    }
    SOCKET ClientSocket = INVALID_SOCKET;
    for (struct addrinfo* curr = result; curr != NULL; curr = curr->ai_next) {
        ClientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);//maybe more
        if (ClientSocket == INVALID_SOCKET) {
            printf_s("sokcet failed with error %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        //connect
        iResult = connect(ClientSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf_s("connect failed %d\n", WSAGetLastError());
            ClientSocket = INVALID_SOCKET;
        }
        else {
            break;
        }
    }

    freeaddrinfo(result);
    if (ClientSocket == INVALID_SOCKET) {
        printf_s("connect failed %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //recv and send
    char buf[BUF_SIZE];
    //recieve and print welcome msg
    recv(ClientSocket, buf, BUF_SIZE, 0);
    if (iResult > 0) {
        printf_s("%s\n", buf);
    }
    else if (iResult == 0) {
        printf_s("%s\n", buf);
        //printf("Closing connection\n");
    }
    else if (iResult == SOCKET_ERROR) {
        printf_s("bind failed with error %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    char in[BUF_SIZE];
    while (1) {
        printf_s("Input: ");
        //scanf_s("%s", in, BUF_SIZE);
        fgets(in, BUF_SIZE, stdin);
        //in[strlen(in) - 1] = '\0';
        in[strcspn(in, "\n")] = 0;
        send(ClientSocket, in, BUF_SIZE, 0);
        if (strcmp(in, "quit") == 0) {
            printf_s("Closing connection\n");
            break;
        }
        
        //Recieve number of times server will send data
        char n_str[12];
        recv(ClientSocket, n_str, 12, 0);
        int n = atoi(n_str);
        //printf("Recieved: %d\n", n);

        char repeat[BUF_SIZE];
        for (int i = 0; i < n; i++) {
            recv(ClientSocket, repeat, BUF_SIZE, 0);
            printf("%d: %s\n", i, repeat);
        }
    }    

    //cleanup
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}