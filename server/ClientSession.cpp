#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <malloc.h>

#include <glog/logging.h>
#include "ClientSession.hpp"

std::size_t ClientSession::buffer_growth = 1024;

ClientSession::ClientSession(const int sock, const std::string& ip)
: m_name()
, m_sockfd(sock)
, m_client_ip(ip)
, m_recv_buffer(NULL)
, m_buffer_size(0)
, m_data_size(0)
, m_messages()
, m_queue_processor(boost::bind(&ClientSession::process_queue, this))
, m_room()
{
	fcntl(m_sockfd, F_SETFL, fcntl(m_sockfd, F_GETFL, 0) | O_NONBLOCK);

	LOG(INFO) << "Got connection from " << m_client_ip;

	m_io.set<ClientSession, &ClientSession::callback>(this);
	m_io.start(m_sockfd, ev::READ);

	std::stringstream str_name;
	str_name << "[" << ip << " " << sock << "]";
	m_name = str_name.str();

	LOG(INFO) << "Client session created.";
}

ClientSession::~ClientSession()
{
	free(m_recv_buffer);
	LOG(INFO) << "Client " << m_client_ip << " disconnected.";
}

inline IRoom_sptr ClientSession::room() const
{
	return m_room;
}

inline std::string ClientSession::name() const
{
	return m_name;
}

void ClientSession::callback(ev::io &watcher, int revents)
{
	if (EV_ERROR & revents) {
		perror("got invalid event");
		return;
	}

	if (revents & EV_READ)
		read_cb(watcher);

	// Do not refer to self members: read_cb could call leave_room so self could be destroyed here!
}

void ClientSession::read_cb(ev::io &watcher)
{
	char buffer[buffer_growth];
	bzero(buffer, sizeof(buffer));
	ssize_t nread = read(watcher.fd, buffer, sizeof(buffer));

	if (nread < 0) {
		perror("read error");
		return;
	}

	if (nread == 0) {
		// Client disconnected
		m_io.stop();
		close(m_sockfd);
		m_sockfd = 0;
		m_messages.release_consumers();
		m_queue_processor.join();
		// Remove itself from client list of room for destroing
		leave_room();
	} else {
		std::string str(buffer, sizeof(buffer));
		LOG(INFO) << "Receive from " << m_client_ip << ": " << str << ". Processing...";
		// Expand inner buffer if remaind free memory less than size of received data
		if (m_buffer_size == 0) {
			m_buffer_size = buffer_growth;
			m_recv_buffer = malloc(m_buffer_size);
			LOG(INFO) << "Inner buffer expanded to " << m_buffer_size;
		} else if (m_buffer_size - m_data_size < nread) {
			m_buffer_size += buffer_growth;
			void * new_buffer = malloc(m_buffer_size);
			memcpy(new_buffer, m_recv_buffer, m_data_size);
			free(m_recv_buffer);
			m_recv_buffer = new_buffer;
			LOG(INFO) << "Inner buffer expanded to " << m_buffer_size;
		}
		// Copying data at the end of buffer
		char * dest = static_cast<char *>(m_recv_buffer) + m_data_size;
		memcpy(dest, buffer, nread);
		m_data_size += nread;
		// Done
		LOG(INFO) << "Done (read " << nread << ", buffer size " << m_buffer_size << ", remain free " << m_buffer_size - m_data_size << ")";
		process_buffer();
		if (m_sockfd) m_io.set(ev::READ);
	}
}

void ClientSession::process_buffer()
{
	char * bbeg = static_cast<char *>(m_recv_buffer);	// Start position
	char * bcur = bbeg;									// Current position
	char * begin_of_packet = bbeg;						// Position of extracting packet
	const char * bend = bcur + m_data_size - 1;		// Last symbol position of received data
	
	for (; bcur <= bend; ++bcur) {
		if (*bcur != '\0') continue;
		// The end of packet has been reached
		if (begin_of_packet && bcur - begin_of_packet > 0) {
			std::string pack(begin_of_packet, bcur - begin_of_packet);
			m_messages.push(pack);
		}
		begin_of_packet = (bcur < bend && *(bcur + 1) != '\0') ? bcur + 1 : NULL;
	}
	// Removing incomplete part of packet to the beginning of buffer
	if (begin_of_packet > bbeg && bcur - begin_of_packet > 0) {
		m_data_size = bcur - begin_of_packet;
		memcpy(m_recv_buffer, begin_of_packet, m_data_size);
	} else if (!begin_of_packet) m_data_size = 0;
}

void ClientSession::process_queue()
{
	LOG(INFO) << "Client thread: started.";
	while (m_sockfd) {
		std::string msg = m_messages.pop();
		if (msg.empty()) continue;
		process_packet(msg);
	}
	LOG(INFO) << "Client thread: done.";
}

void ClientSession::process_packet(const std::string & packet)
{
	// Send message back to the client
	send_string(packet);
}

void ClientSession::send_string(const std::string & msg)
{
	if (m_sockfd == 0) return;

	ssize_t nwritten = write(m_sockfd, msg.c_str(), msg.length());
	if (nwritten < 0) {
		perror("send error");
		return;
	}
}