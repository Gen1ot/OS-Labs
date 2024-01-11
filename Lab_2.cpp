#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

struct Client {
    int connfd;   
    sockaddr_in addr; 
};

//Объявление обработчика сигнала
volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int sig) {
    wasSigHup = 1;
}

void setupSignalHandler(sigset_t *originalMask) {
    //Регистрация обработчика сигнала
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    //Блокировка сигнала
    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, originalMask);
}

//создаёт сервер на указанном порту
int createServer(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in servaddr{};
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&servaddr), sizeof(servaddr)) != 0) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) != 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

int main() {
    int serverSocket = createServer(3000);
    cout << "Server is listening...\n";

    vector<Client> clients;
    char buffer[1024] = {0};

    sigset_t origSigMask;
    setupSignalHandler(&origSigMask);

    while (true) {
        if (wasSigHup) {
            wasSigHup = 0;
            cout << "Connected Clients: ";
            for (const auto &client : clients) {
                cout << "[" << inet_ntoa(client.addr.sin_addr) << ":" << htons(client.addr.sin_port) << "] ";
            }
            cout << "\n";
        }

        fd_set file_descriptors;
        FD_ZERO(&file_descriptors);
        FD_SET(serverSocket, &file_descriptors);
        int maxFd = serverSocket;

        for (const auto &client : clients) {
            FD_SET(client.connfd, &file_descriptors);
            if (client.connfd > maxFd) {
                maxFd = client.connfd;
            }
        }

        if (pselect(maxFd + 1, &file_descriptors, NULL, NULL, NULL, &origSigMask) < 0) {
            if (errno != EINTR){
                perror("pselect failed");
                return EXIT_FAILURE;
            }
        }

        if (FD_ISSET(serverSocket, &file_descriptors) && clients.size() < 5) {
            clients.emplace_back();
            auto &client = clients.back();
            socklen_t len = sizeof(client.addr);
            client.connfd = accept(serverSocket, reinterpret_cast<sockaddr*>(&client.addr), &len);
            if (client.connfd >= 0) {
                cout << "[" << inet_ntoa(client.addr.sin_addr) << ":" << htons(client.addr.sin_port) << "] Connected!\n";
            } else {
                perror("Accept error");
                clients.pop_back();
            }
        }

        for (auto i = clients.begin(); i != clients.end(); ) {
            auto &client = *i;
            if (FD_ISSET(client.connfd, &file_descriptors)) {
                int readLen = read(client.connfd, buffer, sizeof(buffer) - 1);
                if (readLen > 0) {
                    buffer[readLen - 1] = 0;
                    cout << "[" << inet_ntoa(client.addr.sin_addr) << ":" << htons(client.addr.sin_port) << "] " << buffer << "\n";
                } else {
                    close(client.connfd);
                    cout << "[" << inet_ntoa(client.addr.sin_addr) << ":" << htons(client.addr.sin_port) << "] Connection closed\n";
                    i = clients.erase(i);
                    continue;
                }
            }
            i++;
        }
    }

    return 0;
}
