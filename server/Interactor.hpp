#ifndef INTERACTOR_HPP
#define INTERACTOR_HPP

#include <types.hpp>
#include <string>


class Interactor
{
public:
	Interactor(const std::string & name);
    Interactor(const position_t position, const std::string & name);
    virtual ~Interactor();

	void set_position(const position_t position);
	position_t get_position();

	std::string get_name();

protected:
	virtual void set_name(const std::string & name);

private:
	position_t m_position;	/** Position of character */
	std::string m_name;		/** Name of character */
};

#endif // INTERACTOR_HPP
