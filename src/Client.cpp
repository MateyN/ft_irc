#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

Client::Client():   _fd(0), _connect(false)
{
    _username = "";
    _nick = "";
    _setNick = false;
    _isRegister = false;
}

Client::Client(int fd): _setNick(false), _fd(fd), _connect(false)
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
    _connect = rhs._connect;
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
void		Client::setNickname(std::string nick)
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

bool    Client::isConnect()
{
    return _connect;
}

void 	Client::joinChannel(Channel *channel)
{
	channel->add_user(this);
	this->_channels.insert(std::pair<std::string, Channel *>(channel->getChanName(), channel));
}

void 	Client::leaveChannel(Channel &channel)
{
	channel.remove_user(this->get_fd());
	_channels.erase(channel.getChanName());
	send(MSG(_nick, _username, "PART", channel.getChanName()));
}

bool	Client::inChannel(const std::string& channel_name)
{
	std::map<std::string, Channel *> &channels = this->get_channels();

	std::map<std::string, Channel *>::iterator it = channels.find(channel_name);
	if (it == channels.end())
		return false;
	return true;
}

