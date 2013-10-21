#include "ReceptionRoom.hpp"
#include <glog/logging.h>
#include <PlayerSession.hpp>
#include <WorldSession.hpp>


ReceptionRoom::ReceptionRoom()
: m_players()
, m_worlds()
{
}

ReceptionRoom::~ReceptionRoom()
{
	{
		boost::lock_guard<boost::detail::spinlock> players_guard(m_players_lock);
		m_players.clear();
	}
	{
		boost::lock_guard<boost::detail::spinlock> worlds_guard(m_worlds_lock);
		m_worlds.clear();
	}
}

void ReceptionRoom::join_player(const PlayerSession_sptr player)
{
	boost::lock_guard<boost::detail::spinlock> players_guard(m_players_lock);
	m_players.push_back(player);
	LOG(INFO) << "Player " << player->name() << " has been joined by reception.";
}

void ReceptionRoom::join_world(const WorldSession_sptr world)
{
	boost::lock_guard<boost::detail::spinlock> worlds_guard(m_worlds_lock);
	m_worlds.push_back(world);
	LOG(INFO) << "World " << world->name() << " has been joined by reception.";
}

void ReceptionRoom::release_player(const PlayerSession_sptr player)
{
	boost::lock_guard<boost::detail::spinlock> players_guard(m_players_lock);
	LOG(INFO) << "Player " << player->name() << " is leaving reception.";

	m_players.remove(player);
	LOG(INFO) << "Players now: " << m_players.size();
}

void ReceptionRoom::release_world(const WorldSession_sptr world)
{
	boost::lock_guard<boost::detail::spinlock> worlds_guard(m_worlds_lock);
	LOG(INFO) << "World " << world->name() << " is leaving reception.";

	m_worlds.remove(world);
	LOG(INFO) << "Worlds now: " << m_worlds.size();
}

std::size_t ReceptionRoom::players_count()
{
	return m_players.size();
}

std::size_t ReceptionRoom::worlds_count()
{
	return m_worlds.size();
}
