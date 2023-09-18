#ifndef SERVER_HPP
# define SERVER_HPP

# define	PORT_MIN 1024
# define	PORT_MAX 65535
# define    ERRNOMSG "Error: "
# define    ERROR -1
# define	MAX_CLIENTS 10		// backlog

//num replies
# define TOSTR(a) static_cast<std::string>(a)
# define CRLF "\r\n"

# define MSG(nick, user, cmd, msg)				":" + nick + "!" + user + "@localhost " + cmd + " :" + msg + "\r\n"
# define RPL332_TOPIC(nick, chanName, topic) 	":localhost 332 " + nick + " " + chanName + " :" + topic + "\r\n"
# define RPL353_NAMREPLY(nick, chanName, users)	":localhost 353 " + nick + " = " + chanName + " :" +  users + "\r\n"
# define RPL366_ENDOFNAMES(nick, chanName)		":localhost 366 " + nick + " " + chanName + " :End of /NAMES list\r\n"
# define ERR401_NOSUCHNICK(nick)				":localhost 401 " + nick + " :No such nick/channel\r\n"
# define ERR403_NOSUCHCHANNEL(chan)				":localhost 403 " + chan + " :No such channel\r\n"
# define ERR411_NORECIPIENT(cmd)				":localhost 411 :No recipient given " cmd "\r\n"
# define ERR412_NOTEXTTOSEND					":localhost 412 :No text to send\r\n"
# define ERR431_NONICKNAMEGIVEN					":localhost 431 :No nickname given\r\n"
# define ERR432_ERRONEUSNICKNAME(nick)			":localhost 432 " + nick + " :Erroneous nickname\r\n"
# define ERR433_NICKNAMEINUSE(nick)				":localhost 433 " + nick + " :Nickname is already in use\r\n"
# define ERR442_NOTONCHANNEL(chan)				":localhost 442 " + chan + " :You're not on that channel\r\n"
# define ERR461_NEEDMOREPARAMS(cmd)				":localhost 461 " cmd  ":Not enough parameters\r\n"
# define ERR462_ALREADYREGISTERED				":localhost 462 :Unauthorized command (already registered)\r\n"
# define ERR473_INVITEONLYCHAN(chan)			":localhost 473 " + chan + " :Cannot join channel (+i)\r\n"
# define ERR474_BANNEDFROMCHAN(chan)			":localhost 474 " + chan + " :Cannot join channel (+b)\r\n"
# define ERR475_BADCHANNELKEY(chan)				":localhost 475 " + chan + " :Cannot join channel (+k)\r\n"

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

class Client;
class Channel;
class Server
{
    public:
		// server.cpp
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

        int         getSocket();
        int         getPort();
		std::vector<Channel *>	getChan() { return _chan; }

        bool        setNick;
        void        setPort(int port);

        bool        validPass;
        bool        serverConnect();
        bool        setupServerSocket();
        //void        Sockets(); // maybe won't need that
        
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
		Channel* 	addChan(std::string name, Client* op);

		// cmd.cpp
		void								callCmd(Client &client, const std::string& cmd, std::vector<std::string> &params);
		bool								findNickname(const std::string &nick);
		bool								findUserClient(const std::string &user);
		std::vector<Channel *>::iterator	findChannel(const std::string &chan);
		std::vector<Client *>::iterator		findClientChannel(const std::string &nick, Channel &channel);
		bool								cmdNick(Client &client, const std::string &nick);
		bool								cmdUser(Client &client, std::string user);
		void								sendToUsersInChan(Channel &channel, Client &client, std::string msg);
		void								cmdJoinNames(Client &client, Channel &channel);
		bool								cmdJoin(Client &client, std::vector<std::string> &params);
		bool								cmdPart(Client &client, std::vector<std::string> &params);
		bool								cmdMsg(Client &client, std::vector<std::string> &params);

    private:
		// server.cpp
        int                         _socket;
        int                         _port;
        std::string                 password;
        struct sockaddr_in		    _addr;
        std::vector<pollfd>		    _pfds;
        std::vector<Client *>	    _cli;
        std::vector<Channel *>      _chan;
        std::map<int, std::string>  msgBuffer;

		// cmd.cpp

};

#endif
