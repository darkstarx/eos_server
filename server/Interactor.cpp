#include "Interactor.hpp"


Interactor::Interactor(const std::string & name)
: m_name(name)
{
}

Interactor::Interactor(const position_t position, const std::string & name)
: m_position(position)
, m_name(name)
{
}

Interactor::~Interactor()
{
}

void Interactor::set_position(const position_t position)
{
	m_position = position;
}

inline position_t Interactor::get_position()
{
	return m_position;
}

void Interactor::set_name(const std::string& name)
{
	m_name = name;
}

inline std::string Interactor::get_name()
{
	return m_name;
}
