#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

Client *Server::addClient(int fd)
{
    Client  *client = new Client(fd);
    return client;
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
        // ADD CONNENCTION
        //handleNewClientConnect(client);
    }
    else
        throw (Server::ExceptionServer(ERRNOMSG"error: accept()"));
}

void    Server::handleNewClientConnect(Client *client)
{
    std::cout << "New Client: " << client->getFD() << std::endl;
    if (connect(client->getFD(), (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
    {
        std::cerr << "Error connecting to server!" << std::endl;
    }
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
        handleMessage(send, completeMsg);
        // removing the processed message from the buffer
        msgBuffer[send].erase(0, findEnd + sizeof("\r\n"));
        // finding the next complete message in the buffer
        findEnd = msgBuffer[send].find("\r\n");
    }
}

void    Server::handleMessage(int send, const std::string& msg)
{
    (void) send;
    clientMsg(msg, client, channel);
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
