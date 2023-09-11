#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Server.hpp"

class   Channel
{
    public:
        Channel();
        Channel(std::string name);
        Channel(const Channel& src);
        Channel& operator=(const Channel &rhs);
        ~Channel();

        std::string getChanName();
        std::string getPassword();
        std::string getTopic();

    private:
        std::string _chanName;
        std::string _topic;
        std::string _password;
};

#endif