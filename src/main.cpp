#include <iostream>
#include <string>

static int	err(std::string s)
{
	std::cerr << s << std::endl;
	return (1);
}

int	main (int ac, char *av[])
{
	(void) av;
	if (ac != 3) 
		return (err("error: bad arguments, must have <port> <password>"));
//	ConfigFile 	IRCconfig("./srcs/irc.config");
//	Server		*serv = NULL;
//	try
//	{
//		serv = new Server(av[1], av[2], &IRCconfig);
//		serv->monitoring();
//		serv->stop();
//	}
//	catch (std::exception &e)
//	{
//		std::cerr << "error: exception catched durng serv initialization: " << e.what() << std::endl;
//		exit(EXIT_FAILURE);
//	}
//	delete serv;
//	exit(EXIT_SUCCESS);
}
