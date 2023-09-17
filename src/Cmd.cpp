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
	std::string valid_commands[15] =
	{
		"NICK",
		"USER",
		"JOIN",
		"PART",
		"PRIVMSG", // receive PM
		"NOTICE",
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

	for (idx = 0; idx < 15; idx++)
	{
		if (cmd == valid_commands[idx])
			break;
	}
	switch (idx + 1)
	{
		case 1: cmdNick(client, params[0]); break;
		case 2: cmdUser(client, params[0]); break;
		case 3: cmdJoin(*this, client, params); break;
		case 4: cmdJPart(client, params[0]); break;
		case 5: cmdMsg(client, "PRIVMSG", params); break;
		case 6: cmdMsg(client, "NOTICE", params); break;
		case 7: cmdMotd(client.get_fd(), client.get_nick()); break;
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

bool	Server::findUser(std::string &user)
{
	std::vector<Client *>::iterator it;
	for (it = _cli.begin(); it != _cli.end(); it++)
	{
		if (!nick.empty() && user == ((*it))->get_user())
			return (false);
	}
	return (true);
}

std::vector<Channel *>::iterator	Server::findChannel(std::string &chan)
{
	std::vector<Channel *>::iterator it;
	for (it = _chan.begin(); it != _chan.end(); it++)
	{
		if (!chan.empty() && chan == ((*it))->getChanName())
			return (it);
	}
	return (_chan.end());
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
	else if (!this->findNickname(nick))
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
	else if (!this->findUser())
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
			std::vector<Client *> users = channel->getUsr();
			for (std::vector<Client *>::iterator u_it = users.begin(); u_it != users.end(); u_it++)
			{
				if ((*u_it)->getNickname() != client.getNickname())
					send((*u_it).getFD(), MSG(client.getNickname(), client.getUser, "JOIN", chanName));
			}
			if (!channel->get_topic().empty())
				send(fdc, 332RPL_TOPIC(client.getNickname(), chanName, channel.getTopic());
			cmdJoinNames(client, *channel);
		}
	}
}


// KR : when to use CAP ?
void	Server::cmdCAP(void)
{
	std::cout << "Server: Execute cmdCAP" << std::endl;
}
