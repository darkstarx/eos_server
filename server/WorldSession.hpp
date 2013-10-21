#ifndef WORLDSESSION_HPP
#define WORLDSESSION_HPP

#include <ClientSession.hpp>


class WorldSession : public ClientSession
{
public:
	WorldSession(const int id, const int sock, const std::string &ip, const free_client_f free_callback);
	virtual ~WorldSession();
private:
	std::string name;	/** Name of world. Unique for one proxy. */
};

#endif // WORLDSESSION_HPP
