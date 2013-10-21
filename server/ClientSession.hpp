#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <ev++.h>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <wqueue.hpp>
#include <Room_fwd.hpp>
#include "IRoom.hpp"

class ClientSession
{
public:
	ClientSession(const int sock, const std::string &ip);
	virtual ~ClientSession();

	virtual IRoom_sptr room() const;
	virtual std::string name() const;

	virtual void enter_room(IRoom_sptr room) = 0;
	virtual void leave_room() = 0;

protected:
	IRoom_sptr m_room;

private:
	static std::size_t buffer_growth;	/** size of buffer growth */
	std::string m_name;					/** unique name of client */

	ev::io m_io;
	int m_sockfd;
	std::string m_client_ip;			/** ip-addres of client */

	void * m_recv_buffer;				/** buffer for received data */
	std::size_t m_buffer_size;			/** size of buffer of received data */
	std::size_t m_data_size;			/** size of received data in buffer */

	wqueue<std::string> m_messages;	/** threadsafe queue of received messages */

	boost::thread m_queue_processor;

	void callback(ev::io &watcher, int revents);
	void read_cb(ev::io &watcher);

	/** @brief Processing received data
	 * Extracts messages from buffer and pushes them into the message queue */
	void process_buffer();

	/** @brief Processing the queue of received messages
	 * Extracts message from queue and sends one for processing */
	void process_queue();

	/** @brief Processing the message
	 * Sends the message to dispatcher for processing */
	void process_packet(const std::string & packet);

	/** @brief Sending message to client */
	void send_string(const std::string & msg);
};

#endif // CLIENTSESSION_H
