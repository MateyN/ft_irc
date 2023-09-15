#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/xtos.hpp"

static void handler(int ac, char* av[])
{
	// note : no try catch in main because we do not expect exceptions
	// check(1) : only 2 arguments
	if (ac != 3) 
	{
		std::cerr << "error: bad arguments: must have <port> <password>" << std::endl;
		exit(1);
	}

	// check(2) port : digits only
	for (unsigned long i = 0; i < xtos(av[1]).length(); i++)
	{
		if (!std::isdigit(av[1][i]))
		{
			std::cerr << "error: port: it is not made of digits" << std::endl;
			exit(2);
		}
	}

	// check(3) port : between [1024:65535]
	int i = std::stoi(av[1]);
	if (i < PORT_MIN || i > PORT_MAX)
	{
		std::cerr << "error: port: number should be >= 1024 and <= 65535" << std::endl;
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

		server.setPort(atoi(av[1]));
		server.setupServerSocket();
		server.setPass(av[2]);
		server.serverConnect();
		// need connection
		return 0;
	}
	
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

/*
#include "../inc/xtos.hpp"

static void			handler(int ac, char* av[])
{
	// note : no try catch in main because we do not expect exceptions

	// check(1) : only 2 arguments
	if (ac != 3) 
	{
		std::cerr << "error: bad arguments: must have <port> <password>" << std::endl;
		exit(1);
	}

	// check(2) port : digits only
	for (unsigned long i = 0; i < xtos(av[1]).length(); i++)
	{
		if (!std::isdigit(av[1][i]))
		{
			std::cerr << "error: port: it is not made of digits" << std::endl;
			exit(2);
		}
	}

	// check(3) port : between [1024:65535]
	int i = std::stoi(av[1]);
	if (i < PORT_MIN || i > PORT_MAX)
	{
		std::cerr << "error: port: number should be >= 1024 and <= 65535" << std::endl;
		exit(3);
	}
}

int	main (int ac, char *av[])
{
	// * arguments' checks
	handler(ac, av);
	try
	{
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

	// https://beej.us/guide/bgnet/html/split/system-calls-or-bust.html#system-calls-or-bust
		int			status;
		int			sockfd;
		struct		addrinfo hints;
		struct		addrinfo *servinfo;							// points to the results

		// getaddrinfo : load up socket address structure to host and service name
		// connect to 3460, NULL can be changed to www.example.net
		memset(&hints, 0, sizeof hints);	// empty struct
		hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6, AF = Address Family
		hints.ai_socktype = SOCK_STREAM;	// TCP stream sockets
		hints.ai_flags = AI_PASSIVE;		// fill in my IP for me, if we have a localIP address, drop this and put an IP address in first arg of getaddrinfo()
		if ((status = getaddrinfo(NULL, av[1], &hints, &servinfo)) != 0) // null should be 
		{
			std::cerr << "error: server: getaddrinfo() failed: " << gai_strerror(status) << std::endl;
			throw std::runtime_error("");
		}

		// socket : get the f(s)d
		if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
			throw std::runtime_error("error: server: socket() failed");

		// bind : bind socket descriptor to the port we passed in to getaddrinfo()
		int	bind_res;
		if ((bind_res = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
			throw std::runtime_error("error: server: bind() failed");

		// connect : connects !
		if ((connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
			throw std::runtime_error("error: server: connect() failed");

		freeaddrinfo(servinfo);	// free the linked list

		int port = std::atoi(av[1]);
        const char* password = av[2];

        Server ircServer(port, password);
        ircServer.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	return (0);
}

	// To add before bind : lose the pesky "Address already in use" error essage, see end of 5.3
	// int	yes = 1;
	// if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
	// {
	// 	std::cerr << "error: setsockopt" << std::endl;
	// 	exit(4);
	// }

	*/

	
