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
	typedef std::vector<ClientSession_sptr> clients_t;

public:
	enum { BUFFER_SIZE = 10 };

	static Server& Instance()
	{
		static Server singleton;
		return singleton;
	}
	~Server();

	void start(const int port);
	void stop();

	bool kick_client(const int client_id);

private:
	Server();
	Server(const Server& copy);
	Server& operator=(const Server&);

	ev::io io;
	ev::sig sig;

	int active;
	int portno;
	int sockfd;

	void accept_cb(ev::io &watcher, int revents);
	static void signal_cb(ev::sig &signal, int revents);

	clients_t clients;
	boost::detail::spinlock clients_lock;
};

#endif // SERVER_HPP
