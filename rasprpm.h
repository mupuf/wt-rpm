#ifndef RASPRPM_H
#define RASPRPM_H

#include "abstractrpm.h"

#include <vector>

class RaspRPM : public AbstractRPM
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

		Gpio powerLed;
		Gpio powerSwitch;
		Gpio atxSwitch;
	};

	std::vector<Computer> _computers;

	static void pollInputs(int signal);

	bool parseConfiguration(Wt::Json::Object &conf);
	bool parseComputer(Wt::Json::Object &computer);
	Gpio parseGpio(Wt::Json::Object &gpio);

public:
	RaspRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf);

	void atx_force_off(const Wt::WString &computerName);
	void atx_force_on(const Wt::WString &computerName);
	void atx_reset(const Wt::WString &computerName);
	void pw_switch_press(const Wt::WString &computerName);
	void pw_switch_force_off(const Wt::WString &computerName);
};

#endif // RASPRPM_H
