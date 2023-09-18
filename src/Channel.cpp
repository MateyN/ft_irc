#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

Channel::Channel()
{
    _chanName = "";
    _topic = "";
}

Channel::Channel(std::string name, Client* op): _chanName(name)
{
	_op.push_back(op); // add operator to operator list
    _topic = "";
}

Channel::Channel(const Channel& src)
{
    *this = src;
}

Channel& Channel::operator=(Channel const & rhs)
{
    _chanName = rhs._chanName;
    _topic = rhs._topic;
    _password = rhs._password;
    _usr = rhs._usr;
    _op = rhs._op;

    return *this;
}

Channel::~Channel()
{

}

// setters
void    Channel::setTopic(std::string topic)
{
    _topic = topic;
}

void    Channel::addUser(Client *client)
{
    _usr.push_back(client);
    return ;
}

void    Channel::eraseUser(Client *client, int fd)
{
    (void) client;
    for (std::vector<Client *>::iterator usr = _usr.begin(); usr != _usr.end(); usr++)
    {
        if ((*usr)->getFD() == fd)
        {
            _usr.erase(usr);
			// also erase from op if usr is op
            return ;
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

bool    Channel::User(Client *client)
{
    for (std::vector<Client *>::iterator usr = _usr.begin(); usr != _usr.end(); usr++)
    {
        if ((*usr)->getFD() == client->getFD())
        return true;
    }
    return false;
}

void    Channel::addOp(Client *client)
{
    _op.push_back(client);
    return ;
}

void    Channel::eraseOp(Client *client, int fd)
{
    (void) client; // unused for now
    for (std::vector<Client *>::iterator op = _op.begin(); op != _op.end(); op++)
    {
        if ((*op)->getFD() == fd)
        {
            _op.erase(op);
            return ;
        }
    }
}

bool    Channel::Op(Client *client)
{
    (void) client; // unused for now
    for (std::vector<Client *>::iterator op = _op.begin(); op != _op.end(); op++)
    {
        if ((*op)->getFD() == client->getFD())
        return true;
    }
    return false;
}
