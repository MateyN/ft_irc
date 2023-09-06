#ifndef SERVER_HPP
# define SERVER_HPP

# define	PORT_MIN 1024
# define	PORT_MAX 65535
# define    ERRNOMSG "Error: "
# define    ERROR -1
//# define	MAX_CLIENTS 10		// backlog

#include "Client.hpp"
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <map>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <string>
#include <cstring>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

class Server
{
    public:
        Server();
        Server(const Server& src);
        Server& operator=(const Server &rhs);
        ~Server();

        class ExceptionServer : public std::exception
        {
			public:
				ExceptionServer(const char* msg) : _msg(msg) {}
				const char* what() const throw()
                {
					return _msg;
				}

			private:
				const char* _msg;
		};

        std::string token;
        std::string cmd;
        bool        setNick;
        bool        validPass;

        bool        setupServerSocket();
        void        Sockets();

        int         getSocket();
        int         getPort();
        void        setPort(int port);

        std::string getPass();
        void        setPass(std::string pass);

    private:
        int                     _socket;
        int                     _port;
        std::string             password;
        struct sockaddr_in		_addr;

};

#endif