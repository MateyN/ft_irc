#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

// NOTES :
// 1. Change "!!!" which corresponds to var names
// 2. Do connection registration
//

void	Server::reply(int fdc, std:string num, std::string msg)
{
	std::string	reply = ":localhost " + num + " " + msg + "\r\n";
	send(fdc, reply.c_str(), reply.length(), 0);
}

void	Server::callCmd(Client &client, const std::string& cmd, std::vector<std::string> &params)
{
	std::string valid_commands[14] =
	{
		"NICK",
		"USER",
		"JOIN",
		"PART",
		"PRIVMSG", // receive PM
		"MOTD",
		// operators
		"MODE",
		"OPER",
		"KICK",
		"INVITE",
		"TOPIC",
		// 
		"PING",
		"QUIT",
		"KILL"
	};

	for (idx = 0; idx < 14; idx++)
	{
		if (cmd == valid_commands[idx])
			break;
	}
	switch (idx + 1)
	{
		case 1: cmdNick(client, params[0]); break;
		case 2: cmdUser(client, params[0]); break;
		case 3: cmdJoin(client, chanName, serv, pwd); break;
		case 4: cmdPart(client, chanName); break;
		case 5: cmdMsg(client, params); break;
		case 6: cmdMotd(client.get_fd(), client.get_nick()); break;
		case 8: cmdMode(*this, client, params); break;
		case 9: cmdOper(*this, client, params); break;
		case 10: cmdKick(void); break;
		case 11: cmdInvite(void); break;
		case 12: cmdTopic(*this, client, params); break;
		case 13: cmdPing(client); break;
		case 14: cmdQuit(client); break;
    	case 15: cmdKill(*this, client, params); break;
	}
}

bool	Server::findNickname(std::string &nick)
{
	std::vector<Client *>::iterator it;
	for (it = _cli.begin(); it != _cli.end(); it++)
	{
		if (!nick.empty() && nick == ((*it))->get_nick())
			return (false);
	}
	return (true);
}

bool	Server::findUserClient(std::string &user)
{
	std::vector<Client *>::iterator it;
	for (it = _cli.begin(); it != _cli.end(); it++)
	{
		if (!user.empty() && user == ((*it))->get_user())
			return (false);
	}
	return (true);
}

std::vector<Channel *>::iterator	Server::findChannel(std::string &chan)
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

std::vector<Client *>::iterator	Server::findClientChannel(std::string &nick, Channel &channel)
{
	std::vector<Client *>	usrList = channel.getUsr();
	for (std::vector<Client *>::iterator it = usrList.begin(); it != usrList.end(); it++)
	{
		if (!nick.empty() && nick == ((*it))->getNickname())
			return (it);
	}
	return (usrList.end());
}

bool	Server::cmdNick(Client &client, std::string nick)
{
// input :
// callCmd(client, bufcmd, newnick); // with bufcmd being the command in buffer, newnick being the new nickname chosen by client
	std::cout << "Server: Execute cmdNick" << std::endl;
	int	fdc = client.getFD();
	// err:431, no nickname
	if (nick.length() <= 0)
	{
		send(fdc, 431ERR_NONICKNAMEGIVEN.c_str(), 431ERR_NONICKNAMEGIVEN.length(), 0);
		return (false);
	}
	// err:432, nick not valid
	else if (nick.length() <= 8)
	{
		send(fdc, 432ERR_ERRONEUSNICKNAME(nick).c_str(), 432ERR_ERRONEUSNICKNAME(nick).length(), 0);
		return (false);
	}
	// err:433, nick already exists
	else if (!findNickname(nick))
	{
		send(fdc, 433ERR_NICKNAMEINUSE(nick).c_str(), 433ERR_NICKNAMEINUSE(nick).length(), 0);
		return (false);
	}
	// TRUE, change nick
	// CHANGE IT IF YOU MUST DO IT AT BEGINING OF CONNECTION
	std::string	nickMsg = MSG(client.getNickname(), client.getUser(), "NICK", nick);
	if (client.getNickname()[0] == "@") // for chop
		client.setNickname("@" + nick);
	else
		client.setNickname(nick);
	send(fdc, nickMsg.c_str(), nickMsg.length(), 0);
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
	int	fdc = client.getFD();
	// err:461, need more params
	if (user.length() <= 0)
	{
		send(fdc, 461ERR_NEEDMOREPARAMS("USER").c_str(), 461ERR_NEEDMOREPARAMS("USER").length(), 0);
		return (false);
	}
	// err:462, already registered
	else if (!this->findUserClient())
	{
		send(fdc, 462ERR_ALREADYREGISTERED.c_str(), 462ERR_ALREADYREGISTERED.length(), 0);
		return (false);
	}
	// TRUE, set user
	client.setUser(user);
	std::string	userMsg = ":" + NICKUSERHOST(client.getNickname(), client.getUser()) + " USER :" + user + "\r\n";
	send(fdc, userMsg.c_str(), userMsg.length(), 0);
// output
// 1. try with no args, should 461
// 2. try with already used user, should 462
// 3. normal behavior set user
}

void	Server::sendToUsersInChan(Channel &channel, Client &client, std::string msg)
{
	std::vector<Client *> users = channel->getUsr();
	for (std::vector<Client *>::iterator u_it = users.begin(); u_it != users.end(); u_it++)
	{
		if ((*u_it)->getNickname() != client.getNickname())
			send((*u_it).getFD(), msg);
	}
}

void	Server::cmdJoinNames(Client &client, Channel &channel)
{
	int	fdc = client.getFD();
	std::vector<Client*> users = channel.getUsr();
	std::ostringstream msg;
	for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); ++it)
	{
		Client* user = *it;
		std::vector<Client*> ops = channel.getOp();
		for (std::vector<Client*>::iterator itOp = ops.begin(); itOp != ops.end(); ++itOp)
		{
			if (user->getNickname() == (*itOp)->getNickname())
				msg << "@" << user->getNickname();
			else
				msg << user->getNickname();
		}
		if (std::distance(it, users.end()) > 1) {
			msg << " "; // Add a space after each nickname, except for the last one
		}
	}
	std::string nickList = msg.str();
	send(fdc, 353RPL_NAMREPLY(client.getNickname(), client.getUser(), nickList).c_str(), 353RPL_NAMREPLY(client.getNickname(), client.getUser(), nickList).length(), 0);
	send(fdc, 366RPL_ENDOFNAMES(client.getNickname(), client.getUser()).c_str(), 366RPL_ENDOFNAMES(client.getNickname(), client.getUser()).length(), 0);
}

bool	Server::cmdJoin(Client &client, std::string chanName, Server &serv, std::string pwd)
{
// input :
// pwd is password from buffer
	std::cout << "Server: Execute cmdJoin" << std::endl;
	int									fdc = client.getFD();
	// err:461, need more params
	if (chanName.length() <= 0)
	{
		send(fdc, 461ERR_NEEDMOREPARAMS("JOIN").c_str(), 461ERR_NEEDMOREPARAMS("JOIN").length(), 0);
		return (false);
	}
	else if (chanName[0] != "#")
	{
		send(fdc, 403ERR_NOSUCHCHANNEL(chanName).c_str(), 403ERR_NOSUCHCHANNEL(chanName).length(), 0);
		return (false);
	}
	std::vector<Channel *>::iterator	chanIt = findChannel(std::string &chan);
	if (chanIt == _chan.end) // if chan does not exist, create it
		addChan(chanName, client.getNickname);
	else // if chan exists
	{
		Channel *channel = (*chanIt);
		// err:473, invit only
		if (!(*chanIt)->fdIsInvited(client.getFD()))
		{
			send(fdc, 473ERR_INVITEONLYCHAN(chanName).c_str(), 473ERR_INVITEONLYCHAN(chanName).length(), 0);
			return (false);
		}
		// err:474, banned
		if (!channel->fdIsBanned(client.getFD()))
		{
			send(fdc, 474ERR_BANNEDFROMCHAN(chanName).c_str(), 464ERR_BANNEDFROMCHAN(chanName).length(), 0);
			return (false);
		}
		// err:475, bad pass
		if (channel->_password != pwd)
		{
			send(fdc, 474ERR_BANNEDFROMCHAN(chanName).c_str(), 464ERR_BANNEDFROMCHAN(chanName).length(), 0);
			return (false);
		}
		// TRUE, let client enter it and msg all
		if (!client.inChannel(chanName))
		{
			client.joinChannel(channel);
			send(fdc, MSG(client.getNickname(), client.getUser, "JOIN", chanName));
			sendToUsersInChan(channel, client, MSG(client.getNickname(), client.getUser, "JOIN", chanName), 
			/* std::vector<Client *> users = channel->getUsr(); */
			/* for (std::vector<Client *>::iterator u_it = users.begin(); u_it != users.end(); u_it++) */
			/* { */
			/* 	if ((*u_it)->getNickname() != client.getNickname()) */
			/* 		send((*u_it).getFD(), MSG(client.getNickname(), client.getUser, "JOIN", chanName)); */
			/* } */
			if (!channel->get_topic().empty())
				send(fdc, 332RPL_TOPIC(client.getNickname(), chanName, channel.getTopic());
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

bool	Server::cmdPart(Client &client, std::string chanName)
{
	std::cout << "Server: Execute cmdPart" << std::endl;
	int	fdc = client.getFD();
	// err:461, need more params
	if (chanName.length() <= 0)
	{
		send(fdc, 461ERR_NEEDMOREPARAMS("PART").c_str(), 461ERR_NEEDMOREPARAMS("PART").length(), 0);
		return (false);
	}
	// err:403, no such channel
	if (findChannel(chanName) == _chan.end())
	{
		send(fdc, 403ERR_NOSUCHCHANNEL.c_str(), 403ERR_NOSUCHCHANNEL.length(), 0);
		return (false);
	}
	// err:442, not on channel
	if (findClientChannel(chanName, client) == channel.getUsr().end())
	{
		send(fdc, 442ERR_NOTONCHANNEL(chanName).c_str(), 442ERR_NOTONCHANNEL(chanName).length(), 0);
		return (false);
	}
	Channel *channel = *findChannel(chanName); // NOT SURE : UNDEFINED BEHAVIOR
	client.leaveChannel(*channel);
	if (channel->getUsr().size() == 0) // if no more user, delete chan
	{
		delete channel;
		this->_chan.erase(findChannel(chanName);
	}
	else // send part to all users
	{
		sendToUsersInChan(channel, client, MSG(client.getNickname(), client.getUser, "PART", chanName));
		return (true);
	}
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
	int	fdc = client.getFD();
	// err:411, no recipient
	if (params.size() == 0)
	{
		send(fdc, 411ERR_NORECIPIENT("PRIVMSG").c_str(), 411ERR_NORECIPIENT("PRIVMSG").length(), 0);
		return (false);
	}
	// err:412, no text to send
	if (params.size() == 1)
	{
		send(fdc, 412ERR_NOTEXTTOSEND.c_str(), 412ERR_NOTEXTTOSEND.length(), 0);
		return (false);
	}
	std::vector<Channel *>::iterator itCh = findChannel(params[0]);
	if (itCh != _chan.end()) // if target is channel, send to all 
	{
		sendToUsersInChan((*itCh), client, MSG(client.getNickname(), client.getUser, "PRIVMSG", params[1]));
		return (true);
	}
	else if (findNickname(client.getNickname())) // if target is client
	{
		send(fdc, MSG(client.getNickname(), client.getUser, "PRIVMSG", params[1]));
		return (true);
	}
	send(fdc, 401ERR_NOSUCHNICK(client.getNickname).c_str(), 401ERR_NOSUCHNICK(client.getNickname).length(), 0);
	return (false);
//output:
//1. no param gives 411
//2. one param gives 412
//3. if target is channel, see that it is received by ALL chan clients and not others
//4. if target is user, see that received only by her
//5. if target is wrong, send 401
}

// KR : when to use CAP ?
bool	Server::cmdCAP(void)
{
	std::cout << "Server: Execute cmdCAP" << std::endl;
}
