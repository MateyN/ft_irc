#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

Channel *Server::addChan(std::string name)
{
    Channel *chan = new Channel(name);
    _chan.push_back(chan);
    return chan;
}

// check if the channel already exists
bool    Server::isChannel(std::string chan)
{
    for (std::vector<Channel *>::iterator iter = _chan.begin(); iter != _chan.end(); iter++)
    {
        if ((*iter)->getChanName() == chan)
        return 1;
    }
    return 0;
}

Channel *Server::getChan(std::string msg)
{
    std::string chanName = channelParse(msg, 0);
    for (std::vector<Channel *>::iterator iter = _chan.begin(); iter != _chan.end(); iter++)
    {
        if ((*iter)->getChanName() == chanName)
            return (*iter);
    }
    return (NULL);
}

void    Server::chanErase(Channel *chan)
{
    for (std::vector<Channel *>::iterator iter = _chan.begin(); iter != _chan.end(); iter++)
    {
        if (chan->getChanName() == (*iter)->getChanName())
        {
            _chan.erase(iter);
            break;
        }
    }
}