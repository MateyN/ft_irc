#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Server.hpp"
//#include <string>
//class Client;

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
        std::string getTopic();
        std::string getPassword();
		std::string	getUsers();
        std::vector <Client *>  getUser();
		bool		getTopicMode();
		bool		getLimitMode();
		bool		getPassMode();
		bool		getInviteMode();
		int			getLimit();

        // setters
        void        setTopic(std::string topic, Client *client);
		void		setTopicMode(bool mode);
		void		setPassMode(bool mode);
		void		setInviteMode(bool mode);
		void		setChanPass(std::string password);
		void		setLimit(bool mode, int limit);
		bool		setOp(bool mode, std::string user);

		void		addUser(Client *client);
		void		addGuest(Client *client);
		void		eraseUser(Client *client, int fd);
		bool		addOp(Client *client);
		bool		eraseOp(Client *client);
		void		eraseGuest(Client *client);
		bool		Op(Client *client);
		bool		isNumber(std::string arg);						
		bool		User(Client *client);
		bool 		userExist(const std::string& nickname);
		bool		nickMember(std::string nickname);
		bool		Guest(Client *client);		
		//bool		fdIsInvited(int fdc);
		//bool		fdIsBanned(int fdc);

    private:
		std::vector<Client *>   _usr;
        std::vector<Client *>   _op; // op is also in usr
		std::vector<Client *>	_guest;

        std::string _chanName; // must begin only by '#'
        std::string _topic;
        std::string	_password;
		
		//std::vector<int>		_invite;
		//std::vector<int>		_banned;

		bool						_topicOp;
		bool						_isLimitSet;
		bool						_isPassSet;
		bool						_isInvite;

		int							_coutLimit;
};

#endif