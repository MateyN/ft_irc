#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Server.hpp"
# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cerrno>
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

        bool        _setNick;
        bool        isRegister();

        int         getFD()const;

        void        setNickname(std::string nickname);
        void        setIsRegister(bool registered);
        void        setUser(std::string user);
        bool        isConnect();
        
        std::string getUser();
        std::string getNickname();

    private:
        int         _fd;
        
        std::string _nick;
        std::string _username;

        bool        _isRegister;
        bool        _connect;
};

#endif