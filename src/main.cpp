#include "../inc/Server.hpp"

int	main(int ac, char *av[])
{
	try
	{
		Server	server;
		Client	client;

		if (ac < 3)
			throw (Server::ExceptionServer(ERRNOMSG"Bad arguments. -> ./ircserv <port> <password>"));
		server.setPort(atoi(av[1]));
		server.setupServerSocket();
		server.setPass(av[2]);
		server.serverConnect();
		return (0);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}