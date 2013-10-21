#include "PlayerSession.hpp"
#include <IRoom.hpp>


PlayerSession::PlayerSession(const int sock, const std::string &ip)
: ClientSession(sock, ip)
{
}

PlayerSession::~PlayerSession()
{
}

void PlayerSession::enter_room(IRoom_sptr room)
{
	if (m_room) m_room->release_player(shared_from_this());
	m_room = room;
	m_room->join_player(shared_from_this());
}

void PlayerSession::leave_room()
{
	if (!m_room) return;
	m_room->release_player(shared_from_this());
	m_room.reset();
}