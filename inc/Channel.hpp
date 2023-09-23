#ifndef CHANNEL_HPP
# define CHANNEL_HPP

//#include "Client.hpp"
#include "Server.hpp"
//#include <string>
//#include <iostream>

class Channel
{
	public:
			Channel();
			Channel(std::string name);
			Channel(const Channel &src);
			Channel &operator=(const Channel &rhs);
			~Channel();

			// getters
			std::string					getChanName() { return _chanName; }
			std::string					getTopic() { return _topic; }
			std::string					getPassword() { return _password; }
			std::string					getUsers();
			std::vector<Client*>		getUser() { return _usr; }
			int							getLimit();
			bool						getTopicMode();
			bool						getLimitMode();
			bool						getPassMode();
			bool						getInviteMode();

			// setters
			void						setTopic(std::string topic, Client *client);
			void						setTopicMode(bool mode);
			void						setPassMode(bool mode);
			void						setInviteMode(bool mode);
			void						setChannelPassword(std::string password);
			void						setLimit(bool mode, int limit);
			bool						setOperator(bool mode, std::string username);

			void						addUser(Client *client);
			void						addGuest(Client *client);
			void						eraseUser(Client *client, int fd);
			bool						addOp(Client *client);
			bool						eraseOp(Client *client);
			void						eraseGuest(Client *client);
			bool						Op(Client *client);
			bool						isNumber(std::string arg);						
			bool						User(Client *client);
			bool 						userExist(const std::string& nickname);
			bool						nickMember(std::string nickname);
			bool						Guest(Client *client);
			bool						fdIsInvited(int fdc);
			bool						fdIsBanned(int fdc);

	private:
			std::vector<Client*>		_usr;
			std::vector<Client*>		_op; // op is also in usr
			std::vector<Client*>		_guest;
			std::string 				_chanName; // must begin only by '#'
			std::string					_topic;
			std::string					_password;
			std::vector<int>		_invite;
			std::vector<int>		_banned;
			bool						_topicOp;
			bool						_isLimitSet;
			bool						_isPassSet;
			bool						_isInvite;
			int							_countLimit;
};

#endif