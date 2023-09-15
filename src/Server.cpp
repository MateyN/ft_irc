#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

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
    _cli = rhs._cli;
    _chan = rhs._chan;

    return *this;
}

Server::Server(const Server& src)
{
    *this = src;
}

// Default Destructor 
Server::~Server(void)
{
    if (client != NULL)
    {
        delete client;
        client = NULL;
    }
    if (channel != NULL)
    {
        delete channel;
        channel = NULL;
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
    if (clientSocket == ERROR)
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

        return (1);
}

void    Server::initServSocket()
{
    pollfd  pfds;
    memset(&pfds, 0, sizeof(pfds));
    pfds.fd = _socket;
    pfds.events = POLLIN;
    _pfds.push_back(pfds);
}

// handling client connections and data reception.
bool    Server::serverConnect()
{
    initServSocket();
    //int nfds;
    while(1)
    {
        // waiting for events
       int pollResult = poll(_pfds.data(), _pfds.size(), -1); // -1 listen for incoming data or client connections without timeout
       if (pollResult == ERROR)
            throw (Server::ExceptionServer(ERRNOMSG"error: poll()"));
        if (pollResult > 0) // if there are events to process
        {
            for (size_t i = 0; i < _pfds.size(); i++)
            {
                if (_pfds[i].revents & POLLIN) // checks if the event data is ready to be read
                {
                    if (_pfds[i].fd == _socket)
                    {
                        newClientConnect();
                    }
                    else
                    {
                        clientData(_pfds[i]);
                    }
                }
            }
        }
    }
}

// check if the nickname already exist
bool    Server::isNick(std::string nick)
{
    for (std::vector<Client *>::iterator iter = _cli.begin(); iter != _cli.end(); iter++)
    {
        if ((*iter)->getNickname() == nick)
        return 1;
    }
    return 0;
}

void    Server::clientMsg(std::string msg, Client *client, Channel *channel)
{
    (void) client; // unused for now 
    (void) channel; // unused for now
    size_t pos = msg.find("\r\n");
    while (pos != std::string::npos)
    {
        std::string line = msg.substr(0, pos);
        msg.erase(0, pos + 2);
        pos = msg.find("\r\n");
        std::cout << "Received: " << line << std::endl;
        if (!line.empty())
        {
            parseCmds(line);
        }
        else
            std::cerr << "Error receiving message!" << std::endl;
    }
}