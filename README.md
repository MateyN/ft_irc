# ft_irc

# Todo

- [ ] subject requirements :
	- [ ] C++ 98
	- [ ] do not develop a client
	- [ ] do not handle communication server-server
	- [ ] ./ircserv <port> <password>
	- [ ] poll() OR select() OR kqueue() OR epoll()
	- [ ] simultaneously handle multiple clients without blocking ([non-blocking tcp tuto](https://bousk.developpez.com/cours/reseau-c++/TCP/06-client-non-bloquant/))
	- [ ] no fork
	- [ ] only ONE poll() to handle read(), write(), listen(), etc.
	- [ ] do not use read/recv or write/send with any FD without using poll()
	- [ ] choose one of the multiple client IRC as a reference ([irssi](https://irssi.org/)), but your reference client will be used during the assessment
	- [ ] the reference client must connect to server without errors
	- [ ] client-server communication is TCP/IP (v4 or v6)
	- [ ] reference client must have those functionalities :
		- [ ] authentification
		- [ ] define nickname
		- [ ] define username
		- [ ] join channel
		- [ ] send private message (PM)
		- [ ] receive PM
		- [ ] all sent messages in a channel must be transmitted to all clients who joined this specific channel
		- [ ] we must have operators and basic users
		- [ ] implement specific commands to channel operators :
			- [ ] KICK : kick client from chan
			- [ ] INVITE : invite client to chan
			- [ ] TOPIC : modify or display chan theme
			- [ ] MODE :
				- [ ] -i : define/delete chan as invitation only
				- [ ] -t : define/delete restrictions on TOPIC comand for chan operators
				- [ ] -k : define/delete password chan
				- [ ] -o : give/remove operator chan privilege
				- [ ] -l : define/delete Nmax users for this chan
	- [ ] fcntl(fd, F_SETFL, O_NONBLOCK) instead of write() (MACOS), use file descriptors in non-blocking fd ([tuto](https://www.linuxtoday.com/blog/blocking-and-non-blocking-i-0/))
	- [ ] tests / error handling :
		- [ ] server : partially received data
		- [ ] weak bandwidth
		- [ ] nc 127.0.0.1 6667 (see subject)
- [ ] server :
	- [ ] create a server
	- [ ] make the access to the server password-protected
	- [ ] receive clients
- [ ] client :
	- [ ] choose which client to use ([irssi](https://irssi.org/) ?)
	- [ ] connect to port password :
		- [ ] *register* its connection (NICK, USER)

# Doc

- Internet Relay Chat : textual communication protocol on Intrnet, instant, discussions in groups via channels and 1-1
- [Socket](https://www.tutorialspoint.com/unix_sockets/what_is_socket.htm)
	- Allow communication bw two processes on the same or different machines : a way to talk to other computers using std Unix file descriptors.
- [RFC1459 (Oikarinen & Reed, 1993)](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC2810 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2810)
- [RFC2811 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2811)
- [RFC2812 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2812)
- [RFC2813 (Kalt, 2000)](https://datatracker.ietf.org/doc/html/rfc2813)
- [How to make an IRC server connection (Sotiriou, 2008)](https://oramind.com/tutorial-how-to-make-an-irc-server-connection/)
- [Beej's Guide to Network Programming (Hall, 2023)](https://beej.us/guide/bgnet/html/)
- [IRCgod](https://ircgod.com/posts/)
- [Socket Programming in C/C++ (GG)](https://www.geeksforgeeks.org/socket-programming-cc/)
- [Socket Programming In C++:A Beginner's Guide](https://marketsplash.com/tutorials/cpp/cplusplus-scoket/)
- [UChicago X-Projects](http://chi.cs.uchicago.edu/chirc/irc_examples.html)
- [Modern IRC Client Protocol (Allnutt, Oaks & Lorentz, WIP)](https://modern.ircdocs.horse/)

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
