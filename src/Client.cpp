#include "../inc/Client.hpp"

Client::Client(): _fd(0), _connect(false)
{
	_username = "";
	_nick = "";
	_hostname = "";
	_isRegister = false;
	_setNick = false;
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
	_nick = rhs._nick;
	_username = rhs._username;
	_connect = rhs._connect;
	_hostname = rhs._hostname;
	_isRegister = rhs._isRegister;

	return *this;
}

Client::~Client()
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

void	Client::setUser(std::string user)
{
	_username = user;
}

std::string	Client::getUsername()
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
/*
void 	Client::joinChannel(Channel *channel)
{
	channel->addUser(this);
	this->_channels.insert(std::pair<std::string, Channel *>(channel->getChanName(), channel));
}

void 	Client::leaveChannel(Channel &channel)
{
	channel.eraseUser(this, this->getFD());
	_channels.erase(channel.getChanName());
	send(this->getFD(), MSG(_nick, _username, "PART", channel.getChanName()).c_str(), MSG(_nick, _username, "PART", channel.getChanName()).length(), 0);
}

bool	Client::inChannel(const std::string& channel_name)
{
	std::map<std::string, Channel *> &channels = this->getChannels();

	std::map<std::string, Channel *>::iterator it = channels.find(channel_name);
	if (it == channels.end())
		return false;
	return true;
}
*/
