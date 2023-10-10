#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Message.hpp"
#include "../inc/Channel.hpp"

void	Server::callCmd(std::string cmd, Client *client, Channel *channel)
{
	std::string valid_commands[12] = {"CAP", "PING", "NICK", "USER", "JOIN", "PART", \
		"PASS", "QUIT", "KICK", "INVITE", "TOPIC", "PRIVMSG" /*, "NOTICE"*/ };

	void	(Server::*funcPtr[])(Client *client, Channel *channel) =
	{
		&Server::CAP,
		&Server::PING,
		&Server::NICK,
		&Server::USER,
		&Server::JOIN,
		&Server::PART,
		&Server::PASS,
		&Server::QUIT,
		&Server::KICK,
		&Server::INVITE,
		&Server::TOPIC,
		&Server::PRIVMSG
		//&Server::NOTICE
		//&Server::LIST // channel debug 
	};
	for (int i = 0; i < 12; i++)
	{
		if (cmd.compare(valid_commands[i]) == 0)
		{
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

    if (!client->isRegister())
	{
        return;
    }
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
    if (newNick.size() > 25 || isNickValid(newNick))
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

	if (client->isRegister() == false)
	{
		return;
	}
	colonPos = cmd.find(':');
	if (colonPos != std::string::npos)
	{
		std::string user = cmd.substr(colonPos + 1);
		std::string	msg = "USER : " + user + CRLF;
		client->setUser(user);
        welcomeMsg(client);
		//printIRCBanner();
	}
}

void Server::JOIN(Client *client, Channel *channel)
{
	std::cout << GREEN << "COMMAND JOIN" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;
    (void)channel;

    std::vector<std::string> channelsToJoin;
    std::string pass;
	std::string	msg;
	std::string	name;
	size_t		startPos = 0;
	size_t		pos = 0;
	bool		isChanExist = false;

	if (cmd == ":")
		return ;
    //parseJoinCommand(cmd, channelsToJoin, pass);

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

	if ((pos = cmd.find(" ")) != std::string::npos)
	{
		if (cmd.find(":") == std::string::npos)
			pass = cmd.substr(pos + 1);
	}
	for (std::vector<std::string>::iterator iter = channelsToJoin.begin(); iter != channelsToJoin.end(); iter++)
	{
		for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); it++)
		{
			if (((*it)->getChanName() == (*iter)))
			{
				isChanExist = true;

				if (channel->getLimitMode())
				{
					if (TOINT(channel->getUser().size()) >= channel->getLimit())
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
				}

				std::cout << "Channel [" + (*iter) + "] already exist." << std::endl;
				channels = (*it);
				channels->addUser(client);
				break;
			}
		}
		if (!isChanExist)
		{
			channels = addChan((*iter));
			std::cout << "Channel [" + (*iter) + "] created." << std::endl;
			channels->addUser(client);
			channels->addOp(client);
		}
		pos = cmd.find(" :");

		if (pos != std::string::npos && (std::string::npos + 1) != (*iter).size())
			channels->setTopic(cmd.substr(pos + 2, cmd.size()), client);
		msg = ":" + client->getNickname() + "@" + client->getHost() + " JOIN " + (*iter);
		msgSend(msg, client->getFD());

		if (!isChanExist)
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
	{
        sendToUsersInChan(partMsg, client->getFD());
    }
	else if (channel->getUser().size() == 0)
	{
        // If there are no members left, remove the channel
        chanErase(channel);
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
    // Check if the user is an operator in the channel
    if (channel->Op(client) == true)
	{
        //errorMsg(ERR482_CHANOPRIVSNEEDED, client->getFD(), client->getNickname(), channel->getChanName(), "Not allowed", "");
		//std::string msg = channel->getChanName() + " You must be a channel operator";
        //return;
    //}

    // Parse the KICK command
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

				int recipientFD = (*it)->getFD();

				it = channel->getUser().erase(it);
				
				std::string notifyMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getHost() + " KICK " + channel->getChanName() + " " + recipient + " :" + reason;
				sendToUsersInChan(notifyMsg, client->getFD());
				close(recipientFD);

				break;
			}
		}

		if (!recipientFound)
		{
			errorMsg(ERR441_USERNOTINCHANNEL, client->getFD(), recipient, channel->getChanName(), "", "");
		}
	}
	else
	{
		errorMsg(ERR482_CHANOPRIVSNEEDED, client->getFD(), client->getNickname(), channel->getChanName(), "Not allowed", "");
        //std::string errorMsg = channel->getChanName() + " :You must be a channel operator!";
        //sendToUsersInChan(errorMsg, client->getFD());
		//return ;
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
		{
            chanName = cmd.substr(chan, space - (chan + 1));
        } 
		else 
		{
            chanName = cmd.substr(chan);
        }
        if (chanExist(chanName) == 0)
		{
            std::cout << "Channel does not exist" << std::endl;
            errorMsg(ERR403_NOSUCHCHANNEL, client->getFD(), chanName, "", "", "");
            return;
        }
        if (channel->User(client) == false)
		{
            errorMsg(ERR442_NOTONCHANNEL, client->getFD(), chanName, "", "", "");
        }
        else if (channel->User(client) == false)
		{
            errorMsg(ERR482_CHANOPRIVSNEEDED, client->getFD(), chanName, "", "", "");
        }
        else if (nickExist(invited) == false)
		{
            errorMsg(ERR401_NOSUCHNICK, client->getFD(), invited, "", "", "");
        }
        else if (channel->nickMember(invited) == true)
		{
            errorMsg(ERR441_USERNOTINCHANNEL, client->getFD(), chanName, invited, "", "");
        } 
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
        if (pos != std::string::npos && pos + 1 != cmd.size())
            topicName = cmd.substr(pos + 1);
        else
            topicName = cmd;
		std::string oldTopic = channel->getTopic();
		channel->setTopic(topicName, client);
		if (topicName.empty())
		{
            msg = ":" + client->getNickname() + " TOPIC " + channel->getChanName() + " :" + topicName;
        }
        else
        {
            msg = ":" + client->getNickname() + " TOPIC " + channel->getChanName() + " :" + topicName + " (was: " + oldTopic + ")";
        }
    }
	msgSend(msg, client->getFD());
    sendToUsersInChan(msg, client->getFD());
    std::string reply = ": 332 " + client->getNickname() + " " + channel->getChanName() + " :" + channel->getTopic();
    msgSend(reply, client->getFD());
}

void	Server::PRIVMSG(Client* client, Channel* channel) 
{
	std::cout << GREEN << "COMMAND PRIVMSG" << RESET << std::endl;
	std::cout << GREEN << "-------------" << RESET << std::endl;

    bool 		messageSend = false;
    size_t 		msgStart = cmd.find(':');
    std::string recipient = cmd.substr(1, msgStart - 1);
    std::string msgContent = cmd.substr(msgStart + 1);

    recipient = '#' + recipient;
    if (cmd.find(recipient) != std::string::npos)
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
                for (std::vector<Client*>::iterator it = _cli.begin(); it != _cli.end(); it++) 
				{
                    if (nickname == (*it)->getNickname()) 
					{
						//sending the prriv msg to the user
                        std::string privmsg = ":" + client->getNickname() + " " + token + " " + (*it)->getNickname() + " :" + priv;
                        msgSend(privmsg, (*it)->getFD());
                        return;
                    }
                }
                errorMsg(ERR401_NOSUCHNICK, client->getFD(), client->getNickname(), "", "", "");
                return;
            }
        }
    }
    //errorMsg(ERR404_CANNOTSENDTOCHAN, client->getFD(), channel->getChanName(), "", "", "");
}

// TODO
/*
//Should be as PRIVMSG when done

void	Server::NOTICE(Client *client, Channel *channel)
{
	std::cout << "COMMAND NOTICE" << std::endl;
	std::cout << "---------------" << std::endl;

*/
/*
// parse the KICK command and extract channel, nick, and reason.
bool Server::parseKickCommand(const std::string &kick, std::string &chan, std::string &nick, std::string &reason)
{
    size_t doubleColonPos = kick.find(':');
    if (doubleColonPos == std::string::npos)
	{
        return false; // invalid command format, missing ':'.
    }
    size_t space = kick.find(' '); // space position
    if (space == std::string::npos)
	{
        return false; // invalid command format, missing space.
    }
    chan = kick.substr(space + 1, doubleColonPos - space - 1);

    size_t nickStart = space + chan.size() + 2; // Skip ' ' and ':'
    size_t reasonStart = doubleColonPos + 1;
    if (reasonStart >= kick.size())
	{
        return false; // invalid command format, missing reason.
    }
    size_t reasonLength = kick.size() - reasonStart;
    nick = kick.substr(nickStart, doubleColonPos - nickStart);
    reason = kick.substr(reasonStart, reasonLength);

    return true;
}

//////////////////////
//  CHANNEL DEBUG  //
////////////////////
void Server::LIST(Client *client, Channel *channel)
{
	(void)channel;
    std::string channelList;
    for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); ++it)
    {
        channelList += (*it)->getChanName() + " ";
    }
    std::string response = "Channel List: " + channelList;
    msgSend(response, client->getFD());
}
*/

/*
std::vector<Client *>::iterator	Server::findClientChannel(const std::string &nick, Channel &channel)
{
	std::vector<Client *>	usrList = channel.getUser();
	std::vector<Client *>::iterator it = usrList.begin();
	while (it != usrList.end())
	{
		if (!nick.empty() && nick == ((*it))->getNickname())
			return (it);
		it++;
	}
	return (it);
}
*/
/*
// NOTES :
// 1. Change "!!!" which corresponds to var names
// 2. Do connection registration
//

void	Server::callCmd(Client &client, const std::string& cmd, std::vector<std::string> &params)
{
	std::string valid_commands[5] =
	{
		"NICK",
		"USER",
		"JOIN",
		"PART",
		"PRIVMSG", // receive PM
		// operators
		//"KICK", 
		// "MODE", 
		// "OPER",
		// "INVITE",
		// "TOPIC", 
		// //  
		// "PING", 
		// "QUIT", 
		// "KILL" 
	};

	int	idx = 0;
	while (idx < 5)
	{
		if (cmd == valid_commands[idx])
			break;
		idx++;
	}
	switch (idx + 1)
	{
		case 1: cmdNick(client, params[0]); break;
		case 2: cmdUser(client, params[0]); break;
		case 3: cmdJoin(client, params); break;
		case 4: cmdPart(client, params); break;
		case 5: cmdMsg(client, params); break;
		case 6: cmdKick(client, params); break;
		// case 11: cmdInvite(void); break; 
		//case 12: cmdTopic(*this, client, params); break; 
		// case 6: cmdMode(*this, client, params); break; 
		// case 9: cmdOper(*this, client, params); break; 
		// case 13: cmdPing(client); break; 
		// case 14: cmdQuit(client); break; 
    	// case 15: cmdKill(*this, client, params); break;
	}
}

bool	Server::findNickname(const std::string &nick)
{
	std::vector<Client *>::iterator it;
	for (it = _cli.begin(); it != _cli.end(); it++)
	{
		if (!nick.empty() && nick == ((*it))->getNickname())
			return (false);
	}
	return (true);
}

bool	Server::findUserClient(const std::string &user)
{
	std::vector<Client *>::iterator it;
	for (it = _cli.begin(); it != _cli.end(); it++)
	{
		if (!user.empty() && user == ((*it))->getUser())
			return (false);
	}
	return (true);
}

std::vector<Channel *>::iterator	Server::findChannel(const std::string &chan)
{
	std::vector<Channel *> channels = getChan();
	std::vector<Channel *>::iterator it;
	for (it = channels.begin(); it != channels.end(); it++)
	{
		if (!chan.empty() && chan == ((*it))->getChanName())
			return (it);
	}
	return (_chan.end());
}

std::vector<Client *>::iterator	Server::findClientChannel(const std::string &nick, Channel &channel)
{
	std::vector<Client *>	usrList = channel.getUsr();
	std::vector<Client *>::iterator it = usrList.begin();
	while (it != usrList.end())
	{
		if (!nick.empty() && nick == ((*it))->getNickname())
			return (it);
		it++;
	}
	return (it);
}

bool	Server::cmdNick(Client &client, const std::string &nick)
{
// input :
// callCmd(client, bufcmd, newnick); // with bufcmd being the command in buffer, newnick being the new nickname chosen by client
	std::cout << "Server: Execute cmdNick" << std::endl;
	int			fdc = client.getFD();
	std::string	msg;
	// err:431, no nickname
	if (nick.length() <= 0)
	{
		msg = ERR431_NONICKNAMEGIVEN;
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// err:432, nick not valid
	else if (nick.length() <= 8)
	{
		msg = ERR432_ERRONEUSNICKNAME(nick);
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// err:433, nick already exists
	else if (!findNickname(nick))
	{
		msg = ERR433_NICKNAMEINUSE(nick);
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// TRUE, change nick
	// CHANGE IT IF YOU MUST DO IT AT BEGINING OF CONNECTION
	msg = TOSTR(MSG(client.getNickname(), client.getUser(), "NICK", nick));
	if (client.getNickname()[0] == '@') // for chop
		client.setNickname("@" + nick);
	else
		client.setNickname(nick);
	send(fdc, msg.c_str(), msg.length(), 0);
	return (true);
}

bool	Server::cmdUser(Client &client, std::string user)
{
// input :
// callCmd(client, bufcmd, newuser); // with bufcmd being the command in buffer, newuser being the new user chosen by client
	std::cout << "Server: Execute cmdUser" << std::endl;
	int			fdc = client.getFD();
	std::string	msg;
	// err:461, need more params
	if (user.length() <= 0)
	{
		msg = ERR461_NEEDMOREPARAMS("USER");
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// err:462, already registered
	else if (!this->findUserClient(user))
	{
		msg = ERR462_ALREADYREGISTERED;
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// TRUE, set user
	client.setUser(user);
	msg = MSG(client.getNickname(), client.getUser(), "USER", user);
	send(fdc, msg.c_str(), msg.length(), 0);
	return (true);
}

void	Server::cmdJoinNames(Client &client, Channel &channel)
{
	int			fdc = client.getFD();
	std::vector<Client*> users = channel.getUsr();
	std::string	msg;
	std::ostringstream msgO;
	for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); ++it)
	{
		Client* user = *it;
		std::vector<Client*> ops = channel.getOp();
		for (std::vector<Client*>::iterator itOp = ops.begin(); itOp != ops.end(); ++itOp)
		{
			if (user->getNickname() == (*itOp)->getNickname())
				msgO << "@" << user->getNickname();
			else
				msgO << user->getNickname();
		}
		if (std::distance(it, users.end()) > 1) {
			msgO << " "; // Add a space after each nickname, except for the last one
		}
	}
	std::string nickList = msgO.str();
	msg = RPL353_NAMREPLY(client.getNickname(), client.getUser(), nickList);
	send(fdc, msg.c_str(), msg.length(), 0);
	msg = RPL366_ENDOFNAMES(client.getNickname(), channel.getChanName());
	send(fdc, msg.c_str(), msg.length(), 0);
}

bool	Server::cmdJoin(Client &client, std::vector<std::string> &params)
{
// input :
// pwd is password from buffer
	(void)params; // delete when retreive params
	std::string chanName = ""; // change to appropriate params[i] or what should fit
	std::string pwd = ""; // change to appropriate params[i] or what should fit
	std::string	msg;
	std::cout << "Server: Execute cmdJoin" << std::endl;
	int									fdc = client.getFD();
	// err:461, need more params
	if (chanName.length() <= 0)
	{
		msg = ERR461_NEEDMOREPARAMS("JOIN");
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	else if (chanName[0] != '#')
	{
		msg = ERR403_NOSUCHCHANNEL(chanName);
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	std::vector<Channel *>::iterator	chanIt = findChannel(chanName);
	if (chanIt == _chan.end()) // if chan does not exist, create it
		addChan(chanName, &client); // UNDEFINED : client or *client ??
	else // if chan exists
	{
		Channel *channel = (*chanIt);
		// err:473, invit only
		if (!(*chanIt)->fdIsInvited(client.getFD()))
		{
			msg = ERR473_INVITEONLYCHAN(TOSTR(chanName));
			send(fdc, msg.c_str(), msg.length(), 0);
			return (false);
		}
		// err:474, banned
		if (!channel->fdIsBanned(client.getFD()))
		{
			msg = ERR474_BANNEDFROMCHAN(chanName);
			send(fdc, msg.c_str(), msg.length(), 0);
			return (false);
		}
		// err:475, bad pass
		if (channel->getPassword() != pwd)
		{
			msg = ERR475_BADCHANNELKEY(chanName);
			send(fdc, msg.c_str(), msg.length(), 0);
			return (false);
		}
		// TRUE, let client enter it and msg all
		if (!client.inChannel(chanName))
		{
			client.joinChannel(channel);
			msg = MSG(client.getNickname(), client.getUser(), "JOIN", chanName);
			send(fdc, msg.c_str(), msg.length(), 0);
			msg = MSG(client.getNickname(), client.getUser(), "JOIN", chanName);
			sendToUsersInChan(*channel, client, msg);
			// std::vector<Client *> users = channel->getUsr(); 
			// for (std::vector<Client *>::iterator u_it = users.begin(); u_it != users.end(); u_it++) 
			// { 
			// 	if ((*u_it)->getNickname() != client.getNickname()) 
			// 		send((*u_it).getFD(), TOSTR(MSG(client.getNickname(), client.getUser() "JOIN", chanName)); 
			// }
			if (!channel->getTopic().empty())
			{
				msg = RPL332_TOPIC(client.getNickname(), chanName, channel->getTopic());
				send(fdc, msg.c_str(), msg.length(), 0);
			}
			cmdJoinNames(client, *channel);
			return (true);
		}
	}
	return (false);
}

bool	Server::cmdMsg(Client &client, std::vector<std::string> &params)
{
//input:
//params : msgtarget, texttobesent
	std::cout << "Server: Execute cmdMsg" << std::endl;
	int			fdc = client.getFD();
	std::string	msg;
	// err:411, no recipient
	if (params.size() == 0)
	{
		msg = ERR411_NORECIPIENT("PRIVMSG");
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// err:412, no text to send
	if (params.size() == 1)
	{
		msg = ERR412_NOTEXTTOSEND;
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	std::vector<Channel *>::iterator itCh = findChannel(params[0]);
	if (itCh != _chan.end()) // if target is channel, send to all 
	{
		msg = MSG(client.getNickname(), client.getUser(), "PRIVMSG", params[1]);
		sendToUsersInChan(*(*itCh), client, msg);
		return (true);
	}
	else if (findNickname(client.getNickname())) // if target is client
	{
		msg = MSG(client.getNickname(), client.getUser(), "PRIVMSG", params[1]);
		send(fdc, msg.c_str(), msg.length(), 0);
		return (true);
	}
	msg = ERR401_NOSUCHNICK(client.getNickname());
	send(fdc, msg.c_str(), msg.length(), 0);
	return (false);
}

// KR : when to use CAP ?
// bool	Server::cmdCAP(void) 
// { 
// 	std::cout << "Server: Execute cmdCAP" << std::endl; 
// } 

bool	Server::cmdMode(Client &client, std::vector<std::string> &params)
{
//input:
//params : target modes
	(void)params;
	std::cout << "Server: Execute cmdMode" << std::endl;
	int			fdc = client.getFD();
	std::string	msg;
	// err:461, need more params
	if (params.size() < 2)
	{
		msg = ERR461_NEEDMOREPARAMS("MODE");
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
}
*/
