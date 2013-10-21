#include "WorldSession.hpp"
#include <IRoom.hpp>


WorldSession::WorldSession(const int sock, const std::string &ip)
: ClientSession(sock, ip)
{
}

WorldSession::~WorldSession()
{

}

void WorldSession::enter_room(IRoom_sptr room)
{
	if (m_room) m_room->release_world(shared_from_this());
	m_room = room;
	m_room->join_world(shared_from_this());
}

void WorldSession::leave_room()
{
	if (!m_room) return;
	m_room->release_world(shared_from_this());
	m_room.reset();
}
