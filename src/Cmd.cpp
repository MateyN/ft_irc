#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

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
		/* "KICK", */
		/* "MODE", */
		/* "OPER", */
		/* "INVITE", */
		/* "TOPIC", */
		/* // */ 
		/* "PING", */
		/* "QUIT", */
		/* "KILL" */
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
		/* case 11: cmdInvite(void); break; */
		/* case 12: cmdTopic(*this, client, params); break; */
		/* case 6: cmdMode(*this, client, params); break; */
		/* case 9: cmdOper(*this, client, params); break; */
		/* case 13: cmdPing(client); break; */
		/* case 14: cmdQuit(client); break; */
    	/* case 15: cmdKill(*this, client, params); break; */
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

void	Server::sendToUsersInChan(Channel &channel, Client &client, std::string msg)
{
	std::vector<Client *> users = channel.getUsr();
	for (std::vector<Client *>::iterator u_it = users.begin(); u_it != users.end(); u_it++)
	{
		if ((*u_it)->getNickname() != client.getNickname())
			send((*u_it)->getFD(), msg.c_str(), msg.length(), 0);
	}
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
			/* std::vector<Client *> users = channel->getUsr(); */
			/* for (std::vector<Client *>::iterator u_it = users.begin(); u_it != users.end(); u_it++) */
			/* { */
			/* 	if ((*u_it)->getNickname() != client.getNickname()) */
			/* 		send((*u_it).getFD(), TOSTR(MSG(client.getNickname(), client.getUser() "JOIN", chanName)); */
			/* } */
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

bool	Server::cmdPart(Client &client, std::vector<std::string> &params)
{
	(void)params; // delete when use params
	std::string chanName = ""; //change to appropriate params[i]
	std::cout << "Server: Execute cmdPart" << std::endl;
	int			fdc = client.getFD();
	std::string	msg;
	// err:461, need more params
	if (chanName.length() <= 0)
	{
		msg = ERR461_NEEDMOREPARAMS("PART");
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// err:403, no such channel
	if (findChannel(chanName) == _chan.end())
	{
		msg = ERR403_NOSUCHCHANNEL(chanName);
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// err:442, not on channel
	if (findClientChannel(client.getNickname(), *channel) == channel->getUsr().end())
	{
		msg = ERR442_NOTONCHANNEL(chanName);
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	Channel *channel = *findChannel(chanName); // NOT SURE : UNDEFINED BEHAVIOR
	client.leaveChannel(*channel);
	if (channel->getUsr().size() == 0) // if no more user, delete chan
	{
		delete channel;
		this->_chan.erase(findChannel(chanName));
	}
	else // send part to all users
	{
		msg = MSG(client.getNickname(), client.getUser(), "PART", chanName);
		sendToUsersInChan(*channel, client, msg);
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
/* bool	Server::cmdCAP(void) */
/* { */
/* 	std::cout << "Server: Execute cmdCAP" << std::endl; */
/* } */

bool	Server::cmdKick(Client &client, std::vector<std::string> &params)
{
//input:
//params : channnel(begining by #) target
	(void)params;
	std::cout << "Server: Execute cmdKick" << std::endl;
	int			fdc = client.getFD();
	std::string	msg;
	// err:461, need more params
	if (params.size() < 2)
	{
		msg = ERR461_NEEDMOREPARAMS("KICK");
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	else if (params[0][0] != '#' || findChannel() == _chan.end())
	{
		msg = ERR403_NOSUCHCHANNEL(chanName);
		send(fdc, msg.c_str(), msg.length(), 0);
		return (false);
	}
	// ERR441_USERNOTINCHANNEL(nick, chanName) // for client when nick is not on channel
	// ERR442_NOTONCHNNEL(chanName) // when client is not user on channel
	Channel *channel = *findChannel(param[0]);
    channel->eraseUser(client, fdc);
	msg = MSG(client.getNickname(), client.getUser(), "KICK", "You have been kicked from channel :" + param[0]);
	send(fdc, msg.c_str(), msg.length(), 0);
	return (true);
}

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
