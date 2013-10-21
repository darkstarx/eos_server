#ifndef RECEPTIONROOM_HPP
#define RECEPTIONROOM_HPP

#include <IRoom.hpp>
#include "ClientSession_fwd.hpp"
#include <list>


class ReceptionRoom : public IRoom
{
	typedef std::list<PlayerSession_sptr> players_t;
	typedef std::list<WorldSession_sptr> worlds_t;
public:
	ReceptionRoom();
	virtual ~ReceptionRoom();

	virtual void join_player(const PlayerSession_sptr player);
	virtual void join_world(const WorldSession_sptr world);

	virtual void release_player(const PlayerSession_sptr player);
	virtual void release_world(const WorldSession_sptr world);

	virtual std::size_t players_count();
	virtual std::size_t worlds_count();

private:
	players_t m_players;
	worlds_t m_worlds;
	boost::detail::spinlock m_players_lock;
	boost::detail::spinlock m_worlds_lock;
};

#endif // RECEPTIONROOM_HPP
