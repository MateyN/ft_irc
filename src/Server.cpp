#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Message.hpp"


Server::Server(): validPass(false), _socket(0), _auth(0), _port(0)
{

}

Server::Server(const Server &src)
{
	*this = src;
}

Server	&Server::operator=(const Server &rhs)
{
	token = rhs.token;
	cmd = rhs.cmd;
	setNick = rhs.setNick;
	validPass = rhs.validPass;

	_socket = rhs._socket;
	_auth = rhs._auth;
	_port = rhs._port;
	_addr = rhs._addr;
	_sockets = rhs._sockets;
	_pfds = rhs._pfds;
	_cli = rhs._cli;
	_chan = rhs._chan;
	_password = rhs._password;
	_ping = rhs._ping;

	return *this;
}

Server::~Server()
{
	if (clients != NULL)
	{
		delete clients;
		clients = NULL;
	}

	if (channels != NULL)
	{
		delete channels;
		channels = NULL;
	}
}

void	Server::setPort(int port)
{
	_port = port;
}

int		Server::getPort()
{
	return (_port);
}

bool	Server::setupServerSocket()
{
	int	opt_len = 1;
	int	clientSocket;

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"SOCK_STREAM"));

	clientSocket = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt_len, sizeof(opt_len));
	if (clientSocket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"socket"));
	// Set client socket to non-blocking
	clientSocket = fcntl(_socket, F_SETFL, O_NONBLOCK);
	if (_socket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"setsockopt"));

	_sockets.push_back(_socket);

	bzero(&_addr, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY; // setting the server's IP address to INADDR_ANY, it will bind to all available network interfaces.
	_addr.sin_port = htons(_port); // setting the server's port number & converts it to network byte order.

	clientSocket = bind(_socket, (struct sockaddr *)&_addr, sizeof(_addr));
	if (clientSocket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"fail bind"));

	clientSocket = listen(_socket, MAX_CLIENTS);
	if (clientSocket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG "fail listen"));
	return (true);
}

int	Server::getSocket()
{
	return (_socket);
}

bool	Server::serverConnect()
{
	// init a new pollfd struct to monitor the client's socket
	pollfd		pfdc;
	int			cliSocket;
	int			pollResult;
	char		buf[250];
	socklen_t	addrlen;
	std::map<int, std::string>	msg;

	channels = NULL;
	bzero(&pfdc, sizeof(pfdc));
	pfdc.fd = _socket;
	pfdc.events = POLLIN;
	_pfds.push_back(pfdc);

	while (true)
	{
		pollResult = poll(_pfds.data(), _pfds.size(), -1);

		if (pollResult == ERROR)
			std::cout << ERRNOMSG << strerror(errno) << std::endl;

		for (unsigned int i = 0; i < _pfds.size(); i++)
		{
			if (_pfds[i].revents & POLLIN)
			{
				if (_pfds[i].fd == _socket)
				{
					addrlen = sizeof(_addr);
					cliSocket = accept(_socket, (struct sockaddr *)&_addr, &addrlen);

					if (cliSocket != ERROR)
								// KR : add MAXCLIENT here ???
					{
						pfdc.fd = cliSocket;
						pfdc.events = POLLIN;
						_pfds.push_back(pfdc);
						clients = addClient(cliSocket); // create and init a new client obj
								// KR : put client to _cli vector ?
						std::cout << "New client connected" << std::endl;
						isCAP(clients);
					}
					else
						std::cout << ERRNOMSG << strerror(errno) << std::endl;
				}
				else
				{
					bzero(&buf, sizeof(buf));

					int	storedBytes = recv(_pfds[i].fd, buf, sizeof(buf), 0); // receive and store data
					for (std::vector<Client*>::iterator	it = _cli.begin(); it != _cli.end(); it++)
					{
						if ((*it)->getFD() == _pfds[i].fd)
						{
							clients = (*it);
							break;
						}
					}
					int	send = _pfds[i].fd;
					msg[send] += static_cast<std::string>(buf);
					std::string	completeMsg = msg[send].substr(0, msg[send].find(CRLF));
					channels = getChan(completeMsg);
					if (static_cast<std::string>(buf).find("\n") != std::string::npos)
					{
						processRecvData(msg[send], clients, channels);
						msg[send].clear();
					}
					if (storedBytes <= 0)
					{
						if (storedBytes == 0)
						{
							std::cout << "Socket number: " << send << " has been disconnected." << std::endl;
						}
						else
							std::cout << ERRNOMSG << strerror(errno) << std::endl;

						for (std::vector<Client*>::iterator	it = _cli.begin(); it != _cli.end(); it++)
						{
							if ((*it)->getFD() == _pfds[i].fd)
							{
								for (std::vector<Channel*>::iterator itc = _chan.begin(); itc != _chan.end(); itc++)
								{
									if ((*itc)->User(*it))
										(*itc)->eraseUser((*it), (*it)->getFD());

									if ((*itc)->Op(*it))
										((*itc)->eraseOp(*it));

									if ((*itc)->Guest(*it))
										((*itc)->eraseGuest(*it));
								}
								delete (*it);
								_cli.erase(it);
								break;
							}
						}
						msg[_pfds[i].fd].clear();
						close(_pfds[i].fd);
						_pfds.erase(_pfds.begin() + i);
						i--;
					}
					else
					{
						continue;
					}
				}
			}
		}
	}
}

void Server::parseCmd(std::string buf)
{
	std::string input(buf);
	size_t space = input.find(' ');

	if (space != std::string::npos)
	{
		token = input.substr(0, space);
		cmd = input.substr(space + 1, input.size());
	}
	else
	{
		token = input;
		cmd.clear();
	}
	std::cout << std::endl;
}


void	Server::processRecvData(std::string buf, Client *client, Channel *channel)
{
	size_t pos = buf.find(CRLF);

	while (pos != std::string::npos)
	{
		std::string line = buf.substr(0, pos);
		buf.erase(0, pos + 2);
		pos = buf.find(CRLF);

		std::cout << "Received : " << line << std::endl;
		if (! line.empty())
		{
			parseCmd(line);
			callCmd(token, client, channel);
		}
		else
			errorMsg(461, client->getFD(), "", "", "", "");
	}
}

Client* Server::addClient(int fd)
{
	Client* client = new Client(fd);
	_cli.push_back(client);
	return client;
}

Channel* Server::addChan(std::string name)
{
	Channel* channel = new Channel(name);
	_chan.push_back(channel);
	return channel;
}

void	Server::chanErase(Channel *channel)
{
	for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
	{
		if (channel->getChanName() == (*it)->getChanName())
		{
			_chan.erase(it);
			break;
		}
	}
}

std::string Server::parseChannel(std::string input, size_t start)
{
    std::string chanName;
    size_t space = start;
    size_t end;

    // Find the '#' character in the input string, if not provided.
    if (space == std::string::npos)
        space = input.find("#");

    if (space != std::string::npos && space != input.size() - 1)
    {
        size_t nextSpace = input.find(" ", space);
        size_t comma = input.find(",", space);

        // Determine the position of the next space or comma.
        end = std::min(nextSpace, comma);
        
        // If no space or comma was found, set end to the end of the input.
        if (end == std::string::npos)
            end = input.size();
        
        // Extract the channel name.
        chanName = input.substr(space, end - space);
    }
    else
    {
        // If '#' was not found or it's at the end, set chanName to the entire input.
        chanName = input;
    }
    return chanName;
}

Channel*	Server::getChan(std::string msgBuf)
{
	std::string		chanName;

	chanName = parseChannel(msgBuf, 0);

	for(std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
	{
		if ((*it)->getChanName() == chanName)
			return (*it);
	}
	return (NULL);
}

std::string	Server::getPassword()
{
	return _password;
}

void	Server::setPass(std::string pass) 
{
	_password = pass;
}

void	Server::isCAP(Client *client)
{
	if (connect(client->getFD(), (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
	{
		std::cerr <<  "there is another connection"  << std::endl;
		msgSend("PING", client->getFD());
		return;
	}
}

bool		Server::chanExist(std::string channel)
{
	for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
	{
		if ((*it)->getChanName() == channel)
			return true;
	}
	return false;
}

bool	Server::nickExist(std::string nick)
{
	for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); ++it)
	{
		if ((*it)->getNickname() == nick) 
		{
			return true;
		}
	}
	return false;
}