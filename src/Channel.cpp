#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Channel.hpp"

Channel::Channel()
{
	_chanName = "";
	_topic = "";
	_topicOp = false;
	_isLimitSet = false;
}

Channel::Channel(std::string name): _chanName(name)
{
	_topic = "";
	_topicOp = false;
	_isLimitSet = false;
}

Channel::Channel(const Channel &src)
{
	*this = src;
}

Channel &Channel::operator=(const Channel &rhs)
{
	_chanName = rhs._chanName;
	_usr = rhs._usr;
	_op = rhs._op;
	_topic = rhs._topic;
	_topicOp = rhs._topicOp;
	_isLimitSet = rhs._isLimitSet;
	_countLimit = rhs._countLimit;

	return *this;
}

Channel::~Channel()
{

}

// getters
bool	Channel::getTopicMode()
{
	return _topicOp;
}

bool	Channel::getLimitMode()
{
	return _isLimitSet;
}

bool	Channel::getPassMode()
{
	return _isPassSet;
}

bool	Channel::getInviteMode()
{
	return _isInvite;
}

int	Channel::getLimit()
{
	return _countLimit;
}

std::string	Channel::getUsers()
{
	std::string	users;

	for (std::vector<Client*>::iterator it = _usr.begin(); it != _usr.end(); it++)
	{
		users += (*it)->getNickname() + "@" + (*it)->getHost() + " ";
	}
	return users;
}

// setters
void	Channel::setTopic(std::string topic, Client *client)
{
	if (_topicOp && (!Op(client)))
	{
		return ;
	}
	_topic = topic;
}

void	Channel::setTopicMode(bool mode)
{
	_topicOp = mode;
}

void	Channel::setLimit(bool mode, int limit)
{
	_isLimitSet = mode;

	if (mode)
		_countLimit = limit;
	else
		_countLimit = -1;
	
	std::cout << "Limit = " << _countLimit << std::endl;
}

bool	Channel::setOp(bool mode, std::string username)
{
	for(std::vector<Client*>::iterator it = _usr.begin(); it != _usr.end(); it++)
	{
		if (mode)
		{
			if ((*it)->getNickname() == username)
				return (addOp(*it));
		}
		else
		{
			if ((*it)->getNickname() == username)
				return (eraseOp(*it));
		}
	}
	return (false);
}

void	Channel::setPassMode(bool mode)
{
	_isPassSet = mode;
}

void	Channel::setChanPass(std::string password)
{
	_password = password;
}

void	Channel::setInviteMode(bool mode)
{
	_isInvite = mode;
}

void	Channel::addUser(Client *client)
{
	_usr.push_back(client);
	return;
}

void	Channel::addGuest(Client *client)
{
	_guest.push_back(client);
	return;
}

void	Channel::eraseUser(Client *client, int fd)
{
	(void)client;
	for (std::vector<Client*>::iterator it = _usr.begin(); it != _usr.end(); it++)
	{
		if ((*it)->getFD() == fd)
		{
			_usr.erase(it);
			return;
		}
	}
}

bool	Channel::fdIsInvited(int fdc)
{
	for (std::vector<int>::iterator	it = _invite.begin(); it != _invite.end(); it++)
	{
		if ((*it) == fdc)
			return (true);
	}
    return (false);
}

bool	Channel::fdIsBanned(int fdc)
{
	for (std::vector<int>::iterator	it = _banned.begin(); it != _banned.end(); it++)
	{
		if ((*it) == fdc)
			return (true);
	}
    return (false);
}

bool	Channel::User(Client *client)
{
	for (std::vector<Client*>::iterator it = _usr.begin(); it != _usr.end(); it++)
	{
		if ((*it)->getFD() == client->getFD())
			return true;
	}
	return false;
}

bool	Channel::userExist(const std::string& nickname)
{
	for (std::vector<Client*>::iterator it = _usr.begin(); it != _usr.end(); it++)
	{
		if ((*it)->getNickname() == nickname)
			return true;
	}
	return false;
}

bool Channel::addOp(Client *client)
{
    if (!_op.empty())
    {
        std::cout << "Channel already has operators." << std::endl;
        return false;
    }
    bool user = false;
    for (std::vector<Client *>::iterator it = _usr.begin(); it != _usr.end(); it++)
    {
        if (client->getFD() == (*it)->getFD())
        {
            user = true;
            break;
        }
    }

    if (user)
    {
        _op.push_back(client);
        return (true);
    }

    std::cout << "Not a member -> " + client->getNickname() << std::endl;
    return (false);
}


bool	Channel::eraseOp(Client *client)
{
	bool		user = false;
	std::string	msg;

	for (std::vector<Client*>::iterator it = _usr.begin(); it != _usr.end(); it++)
	{
		if (client->getFD() == (*it)->getFD())
		{
			user = true;
			break ;
		}
	}
	if (user)
	{
		for (std::vector<Client*>::iterator itc = _op.begin(); itc != _op.end(); itc++)
		{
			if (client->getFD() == (*itc)->getFD())
			{
				std::cout << "The operator has removed -> " + (*itc)->getNickname() << std::endl;
				_op.erase(itc);
				return (true);
			}
		}
	}
	std::cout << "Not a member -> " + client->getNickname() << std::endl;
	return (false);

}

bool	Channel::Op(Client *client)
{
	for (std::vector<Client*>::iterator it = _op.begin(); it != _op.end(); it++)
	{
		if ((*it)->getFD() == client->getFD())
			return true;
	}
	return false;
}

bool	Channel::isNumber(std::string arg)
{
	for (unsigned int i = 0; i < arg.length(); i++)
	{
		if (!isdigit(arg[i]))
			return false;
	}
	return true;
}

bool	Channel::nickMember(std::string nickname)
{
	for (std::vector<Client*>::iterator it = _usr.begin(); it != _usr.end(); it++)
	{
		if ((*it)->getNickname() == nickname)
			return true;
	}
	return false;
}


bool	Channel::Guest(Client *client)
{
	for (std::vector<Client*>::iterator it = _guest.begin(); it != _guest.end(); it++)
	{
		if ((*it)->getFD() == client->getFD())
			return true;
	}

	return false;
}

void	Channel::eraseGuest(Client *client)
{
	for(std::vector<Client*>::iterator it = _guest.begin(); it != _guest.end(); it++)
	{
		if ((*it)->getFD() == client->getFD())
		{
			_guest.erase(it);
			break;
		}
	}
}
