#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"

void    Server::parseCmds(std::string msg)
{
    token.clear();
    cmd.clear();

    size_t  spacePosition = msg.find(' '); // search for the first space char
    if (spacePosition != std::string::npos)
    {
        token = msg.substr(0, spacePosition); // token is everything before the space
        cmd = msg.substr(spacePosition + 1); // msg is everything after the space
    }
    else
    {
        token = msg;
    }
    std::cout << "Token: " << token << std::endl;
    std::cout << "Cmd: " << cmd << std::endl;
    std::cout << std::endl;
}

std::string Server::channelParse(std::string input, size_t start)
{
    std::string chanName;
    size_t      end = start;

    if (end == 0)
        end = input.find("#");
    if (end != std::string::npos && end != input.size() - 1)
    {
        size_t space = input.find(" ", end);
        size_t comma = input.find(",", end);
        if (comma != std::string::npos) // to avoid unused variable error
        {
            chanName = input.substr(end, comma - end);
        }
        else
        {
        if (space == std::string::npos)
            space = input.size();
        chanName = input.substr(end, space - end); // extracting the chanName
        }
    }
    else
    {
        chanName = input; // if '#' was not found or it's at the end, set chanName to the entire input.
    }
    return chanName;
}