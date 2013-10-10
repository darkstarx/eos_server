#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <ev++.h>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <wqueue.hpp>

class ClientSession
{
	typedef boost::function<bool(int)> free_client_f;
public:
	ClientSession(const int id, const int sock, const std::string &ip, const free_client_f free_callback);
	virtual ~ClientSession();

	int id();

private:
	int _id;

	ev::io io;
	int sockfd;
	std::string client_ip;		/** ip-addres of client */

	void * recv_buffer;			/** buffer for received data */
	std::size_t buffer_size;	/** size of buffer of received data */
	std::size_t data_size;		/** size of received data in buffer */

	wqueue<std::string> messages;	/** threadsafe queue of received messages */

	void callback(ev::io &watcher, int revents);
	void read_cb(ev::io &watcher);

	free_client_f free_client;
	boost::thread queue_processor;

	/** @brief Processing received data
	 * Extracts messages from buffer and pushes them into the message queue */
	void process_buffer();

	void process_queue();
	void process_packet(const std::string & packet);

	void send_string(const std::string & msg);
};

#endif // CLIENTSESSION_H
