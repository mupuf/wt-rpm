#include "rasprpm.h"

#include <iostream>
#include <cstdlib>

void RaspRPM::pollInputs()
{
	int random = (rand() % 2);
	std::cerr << "Poll: random = " << random << std::endl;
	setPowerLedState("cathaou", random);
}

RaspRPM::RaspRPM(View *view) : AbstractRPM(view)
{
	/* TODO: Parse the configuration file */
	addComputer("reator");
	addComputer("cathaou");

	/* setup the timer to update the LED status */
	pollingTimer.setInterval(250);
	pollingTimer.timeout().connect(this, &RaspRPM::pollInputs);
	pollingTimer.start();
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
