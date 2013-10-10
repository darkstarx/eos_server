#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <malloc.h>

#include <glog/logging.h>
#include "ClientSession.hpp"


ClientSession::ClientSession(const int id, const int sock, const std::string& ip, const free_client_f free_callback)
: _id(id)
, sockfd(sock)
, client_ip(ip)
, recv_buffer(NULL)
, buffer_size(0)
, data_size(0)
, messages()
, free_client(free_callback)
, queue_processor(boost::bind(&ClientSession::process_queue, this))
{
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

	LOG(INFO) << "Got connection from " << client_ip;

	io.set<ClientSession, &ClientSession::callback>(this);
	io.start(sockfd, ev::READ);

	LOG(INFO) << "Client session created.";
}

ClientSession::~ClientSession()
{
	free(recv_buffer);
	LOG(INFO) << "Client " << client_ip << " disconnected.";
}

inline int ClientSession::id()
{
	return _id;
}

void ClientSession::callback(ev::io &watcher, int revents)
{
	if (EV_ERROR & revents) {
		perror("got invalid event");
		return;
	}

	if (revents & EV_READ)
		read_cb(watcher);

	// Do not refer to self members: read_cb could call free_client so self could be destroyed here!
}

void ClientSession::read_cb(ev::io &watcher)
{
	char buffer[10];
	bzero(buffer, sizeof(buffer));
	ssize_t nread = read(watcher.fd, buffer, sizeof(buffer));

	if (nread < 0) {
		perror("read error");
		return;
	}

	if (nread == 0) {
		// Client disconnected
		io.stop();
		close(sockfd);
		sockfd = 0;
		messages.release_consumers();
		queue_processor.join();
		// Remove itself from client list of server for destroing
		free_client(_id);
	} else {
		std::string str(buffer, sizeof(buffer));
		LOG(INFO) << "Receive from " << client_ip << ": " << str << ". Processing...";
		// Expand inner buffer if remaind free memory less than size of received data
		if (buffer_size == 0) {
			buffer_size = 10;
			recv_buffer = malloc(buffer_size);
			LOG(INFO) << "Inner buffer expanded to " << buffer_size;
		} else if (buffer_size - data_size < nread) {
			buffer_size += 10;
			void * new_buffer = malloc(buffer_size);
			memcpy(new_buffer, recv_buffer, data_size);
			free(recv_buffer);
			recv_buffer = new_buffer;
			LOG(INFO) << "Inner buffer expanded to " << buffer_size;
		}
		// Copying data at the end of buffer
		char * dest = static_cast<char *>(recv_buffer) + data_size;
		memcpy(dest, buffer, nread);
		data_size += nread;
		// Done
		LOG(INFO) << "Done (read " << nread << ", buffer size " << buffer_size << ", remain free " << buffer_size - data_size << ")";
		process_buffer();
		if (sockfd) io.set(ev::READ);
	}
}

void ClientSession::process_buffer()
{
	char * bbeg = static_cast<char *>(recv_buffer);	// Start position
	char * bcur = bbeg;								// Current position
	char * begin_of_packet = bbeg;					// Position of extracting packet
	const char * bend = bcur + data_size - 1;		// Last symbol position of received data
	
	for (; bcur <= bend; ++bcur) {
		if (*bcur != '\0') continue;
		// The end of packet has been reached
		if (begin_of_packet && bcur - begin_of_packet > 0) {
			std::string pack(begin_of_packet, bcur - begin_of_packet);
			messages.push(pack);
		}
		begin_of_packet = (bcur < bend && *(bcur + 1) != '\0') ? bcur + 1 : NULL;
	}
	// Removing incomplete part of packet to the beginning of buffer
	if (begin_of_packet > bbeg && bcur - begin_of_packet > 0) {
		data_size = bcur - begin_of_packet;
		memcpy(recv_buffer, begin_of_packet, data_size);
	} else if (!begin_of_packet) data_size = 0;
}

void ClientSession::process_queue()
{
	LOG(INFO) << "Client thread: started.";
	while (sockfd) {
		std::string msg = messages.pop();
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
	if (sockfd == 0) return;

	ssize_t nwritten = write(sockfd, msg.c_str(), msg.length());
	if (nwritten < 0) {
		perror("send error");
		return;
	}
}