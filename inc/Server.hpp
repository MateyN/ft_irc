#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

class Server {
public:
    Server(int port, const std::string &pass);
    Server(const Server& src);
    Server& operator=(const Server &rhs);
    virtual ~Server();

    int getServer() const;
    int getNbInst();

    static int err(const std::string &s)
	{
        std::cerr << s << std::endl;
        return 1;
    }

    void run();

private:
    int serverSocket;
    int maxClients;
    std::string password;

	static int _nbInst;

    void setupServerSocket(int port);
    void acceptClients();
    void handleClientConnection(int clientSocket);
};

#endif