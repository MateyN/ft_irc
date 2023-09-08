#include "../inc/Server.hpp"

Server::Server():   validPass(false), _socket(0), _port(0)
{

}

// Copy assignment operator
Server& Server::operator=(Server const & rhs)
{
    token = rhs.token;
    cmd = rhs.cmd;
    setNick = rhs.setNick;
    validPass = rhs.validPass;
    _port = rhs._port;
    _socket = rhs._socket;
    password = rhs.password;
    _addr = rhs._addr;

    return *this;
}

// Default Destructor 
Server::~Server(void)
{
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

bool    Server::serverConnect()
{
    int nfds; // fd to be monitored
    int socket_i; // store the client socket
    int poll_i; // store result of poll
    pollfd  pfds[5];
    char    buff[500];
    socklen_t   addrlen;

    pfds[0].fd = _socket;
    pfds[0].events = POLLIN;

    while(true)
    {
        // waiting for events
        poll_i = poll(pfds, nfds, -1); // -1 listen for incoming data or client connections without timeout
        if (poll_i == ERROR)
            throw (Server::ExceptionServer(ERRNOMSG"error: poll()"));
        for (int i = 0; i < nfds; i++)
        {
            if (pfds[i].revents && POLLIN) // ready to read?
            {
               if (pfds[i].fd == _socket) // check if the event is in the server socket
               {
                    addrlen = sizeof(_addr);
                    socket_i = accept(_socket, (struct sockaddr *)&_addr, &addrlen);
                    if (socket_i != ERROR)
                    {
                        // adding the new client socket for monitoring.
                        pfds[nfds].fd = socket_i;
                        pfds[nfds].events = POLLIN;
                        nfds++;
                    }
                    else
                        throw (Server::ExceptionServer(ERRNOMSG"error: accept()"));
               }
            }
        }
    }
}