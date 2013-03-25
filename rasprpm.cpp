#include "rasprpm.h"

#include <signal.h>

#include <iostream>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef USE_PING
	#include "oping.h"
#endif

#include <Wt/Json/Array>
#include "confparser.h"

#define PING_MISSING -1000.0
#define PING_TIMEOUT -1.0

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

	c.ping = PING_MISSING;

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

void RaspRPM::pollInputs()
{
	while (1)
	{
		{
			boost::lock_guard<boost::mutex> lock(pollingThreadsExitLock);
			if (shouldExit)
				return;
		}

		int random = (rand() % 2);
		std::cerr << "Poll: random = " << random << std::endl;
		setPowerLedState("cathaou", random);

		usleep(1000000);
	}
}

void RaspRPM::pollPings()
{
#ifdef USE_PING
	pingobj_t *pingContext = ping_construct();

	std::map<Wt::WString, Wt::WString> ipToComputer;

	double timeout = 0.1;
	if (ping_setopt(pingContext, PING_OPT_TIMEOUT, (void *)&timeout))
		std::cerr << "ping_setopt failed: " << ping_get_error(pingContext) << std::endl;

	for (size_t i = 0; i < _computers.size(); i++) {
		if (_computers[i].ipAddress != Wt::WString()) {
			if (ping_host_add(pingContext, _computers[i].ipAddress.toUTF8().c_str()))
				std::cerr << "ping_host_add failed: " << ping_get_error(pingContext) << std::endl;
			ipToComputer[_computers[i].ipAddress] = _computers[i].name;
		}
	}

	while (1)
	{
		{
			boost::lock_guard<boost::mutex> lock(pollingThreadsExitLock);
			if (shouldExit)
				break;
		}

		if (ping_send(pingContext) < 0)
			std::cerr << "ping_send failed: " << ping_get_error(pingContext) << std::endl;

		pingobj_iter_t *iter;
		for (iter = ping_iterator_get(pingContext); iter != NULL; iter = ping_iterator_next(iter)) {
			double ping;
			size_t size = sizeof(double);
			if (ping_iterator_get_info(iter, PING_INFO_LATENCY, (void*)&ping, &size))
				std::cerr << "ping_iterator_get_info/latency failed" << std::endl;

			char hostname[101];
			size_t hostname_len = sizeof(hostname);
			if (ping_iterator_get_info(iter, PING_INFO_USERNAME, (void*)hostname, &hostname_len))
				std::cerr << "ping_iterator_get_info/hostname failed" << std::endl;

			setPingDelay(ipToComputer[hostname], ping);
		}

		sleep(1);
	}

	ping_destroy(pingContext);
#endif
}

RaspRPM::RaspRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf) :
	AbstractRPM(server), shouldExit(false)
{
	parseConfiguration(conf);

	for (size_t i = 0; i < _computers.size(); i++)
		addComputer(_computers[i].name);

	gpioPollingThread = boost::thread(&RaspRPM::pollInputs, this);
	pingPollingThread = boost::thread(&RaspRPM::pollPings, this);
}

RaspRPM::~RaspRPM()
{
	{
		boost::lock_guard<boost::mutex> lock(pollingThreadsExitLock);
		shouldExit = true;
	}

	std::cerr << "RaspRPM: Waiting for the GPIO polling thread to finish" << std::endl;
	gpioPollingThread.join();
	std::cerr << "RaspRPM: Waiting for the ping polling thread to finish" << std::endl;
	pingPollingThread.join();
	std::cerr << "RaspRPM: polling threads finished" << std::endl;
}

void RaspRPM::atx_force_off(const Wt::WString &computerName)
{
	consoleAddData(computerName, "ATX force off");
}

void RaspRPM::atx_force_on(const Wt::WString &computerName)
{
	consoleAddData(computerName, "ATX force on");
}

void RaspRPM::atx_reset(const Wt::WString &computerName)
{
	consoleAddData(computerName, "ATX reset");
}

void RaspRPM::pw_switch_press(const Wt::WString &computerName)
{
	consoleAddData(computerName, "Power switch pressed");
}

void RaspRPM::pw_switch_force_off(const Wt::WString &computerName)
{
	consoleAddData(computerName, "Power switch forced off");
}
