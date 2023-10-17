//#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
//#include "../inc/Channel.hpp"

template<typename T>
static std::string	xtos(T conv)
{
	std::stringstream	ss;
	ss << conv;
	return(ss.str());
}

static void handler(int ac, char* av[])
{
	// note : no try catch in main because we do not expect exceptions
	// check(1) : only 2 arguments
	if (ac != 3) 
	{
		std::cerr << RED << "Error: bad arguments -> example: ./ircserv <port> <password> " << RESET << std::endl;
		exit(1);
	}

	// check(2) port : digits only
	for (unsigned long i = 0; i < xtos(av[1]).length(); i++)
	{
		if (!std::isdigit(av[1][i]))
		{
			std::cerr << RED << "Error: the port it is not made of digits" << RESET << std::endl;
			exit(2);
		}
	}

	// check(3) port : between [1024:65535]
	int i = std::stoi(av[1]);
	if (i < PORT_MIN || i > PORT_MAX)
	{
		std::cerr << RED << "Error: port number should be >= 1024 and <= 65535" << RESET << std::endl;
		exit(3);
	}
}

int main(int ac, char *av[])
{
	handler(ac, av);
	try
	{
		Server	server;
		Client	client;

		server.setupServerSocket(av);
		server.serverConnect();
		return 0;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}
