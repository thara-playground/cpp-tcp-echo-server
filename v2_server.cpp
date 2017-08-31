#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <spawn.h>

bool running = true;

extern char **environ;

void* connectHandler(void* param) {
    int client = *(int*)param;

    while (true) {
        char bf[1024];
        ssize_t recvBytes = recv(client, bf, sizeof(bf), 0);

        if (0 < recvBytes) {
            std::cout << "Received a message from a socket" << std::endl;

            if (strcmp(bf, "quit") == 0) {
                std::cout << "Received 'quit'" << std::endl;
                running = false;
                close(client);
                break;
            }

            ssize_t bytes = send(client, bf, recvBytes, 0);
            close(client);

            if (bytes == -1) {
                std::cerr << "send(2) failed. errno=" << errno << std::endl;
            } else {
                std::cout << "Sent a message from a socket" << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    if (argc != 2) {
        std::cerr << "Usage:" << argv[0] << " <port>" << std::endl;
        return 1;
    }

    // Daemonize
    if (fork() != 0) {
        return 0;
    }
    setsid();
    if (fork() != 0) {
        return 0;
    }
    chdir("/");
    

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0 ) {
        std::cerr << "sock(2) failed . errno=" << errno << std::endl;
        return 1;
    }
    std::cout << "Created an endpoint for communication" << std::endl;

    int port = std::atoi(argv[1]);

    struct sockaddr_in serverAddr{0,};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "bind(2) failed. errno=" << errno << std::endl;
        close(server);
        return 1;
    }
    std::cout << "Bind a name to socket" << std::endl;

    if (listen(server, 10) == -1) {
        std::cerr << "listen(2) failed. errno=" << errno << std::endl;
        close(server);
        return 1;
    }
    std::cout << "Listen for connections on a socket" << std::endl;

    while (running) {
        struct sockaddr_in clientAddr{0,};
        socklen_t clientAddrSize = sizeof(clientAddr);

        struct timeval tv={0,1000};
        fd_set fdz{};
        FD_ZERO(&fdz);
        FD_SET(server, &fdz);

        if (select(server + 1, &fdz, 0, 0, &tv) == 0) {
            if (!running) {
                break;
            }
            continue;
        }
        std::cout << "Synchronous I/O multiplexing" << std::endl;

        int client = accept(server, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (client <= 0) {
            std::cerr << "accept(2) failed. errno=" << errno << std::endl;
            close(server);
            return 1;
        }
        std::cout << "Accepted a connection on a socket" << std::endl;

        pthread_t th;
        if (pthread_create(&th, NULL, connectHandler, (void*)(&client)) != 0) {
            pthread_detach(th);
        }
    }

    close(server);
    std::cout << "Terminate server" << std::endl;

    return 0;

}