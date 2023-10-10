#ifndef CLIENT_HPP
# define CLIENT_HPP

//#include "Server.hpp"
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

class Client
{
	public:
		Client();
		Client(int fd);
		Client(const Client &src);
		Client	&operator=(const Client &rhs);
		~Client();

		bool		_setNick;

		int			getFD()const;
		std::string	getNickname();
		//std::map<std::string, Channel *>& 	getChannels() { return _channels; }
		void		setNickname(std::string nick);
		bool		isConnect();
		void		setUser(std::string user);
		std::string	getUser();

		std::string	getHost();
		bool isRegister();
		void setIsRegister(bool registered);

		//void 		joinChannel(Channel *channel);
		//void 		leaveChannel(Channel &channel);
		//bool		inChannel(const std::string& channel_name);


	private:
		int			_fd;
		std::string	_nick;
		std::string	_username;
		std::string _hostname;
		bool		_isRegister;
		bool		_connect;

		//std::map<std::string, Channel *>	_channels;
};

#endif
