#include "../inc/Client.hpp"

Client::Client():   _fd(0)
{
    _username = "";
    _nick = "";
    setNick = false;
}

Client::Client(int fd): setNick(false), _fd(fd)
{

}

Client::Client(const Client& src)
{
    *this = src;
}

Client& Client::operator=(const Client& rhs)
{
    setNick = rhs.setNick;
    _fd = rhs._fd;
    _nick = rhs._nick;
    _username = rhs._username;

    return (*this);
}

Client::~Client(void)
{
    //std::cout << "Destructor Client Called" << std::endl;
	//return;
}

int		Client::getFD() const
{
	return (_fd);
}

std::string	Client::getNickname()
{
	return _nick;
}

void	Client::setNickname(std::string nick)
{
	_nick = nick;
}

void		Client::setUser(std::string user)
{
	_username = user;
}

std::string	Client::getUser()
{
	return _username;
}