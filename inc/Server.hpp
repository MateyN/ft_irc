#ifndef SERVER_HPP
# define SERVER_HPP

//# define	PORT_MIN 1024
//# define	PORT_MAX 65535
# define    ERRNOMSG "Error: "
# define    ERROR -1
# define	MAX_CLIENTS 10		// backlog
# define    TRUE 1
# define    FALSE 0

#include "Client.hpp"
#include "Channel.hpp"
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

        Client*     client;
        Channel*    channel;

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

        bool        setNick;
        bool        validPass;

        bool        serverConnect();
        bool        setupServerSocket();
        bool        isChannel(std::string chan);
        bool        isNick(std::string nick);
        //void        Sockets(); // maybe won't need that

        int         getSocket();
        int         getPort();
        void        setPort(int port);
        
        void        initServSocket();
        void        newClientConnect();
        void        handleNewClientConnect(Client *client);
        void        handleMessage(int send, const std::string& msg);
        void        clientMsg(std::string msg, Client *client, Channel *channel);
        void        clientData(pollfd &pfdc);
        void        processRecvData(int send, char *data, int size);
        void        clientDisc(int pfdc);
        void        clientsErase(Client *client);
        void        chanErase(Channel *chan);
        void        parseCmds(std::string msg);

        std::string token;
        std::string cmd;
        std::string getPass();
        std::string channelParse(std::string input, size_t start);

        void        setPass(std::string pass);

        Client*     addClient(int fd);
        Channel*    addChan(std::string name);
        Channel*    getChan(std::string msg);

    private:
        int                         _socket;
        int                         _port;
        std::string                 password;
        struct sockaddr_in		    _addr;
        std::vector<pollfd>		    _pfds;
        std::vector<Client *>	    _cli;
        std::vector<Channel *>      _chan;
        std::map<int, std::string>  msgBuffer;

};

#endif