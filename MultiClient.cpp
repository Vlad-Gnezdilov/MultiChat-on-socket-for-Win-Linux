#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // Для close
#endif

#include <iostream>
#include <cstring> // Для memset и других функций обработки строк
#include <thread>

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
string def_col = "\033[0m";

void init_winsock() {
    #ifdef _WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << endl;
        exit(1);
    }
    #endif
}

void cleanup_winsock() {
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void close_socket(int sockfd) {
    #ifdef _WIN32
    closesocket(sockfd);
    #else
    close(sockfd);
    #endif
}

void send_message(int client_socket) {
    char str[MAX_LEN];
    while(true) {
        cout << colors[1] << "You: " << def_col;
        cin.getline(str, MAX_LEN);
        send(client_socket, str, strlen(str), 0);
        if(strcmp(str, "#exit") == 0) break;
    }
}

void recv_message(int client_socket) {
    char str[MAX_LEN];
    while(true) {
        int bytes_received = recv(client_socket, str, MAX_LEN-1, 0);
        if(bytes_received <= 0) break;
        str[bytes_received] = '\0'; // Ensure null-termination
        cout << str << endl;
    }
}

int main() {
    init_winsock();

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1) {
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(10000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Укажите IP адрес сервера

    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect: ");
        exit(-1);
    }

    cout << colors[NUM_COLORS-1] << "\n\t********* CHAT ROOM ***********\n" << def_col;
    cout << "Enter your name: ";
    char name[MAX_LEN];
    cin.getline(name, MAX_LEN);
    send(client_socket, name, strlen(name), 0); // Отправляем имя на сервер

    thread recvThread(recv_message, client_socket);
    send_message(client_socket);

    if(recvThread.joinable()) recvThread.join();

    close_socket(client_socket);
    cleanup_winsock();
    return 0;
}