#include <boost/shared_ptr.hpp>

class ClientSession;
typedef boost::shared_ptr<ClientSession> ClientSession_sptr;

class PlayerSession;
typedef boost::shared_ptr<PlayerSession> PlayerSession_sptr;

class WorldSession;
typedef boost::shared_ptr<WorldSession> WorldSession_sptr;
