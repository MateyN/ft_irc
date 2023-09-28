#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Message.hpp"

void	Server::msgSend(std::string msg, int fd)
{
	std::cout << CYAN << "Send -> " << RESET << msg.append(CRLF) << std::endl;
 	send(fd, msg.c_str(), msg.size(), 0);
}

void	Server::sendToUsersInChan(std::string msg, int fd)
{
	std::vector<Client*>users = channels->getUser();
	for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); it++)
	{
		if ((*it)->getFD() != fd)
			msgSend(msg, (*it)->getFD());
	}
}

void Server::errorMsg(int errCode, int fd, std::string str1, std::string str2, std::string str3, std::string info)
{
	std::stringstream ss;
	ss << errCode;
	(void)info;
	std::string msg = ss.str();

	switch(errCode)
	{
		case ERR401_NOSUCHNICK:
			msg += ":localhost 401 " + str1 + " :No such nick/channel\r\n";
			break;
		case ERR403_NOSUCHCHANNEL:
			msg += ":localhost 403 " + str1 + " :No such channel\r\n";
			break;
		case ERR404_CANNOTSENDTOCHAN:
			msg += ":localhost 404 " + str1 + " :Cannot send to channel\r\n";
			break;
		case ERR411_NORECIPIENT:
			msg += ":localhost 411 :No recipient given " + str1 + "\r\n";
			break;
		case ERR412_NOTEXTTOSEND:
			msg += ":localhost 412 :No text to send\r\n";
			break;
		case ERR421_UNKNOWNCOMMAND:
			msg += ":localhost 421 " + str1 + " :Unknown command";
			break;
		case ERR431_NONICKNAMEGIVEN:
			msg += ":localhost 431 :No nickname given\r\n";
			break;
		case ERR432_ERRONEUSNICKNAME:
			msg += ":localhost 432 " + str1 + " :Erroneous nickname\r\n";
			break;
		case ERR433_NICKNAMEINUSE:
			msg += ":localhost 433 " + str1 + " :Nickname is already in use\r\n";
			break;
		case ERR436_NICKCOLLISION:
			msg += ":localhost 436 " + str1 + " :Nickname collision KILL from " + str2 + "@" + str3;
			break;
		case ERR441_USERNOTINCHANNEL:
			msg += ":localhost 441 " + str1 + " " + str2 + " :They aren't on that channel\r\n";
			break;
		case ERR442_NOTONCHANNEL:
			msg += ":localhost 442 " + str1 + " :You're not on that channel\r\n";
			break;
		case ERR443_USERONCHANNEL:
			msg += ":localhost 443 " + str1 + " " + str2 + " :is already on channel\r\n";
			break;
		case ERR461_NEEDMOREPARAMS:
			msg += ":localhost 461 " + str1 + str2 + ":Not enough parameters\r\n";
			break;
		case ERR462_ALREADYREGISTERED:
			msg += ":localhost 462 :Unauthorized command (already registered)\r\n";
			break;
		case ERR464_PASSWDMISMATCH:
			msg += ":localhost 464 :Password incorrect\r\n";
			break;
		case ERR471_CHANNELISFULL:
			msg += ":localhost 471 " + str1 + " " + str2 + " :Cannot join channel (+l)\r\n";
			break;
		case ERR472_UNKNOWNMODE:
			msg += ":localhost 472 " + str1 + " " + str2 + " :is not a recognised channel mode.\r\n";
			break;
		case ERR473_INVITEONLYCHAN:
            msg += ":localhost 473 " + str1 + " " + str2 + " :Cannot join channel (+i)\r\n";
			break;
        case ERR474_BANNEDFROMCHAN:
            msg += ":localhost 474 " + str1 + " :Cannot join channel (+b)\r\n";
            break;
		case ERR475_BADCHANNELKEY:
			msg += ":localhost 475 " + str2 + " :Cannot join channel (+k)\r\n";
			break;
		case ERR482_CHANOPRIVSNEEDED:
			msg += + ":localhost 482 " + str1 + " " + str2 + " :You're not a channel operator\r\n";
			break;
		case ERR501_UMODEUNKNOWNFLAG:
			msg += ":localhost 501 :Unknown MODE flag\r\n";
			break;
		case 650:
			msg += str1 + ":<channel> [:<topic>]";
			break;
		case ERR696_INVALIDMODEPARAM:
			msg += + ":localhost 696 " + str1 + " " + str2 + " " + str3 + " *" + " :You must specify a parameter for the " + str3 + " mode\r\n";
			break;
		default:
			msg += " " + str1 + " :Unknown error";
			break;
	}
	msg = ": " + msg + CRLF ;
	msgSend(msg, fd);
}

void Server::welcomeMsg(Client *client)
{
    std::string msg = "001 " + client->getNickname() + " : " + "\033[1;32mWelcome -> " + client->getNickname() + " @" + client->getHost() + CRLF;

   	msg += "\t  ________     __________________           _____________\n";
    msg += "\t |        |   |                 ||        |||            |\n";
    msg += "\t |********|   |******||||||******|      ||***************|\n";
    msg += "\t ||******||   ||*****|     |*****|     |*****||||||||****|\n";
    msg += "\t   |****|       |****|     |*****|    |*****|       ||||||\n";
    msg += "\t   |****|       |****|     |*****|    |*****|\n";
    msg += "\t   |****|       |****|||||||*****|    |*****|\n";
    msg += "\t   |****|       |*************||      |*****|\n";
    msg += "\t   |****|       |****//////*****|     |*****|\n";
    msg += "\t   |****|       |****|     |****|     |*****|\n";
    msg += "\t   |****|       |****|     |****|     |*****|\n";
    msg += "\t   |****|       |****|     |****|      |*****|        ||||||\n";
    msg += "\t  _|****|_     _|****|_   _|****|_      |*****|||||||||****|\n";
    msg += "\t |        |   ||      || ||      ||      ||                |\n";
    msg += "\t |________|   ||______|| ||______||       |||_____________|\n";

    msgSend(msg, client->getFD());
}
