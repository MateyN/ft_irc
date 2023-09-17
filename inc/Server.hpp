#ifndef SERVER_HPP
# define SERVER_HPP

# define	PORT_MIN 1024
# define	PORT_MAX 65535
# define    ERRNOMSG "Error: "
# define    ERROR -1
# define	MAX_CLIENTS 10		// backlog

//num replies
# define HOST "localhost"
# define MSG(nick, user, cmd, msg) (":" + nick + "!" + user + "@" + HOST + " " + cmd + " :" + msg + "\r\n")
# define PREFIX(num) (":" + HOST + " " + num + " ")

# define 332RPL_TOPIC(nick, chanName, topic) (PREFIX("332") + nick + " " + chanName + " :" + topic + "\r\n")
# define 353RPL_NAMREPLY(nick, chanName, users) (PREFIX("353") + nick + " = " + chanName + " :" + users + "\r\n")
# define 366RPL_ENDOFNAMES(nick, chanName) (PREFIX("366") + nick + " " + chanName + " :End of /NAMES list" + "\r\n")

# define 431ERR_NONICKNAMEGIVEN (PREFIX("431") + ":No nickname given" + "\r\n")
# define 432ERR_ERRONEUSNICKNAME(nick) (PREFIX("432") + nick + " :Erroneous nickname" + "\r\n")
# define 433ERR_NICKNAMEINUSE(nick) (PREFIX("433") + nick + " :Nickname is already in use" + "\r\n")
# define 461ERR_NEEDMOREPARAMS(command) (PREFIX("461") + command + ":Not enough parameters")
# define 462ERR_ALREADYREGISTERED (PREFIX("462") + ":Unauthorized command (already registered)")
# define 473ERR_INVITEONLYCHAN(chan) (PREFIX("473") + chan + " :Cannot join channel (+i)")
# define 474ERR_BANNEDFROMCHAN(chan) (PREFIX("474") + chan + " :Cannot join channel (+b)")
# define 475ERR_BADCHANNELKEY(chan) (PREFIX("475") + chan + " :Cannot join channel (+k)")

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
        //void        Sockets(); // maybe won't need that

        int         getSocket();
        int         getPort();
        void        setPort(int port);
        
        void        initServSocket();
        void        newClientConnect();
        void        clientData(pollfd &pfdc);
        void        processRecvData(int send, char *data, int size);
        void        clientDisc(int pfdc);
        void        clientsErase(Client *client);
        void        chanErase(Channel *chan);

        std::string token;
        std::string cmd;
        std::string getPass();
        void        setPass(std::string pass);

        Client*     addClient(int fd);
        Channel*    addChan(std::string name);

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
