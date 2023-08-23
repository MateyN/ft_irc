#include "Server.hpp"

int main(int ac, char *av[])
{
    (void) av;
    if (ac != 3)
    {
        return Server::err("error: bad arguments, must have <port> <password>");
    }

    int port = std::atoi(av[1]);
    const char* password = av[2];

    Server ircServer(port, password);
    ircServer.run();

    return 0;
}
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
//}
