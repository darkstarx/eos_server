#include "Server.hpp"
#include "ClientSession.hpp"
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>


Server::Server(const Server& copy)
: active(false)
, portno(0)
, sockfd(0)
, clients()
{
}

Server::Server()
: active(false)
, portno(0)
, sockfd(0)
, clients()
{
}

Server::~Server()
{
	stop();
}

Server& Server::operator=(const Server& )
{
}

void Server::start(const int port)
{
	assert(port >= 1024);
	if (active) stop();
	printf("Listening on port %d\n", port);
	portno = port;

	sockfd = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		perror("bind");
	}

	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

	listen(sockfd, 5);

	io.set<Server, &Server::accept_cb>(this);
	io.start(sockfd, ev::READ);

	sig.set<&Server::signal_cb>();
	sig.start(SIGINT);

	active = true;
	printf("Server started.\n");
}

void Server::stop()
{
	io.stop();
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	active = false;

	boost::lock_guard<boost::detail::spinlock> clients_guard(clients_lock);
	clients.clear();

	printf("Server stopped.\n");
}

void Server::signal_cb(ev::sig &signal, int revents)
{
	printf("Catch interruption.\n");
	signal.loop.break_loop();
}

void Server::accept_cb(ev::io &watcher, int revents)
{
	if (EV_ERROR & revents) {
		perror("got invalid event");
		return;
	}

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);

	if (client_sd < 0) {
		perror("accept error");
		return;
	}

	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

	printf("client accepted. creating client session\n");

	boost::lock_guard<boost::detail::spinlock> clients_guard(clients_lock);
	ClientSession_sptr client(new ClientSession(clients.size(), client_sd, std::string(client_ip),
		boost::bind(&Server::kick_client, this, _1)));
	clients.push_back(client);
}

bool Server::kick_client(const int client_id)
{
	boost::lock_guard<boost::detail::spinlock> clients_guard(clients_lock);
	printf("client %i will be kicked\n", client_id);
	if (client_id < 0 || client_id >= clients.size()) return false;

	clients.erase(clients.begin() + client_id);
	printf("clients now: %i\n", clients.size());
	return true;
}
