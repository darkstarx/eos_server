#ifndef WORLDSESSION_HPP
#define WORLDSESSION_HPP

#include <ClientSession.hpp>


class WorldSession : public ClientSession, public boost::enable_shared_from_this<WorldSession>
{
	typedef boost::shared_ptr<WorldSession> WorldSession_sptr;
public:
	WorldSession(const int sock, const std::string &ip);
	virtual ~WorldSession();

	virtual void enter_room(IRoom_sptr room);
	virtual void leave_room();
};

#endif // WORLDSESSION_HPP
