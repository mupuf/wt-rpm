#ifndef RASPRPM_H
#define RASPRPM_H

#ifdef USE_RASPRPM

#include "abstractrpm.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

class RaspRPM : public AbstractRPM
{
	boost::thread gpioPollingThread;
	boost::thread pingPollingThread;
	boost::mutex pollingThreadsExitLock;
	bool shouldExit;

	bool gpioIsValid(const struct Gpio &gpio);

	bool readGPIO(const struct Gpio &gpio);
	void writeGPIO(const struct Gpio &gpio, int value);
	void setupGPIO(const char *tag, int mode, const struct Gpio &gpio);
	void prepareGPIOs();

	void pollInputs();
	void pollPings();

public:
	RaspRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf);
	~RaspRPM();

	void atx_force_off(const Wt::WString &computerName);
	void atx_force_on(const Wt::WString &computerName);
	void atx_reset(const Wt::WString &computerName);
	void pw_switch_press(const Wt::WString &computerName);
	void pw_switch_force_off(const Wt::WString &computerName);
};

#endif

#endif // RASPRPM_H
