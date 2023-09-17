#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

// ******************************************************
/* // pf BUT NO because different arguments to pass */
/* //h server */
/* #include <map> */

/* typedef	void 				(Server::*pf)(void); */
/* std::map<std::string, pf> 	_cmdMap = // https://stackoverflow.com/questions/138600/initializing-a-static-stdmapint-int-in-c */
/* { */
/* 	{"CAP", &Server::cmdCAP}, */
/* 	{"INVITE", &Server::cmdInvite}, */
/* 	{"JOIN", &Server::cmdJoin}, */
/* 	{"KICK", &Server::cmdKick}, */
/* 	{"MODE", &Server::cmdMode}, */
/* 	{"MSG", &Server::cmdMsg}, */
/* 	{"NICK", &Server::cmdNick}, */
/* 	{"NOTICE", &Server::cmdNotice}, */
/* 	{"PART", &Server::cmdPart}, */
/* 	{"QUIT", &Server::cmdQuit}, */
/* 	{"TOPIC", &Server::cmdTopic}, */
/* 	{"USER", &Server::cmdUser}, */
/* 	{"WHOIS", &Server::cmdWhois} */
/* }; */

/* //c */
/* void	Server::callCmd(const std::string key) // https://stackoverflow.com/questions/2136998/using-a-stl-map-of-function-pointers */
/* { */
/* 	std::map<std::string, pf>::iterator	it = _cmdMap.find(key); */

/* 	// check for no find - error or exception ? */
/* 	if (it == _cmdMap.end()) */
/* 		std::cerr << "error: cmd: not found" << std::endl; */
/* 	(*it->second)(); */
/* } */
// ******************************************************

void Server::_callCmd(Client &client, const std::string& cmd, std::vector<std::string> &params) {
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
		case 1: cmdNick(params[0], *this); break;
		case 2: cmdUser("error"); break;
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

void	Server::cmdNick(std::string nick, Server &serv)
{
	std::cout << "Server: Execute cmdNick" << std::endl;
}

// KR : when to use CAP ?
void	Server::cmdCAP(void)
{
	std::cout << "Server: Execute cmdCAP" << std::endl;
}

void	Server::cmdInvite(void)
{
	std::cout << "Server: Execute cmdInvite" << std::endl;
}
