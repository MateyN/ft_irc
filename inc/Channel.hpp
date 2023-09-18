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
        Channel(std::string name, Client* op);
        Channel(const Channel& src);
        Channel& operator=(const Channel &rhs);
        ~Channel();

        // getters
        std::string getChanName() { return _chanName; }
        std::string getTopic() { return _password; }
        std::string getMode() { return _mode; }
        std::string getPassword() { return _password; }
        std::vector <Client *>  getUsr() { return _usr; }
        std::vector <Client *>  getOp() { return _op; }

        // setters
        void        setTopic(std::string topic);

        void        addOp(Client *client);
        void        eraseOp(Client *client, int fd);
        void        addUser(Client *client);
        void        eraseUser(Client *client, int fd);
		bool		fdIsInvited(int fdc);
		bool		fdIsBanned(int fdc);

        bool        Op(Client *client);
        bool        User(Client *client);

    private:
        std::string _chanName; // must begin only by '#'
        std::string _topic;
        std::string _mode;
        std::string	_password;
		std::vector<int>		_invite;
		std::vector<int>		_banned;

        std::vector<Client *>   _usr;
        std::vector<Client *>   _op; // op is also in usr
};

#endif
