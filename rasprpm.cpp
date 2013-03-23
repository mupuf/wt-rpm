#include "rasprpm.h"

#include <signal.h>

#include <iostream>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

RaspRPM *thiss;

void RaspRPM::pollInputs(int signal)
{
	int random = (rand() % 2);
	std::cerr << "Poll: random = " << random << std::endl;
	thiss->setPowerLedState("cathaou", random);
	alarm(1);
}

RaspRPM::RaspRPM(std::shared_ptr<Wt::WServer> server) : AbstractRPM(server)
{
	/* TODO: Parse the configuration file */
	addComputer("reator");
	addComputer("cathaou");
	thiss = this;

	::signal(SIGALRM, RaspRPM::pollInputs);
	alarm(1);
}

void RaspRPM::atx_force_off(const Wt::WString &computerName)
{
	std::cerr << computerName << " : ATX force off" << std::endl;
}

void RaspRPM::atx_force_on(const Wt::WString &computerName)
{
	std::cerr << computerName << " : ATX force on" << std::endl;
}

void RaspRPM::atx_reset(const Wt::WString &computerName)
{
	std::cerr << computerName << " : ATX reset" << std::endl;
}

void RaspRPM::pw_switch_press(const Wt::WString &computerName)
{
	std::cerr << computerName << " : Power switch clicked" << std::endl;
}

void RaspRPM::pw_switch_force_off(const Wt::WString &computerName)
{
	std::cerr << computerName << " : Power switch force off" << std::endl;
}
