#include "../inc/Server.hpp"

Server::Server(Server const & src)
{
	std::cout << "Copy Constructor Server Called" << std::endl;
	*this = src;
	return;
}

// Copy assignment operator
Server& Server::operator=(Server const & rhs)
{
    std::cout << "Copy Assignment Operator Server Called" << std::endl;
    if (this != &rhs)
    {
        this->serverSocket = rhs.serverSocket;
        this->maxClients = rhs.maxClients;
        this->password = rhs.password;
    }
    return *this;
}

// Default Destructor 
Server::~Server(void)
{
	std::cout << "Destructor Server Called" << std::endl;
	return;
}

Server::Server(int port, const std::string &pass) : maxClients(10), password(pass)
{
    setupServerSocket(port);
}

void Server::setupServerSocket(int port) 
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
	{
        err("Socket error");
        exit(1);
    }
    // ...
}

void Server::run()
{
    while (true)
	{
        acceptClients();
        // Handle client connections and other server tasks
    }
}

void Server::acceptClients()
{
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);

    if (clientSocket != -1)
	{
        // Set client socket to non-blocking
        fcntl(clientSocket, F_SETFL, O_NONBLOCK);

        handleClientConnection(clientSocket);
    }
}

void Server::handleClientConnection(int clientSocket)
{
    
}

int Server::getServer() const
{
    return serverSocket;
}

int Server::getNbInst()
{
    return Server::_nbInst;
}