#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"

//h server
#include <map>

typedef	void 				(Server::*pf)(void);
std::map<std::string, pf> 	_cmdMap = // https://stackoverflow.com/questions/138600/initializing-a-static-stdmapint-int-in-c
{
	{"CAP", &Server::cmdCAP},
	{"INVITE", &Server::cmdInvite},
	{"JOIN", &Server::cmdJoin},
	{"KICK", &Server::cmdKick},
	{"MODE", &Server::cmdMode},
	{"MSG", &Server::cmdMsg},
	{"NICK", &Server::cmdNick},
	{"NOTICE", &Server::cmdNotice},
	{"PART", &Server::cmdPart},
	{"QUIT", &Server::cmdQuit},
	{"TOPIC", &Server::cmdTopic},
	{"USER", &Server::cmdUser},
	{"WHOIS", &Server::cmdWhois}
};

//c
void	Server::callCmd(const std::string key) // https://stackoverflow.com/questions/2136998/using-a-stl-map-of-function-pointers
{
	std::map<std::string, pf>::iterator	it = _cmdMap.find(key);

	// check for no find - error or exception ?
	if (it == _cmdMap.end())
		std::cerr << "error: cmd: not found" << std::endl;
	(*it->second)();
}

void	Server::cmdCAP(void)
{
	std::cout << "Execute cmdCAP" << std::endl;
}
