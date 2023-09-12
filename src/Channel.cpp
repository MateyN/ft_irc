#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

Channel::Channel()
{
    _chanName = "";
    _topic = "";
}

Channel::Channel(std::string name): _chanName(name)
{
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

    return *this;
}

Channel::~Channel()
{

}

std::string Channel::getChanName()
{
    return _chanName;
}

std::string Channel::getTopic()
{
    return _topic;
}

std::string Channel::getPassword()
{
    return _password;
}

void    Channel::setTopic(std::string topic)
{
    _topic = topic;
}