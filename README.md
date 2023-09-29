# ft_irc

# TODO

- [o] subject requirements :
	- [X] C++ 98
	- [X] do not develop a client
	- [X] do not handle communication server-server
	- [X] ./ircserv <port> <password>
	- [X] poll() OR select() OR kqueue() OR epoll()
	- [X] simultaneously handle multiple clients without blocking ([non-blocking tcp tuto](https://bousk.developpez.com/cours/reseau-c++/TCP/06-client-non-bloquant/))
	- [X] no fork
	- [X] only ONE poll() to handle read(), write(), listen(), etc.
	- [X] do not use read/recv or write/send with any FD without using poll()
	- [X] choose one of the multiple client IRC as a reference ([irssi](https://irssi.org/)), but your reference client will be used during the assessment
	- [X] the reference client must connect to server without errors
	- [X] client-server communication is TCP/IP (v4 or v6)
	- [ ] reference client must have those functionalities :
		- [X] authentification
		- [X] define nickname
		- [X] define username
		- [X] join channel
		- [ ] send private message (PM) TO DO
		- [ ] receive PM TO DO
		- [X] all sent messages in a channel must be transmitted to all clients who joined this specific channel
		- [X] we must have operators and basic users
		- [ ] implement specific commands to channel operators :
			- [X] KICK : kick client from chan
			- [ ] INVITE : invite client to chan TO TEST
			- [X] TOPIC : modify or display chan theme
			- [ ] MODE : TO DO
				- [ ] -i : define/delete chan as invitation only
				- [ ] -t : define/delete restrictions on TOPIC comand for chan operators
				- [ ] -k : define/delete password chan
				- [ ] -o : give/remove operator chan privilege
				- [ ] -l : define/delete Nmax users for this chan
	- [X] fcntl(fd, F_SETFL, O_NONBLOCK) instead of write() (MACOS), use file descriptors in non-blocking fd ([tuto](https://www.linuxtoday.com/blog/blocking-and-non-blocking-i-0/))
	- [ ] tests / error handling :
		- [ ] server : partially received data
		- [ ] weak bandwidth
		- [X] nc 127.0.0.1 6667 (see subject)
- [ ] server :
	- [X] create a server
	- [X] make the access to the server password-protected
	- [X] receive clients
- [ ] client :
	- [X] choose which client to use ([irssi](https://irssi.org/) ?)
	- [X] connect to port password :
		- [ ] *register* its connection (NICK, USER)

# Doc

- Internet Relay Chat : textual communication protocol on Intrnet, instant, discussions in groups via channels and 1-1
- [Socket](https://www.tutorialspoint.com/unix_sockets/what_is_socket.htm)
	- Allow communication bw two processes on the same or different machines : a way to talk to other computers using std Unix file descriptors.
- [RFC1459 (Oikarinen & Reed, 1993)](https://datatracker.ietf.org/doc/html/rfc1459)
	- [X] Client have a unique nickname having a max(9) char
	- [X] All servers must have : real name of the host that the client is running on, the username of the client on that host, and the server to which the client is connected
	- Operators (special class of clients) perform general maintenance functions on the network
	- [X] Channel is created implicitly when the first client joins it (and becomes the operator, named 'chop' | 'chanop') and ceases to exist when the last client leaves it
	- [X] Channel name = string beginning with '&' or '#' and maxlen = 200, no spaces, no ^G or ASCII 7 or ',' (= list item separator by the protocol)
	- [ ] Distributed channel (&) = known to all TO CHECK
	- [ ] A limit of 10 channels for a client to be part of TO CHECK?
	- [X] Chop is identified by the '@' symbol 
	- [ ] {}| are lowercase equivalents of []\ (critical when determining the equivalence of two nicknames)
	- [X] make an error when msg begins with ':', say : "error: no prefix begining by ':' is allowed"
	- [X] IRC messages are always lines of char terminated with a CR-LF (carriage return - line feed) pair, and these messages shall not exceed 512 char couting all char including the CR-LF (\r\n, so max 510 for command and its params)
	- [ ] See BNF (Backus-Naur form) for the messages
- [RFC2810 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2810)
- [RFC2811 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2811)
- [RFC2812 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2812)
	- Labels :
		- [ ] when connecting, client unique id and the server which introduced the client
		- [ ] allow operators who can :
			- [ ] disconnect and reconnect servers (see if applicable)
			- [X] remove a user from server (KICK)
		- [X] channels begin with #, case insensitive
- [RFC2813 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2813)
- [UChicago X-Projects Tutorial](http://chi.cs.uchicago.edu/chirc/irc.html)
- [How to make an IRC server connection (Sotiriou, 2008)](https://oramind.com/tutorial-how-to-make-an-irc-server-connection/)
- [Beej's Guide to Network Programming (Hall, 2023)](https://beej.us/guide/bgnet/html/)
- [IRCgod](https://ircgod.com/posts/)
- [Socket Programming in C/C++ (GG)](https://www.geeksforgeeks.org/socket-programming-cc/)
- [Socket Programming In C++:A Beginner's Guide](https://marketsplash.com/tutorials/cpp/cplusplus-scoket/)
- [Modern IRC Client Protocol (Allnutt, Oaks & Lorentz, WIP)](https://modern.ircdocs.horse/)
	- [Capability negociation](https://modern.ircdocs.horse/#capability-negotiation) :
		-  
	- [Connection registration](https://modern.ircdocs.horse/#connection-registration) :
		1. CAP LS 302 : 
- [Similar project with guidelines](https://www.cs.cmu.edu/~srini/15-441/S10/project1/pj1_description.pdf)
	- IRC is composed of a set of nodes
	- Each node runs a process called routing daemon
	- `nodeID` = 1
	- `destination` = IRC nickname or channel as a null terminated char string (9 char long and no spaces)
	- [X] read sections 1-3 o RFC 1459 (do not focus on details, understand role of clients and server)
	- [ ] read 4 and 6 of RFC 1459 (high level)
	- [ ] return to 1-3 with implementation eyes + think about data structures to maintiain, what info needs to be stored about each client ?
	- [ ] start with simple server that accepts connections from multiple clients. Take a message and refet to all clients (including sender). This will let you focus on socket programming aspects of a server
	- [X] write standalone IRC srrver, decompose problem to test each part, find common tasks among different commands and group them into procedure to avoid writing the same code twice : start by implementing the routines that read and parse commands, then implement commands one by one testing each
	- [ ] Be liberal in what you accept and conervative in what you send (RFC 1122: http://www.ietf.org/rfc/rfc1122.txt, page 11)
	- [ ] Your code should be modular, extensible, readable
	- [ ] [Unix socket FAQ C++](http://developerweb.net/viewforum.php?id=59)

<!--
#include <arpa/inet.h>	// htons(), htonl(), htohl(), inet_addr(), inet_ntoa()
#include <fcntl.h>		// fcntl()
#include <netdb.h> 		// getprotobyname(), gethostbyname(), getaddrinfo(), freeaddrinfo()
#include <poll.h>		// poll()
#include <signal.h>		// signal(), sigaction()
#include <sys/socket.h>	// socket(), setsockopt(), getsockname(), getaddrinfo(), freeaddrinfo(), bind(), connect(), listen(), accept(), send(), recv()
#include <sys/stat.h>	// fstat()
#include <sys/types.h>	// getaddrinfo(), freeaddrinfo(), connect()
#include <unistd.h>		// close(), lseek()
-->
