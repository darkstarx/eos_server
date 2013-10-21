#include "PlayerSession.hpp"


PlayerSession::PlayerSession(const int id, const int sock, const std::string &ip, const free_client_f free_callback)
: ClientSession(id, sock, ip, free_callback)
{

}

PlayerSession::~PlayerSession()
{

}
