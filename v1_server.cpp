#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    if (argc != 2) {
        std::cerr << "Usage:" << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);

    int server = socket(PF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        std::cerr << "sock(2) failed . errno=" << errno << std::endl;
        return 1;
    }
    std::cout << "Created an endpoint for communication" << std::endl;

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

    while (true) {
        struct sockaddr_in clientAddr{0,};
        socklen_t clientAddrSize = sizeof(clientAddr);
        int client = accept(server, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (client <= 0) {
            std::cerr << "accept(2) failed. errno=" << errno << std::endl;
            close(server);
            return 1;
        }
        std::cout << "Accepted a connection on a socket" << std::endl;

        char bf[1024];
        if (0 < recv(client, bf, sizeof(bf), 0)) {
            std::cout << "Received a message from a socket" << std::endl;

            char sendData[64];
            snprintf(sendData, sizeof(sendData), bf);
            ssize_t bytes = send(client, sendData, std::strlen(sendData) + 1, 0);
            close(client);

            if (bytes == -1) {
                std::cerr << "send(2) failed. errno=" << errno << std::endl;
                continue;
            }

            std::cout << "Sent a message from a socket" << std::endl;

            if (strcmp(bf, "quit") == 0) {
                std::cout << "Received 'quit'" << std::endl;
                close(server);
                return 0;
            }
        }
    }
}