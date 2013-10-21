#ifndef IROOM_HPP
#define IROOM_HPP

#include "ClientSession_fwd.hpp"
#include <boost/enable_shared_from_this.hpp>


class IRoom : public boost::enable_shared_from_this<IRoom>
{
public:
	IRoom();
	virtual ~IRoom();

	virtual void join_player(const PlayerSession_sptr player) = 0;
	virtual void join_world(const WorldSession_sptr world) = 0;

	virtual void release_player(const PlayerSession_sptr player) = 0;
	virtual void release_world(const WorldSession_sptr world) = 0;

	virtual std::size_t players_count() = 0;
	virtual std::size_t worlds_count() = 0;
};

#endif // IROOM_HPP
