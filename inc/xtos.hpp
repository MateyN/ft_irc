#ifndef XTOS
# define XTOS

#include <iostream>
#include <sstream>

template<typename T>
std::string	xtos(T conv)
{
	std::stringstream	ss;
	ss << conv;
	return(ss.str());
}

#endif //XTOS