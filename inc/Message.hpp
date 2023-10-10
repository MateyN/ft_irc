#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# define MSG(nick, user, cmd, msg)				":" + nick + "!" + user + "@localhost " + cmd + " :" + msg + "\r\n"
# define RPL332_TOPIC(nick, chanName, topic) 	":localhost 332 " + nick + " " + chanName + " :" + topic + "\r\n"
# define RPL353_NAMREPLY(nick, chanName, users)	":localhost 353 " + nick + " = " + chanName + " :" +  users + "\r\n"
# define RPL366_ENDOFNAMES(nick, chanName)		":localhost 366 " + nick + " " + chanName + " :End of /NAMES list\r\n"
# define ERR401_NOSUCHNICK 401
# define ERR403_NOSUCHCHANNEL 403
# define ERR404_CANNOTSENDTOCHAN 404
# define ERR411_NORECIPIENT 411
# define ERR412_NOTEXTTOSEND 412
# define ERR421_UNKNOWNCOMMAND 421
# define ERR431_NONICKNAMEGIVEN 431
# define ERR432_ERRONEUSNICKNAME 432
# define ERR433_NICKNAMEINUSE 433
# define ERR436_NICKCOLLISION 436
# define ERR441_USERNOTINCHANNEL 441
# define ERR442_NOTONCHANNEL 442
# define ERR443_USERONCHANNEL 443
# define ERR461_NEEDMOREPARAMS 461
# define ERR462_ALREADYREGISTERED 462
# define ERR464_PASSWDMISMATCH 464
# define ERR473_INVITEONLYCHAN 473
# define ERR472_UNKNOWNMODE 472
# define ERR471_CHANNELISFULL 471
# define ERR474_BANNEDFROMCHAN 474
# define ERR475_BADCHANNELKEY 475
# define ERR482_CHANOPRIVSNEEDED 482
# define ERR696_INVALIDMODEPARAM 696
# define ERR501_UMODEUNKNOWNFLAG 501

#endif