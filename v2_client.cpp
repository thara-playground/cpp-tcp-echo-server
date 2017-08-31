#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    if (argc != 4) {
        std::cerr << "Usage:" << argv[0] << " <server> <port> <message>" << std::endl;
        return 1;
    }

    std::string serverAddr = argv[1];
    int port = std::atoi(argv[2]);
    std::string msg = argv[3];

    int server = socket(PF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        std::cerr << "sock(2) failed . errno=" << errno << std::endl;
        return 1;
    }
    std::cout << "Created an endpoint for communication" << std::endl;

    struct sockaddr_in addr={0,};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(serverAddr.c_str());

    if (connect(server, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        std::cerr << "connect(2) failed. errno=" << errno << std::endl;
        return 1;
    }
    std::cout << "Initiate a connection on a socket" << std::endl;

    int msgSize = sizeof(msg.c_str());
#ifdef MSG_NOSIGNAL
    ssize_t bytes = send(server, msg.c_str(), msgSize, MSG_NOSIGNAL);
#else
    ssize_t bytes = send(server, msg.c_str(), msgSize, 0);
#endif

    if (bytes != msgSize) {
        std::cerr << "send(2) failed. errno=" << errno << std::endl;
        return 1;
    }

    struct timeval tv = {3,0};
    fd_set fdz{};
    FD_ZERO(&fdz);
    FD_SET(server, &fdz);

    int ret = select(server + 1, &fdz, 0, 0, &tv);

    if (ret == 0) {
        std::cerr << "select(2) timeout. errno=" << errno << std::endl;
        return 1;
    } else if (ret < 0) {
        std::cerr << "select(2) failed. errno=" << errno << std::endl;
        return 1;
    } else {
        if (FD_ISSET(server, &fdz)) {
            char bf[1024];
            ssize_t recvBytes = recv(server, bf, sizeof(bf), 0);
            if (recvBytes == 0) {
                std::cerr << "Disconnected from client" << std::endl;
                close(server);
                return 1;
            } else if (recvBytes == -1) {
                std::cerr << "accept(2) failed. errno=" << errno << std::endl;
                close(server);
                return 1;
            } else {
                std::cout << "Read bytes from socket" << std::endl;
                std::cout << bf << std::endl;
            }
        }
    }

    close(server);

    return 0;
}