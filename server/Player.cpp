#include "Player.hpp"


Player::Player(ClientSession_sptr session, const std::string name)
: Interactor(name)
, m_session(session)
{

}

Player::~Player()
{

}

