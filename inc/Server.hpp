#ifndef SERVER_HPP
# define SERVER_HPP

# define	PORT_MIN 1024
# define	PORT_MAX 65535
//# define	MAX_CLIENTS 10		// backlog

#include <iostream>
#include <string>
#include <sstream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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