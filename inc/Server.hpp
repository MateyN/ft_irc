#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cerrno>
# include <stdexcept>
# include <vector>
# include <map>
# include <fcntl.h>
# include <sys/poll.h>
# include <string>
# include <cstring>
# include <sstream>
# include <algorithm>
# include <poll.h>
# include <cstdlib>
#include <sstream>

# include "Client.hpp"
# include "Channel.hpp"

# define PORT_MIN 1024
# define PORT_MAX 65535
# define ERRNOMSG "Error: "
# define ERROR -1
# define MAX_CLIENTS 32		// backlog
# define CRLF "\r\n"
# define RESET   "\033[0m"
# define BLACK   "\033[30m"      /* Black */
# define RED     "\033[31m"      /* Red */
# define GREEN   "\033[32m"      /* Green */
# define YELLOW  "\033[33m"      /* Yellow */
# define BLUE    "\033[34m"      /* Blue */
# define MAGENTA "\033[35m"      /* Magenta */
# define CYAN    "\033[36m"      /* Cyan */
# define WHITE   "\033[37m"      /* White */
# define LIGHTGREEN "\033[1;32m" /* Light Green */

//class Channel;

class Server
{
	public:
		// server.cpp
        Server();
        Server(const Server& src);
        Server& operator=(const Server &rhs);
        ~Server();

		Client		*clients;
		Channel		*channels;

		std::string token;
        std::string cmd;
		bool    	setNick;
		bool   	 	validPass;

		bool    	setupServerSocket();
        bool    	serverConnect();
        //void        				Sockets(); // maybe won't need that
		Client*     addClient(int fd);
		Channel* 	addChan(std::string name);
		void  		chanErase(Channel *chan);
		bool		chanExist(std::string channel);
		bool		nickExist(std::string nick);
		void		processRecvData(std::string buf, Client *client, Channel *channel);
		void		isCAP(Client *client);
		void		msgSend(std::string msg, int fd);
		void		welcomeMsg(Client *client);
		void		errorMsg(int errCode, int fd, std::string str1, std::string str2, std::string str3, std::string info);

		int     	getPort();
		int			getSocket();
		void    	setPort(int port);
        std::string getPassword();
        void        setPass(std::string pass);
		Channel*	getChan(std::string msg);

		void		parseCmd(std::string buf);
		std::string	parseChannel(std::string input, size_t start);

		// cmd.cpp
		void		callCmd(std::string cmd, Client *client, Channel *channel);
		//bool								findNickname(const std::string &nick);
		//bool								findUserClient(const std::string &user);
		//std::vector<Channel *>::iterator	findChannel(const std::string &chan);
		//std::vector<Client *>::iterator		findClientChannel(const std::string &nick, Channel &channel);
		//bool								cmdNick(Client &client, const std::string &nick);
		//bool								cmdUser(Client &client, std::string user);
		void		sendToUsersInChan(std::string msg, int fd);
		//void								cmdJoinNames(Client &client, Channel &channel);
		//bool								cmdJoin(Client &client, std::vector<std::string> &params);
		//bool								cmdPart(Client &client, std::vector<std::string> &params);

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

	private:
				// server.cpp
			int						_socket;
			int						_auth;
			int						_port;
			struct sockaddr_in		_addr;
			std::vector<int>		_sockets;
			std::vector<pollfd>		_pfds;
			std::vector<Client*>	_cli;
			std::vector<Channel*>	_chan;
			std::string				_password;
			int						_ping;
			std::string				valid_commands[7];

				// cmd.cpp
			void	CAP(Client *client, Channel *channel);
			void	PING(Client *client, Channel *channel);
			void	NICK(Client *client, Channel *channel);
			void	USER(Client *client, Channel *channel);
			void	PASS(Client *client, Channel *channel);
			void	JOIN(Client *client, Channel *channel);
			void	QUIT(Client *client, Channel *channel);
};

#endif