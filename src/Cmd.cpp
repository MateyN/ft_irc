#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Message.hpp"

// NOTES :
// 1. Change "!!!" which corresponds to var names
// 2. Do connection registration
//

void	Server::callCmd(std::string cmd, Client *client, Channel *channel)
{
	std::string valid_commands[7] = {"CAP", "PING", "NICK", "USER", "JOIN", "PASS", "QUIT" };
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
	std::string pingContent = cmd;
	std::string pongResponse = "PONG " + pingContent;
	msgSend(pongResponse, client->getFD());
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
		int numberFd = client->getFD();
		std::stringstream ss;
		ss << numberFd;
		std::string strNumberFd = ss.str();
		nickname += strNumberFd;
		client->setNickname(nickname);
		std::string msg = ":" + cmd + " NICK " + nickname;
		msgSend(msg, client->getFD());
		client->_setNick = true;
	}
	else {
		std::string newNick = cmd;
		if(newNick.size() > 30)
		{
		errorMsg(ERR432_ERRONEUSNICKNAME, client->getFD(), newNick, "", "", "");
		std::cerr << "Error: Nickname is longer than 30 characters." << std::endl;
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
		std::string UserContent = cmd.substr(colonPos + 1);
		std::string	msg = "USER : " + UserContent + CRLF;
		client->setUser(UserContent);
        welcomeMsg(client);
		}
	}

void	Server::JOIN(Client *client, Channel *channel)
{
	(void)channel;
	std::cout << LIGHTGREEN << "JOIN COMMAND" << RESET << std::endl;

	bool						channelExists = false;
	std::string					chanName;
	size_t						pos = 0;
	size_t						hashtagPos = 0;
	std::vector<std::string>	channelsToAdd;
	std::string					msg;
	std::string					passwordEntered;

	if (cmd == ":")
		return;
	while ((hashtagPos = cmd.find("#", hashtagPos)) != std::string::npos)
	{
		chanName = parseChannel(cmd, hashtagPos);

		if (chanName[0] != '#')
			chanName = '#' + chanName;

		channelsToAdd.push_back(chanName);

		hashtagPos = hashtagPos + 1;
	}
	if (channelsToAdd.empty())
	{
		errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), client->getNickname(), "JOIN", "Not enough parameters", "");
		return;
	}
	if ((pos = cmd.find(" ")) != std::string::npos)
	{
		if (cmd.find(":") == std::string::npos)
			passwordEntered = cmd.substr(pos + 1);
	}
	std::cout << ": [" + passwordEntered + "]" << std::endl;

	for (std::vector<std::string>::iterator itc = channelsToAdd.begin(); itc != channelsToAdd.end(); itc++)
	{
		for (std::vector<Channel*>::iterator	it = _chan.begin(); it != _chan.end(); it++)
		{
			if (((*it)->getChanName() == (*itc)))
			{
				channelExists = true;

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
					if (passwordEntered != channel->getPassword())
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
				std::cout << "Channel [" + (*itc) + "] already exists" << std::endl;
				channels = (*it);
				channels->addUser(client);
				break;
			}
		}
		if (!channelExists)
		{
			channels = addChan((*itc));
			std::cout << "Channel [" + (*itc) + "] created." << std::endl;
			channels->addUser(client);
			channels->addOp(client);
		}
		pos = cmd.find(" :");
		if (pos != std::string::npos && (std::string::npos + 1) != (*itc).size())
			channels->setTopic(cmd.substr(pos + 2, cmd.size()), client);
		msg = ":" + client->getNickname() + "@" + client->getHost() + " JOIN " + (*itc);
		msgSend(msg, client->getFD());
		if (!channelExists)
		{
			msg = "MODE " + (*itc) + " +o "+ client->getNickname();
			msgSend(msg, client->getFD());
		}
		msg = ":" + client->getNickname() + "@" + client->getHost() + " JOIN " + (*itc);
		sendToUsersInChan(msg, client->getFD());

		if (!channels->getTopic().empty())
		{
			msg = "TOPIC " + (*itc) + " :" + channels->getTopic();

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
	std::cout << "Client " << client->getNickname() << " has quit." << std::endl;
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