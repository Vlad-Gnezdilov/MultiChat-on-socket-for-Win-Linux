#include <iostream>
#include <cstring> // Для memset и других функций обработки строк
#include <vector>
#include <thread>
#include <mutex>
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

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

vector<string> colors = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
string def_col = "\033[0m";
mutex cout_mtx, clients_mtx;

struct client_info {
    int id;
    string name;
    int socket;
    thread th;
};

vector<client_info> clients;
int seed = 0;

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

string color(int code) {
    return colors[code % NUM_COLORS];
}

void broadcast_message(const string& message, int sender_id) {
    lock_guard<mutex> guard(clients_mtx);
    for(auto& client : clients) {
        if(client.id != sender_id) {
            send(client.socket, message.c_str(), message.length(), 0);
        }
    }
}

void handle_client(int client_socket, int id, string client_name) {
    char str[MAX_LEN];
    string welcome_message = color(id) + client_name + " has joined" + def_col;
    broadcast_message(welcome_message, id);

    while(true) {
        int bytes_received = recv(client_socket, str, MAX_LEN-1, 0);
        if(bytes_received <= 0) break;
        str[bytes_received] = '\0'; // Ensure null-termination
        if(strcmp(str, "#exit") == 0) {
            string leave_message = color(id) + client_name + " has left" + def_col;
            broadcast_message(leave_message, id);
            break;
        }
        string message = color(id) + client_name + ": " + str + def_col;
        broadcast_message(message, id);
    }
    close_socket(client_socket);
}

int main() {
    init_winsock();

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1) {
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(10000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind: ");
        exit(-1);
    }

    if(listen(server_socket, 8) == -1) {
        perror("listen: ");
        exit(-1);
    }

    cout << "\n\t********* CHAT ROOM ***********\n";

    while(true) {
        struct sockaddr_in client_addr;
        socklen_t client_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_size);
        if(client_socket == -1) {
            perror("accept: ");
            continue;
        }
        char name[MAX_LEN];
        recv(client_socket, name, MAX_LEN-1, 0);
        name[MAX_LEN-1] = '\0'; // Ensure null-termination

        seed++;
        string client_name = name;
        thread t([client_socket, id = seed, client_name] { handle_client(client_socket, id, client_name); });
        clients.push_back({seed, client_name, client_socket, move(t)});
    }

    for(auto& client : clients) {
        if(client.th.joinable()) client.th.join();
    }

    close_socket(server_socket);
    cleanup_winsock();
    return 0;
}