#ifndef RASPRPM_H
#define RASPRPM_H

#ifdef USE_RASPRPM

#include "abstractrpm.h"

class RaspRPM : public AbstractRPM
{
	void setupGPIO(const char *tag, int mode, const struct Gpio &gpio);
	void prepareGPIOs();

public:
	RaspRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf);

	bool gpioIsValid(const struct Gpio &gpio);
	bool readGPIO(const struct Gpio &gpio);
	void writeGPIO(const struct Gpio &gpio, int value);
};

#endif

#endif // RASPRPM_H
