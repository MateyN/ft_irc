#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Message.hpp"
#include "../inc/Channel.hpp"

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
		&Server::QUIT,
		//&Server::KICK
		//&Server::PART
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

void 	Server::NICK(Client *client, Channel *channel)
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

/*
bool Server::PART(Client &client, std::vector<std::string> &params)
{
	(void)params; // delete when use params
	std::string chanName = ""; //change to appropriate params[i]
	std::cout << "Server: Execute cmdPart" << std::endl;
	int			fdc = client.getFD();
	std::string	msg;
	// err:461, need more params
	if (chanName.length() <= 0)
	{
		errorMsg(ERR461_NEEDMOREPARAMS, clients->getFD(), clients->getNickname(), "", "", "");
		return (false);
	}
	// err:403, no such channel
	if (findChannel(chanName) == _chan.end())
	{
		errorMsg(ERR403_NOSUCHCHANNEL, clients->getFD(), clients->getNickname(), chanName, "", "");
		return (false);
	}
	// err:442, not on channel
	if (findClientChannel(client.getNickname(), *channels) == channels->getUser().end())
	{
		errorMsg(ERR442_NOTONCHANNEL, clients->getFD(), channels->getChanName(), "", "", "");
		return (false);
	}
	Channel *channel = *findChannel(chanName); // NOT SURE : UNDEFINED BEHAVIOR
	client.leaveChannel(*channel);
	if (channel->getUser().size() == 0) // if no more user, delete chan
	{
		delete channel;
		this->_chan.erase(findChannel(chanName));
	}
	else // send part to all users
	{
		sendToUsersInChan(msg, clients->getFD());
		return (true);
	}
	return (false);
// output when PART:
// 1. no params leads to 461 PART
// 2. no such channel leads to 403
// 3. user not on channel leads to 442
// 4. when last usr, check if channel is deleted
// 5. see if all users receive the msg PART
}
*/
std::vector<Channel*>::iterator Server::findChannel(const std::string &chan)
{
    for (std::vector<Channel*>::iterator it = _chan.begin(); it != _chan.end(); ++it)
    {
        if (!chan.empty() && chan == (*it)->getChanName())
        {
            return it;
        }
    }
    return _chan.end();
}


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
// output :
// 1. try with no arg : should 431 on client
// 2. try with more than 8 char : should 432 on client
// 3. try with an existing client nickname : should 433 on client
// 4. normal behavior changes nickname only
// 5. try on chop, should keep "@"
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
// output
// 1. try with no args, should 461
// 2. try with already used user, should 462
// 3. normal behavior set user
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
// output:
// 1. test no params
// 2. test channame begining by other than #
// 3. chan do not exist
// 4. chan exist
// 5. if fd not invited
// 6. if fd banned
// 7. if fd bad pass to chan
// 8. normal behavior with all names
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
//output:
//1. no param gives 411
//2. one param gives 412
//3. if target is channel, see that it is received by ALL chan clients and not others
//4. if target is user, see that received only by her
//5. if target is wrong, send 401
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
