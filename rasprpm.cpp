#include "rasprpm.h"

#include <signal.h>

#include <iostream>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Wt/Json/Array>
#include "confparser.h"

RaspRPM *thiss;

void RaspRPM::pollInputs(int signal)
{
	int random = (rand() % 2);
	std::cerr << "Poll: random = " << random << std::endl;
	thiss->setPowerLedState("cathaou", random);
	alarm(1);
}

bool RaspRPM::parseConfiguration(Wt::Json::Object &conf)
{
	Wt::Json::Array computers = readJSONValue<Wt::Json::Array>(conf, "computers");

	if (computers.size() == 0) {
		std::cerr << "RaspRPM: No computers found in the configuration file." << std::endl;
		return false;
	}

	for (size_t i = 0; i < computers.size(); i++) {
		parseComputer(computers[i]);
	}

	return true;
}

bool RaspRPM::parseComputer(Wt::Json::Object &computer)
{
	Computer c;

	c.name = readJSONValue<Wt::WString>(computer, "name");
	c.ipAddress = readJSONValue<Wt::WString>(computer, "ip_address");

	Wt::Json::Object power_led = readJSONValue<Wt::Json::Object>(computer, "power_led_gpio");
	c.powerLed = parseGpio(power_led);

	Wt::Json::Object powerSwitch = readJSONValue<Wt::Json::Object>(computer, "power_switch_gpio");
	c.powerSwitch = parseGpio(powerSwitch);

	Wt::Json::Object atxSwitch = readJSONValue<Wt::Json::Object>(computer, "atx_switch_gpio");
	c.atxSwitch = parseGpio(atxSwitch);

	if (c.name == Wt::WString())
		return false;

	_computers.push_back(c);

	return true;
}

RaspRPM::Gpio RaspRPM::parseGpio(Wt::Json::Object &gpio)
{
	Gpio g;

	g.pin = readJSONValue<int>(gpio, "pin", -1);
	g.inverted = readJSONValue<Wt::WString>(gpio, "inverted", "false") == "true";

	if (g.pin == -1)
		return RaspRPM::Gpio();
	else
		return g;
}

RaspRPM::RaspRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf) : AbstractRPM(server)
{
	parseConfiguration(conf);

	for (int i = 0; i < _computers.size(); i++)
		addComputer(_computers[i].name);

	/* to be replaced by something less hackish */
	thiss = this;
	::signal(SIGALRM, RaspRPM::pollInputs);
	alarm(1);
}

void RaspRPM::atx_force_off(const Wt::WString &computerName)
{
	consoleAddData(computerName, "ATX force off");
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
