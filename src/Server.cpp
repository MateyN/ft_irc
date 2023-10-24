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

// Copy assignment operator
Server& Server::operator=(const Server &rhs)
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

	return *this;
}

// Default Destructor 
Server::~Server()
{
    for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); ++it)
    {
        delete *it;
    }
    _cli.clear();
    for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); ++it)
    {
        delete *it;
    }
    _chan.clear();
}

int		Server::getPort()
{
	return (_port);
}

int	Server::getSocket()
{
	return (_socket);
}

std::string	Server::getPassword()
{
	return _password;
}

// Returns Channel or NULL
Channel*	Server::getChan(std::string msg)
{
	std::string		chanName;

	chanName = parseChannel(msg, 0); // Returns channel name or NULL

	for(std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
	{
		if ((*it)->getChanName() == chanName)
			return (*it);
	}
	return (NULL);
}

void	Server::setPort(int port)
{
	_port = port;
}

void	Server::setPass(std::string pass) 
{
	_password = pass;
}

bool	Server::setupServerSocket(char* av[])
{
	int	clientSocket;

	// Set port
	this->setPort(atoi(av[1]));

	// Init socket
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"SOCK_STREAM"));

	// Set socket options
	int	opt_len = 1;
	clientSocket = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt_len, sizeof(opt_len));
	if (clientSocket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"socket"));

	// Set client socket to non-blocking, to never wait for an operation to complete (e.g., switch bw many different connected sockets)
	clientSocket = fcntl(_socket, F_SETFL, O_NONBLOCK);
	if (_socket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"setsockopt"));
	_sockets.push_back(_socket);

	// Populate sockaddr_in struct
	bzero(&_addr, sizeof(_addr));
	_addr.sin_family = AF_UNSPEC;
	//// setting the server's IP address to INADDR_ANY, it will bind to all available network interfaces.
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//// setting the server's port number & converts it to network byte order.
	_addr.sin_port = htons(_port); 

	// Bind the address to the socket to receive data on this socket
	clientSocket = bind(_socket, (struct sockaddr *)&_addr, sizeof(_addr));
	if (clientSocket == ERROR)
	{
		perror("bind");
		throw (Server::ExceptionServer(ERRNOMSG"fail bind"));
	}

	// Listen for connections on a socket
	clientSocket = listen(_socket, MAX_CLIENTS);
	if (clientSocket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG "fail listen"));

	// Set Password
	this->setPass(av[2]);

	// Print result
	std::cout << CYAN << "Server created and launched!" << RESET << std::endl;
	this->printIRCBanner();

	return (true);
}

bool	Server::serverConnect()
{
	int							cliSocket;
	int							pollResult;
	char						buf[250];
	socklen_t					addrlen;
	std::map<int, std::string>	msg;
	pollfd						pfdc;

	channels = NULL;
	bzero(&pfdc, sizeof(pfdc));
	pfdc.fd = _socket;
	pfdc.events = POLLIN;
	_pfds.push_back(pfdc);

	while (true)
	{
		// Init a new pollfd struct to monitor the client's socket
		pollResult = poll(_pfds.data(), _pfds.size(), -1);
		if (pollResult == ERROR)
			std::cout << "Error in poll()" << std::endl;

		for (unsigned int i = 0; i < _pfds.size(); i++)
		{
			// If there is data to read
			if (_pfds[i].revents & POLLIN)
			{
				// If it's a new connection : check the current socket to accept it
				if (_pfds[i].fd == _socket)
				{
					addrlen = sizeof(_addr);
					cliSocket = accept(_socket, (struct sockaddr *)&_addr, &addrlen);
					if (cliSocket == ERROR)
						 std::cout << "Error in accept()" << std::endl;
					else
					// Create and init a new client obj and add to _cli vector
					{
						pfdc.fd = cliSocket;
						pfdc.events = POLLIN;
						_pfds.push_back(pfdc);
						clients = addClient(cliSocket); 
						std::cout << CYAN << "New client connected" << RESET << std::endl;
						isCAP(clients);
						clientReadBuffers[cliSocket] = ""; // read buff for the client
						msgSend("Please set \"PASS <password>\", \"NICK <nickname>\", \"USER <username> <i_mode> * :<realname>\"", clients->getFD());
					}
				}
				// Else there is data to receive and store
				else
				{
					bzero(&buf, sizeof(buf));
					int	storedBytes = recv(_pfds[i].fd, buf, sizeof(buf), 0);

					// Set current client to match _pfds[i].fd
					for (std::vector<Client*>::iterator	it = _cli.begin(); it != _cli.end(); it++)
					{
						if ((*it)->getFD() == _pfds[i].fd)
						{
							clients = (*it);
							break;
						}
					}
					int	send = _pfds[i].fd;

					// Get completeMsg from buffer without \n\r or \n for nc (initiated in recv())
					msg[send] += TOSTR(buf); // buildingn the complete message by appending packets til we have a full cmd or msg
					std::string completeMsg = msg[send];
					if (completeMsg.find(CRLF) != std::string::npos)
						completeMsg = msg[send].substr(0, msg[send].find(CRLF));
					else { // subtlety for nc
						completeMsg = msg[send].substr(0, msg[send].find("\n"));
					}

					// Find channel name in completeMsg, if existing
					channels = getChan(completeMsg);

					// If there is a \n in the buffer, say "Received" and call commands
					if (TOSTR(buf).find("\n") != std::string::npos) // to include nc : finding \n is sufficient, however finding CRLF means not caring about nc
					{
						processRecvData(msg[send], clients, channels);
						msg[send].clear();
					}

					// If no more stored bytes, then disconnect
					if (storedBytes <= 0)
					{
						if (storedBytes == 0)
							std::cout << RED << "Socket number: " << send << " has been disconnected." << RESET << std::endl;
						else
							std::cout << "Error in recv()" << std::endl;

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
					// Process the messages as long as they are complete
					{
						buf[storedBytes] = '\0';
						std::string receivedData = TOSTR(buf);
						clientReadBuffers[_pfds[i].fd] += receivedData; // append received data to the client's read buffer
						std::size_t crlfPos;
						while ((crlfPos = clientReadBuffers[_pfds[i].fd].find(CRLF)) != std::string::npos)
                        {
                            std::string completeMsg = clientReadBuffers[_pfds[i].fd].substr(0, crlfPos);
                            channels = getChan(completeMsg);
                            processRecvData(completeMsg, clients, channels);
                            clientReadBuffers[_pfds[i].fd].erase(0, crlfPos + 2);
                        }
					}
				}
			}
		}
	}
}

void Server::parseCmd(std::string buf)
{
    token.clear(); // token is command
    cmd.clear(); // cmd is args

    size_t space = buf.find(' ');
    if (space != std::string::npos)
    {
        token = buf.substr(0, space);
        cmd = buf.substr(space + 1);
    }
    else
    {
        token = buf;
    }
}

// Say "Received" and immediately leads to callCmd()
void	Server::processRecvData(std::string buf, Client *client, Channel *channel)
{
	size_t pos = buf.find(CRLF);
	if (pos == std::string::npos) // when using irssi
		pos = buf.find("\n"); // when using nc

	while (pos != std::string::npos)
	{
		std::string line = buf.substr(0, pos);
		buf.erase(0, pos + 2);
		pos = buf.find(CRLF);

		std::time_t result = std::time(NULL);
		std::string timeres = std::asctime(std::localtime(&result));

		std::cout << YELLOW << timeres << "Received -> " << RESET << line << std::endl;
		if (!line.empty())
		{
			parseCmd(line);
			callCmd(token, client, channel); // CALL COMMANDS
		}
		else
			errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), "", "", "", "");
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

void	Server::chanErase(Channel *chan)
{
	for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
	{
		if (chan->getChanName() == (*it)->getChanName())
		{
			_chan.erase(it);
			break;
		}
	}
}

std::string Server::parseChannel(std::string input, size_t start)
{
	std::string	chanName;
	size_t		space;
	size_t		comma;
	size_t		end;

	if (start == 0)
		start = input.find("#");

	if (start != std::string::npos && start != input.size() - 1)
	{
		space = input.find(" ", start);
		comma = input.find(",", start);

		end = std::min(space, comma);
		if (end == std::string::npos)
			end = input.size();
		chanName = input.substr(start, end - start);
	}
	else
		chanName = input;
	return chanName;
}

void	Server::isCAP(Client *client)
{
	if (connect(client->getFD(), (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
	{
		msgSend("PING", client->getFD());
		return;
	}
}

bool	Server::chanExist(std::string channel)
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
