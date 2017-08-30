#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>


int main(int argc, char* argv[]) {

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
    ssize_t bytes = write(server, msg.c_str(), msgSize - 1);
    if (bytes != msgSize - 1) {
        std::cerr << "write(2) failed. errno=" << errno << std::endl;
        return 1;
    }
    std::cout << "Write bytes to socket" << std::endl;

    char bf[1024];
    ssize_t n = read(server, bf, 1024);

    if (n < 0) {
        std::cerr << "read(2) failed. errno=" << errno << std::endl;
        return 1;
    }
    if (n == 0) {
        std::cerr << "Connection closed. errno=" << errno << std::endl;
        return 1;
    }

    std::cout << "Read bytes from socket" << std::endl;

    std::cout << bf << std::endl;
    close(server);

    return 0;
}

