#ifndef SERVER_HPP
#define SERVER_HPP

// Отключаем совместимость libev4 со старой версией (3)
#define EV_COMPAT3 0

#include <assert.h>
#include <ev++.h>
#include <vector>
#include <list>

#include <ClientSession_fwd.hpp>
#include <Room_fwd.hpp>


class Server
{
public:
	enum { BUFFER_SIZE = 10 };

	static Server& instance()
	{
		static Server singleton;
		return singleton;
	}
	~Server();

	void start(const int c_port, const int w_port);
	void stop();

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

	ReceptionRoom_sptr reception;
};

#endif // SERVER_HPP
