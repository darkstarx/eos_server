#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <Interactor.hpp>
#include <ClientSession_fwd.hpp>


class Player : public Interactor
{
public:
    Player(ClientSession_sptr session, const std::string name);
    virtual ~Player();

private:
	ClientSession_sptr m_session;	/** Client session */

};

#endif // PLAYER_HPP
