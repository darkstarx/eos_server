#ifndef PLAYERSESSION_H
#define PLAYERSESSION_H

#include <ClientSession.hpp>


class PlayerSession : public ClientSession, public boost::enable_shared_from_this<PlayerSession>
{
	typedef boost::shared_ptr<PlayerSession> PlayerSession_sptr;
public:
	PlayerSession(const int sock, const std::string &ip);
	virtual ~PlayerSession();

	virtual void enter_room(IRoom_sptr room);
	virtual void leave_room();
};

#endif // PLAYERSESSION_H
