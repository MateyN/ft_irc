#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Message.hpp"
#include "../inc/Channel.hpp"


void	Server::callCmd(std::string cmd, Client *client, Channel *channel)
{

	std::string valid_commands[7] = {"CAP", "PING", "NICK", "USER", "JOIN", "PASS", "QUIT"};

	void	(Server::*funcPtr[])(Client *client, Channel *channel) =
	{
		&Server::CAP,
		&Server::PING,
		&Server::NICK,
		&Server::USER,
		&Server::JOIN,
		&Server::PASS,
		&Server::QUIT
	};

	for (int i = 0; i < 7; i++)
	{
		if (cmd.compare(valid_commands[i]) == 0)
		{
			(this->*funcPtr[i])(client, channel);
			return;
		}
	}
}

void	Server::CAP(Client *client, Channel *channel)
{
	(void)channel;
	if (cmd[0] == 'L' && cmd[1] == 'S')
		msgSend("CAP * LS :", client->getFD());
	else
		return;
}

void	Server::PING(Client *client, Channel *channel)
{
	(void)channel;
	std::string ping = token;
	std::string sendPing = cmd;
	std::string clientResponse = "PONG " + sendPing;
	msgSend(clientResponse, client->getFD());
}

void Server::NICK(Client *client, Channel *channel)
{
	(void)channel;
	if (client->isRegister() == false)
	{
		return;
	}

	if (client->_setNick == false)
	{
		std::string nickname = cmd;
		int nick = client->getFD();
		std::stringstream ss;
		ss << nick;
		std::string strNick = ss.str();
		nickname += strNick;
		client->setNickname(nickname);
		std::string msg = ":" + cmd + " NICK " + nickname;
		msgSend(msg, client->getFD());
		client->_setNick = true;
	}
	else 
	{
		std::string newNick = cmd;
		if(newNick.size() > 25)
		{
		errorMsg(ERR432_ERRONEUSNICKNAME, client->getFD(), newNick, "", "", "");
		std::cerr << "Error: Nickname is longer than 25 characters." << std::endl;
		return;
	}
	for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); ++it)
	{
		if ((*it)->getNickname() == newNick)
		{
			errorMsg(ERR433_NICKNAMEINUSE, client->getFD(), newNick, "", "", "");
			return;
		}
	}
	if (newNick.empty() || newNick[0] == '#' || newNick[0] == ':' || newNick.find_first_of("&#") != std::string::npos || newNick.find(' ') != std::string::npos)
	{
		errorMsg(ERR432_ERRONEUSNICKNAME, client->getFD(), newNick, "", "", "");
		return ;
	}
		for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); ++it)
		{
			if ((*it)->getFD() == client->getFD())
			{
				std::string oldNick;
				for (std::vector<Client*>::iterator iter = _cli.begin(); iter != _cli.end(); ++iter)
				{
					if ((*iter)->getFD() == client->getFD()) 
					{
						oldNick = (*iter)->getNickname();
						break;
					}
				}
				(*it)->setNickname(newNick);
				std::string msg;
				msg = ":" + oldNick + " NICK " + newNick;
				msgSend(msg, client->getFD());
				break;
			}
		}
	}
}

void	Server::USER(Client *client, Channel *channel)
{
	(void)channel;

	if (client->isRegister() == false)
	{
		return;
	}
	std::size_t colonPos = cmd.find(':');
	if (colonPos != std::string::npos)
	{
		std::string user = cmd.substr(colonPos + 1);
		std::string	msg = "USER : " + user + CRLF;
		client->setUser(user);
        welcomeMsg(client);
	}
}

void	Server::JOIN(Client *client, Channel *channel)
{
	(void)channel;
	std::cout << GREEN << "COMMAND JOIN" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;

	bool						isChannelFound = false;
	std::string					channelName;
	size_t						currPos = 0;
	size_t						start = 0;
	std::vector<std::string>	channelsToJoin;
	std::string					msg;
	std::string					pass;

	if (cmd == ":")
		return;
	while ((start = cmd.find("#", start)) != std::string::npos)
	{
		channelName = parseChannel(cmd, start);

		if (channelName[0] != '#')
			channelName = '#' + channelName;

		channelsToJoin.push_back(channelName);

		start = start + 1;
	}
	if (channelsToJoin.empty())
	{
		errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), client->getNickname(), "JOIN", "Not enough parameters", "");
		return;
	}
	if ((currPos = cmd.find(" ")) != std::string::npos)
	{
		if (cmd.find(":") == std::string::npos)
			pass = cmd.substr(currPos + 1);
	}
	std::cout << ": [" + pass + "]" << std::endl;
	for (std::vector<std::string>::iterator iter = channelsToJoin.begin(); iter != channelsToJoin.end(); iter++)
	{
		for (std::vector<Channel*>::iterator	it = _chan.begin(); it != _chan.end(); it++)
		{
			if (((*it)->getChanName() == (*iter)))
			{
				isChannelFound = true;

				if (channel->getLimitMode())
				{
					if (static_cast<int>(channel->getUser().size()) >= channel->getLimit())
					{
						errorMsg(ERR471_CHANNELISFULL, client->getFD(), client->getNickname(), channel->getChanName(), "", "");
						return ;
					}
				}
				if (channel->getPassMode())
				{
					if (pass != channel->getPassword())
					{
						errorMsg(ERR475_BADCHANNELKEY, client->getFD(), client->getNickname(), channel->getChanName(), "", "");
						return ;
					}
				}
				if (channel->getInviteMode())
				{
					if (channel->User(client))
					{
						errorMsg(ERR443_USERONCHANNEL, client->getFD(), client->getNickname(), channel->getChanName(), "", "");
						return ;
					}
					if (!(channel->Guest(client)))
					{
						errorMsg(ERR473_INVITEONLYCHAN, client->getFD(), channel->getChanName(), client->getNickname(), "", "");
						return ;
					}
					std::cout << "In the list." << std::endl;
				}
				std::cout << "Channel [" + (*iter) + "] already exists." << std::endl;
				channels = (*it);
				channels->addUser(client);
				break;
			}
		}
		if (!isChannelFound)
		{
			channels = addChan((*iter));
			std::cout << "Channel [" + (*iter) + "] created." << std::endl;
			channels->addUser(client);
			channels->addOp(client);
		}
		currPos = cmd.find(" :");
		if (currPos != std::string::npos && (std::string::npos + 1) != (*iter).size())
			channels->setTopic(cmd.substr(currPos + 2, cmd.size()), client);
		msg = ":" + client->getNickname() + "@" + client->getHost() + " JOIN " + (*iter);
		msgSend(msg, client->getFD());
		if (!isChannelFound)
		{
			msg = "MODE " + (*iter) + " +o "+ client->getNickname();
			msgSend(msg, client->getFD());
		}
		msg = ":" + client->getNickname() + "@" + client->getHost() + " JOIN " + (*iter);
		sendToUsersInChan(msg, client->getFD());

		if (!channels->getTopic().empty())
		{
			msg = "TOPIC " + (*iter) + " :" + channels->getTopic();

			msgSend(msg, client->getFD());
			sendToUsersInChan(msg, client->getFD());
		}
	}
}

void	Server::PASS(Client *client, Channel *channel)
{
	(void)channel;
	std::string pass = cmd;
	if (pass.empty())
	{
		std::cout << "PASS" << std::endl;
		errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), client->getNickname(), "", "", "");
		return ;
	}
	if (pass != getPassword())
	{
		std::cout << "PASS" << std::endl;
		errorMsg(ERR464_PASSWDMISMATCH, client->getFD(),"", "", "", "");
		return ;
	}
	else {
		if (client->isRegister() == true)
		{
			errorMsg(ERR462_ALREADYREGISTERED, client->getFD(),"", "", "", "");
			return ;
		}
		else 
		{
			client->setIsRegister(true);
		}
	}
}

void Server::QUIT(Client *client, Channel *channel)
{
	(void)channel;
	std::cout << RED << "Client " << client->getNickname() << " has quit." << RESET << std::endl;
	int clientSocket = client->getFD();
	close(clientSocket);

	for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
	{
		if ((*it)->Guest(client))
			((*it)->eraseGuest(client));

		if ((*it)->Op(client))
			((*it)->eraseOp(client));

		if ((*it)->User(client))
			(*it)->eraseUser((client), client->getFD());
	}
	for (size_t i = 0; i < _cli.size(); i++)
	{
		if (_cli[i]->getFD() == clientSocket)
		{
			delete (client);
			_cli.erase(_cli.begin() + i);
			break;
		}
	}
}
