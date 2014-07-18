#ifndef DUMMY_RPM_H
#define DUMMY_RPM_H

#ifdef USE_DUMMY_RPM

#include "abstractrpm.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

class DummyRPM : public AbstractRPM
{
	struct Gpio
	{
		int pin;
		bool inverted;
	};

	struct Computer
	{
		Wt::WString name;
		Wt::WString ipAddress;
		double ping;

		Gpio powerLed;
		Gpio powerSwitch;
		Gpio atxSwitch;
	};

	std::vector<Computer> _computers;
	const Computer *findComputer(const Wt::WString &computerName);

	bool parseConfiguration(Wt::Json::Object &conf);
	bool parseComputer(Wt::Json::Object &computer);
	Gpio parseGpio(Wt::Json::Object &gpio);

	boost::thread gpioPollingThread;
	boost::thread pingPollingThread;
	boost::mutex pollingThreadsExitLock;
	bool shouldExit;

	void pollInputs();
	void pollPings();

public:
	DummyRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf);
	~DummyRPM();

	void atx_force_off(const Wt::WString &computerName);
	void atx_force_on(const Wt::WString &computerName);
	void atx_reset(const Wt::WString &computerName);
	void pw_switch_press(const Wt::WString &computerName);
	void pw_switch_force_off(const Wt::WString &computerName);
};

#endif

#endif // DUMMY_RPM_H
