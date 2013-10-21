#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <glog/logging.h>
#include "Server.hpp"
#include "PlayerSession.hpp"
#include "WorldSession.hpp"


Server::Server()
: active(false)
, c_portno(0)
, c_sockfd(0)
, w_portno(0)
, w_sockfd(0)
, players()
, worlds()
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
, players()
, worlds()
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

	boost::lock_guard<boost::detail::spinlock> players_guard(players_lock);
	players.clear();

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

	boost::lock_guard<boost::detail::spinlock> players_guard(players_lock);
	PlayerSession_sptr player(new PlayerSession(players.size(), client_sd, std::string(client_ip),
		boost::bind(&Server::kick_player, this, _1)));
	players.push_back(player);
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

	boost::lock_guard<boost::detail::spinlock> worlds_guard(worlds_lock);
	WorldSession_sptr client(new WorldSession(worlds.size(), client_sd, std::string(client_ip),
		boost::bind(&Server::kick_world, this, _1)));
	worlds.push_back(client);
}

bool Server::kick_player(const int player_id)
{
	boost::lock_guard<boost::detail::spinlock> players_guard(players_lock);
	LOG(INFO) << "Player " << player_id << " will be kicked.";
	if (player_id < 0 || player_id >= players.size()) return false;

	players.erase(players.begin() + player_id);
	LOG(INFO) << "Players now: " << players.size();
	return true;
}

bool Server::kick_world(const int world_id)
{
	boost::lock_guard<boost::detail::spinlock> worlds_guard(worlds_lock);
	LOG(INFO) << "World " << world_id << " will be destroyed.";
	if (world_id < 0 || world_id >= worlds.size()) return false;

	worlds.erase(worlds.begin() + world_id);
	LOG(INFO) << "Worlds now: " << worlds.size();
	return true;
}
