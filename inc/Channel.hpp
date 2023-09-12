#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"
#include <string>
#include <iostream>

class   Channel
{
    public:
        Channel();
        Channel(std::string name);
        Channel(const Channel& src);
        Channel& operator=(const Channel &rhs);
        ~Channel();

        // getters
        std::string getChanName();
        std::string getPassword();
        std::string getTopic();

        // setters
        void        setTopic(std::string topic);

    private:
        std::string _chanName;
        std::string _topic;
        std::string _password;
};

#endif