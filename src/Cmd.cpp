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

	int idx = 0;

	while (idx < 15) {
		if (cmd == valid_commands[idx])
			break;
		idx++;
	}
	switch (idx + 1) {
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
	std::string	nickMsg = ":" + NICKUSERHOST(client.getNickname(), client.getUser()) + " NICK :" + nick + "\r\n";
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

void	Server::cmdJoin(Client &client, std::string chanName, Server &serv)
// TODO: modes, like invit only (473), banned (474), bad pass (475)
{
	int	fdc = client.getFD();

	// err:461, need more params
	if (chanName.length() <= 0)
	{
		send(fdc, 461ERR_NEEDMOREPARAMS("JOIN").c_str(), 461ERR_NEEDMOREPARAMS("JOIN").length(), 0);
		return (false);
	}

	std::vector<Channel *>::iterator	chanIt = findChannel(std::string &chan);

	if (chanIt != _chan.end) // if chan exists
	{
		// err:473, invit only
		if (!(*chanIt)->fdIsInvited(client.getFD()))
		{
			send(fdc, 473ERR_INVITEONLYCHAN(chanName).c_str(), 4673ERR_INVITEONLYCHAN(chanName).length(), 0);
			return (false);
		}

		// err:474, banned
		if (!(*chanIt)->fdIsBanned(client.getFD()))
		{
			send(fdc, 473ERR_INVITEONLYCHAN(chanName).c_str(), 4673ERR_INVITEONLYCHAN(chanName).length(), 0);
			return (false);
		}
	}


}


// KR : when to use CAP ?
void	Server::cmdCAP(void)
{
	std::cout << "Server: Execute cmdCAP" << std::endl;
}
