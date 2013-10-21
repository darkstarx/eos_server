#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <glog/logging.h>
#include "Server.hpp"
#include "ReceptionRoom.hpp"
#include "PlayerSession.hpp"
#include "WorldSession.hpp"


Server::Server()
: active(false)
, c_portno(0)
, c_sockfd(0)
, w_portno(0)
, w_sockfd(0)
, reception(new ReceptionRoom())
{
}

Server::~Server()
{
	stop();
}

/** private (noncopyable) */
Server::Server(const Server& copy)
: active(false)
, c_portno(0)
, c_sockfd(0)
, w_portno(0)
, w_sockfd(0)
, reception(new ReceptionRoom())
{
}

/** private (noncopyable) */
Server& Server::operator=(const Server& )
{
}

void Server::start(const int c_port, const int w_port)
{
	if (active) return;

	assert(c_port > 1024);
	assert(w_port > 1024);
	assert(c_port != w_port);

	if (active) stop();
	c_portno = c_port;
	w_portno = w_port;

	c_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	w_sockfd = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(c_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(c_sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		LOG(FATAL) << "Server starting: binding clientside socket with address";
	}
	fcntl(c_sockfd, F_SETFL, fcntl(c_sockfd, F_GETFL, 0) | O_NONBLOCK);

	addr.sin_port = htons(w_port);
	if (bind(w_sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		LOG(FATAL) << "Server starting: binding worldside socket with address";
	}
	fcntl(w_sockfd, F_SETFL, fcntl(w_sockfd, F_GETFL, 0) | O_NONBLOCK);

	listen(c_sockfd, 5);
	listen(w_sockfd, 5);

	cio.set<Server, &Server::accept_cb>(this);
	cio.start(c_sockfd, ev::READ);
	wio.set<Server, &Server::accept_w_cb>(this);
	wio.start(w_sockfd, ev::READ);

	sig.set<&Server::signal_cb>();
	sig.start(SIGINT);

	active = true;
	LOG(INFO) << "Server started. Listening clients on port " << c_port << ", worlds on port " << w_port;
}

void Server::stop()
{
	if (!active) return;

	cio.stop();
	shutdown(c_sockfd, SHUT_RDWR);
	close(c_sockfd);
	wio.stop();
	shutdown(w_sockfd, SHUT_RDWR);
	close(w_sockfd);
	active = false;

	LOG(INFO) << "Server stopped.";
}

void Server::signal_cb(ev::sig &signal, int revents)
{
	LOG(INFO) << "Catch interruption.";
	signal.loop.break_loop();
}

void Server::accept_cb(ev::io &watcher, int revents)
{
	if (EV_ERROR & revents) {
		LOG(ERROR) << "Got invalid event.";
		return;
	}

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);

	if (client_sd < 0) {
		LOG(ERROR) << "Accept error.";
		return;
	}

	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

	LOG(INFO) << "Player accepted. Creating player session.";

	PlayerSession_sptr player(new PlayerSession(client_sd, std::string(client_ip)));
	player->enter_room(reception);
}

void Server::accept_w_cb(ev::io &watcher, int revents)
{
	if (EV_ERROR & revents) {
		LOG(ERROR) << "Got invalid event.";
		return;
	}

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);

	if (client_sd < 0) {
		LOG(ERROR) << "Accept error.";
		return;
	}

	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

	LOG(INFO) << "World accepted. Creating world session.";

	WorldSession_sptr world(new WorldSession(client_sd, std::string(client_ip)));
	reception->join_world(world);
}
