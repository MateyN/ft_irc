#include "../inc/Client.hpp"

Client::Client(): _fd(0), _connect(false)
{
    _username = "";
    _nick = "";
    _setNick = false;
    _isRegister = false;
}

Client::Client(int fd):	_setNick(false), _fd(fd), _connect(false)
{

}

Client::Client(const Client &src)
{
	*this = src;
}

Client	&Client::operator=(const Client &rhs)
{
    _setNick = rhs._setNick;
    _fd = rhs._fd;
    _connect = rhs._connect;
    _nick = rhs._nick;
    _username = rhs._username;
    _isRegister = rhs._isRegister;

	return *this;
}

Client::~Client()
{
    //std::cout << "Des ctor Client Called" << std::endl;
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

void	Client::setUser(std::string user)
{
	_username = user;
}

std::string	Client::getUser()
{
	return _username;
}

std::string Client::getHost()
{
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == 0)
	{
		return std::string(hostname);
	}
	return "";
}

bool	Client::isRegister()
{
	return _isRegister;
}

void	Client::setIsRegister(bool registered)
{
		_isRegister = registered;
}

bool	Client::isConnect()
{
	return _connect;
}