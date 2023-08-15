# include "Server.hpp"
# include <iostream>

// PUBLIC

// Default Constructor 
Server::Server(void)
{
	std::cout << "Default Constructor Server Called" << std::endl;
	return;
}

// Copy Constructor 
Server::Server(Server const & src)
{
	std::cout << "Copy Constructor Server Called" << std::endl;
	*this = src;
	return;
}

// Copy assignment operator
Server &	Server::operator=(Server const & rhs)
{
	std::cout << "Copy Assignment Operator Constructor Server Called" << std::endl;
	if (this != &rhs)
		this->_server = rhs.getServer();
	return *this;
}

// Default Destructor 
Server::~Server(void)
{
	std::cout << "Destructor Server Called" << std::endl;
	return;
}

int	Server::getServer(void) const
{
	return this->_server;
}

int	Server::getNbInst(void)
{
	return Server::_nbInst;
}
// PRIVATE

