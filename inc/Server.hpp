#ifndef SERVER_HPP
# define SERVER_HPP

class	Server
{
public:
	Server(void);
	Server(Server const & src);
	Server&	operator=(Server const & rhs);
	virtual ~Server(void);

	int			getServer(void) const;

private:
};

#endif // SERVER_HPP

