#include "../inc/Server.hpp"

Server::Server():   validPass(false), _socket(0), _port(0)
{

}

// Copy assignment operator
Server& Server::operator=(Server const & rhs)
{
    setNick = rhs.setNick;
    validPass = rhs.validPass;
    token = rhs.token;
    cmd = rhs.cmd;
    _port = rhs._port;
    _socket = rhs._socket;
    password = rhs.password;
    _addr = rhs._addr;

    return *this;
}

// Default Destructor 
Server::~Server(void)
{
    if (client != NULL)
    {
        delete client;
        client = NULL;
    }
	//std::cout << "Destructor Server Called" << std::endl;
	//return;
}

// Getters
std::string Server::getPass()
{
    return password;
}

int Server::getSocket()
{
    return (_socket);
}

int Server::getPort()
{
    return (_port);
}

// Setters
void    Server::setPort(int port)
{
    _port = port;
}

void    Server::setPass(std::string pass)
{
    password = pass;
}

bool    Server::setupServerSocket()
{
    int clientSocket;
    int opt_len;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == ERROR)
		throw (Server::ExceptionServer(ERRNOMSG"error: SOCK_STREAM"));

    // Set client socket to non-blocking
    clientSocket = fcntl(_socket, F_SETFL, O_NONBLOCK);
    if (_socket == ERROR)
        throw (Server::ExceptionServer(ERRNOMSG"error: socket"));

    clientSocket = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt_len, sizeof (opt_len) + 1);
    if (clientSocket = ERROR)
        throw (Server::ExceptionServer(ERRNOMSG"error: setsockopt"));
    
    memset(&_addr, 0, sizeof _addr);
    _addr.sin_family = AF_UNSPEC;		// don't care IPv4 or IPv6, AF = Address Family
	_addr.sin_addr.s_addr = INADDR_ANY; // setting the server's IP address to INADDR_ANY, it will bind to all available network interfaces.
	_addr.sin_port = htons(_port); // setting the server's port number & converts it to network byte order.

    clientSocket = bind(_socket, (struct sockaddr *)&_addr, sizeof(_addr));
    if (clientSocket == ERROR)
        throw (Server::ExceptionServer(ERRNOMSG"error: fail bind"));

    clientSocket = listen(_socket, MAX_CLIENTS);
    if (clientSocket == ERROR)
        throw (Server::ExceptionServer(ERRNOMSG"error: fail listen"));

        return (true);
}

void    Server::initServSocket()
{
    pollfd  pfds;
    bzero(&pfds, sizeof(pfds));
    pfds.fd = _socket;
    pfds.events = POLLIN;
    _pfds.push_back(pfds);
}

bool    Server::serverConnect()
{
    initServSocket();
    //int nfds;

    while(true)
    {
        // waiting for events
       int poll_i = poll(_pfds.data(), _pfds.size(), -1); // -1 listen for incoming data or client connections without timeout
       if (poll_i == ERROR)
            throw (Server::ExceptionServer(ERRNOMSG"error: poll()"));
        if (poll_i > 0)
        {
            for (size_t i = 0; i < _pfds.size(); i++)
            {
                if (_pfds[i].revents & POLLIN)
                {
                    if (_pfds[i].fd == _socket)
                    {
                        handleNewCliConnect();
                    }
                    else
                    {
                        //handleCliData(); //TBD
                    }
                }
            }
        }
    }
}

void    Server::handleNewCliConnect()
{
    socklen_t   addrlen = sizeof(_addr);
    int         socket_i = accept(_socket, (struct sockaddr *)&_addr, &addrlen);

    if (socket_i != ERROR)
    {
        pollfd  pfdc;
        bzero(&pfdc, sizeof(pfdc));
        pfdc.fd = socket_i;
        pfdc.events = POLLIN;
        _pfds.push_back(pfdc);

        client = addClient(socket_i);
        std::cout << "New client connected" << std::endl;
    }
    else
        throw (Server::ExceptionServer(ERRNOMSG"error: addClient()"));
}


Client *Server::addClient(int fd)
{
    Client  *client = new Client(fd);
    return client;
}