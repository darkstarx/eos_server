#ifndef SERVER_HPP
#define SERVER_HPP

#include <assert.h>
#include <ev++.h>
#include <vector>
#include <list>
#include <boost/smart_ptr/detail/spinlock.hpp>

#include "ClientSession_fwd.hpp"


class Server
{
	typedef std::vector<PlayerSession_sptr> players_t;
	typedef std::vector<WorldSession_sptr> worlds_t;

public:
	enum { BUFFER_SIZE = 10 };

	static Server& Instance()
	{
		static Server singleton;
		return singleton;
	}
	~Server();

	void start(const int c_port, const int w_port);
	void stop();

	bool kick_player(const int player_id);
	bool kick_world(const int world_id);

private:
	Server();
	Server(const Server& copy);
	Server& operator=(const Server&);

	ev::io cio;
	ev::io wio;
	ev::sig sig;

	int active;
	int c_portno;
	int c_sockfd;
	int w_portno;
	int w_sockfd;

	void accept_cb(ev::io &watcher, int revents);
	void accept_w_cb(ev::io &watcher, int revents);
	static void signal_cb(ev::sig &signal, int revents);

	players_t players;
	worlds_t worlds;
	boost::detail::spinlock players_lock;
	boost::detail::spinlock worlds_lock;
};

#endif // SERVER_HPP
