#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Message.hpp"
#include "../inc/Channel.hpp"

void	Server::callCmd(std::string cmd, Client *client, Channel *channel)
{
	std::string valid_commands[13] = {"CAP", "PING", "PASS",  "NICK", "USER", "JOIN", \
		"PART", "QUIT", "KICK", "INVITE", "TOPIC", "PRIVMSG", "MODE" };

	void	(Server::*funcPtr[])(Client *client, Channel *channel) =
	{
		&Server::CAP,
		&Server::PING,
		&Server::PASS,
		&Server::NICK,
		&Server::USER,
		&Server::JOIN,
		&Server::PART,
		&Server::QUIT,
		&Server::KICK,
		&Server::INVITE,
		&Server::TOPIC,
		&Server::PRIVMSG,
		&Server::MODE
		//&Server::LIST // channel debug 
	};
	for (int i = 0; i < 13; i++)
	{
		if (cmd.compare(valid_commands[i]) == 0 ||
				(i == 4 && cmd.compare("userhost") == 0)) // for user in irssi
		{
			// PASS needs to be set before calling other commands, CAP, PING, PASS can be done without isRegistered()
			if (!client->isRegister() && i >= 3) 
			{
				std::cout << "PASS" << std::endl;
				errorMsg(ERR464_PASSWDMISMATCH, client->getFD(),"", "", "", "");
				return ;
			}
			(this->*funcPtr[i])(client, channel);
			return;
		}
	}
}

void Server::CAP(Client *client, Channel *channel)
{
    (void)channel;
    // Simulate sending a list of supported capabilities
    std::string capList = "multi-prefix";
    // Send the capabilities list to the client
    std::string clientResponse = "CAP * LS :" + capList;
    msgSend(clientResponse, client->getFD());
    // Log the capabilities for debugging (you can adjust the logging as needed)
    std::cout << "Server sent CAP LS response to client: " << clientResponse << std::endl;
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
	std::cout << GREEN << "COMMAND NICK" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;

    (void)channel;

    std::string newNick = cmd;
    if (client->_setNick)
	{
        handleNicknameChange(client, newNick);
    } 
	else
	{
        handleNickSize(client, newNick);
    }
}

void Server::handleNickSize(Client *client, const std::string &newNick)
{
    if (newNick.size() > 8)
	{
        errorMsg(ERR432_ERRONEUSNICKNAME, client->getFD(), newNick, "", "", "");
        std::cerr << "Error: Nickname is longer than 8 characters." << std::endl;
        return;
    }

    // Check for nickname availability
    if (isNicknameInUse(newNick) || isNickValid(newNick))
	{
        errorMsg(ERR433_NICKNAMEINUSE, client->getFD(), newNick, "", "", "");
        return;
    }

    // Set the nickname and send the NICK message
    setNewNick(client, newNick);
}

void Server::handleNicknameChange(Client *client, const std::string &newNick)
{
    // Check if the new nickname is valid
    if (newNick.size() > 8 || isNickValid(newNick))
	{
        errorMsg(ERR432_ERRONEUSNICKNAME, client->getFD(), newNick, "", "", "");
        return;
    }

    // Check for nickname availability
    if (isNicknameInUse(newNick))
	{
        errorMsg(ERR433_NICKNAMEINUSE, client->getFD(), newNick, "", "", "");
        return;
    }

    // Set the new nickname and send the NICK message
    setNewNick(client, newNick);
}

bool Server::isNicknameInUse(const std::string &nickname)
{
    for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); ++it)
    {
        if ((*it)->getNickname() == nickname)
        {
            return true;
        }
    }
    return false;
}

bool Server::isNickValid(const std::string &nickname)
{
    return nickname.empty() || nickname[0] == '#' || nickname[0] == ':' ||
           nickname.find_first_of("&#") != std::string::npos || nickname.find(' ') != std::string::npos;
}

void Server::setNewNick(Client *client, const std::string &newNick)
{
    std::string oldNick = client->getNickname();
    client->setNickname(newNick);
    std::string msg = ":" + oldNick + " NICK " + newNick;
    msgSend(msg, client->getFD());
}

void	Server::USER(Client *client, Channel *channel)
{
	(void)channel;
	size_t colonPos;
	std::string user;

	colonPos = cmd.find(':');
	if (colonPos != std::string::npos)
	{
		user = cmd.substr(colonPos + 1);
		std::string	msg = "USER : " + user + CRLF;
		client->setUser(user);
        welcomeMsg(client);
	}
	if (colonPos == std::string::npos)
        errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), client->getNickname(), "USER", ":Not enough parameters", "");
	if (user.empty())
		errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), client->getNickname(), "USER", ":Not enough parameters", "");
}

void Server::JOIN(Client *client, Channel *channel)
{
	std::cout << GREEN << "COMMAND JOIN" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;

    std::vector<std::string> channelsToJoin;
    std::string pass;
	std::string	msg;
	std::string	name;
	size_t		startPos = 0;
	size_t		pos = 0;
	bool		isChanExist = false;

	// This counters irssi first "JOIN :" command
	if (cmd == ":")
		return ;

	// Populate channelsToJoin
	while ((startPos = cmd.find("#", startPos)) != std::string::npos)
	{
		name = parseChannel(cmd, startPos);
		if (name[0] != '#')
			name = '#' + name;
		channelsToJoin.push_back(name);
		startPos = startPos + 1;
	}

	if (channelsToJoin.empty())
	{
		errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), client->getNickname(), "JOIN", "Not enough parameters", "");
		return;
	}

	// Keep password
	if ((pos = cmd.find(" ")) != std::string::npos)
	{
		if (cmd.find(":") == std::string::npos)
			pass = cmd.substr(pos + 1);
	}

	for (std::vector<std::string>::iterator iter = channelsToJoin.begin(); iter != channelsToJoin.end(); iter++)
	{
		for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
		{
			// If channelsToJoin[i] == _chan[j]
			if (((*it)->getChanName() == (*iter)))
			{
				isChanExist = true;

				// MODE : limit - channel is full
				if (channel->getLimitMode())
				{
					if (TOINT(channel->getUser().size()) >= channel->getLimit())
					{
						errorMsg(ERR471_CHANNELISFULL, client->getFD(), client->getNickname(), channel->getChanName(), "", "");
						return ;
					}
				}

				// MODE : password - wrong one
				{
					if (pass != channel->getPassword())
					{
						errorMsg(ERR475_BADCHANNELKEY, client->getFD(), client->getNickname(), channel->getChanName(), "", "");
						return ;
					}
				}

				// MODE : invite only
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
				}

				/* std::cout << "Channel [" + (*iter) + "] already exist." << std::endl; */
				channels = (*it);
				channels->addUser(client);
				break;
			}
		}

		// If channel does not exist
		if (!isChanExist)
		{
			// Create chan and add user in it
			channels = addChan((*iter));
			std::cout << "Channel [" + (*iter) + "] created." << std::endl;
			/* std::cout << "Channel Name: " << channels->getChanName() << std::endl; */
			channels->addUser(client);

			/* if (!isChanExist || channels->getOp().begin() != channels->getOp().end()) */
			// Give operator privileges
			msg = "MODE " + (*iter) + " +o "+ client->getNickname();
			msgSend(msg, client->getFD());
			channels->addOp(client);
			// KR : CHAN SET OP CLIENT
			//break;
		}

		// Set topic
		pos = cmd.find(" :");
		if (pos != std::string::npos && (std::string::npos + 1) != (*iter).size())
			channels->setTopic(cmd.substr(pos + 2, cmd.size()), client);

		// Send JOIN blue message to client
		msg = ":" + client->getNickname() + "@" + client->getHost() + " JOIN " + (*iter);
		msgSend(msg, client->getFD());

		// Send JOIN message to all clients in chan
		msg = ":" + client->getNickname() + "@" + client->getHost() + " JOIN " + (*iter);
		sendToUsersInChan(msg, client->getFD());

		// Show chan's topic
		if (!channels->getTopic().empty())
		{
			msg = "TOPIC " + (*iter) + " :" + channels->getTopic();
			msgSend(msg, client->getFD());
			sendToUsersInChan(msg, client->getFD());
		}
	}
}

void Server::PART(Client *client, Channel *channel)
{
	std::cout << GREEN << "COMMAND PART" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;

    if (channel == NULL)
	{
    	std::cout << "Channel does not exist" << std::endl;
    	errorMsg(ERR403_NOSUCHCHANNEL, client->getFD(), "", "", "", "");
        return;
    }
    // cleanup the clients from the channel
    cleanupClients(client, channel);
    	std::cout << "Users that are still on the channel: " << channel->getUser().size() << std::endl;
    std::string partMsg = ":" + client->getNickname() + "@" + client->getHost() + " PART " + channel->getChanName();
     msgSend(partMsg, client->getFD());
    // If there are still members in the channel, send the message to them
    if (channel->getUser().size() > 0)
        sendToUsersInChan(partMsg, client->getFD());
	else if (channel->getUser().size() == 0)
	{
        // If there are no members left, remove the channel
        chanErase(channel);
		std::cout << "Active channels after leaving: " << std::endl;
        for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); ++it)
            std::cout << (*it)->getChanName() << std::endl;
    }
}

void Server::cleanupClients(Client *client, Channel *channel)
{
    if (channel->Guest(client))
	{
        channel->eraseGuest(client);
    }

    if (channel->Op(client))
	{
        channel->eraseOp(client);
    }

    if (channel->User(client))
	{
        channel->eraseUser(client, client->getFD());
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
	else
	{
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
	std::cout << GREEN << "COMMAND QUIT" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;
	(void)channel;
	std::cout << RED << "Client " << client->getNickname() << " has quit." << RESET << std::endl;
	int clientSocket = client->getFD();
	close(clientSocket);

	for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
	{
		cleanupClients(client, *it);
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

void Server::KICK(Client* client, Channel* channel)
{
	std::cout << GREEN << "COMMAND KICK" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;

	if (!channel)
	{
		return ;
	}
    // Check if the user is an operator in the channel
    if (channel->Op(client) == true)
	{
    	std::string recipient;
		std::string reason;
		size_t doubleColonPos = cmd.find(" :");
		if (doubleColonPos != std::string::npos)
		{
			recipient = cmd.substr(cmd.find(channel->getChanName()) + channel->getChanName().size() + 1, doubleColonPos - cmd.find(channel->getChanName()) - channel->getChanName().size() - 1);
			reason = cmd.substr(doubleColonPos + 2);
		} 
		else 
		{
			errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), "KICK", "", "", "");
			return;
		}

		// Check if the recipient is in the channel
		bool recipientFound = false;
		for (std::vector<Client*>::iterator it = channel->getUser().begin(); it != channel->getUser().end(); ++it)
		{
			if ((*it)->getNickname() == recipient)
			{
				recipientFound = true;

				std::string kickMsg = ":" + client->getNickname() + " KICK " + channel->getChanName() + " " + recipient + " :" + reason;
				msgSend(kickMsg, client->getFD());

				/* int recipientFD = (*it)->getFD(); */
				it = channel->getUser().erase(it);
				
				std::string notifyMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHost() + " KICK " + channel->getChanName() + " " + recipient + " :" + reason;
				sendToUsersInChan(notifyMsg, client->getFD());
				/* close(recipientFD); */

				break;
			}
			else
				recipientFound = false;
		}

		if (!recipientFound)
		{
			errorMsg(ERR441_USERNOTINCHANNEL, client->getFD(), recipient, channel->getChanName(), "", "");
		}
	}
	else
	{
		errorMsg(ERR482_CHANOPRIVSNEEDED, client->getFD(), client->getNickname(), channel->getChanName(), "Not allowed", "");
	}
}

void Server::INVITE(Client* client, Channel* channel)
{
	std::cout << GREEN << "COMMAND INVITE" << RESET << std::endl;
	std::cout << GREEN << "---------------" << RESET << std::endl;

    std::string invited;
    std::string chanName;
	size_t 		space;
	size_t 		chan;

    chan = cmd.find("#");
    if (chan != std::string::npos)
	{
        invited = cmd.substr(0, chan - 1);
		space = cmd.find(" ", chan + 1);
        if (space != std::string::npos)
            chanName = cmd.substr(chan, space - (chan + 1));
		else 
            chanName = cmd.substr(chan);
        if (chanExist(chanName) == 0)
		{
            std::cout << "Channel does not exist" << std::endl;
            errorMsg(ERR403_NOSUCHCHANNEL, client->getFD(), chanName, "", "", "");
            return;
        }
        if (channel->User(client) == false)
            errorMsg(ERR442_NOTONCHANNEL, client->getFD(), chanName, "", "", "");
        else if (channel->User(client) == false)
            errorMsg(ERR482_CHANOPRIVSNEEDED, client->getFD(), chanName, "", "", "");
        else if (nickExist(invited) == false)
            errorMsg(ERR401_NOSUCHNICK, client->getFD(), invited, "", "", "");
        else if (channel->nickMember(invited) == true)
            errorMsg(ERR441_USERNOTINCHANNEL, client->getFD(), chanName, invited, "", "");
		else
		{
            for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); ++it)
			{
                if (invited == (*it)->getNickname())
				{
                    channel->addGuest(*it);
                    std::string reply = "341 " + client->getNickname() + " " + invited + " " + chanName + CRLF;
                    msgSend(reply, client->getFD());
                    std::string msg1 = ":" + client->getNickname() + " " + token + " " + invited + " " + chanName;
                    msgSend(msg1, client->getFD());
                    std::string msg2 = ":" + client->getNickname() + " " + token + " " + invited + " " + chanName;
                    msgSend(msg2, (*it)->getFD());
                    break;
                }
            }
        }
    }
}

void Server::TOPIC(Client* client, Channel* channel)
{
	std::cout << GREEN << "COMMAND TOPIC" << RESET << std::endl;
	std::cout << GREEN << "--------------" << RESET << std::endl;

    std::string topicName;
    std::string msg;
	size_t 		pos;

    if (channel == NULL)
	{
        errorMsg(ERR403_NOSUCHCHANNEL, client->getFD(), "", "", "", "");
        return;
    }
    // Check if the command includes "::" to handle /topic alone
    if (cmd.find("::") != std::string::npos)
	{
        msg = ":" + client->getNickname() + " TOPIC " + channel->getChanName();
        channel->setTopic("", client);
		//msg += " (unset by " + client->getNickname() + ")";
    } 
	else
	{
    	pos = cmd.find(":");
        if (pos != std::string::npos && (std::string::npos + 1) != cmd.size())
            topicName = cmd.substr(pos + 1);
        else
            topicName = cmd;
		channel->setTopic(topicName, client);
		msg = ": 332 " + client->getNickname() + " " + channel->getChanName() + " :" + channel->getTopic();
	}
	msgSend(msg, client->getFD());
    sendToUsersInChan(msg, client->getFD());
}

void	Server::PRIVMSG(Client *client, Channel *channel) 
{
	std::cout << GREEN << "COMMAND PRIVMSG" << RESET << std::endl;
	std::cout << GREEN << "----------------" << RESET << std::endl;

    bool 		messageSend = false;
    size_t 		msgStart = cmd.find(':');
    std::string recipient = cmd.substr(1, msgStart - 1);
    std::string msgContent = cmd.substr(msgStart + 1);

    if (msgContent.empty() || recipient.empty()) 
	{
        errorMsg(ERR412_NOTEXTTOSEND, client->getFD(), "", "", "", "");
        return;
    }
	
    recipient = '#' + recipient;
	size_t count = 0;
    size_t pos = 0;

	//	To avoid sending messages when not in channel. How many times the substr appears in the /msg string
	while ((pos = cmd.find(recipient, pos)) != std::string::npos) 
    {
        count++;
        pos += recipient.size();
    }

	if (count > 1)
	{
        std::string msg = ':' + client->getNickname() + '@' + client->getHost() + " " + token + " " + recipient + " :" + msgContent;
        sendToUsersInChan(msg, client->getFD());
        messageSend = true;
    }
    // If the channel name is not found but there's a '#' in the command
    if (!messageSend && cmd.find('#') != std::string::npos)
	{
        std::vector<std::string>	channelList;
		std::string					msgToAll;

        size_t pos = cmd.find("#");
        while (pos != std::string::npos)
		{
            std::string chanName = parseChannel(cmd, pos);
            channelList.push_back(chanName);
            pos = cmd.find('#', pos + 1);
        }

		if (!channelList.empty())
		{
            std::string lastChanName = channelList.back();
            std::size_t lastChanPos = cmd.rfind(lastChanName);
            if (lastChanPos != std::string::npos) 
			{
                msgToAll = cmd.substr(lastChanPos + lastChanName.size() + 2);
            }
        }
         for (size_t i = 0; i < channelList.size(); i++)
		 {
            bool isChanExist = false;
            for (size_t j = 0; j < _chan.size(); ++j)
			{
                if (_chan[j]->getChanName() == channelList[i])
				{
                    if (_chan[j]->User(client))
					{
                        isChanExist = true;
                        std::string msg = ':' + client->getNickname() + '@' + client->getHost() + " " + token + " " + channelList[i] + " :" + msgToAll;
                        sendToUsersInChan(msg, client->getFD());
                        return;
                    } 
					else 
					{
                        errorMsg(ERR404_CANNOTSENDTOCHAN, client->getFD(), channel->getChanName(), "", "", "");
                        return;
                    }
                }
            }
            if (!isChanExist) 
			{
                errorMsg(ERR403_NOSUCHCHANNEL, client->getFD(), channelList[i], "", "", "");
                return;
            }
        }
    } 
	else if (cmd.find('#') == std::string::npos) 
	{ //hanle priv messages as well as the queries for individual users
        std::size_t startPos = cmd.find(":");
        if (startPos != std::string::npos) 
		{
            std::string priv = cmd.substr(startPos + 1);
            std::size_t nick = cmd.find(" ");
            if (nick != std::string::npos) 
			{
                std::string nickname = cmd.substr(0, nick);
				if (nickname == client->getNickname()) 
            	{
                	// Prevent sending the private message to yourself
					std::cout << "Cannot send privmsg to yourself" << std::endl;
                	//errorMsg(ERR401_NOSUCHNICK, client->getFD(), client->getNickname(), "", "", "");
                	return;
            	}
				bool recipientExists = false;
                for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); it++) 
				{
                    if (nickname == (*it)->getNickname()) 
					{
						std::string privmsg = ":" + client->getNickname() + " " + token + " " + (*it)->getNickname() + " :" + priv;
						msgSend(privmsg, (*it)->getFD());
						recipientExists = true;
                        break;
                    }
                }
				if (!recipientExists)
				{
                	errorMsg(ERR401_NOSUCHNICK, client->getFD(), client->getNickname(), "", "", "");
                	return;
				}
            }
        }
    }
}

void Server::MODE(Client *client, Channel *channel) 
{
    std::cout << GREEN << "COMMAND MODE" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;

    std::string msg;
    std::string chanName;
    std::vector<std::string> args;
    size_t pos = 0;
    size_t endPos;
    bool isModeAdded = false;

    if (cmd == (client->getNickname() + " +i"))
        return;

	chanName = parseChannel(cmd, 0);
	bool isChan = chanExist(chanName);

	if (!isChan)
	{
        errorMsg(ERR403_NOSUCHCHANNEL, client->getFD(), client->getNickname(), chanName, "", "");
        return;
    }

    if (!channel->Op(client))
	{
        errorMsg(ERR482_CHANOPRIVSNEEDED, client->getFD(), client->getNickname(), channel->getChanName(), "Not allowed", "");
        return;
    }

    pos = chanName.size() + 1;
	if (cmd.empty())
	{
		errorMsg(ERR472_UNKNOWNMODE, client->getFD(), "", "", "", "");
		return ;
	}

	while ((pos = cmd.find(" ", pos)) != std::string::npos && pos < cmd.size())
	{
        endPos = cmd.find(" ", (pos + 1));
        if (endPos == std::string::npos)
            endPos = cmd.size();
        std::string arg = cmd.substr((pos + 1), ((endPos - pos) - 1));
        args.push_back(arg);
        pos = endPos;
    }

	if (args.size() > 3)
	{
		errorMsg(ERR472_UNKNOWNMODE, client->getFD(), client->getNickname(), "", ":Too many arguments", "");
		return;
	}

		pos = 0;
		if (cmd.find("+") != std::string::npos)
			isModeAdded = true;
		else
			isModeAdded = false;

		if (cmd.find("t") != std::string::npos)
		{
			channel->setTopicMode(isModeAdded);
			if (isModeAdded)
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :Only channel operators may set the channel topic";
			else
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :Channel topic can be set by everyone";	
		}
		else if (cmd.find("o") != std::string::npos)
		{
			if (args.size() == 0)
			{
				errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), channel->getChanName(), "Invalid channel limit", "", "");
				return ;
			}

			std::string username = args.back();
			bool userExists = false;
			for (std::vector<Client *>::iterator it = _cli.begin(); it != _cli.end(); ++it)
			{
				if ((*it)->getNickname() == username)
				{
					userExists = true;
					break;
				}
			}

			if (!userExists)
			{
				errorMsg(ERR401_NOSUCHNICK, client->getFD(), client->getNickname(), username, "", "");
				return;
			}

			bool opAdded = channel->setOp(isModeAdded, username);

			if (opAdded)
			{

				if (isModeAdded)
				{
					msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " +o " + username + " :" + "has been set as an operator";
				}
				else
				{
					msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " -o " + username + " :" + "has been removed from operators";
				}
			}
			else
			{
				errorMsg(ERR482_CHANOPRIVSNEEDED, client->getFD(), client->getNickname(), channel->getChanName(), "Not allowed", "");
				return;
			}
		}

		else if (cmd.find("l") != std::string::npos)
		{
			if (args.size() == 0 && (isModeAdded))
			{
				errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), "", "", "", "");
				return ;
			}

			if (isModeAdded)
			{
				int limit = std::atoi(args.front().c_str());
				if (limit <= 0)
				{
					errorMsg(ERR472_UNKNOWNMODE, client->getFD(), "", "", "", "");
					return ;
				}
				channel->setLimit(true, limit);
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :Channel limit has been set to " + args.front();			
			}
			else
			{
				channel->setLimit(false, 0);
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :Channel limit has been removed";
			}
		}
		else if (cmd.find("k") != std::string::npos)
		{
			if (args.size() == 0 && (isModeAdded))
			{
				errorMsg(ERR461_NEEDMOREPARAMS, client->getFD(), "", "", "", "");
				return ;
			}
			
			if (isModeAdded)
			{
				std::string	passw;
				if (args.size() == 1)
					passw = args.front();
				else if (args.size() == 2)
				{
					if (channel->isNumber(args.front()))
						passw = args[1];
					else
						passw = args.front();
				}
				else
					passw = args[1];

				channel->setChanPass(passw);
			}

			channel->setPassMode(isModeAdded);
			if (isModeAdded)
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :a password key has been set";			
			else
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :the password key has been removed";
		}
		else if (cmd.find("i") != std::string::npos)
		{
			channel->setInviteMode(isModeAdded);

			if (isModeAdded)
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :invite only activated";			
			else
				msg = ":" + client->getNickname() + " MODE " + channel->getChanName() + " :invite only deactivated";
		}
		else
		{
			errorMsg(ERR472_UNKNOWNMODE, client->getFD(), "", "", "", "");
			return;
		}

		if (!msg.empty())
		{
			msgSend(msg, client->getFD());
			sendToUsersInChan(msg, client->getFD());
		}
}
