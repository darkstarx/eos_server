#ifndef PLAYERSESSION_H
#define PLAYERSESSION_H

#include <ClientSession.hpp>


class PlayerSession : public ClientSession
{

public:
	PlayerSession(const int id, const int sock, const std::string &ip, const free_client_f free_callback);
	virtual ~PlayerSession();
};

#endif // PLAYERSESSION_H
