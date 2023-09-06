#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Server.hpp"
# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h> // sockaddr_in, IPPROTO_TCP
# include <arpa/inet.h> // hton*, ntoh*, inet_addr
# include <unistd.h>  // close
# include <cerrno> // errors
# include <stdexcept>
# include <vector>
# include <map>
# include <fcntl.h>

class Client
{
    public:
        Client();
        Client(int fd);
        Client(const Client& src);
        Client &operator=(const Client& rhs);
        ~Client();

        bool    setNick;
        int     getFD()const;
        std::string getNickname();
        void    setNickname(std::string nickname);
        void    setUser(std::string user);
        std::string getUser();

    private:
        int _fd;
        std::string _nick;
        std::string _username;

};

#endif