#include "../inc/Client.hpp"

Client::Client():   _fd(0)
{
    _username = "";
    _nick = "";
    _setNick = false;
    _isRegister = false;
}

Client::Client(int fd): _setNick(false), _fd(fd)
{

}

Client::Client(const Client& src)
{
    *this = src;
}

Client& Client::operator=(const Client& rhs)
{
    _setNick = rhs._setNick;
    _fd = rhs._fd;
    _nick = rhs._nick;
    _username = rhs._username;
    _isRegister = rhs._isRegister;

    return *this;
}

Client::~Client(void)
{
    //std::cout << "Destructor Client Called" << std::endl;
	//return;
}

// Getters
int		Client::getFD() const
{
	return (_fd);
}

std::string	Client::getNickname()
{
	return _nick;
}

std::string	Client::getUser()
{
	return _username;
}

// Setters
void	Client::setNickname(std::string nick)
{
	_nick = nick;
}

void		Client::setUser(std::string user)
{
	_username = user;
}

bool    Client::isRegister()
{
    return _isRegister;
}

void    Client::setIsRegister(bool registered)
{
    _isRegister = registered;
}