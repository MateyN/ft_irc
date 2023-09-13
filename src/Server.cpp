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

        return (true);
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
    while(true)
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

void    Server::newClientConnect()
{
    socklen_t   addrlen = sizeof(_addr);
    int         cliSocket = accept(_socket, (struct sockaddr *)&_addr, &addrlen); // store the client socket

    if (cliSocket != ERROR)
    {
        // init a new pollfd struct to monitor the client's socket
        pollfd  pfdc;
        memset(&pfdc, 0, sizeof(pfdc));
        pfdc.fd = cliSocket;
        pfdc.events = POLLIN;
        _pfds.push_back(pfdc); // add the pollfd struct for event monitoring

        client = addClient(cliSocket); // create and init a new client obj
        std::cout << "New client connected" << std::endl;
    }
    else
        throw (Server::ExceptionServer(ERRNOMSG"error: accept()"));
}

// handling data received from a client
void    Server::clientData(pollfd &pfdc)
{
    char    buf[500];
    int     storedBytes = recv(pfdc.fd, buf, sizeof(buf), 0); // receive and store data

    if (storedBytes <= 0) // if no data received or connected client
    {
       clientDisc(pfdc.fd);
    }
    else
    {
        processRecvData(pfdc.fd, buf, storedBytes); // process what is received
    }
}

void    Server::processRecvData(int send, char *data, int size)
{
    std::string recvData(data, size);
    // add the received data to the message buffer for the sender
    msgBuffer[send] += recvData;
    // checks if the received data contains a complete message
    size_t  findEnd = msgBuffer[send].find("\r\n");
    while (findEnd != std::string::npos)
    {
        std::string completeMsg = msgBuffer[send].substr(0, findEnd);
        // removing the processed message from the buffer
        msgBuffer[send].erase(0, findEnd + sizeof("\r\n"));
        // finding the next complete message in the buffer
        findEnd = msgBuffer[send].find("\r\n");
    }
}

// handling the client disconnection
void    Server::clientDisc(int pfdc)
{
    for (size_t i = 0; i < _pfds.size(); i++)
    {
        if (_pfds[i].fd == pfdc) // checks for matching poll struct for the disc client
        {
            for (std::vector<Client *>:: iterator cli = _cli.begin(); cli != _cli.end(); cli++)
            {
                if ((*cli)->getFD() == pfdc)
                {
                    clientsErase(*cli); // cleanup disc clients
                    break;
                }
            }
            _pfds.erase(_pfds.begin() + i); // removing pollfd struct with the disc. client
            break ;
        }
    }
}

// cleaning up 
void    Server::clientsErase(Client *client)
{
    for (std::vector<Channel *>::iterator iter = _chan.begin(); iter != _chan.end(); iter++)
    {
        if ((*iter)->User(client)) // checks if its a user
        {
            (*iter)->eraseUser(client, client->getFD());
        }
        if ((*iter)->Op(client)) // checks if its an operator in the channnel or not
        {
            (*iter)->eraseOp(client, client->getFD());
        }
    }
    delete client;
    _cli.erase(std::remove(_cli.begin(), _cli.end(), client), _cli.end()); // remove the client pointer from the list
}


Client *Server::addClient(int fd)
{
    Client  *client = new Client(fd);
    return client;
}

Channel *Server::addChan(std::string name)
{
    Channel *chan = new Channel(name);
    return chan;
}

void    Server::chanErase(Channel *chan)
{
    for (std::vector<Channel *>::iterator iter = _chan.begin(); iter != _chan.end(); iter++)
    {
        if (chan->getChanName() == (*iter)->getChanName())
        {
            _chan.erase(iter);
            break;
        }
    }
}