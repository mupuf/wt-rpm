#ifdef USE_RASPRPM

#include "rasprpm.h"

#include <iostream>
#include <cstdlib>

#include <wiringPi.h>

void RaspRPM::setupGPIO(const char *tag, int mode, const struct Gpio &gpio)
{
	fprintf(stderr,
		"Configure '%s' GPIO: direction = %s, pin = %i, inverted = %s\n",
		tag, mode == INPUT?"input":"output", gpio.pin, gpio.inverted?"true":"false");

	if (!gpioIsValid(gpio))
		return;

	pinMode(gpio.pin, mode);

	if (mode == OUTPUT)
		writeGPIO(gpio, LOW);
}

void RaspRPM::prepareGPIOs()
{
	if (wiringPiSetup() == -1)
	{
		perror("wiringPiSetup");
		exit(1);
	}

	for (size_t i = 0; i < _computers.size(); i++)
	{
		fprintf(stderr, "Configure computer '%s':\n", _computers.at(i).name.toUTF8().c_str());
		setupGPIO("power LED", INPUT, _computers.at(i).powerLed);
		setupGPIO("power switch", OUTPUT, _computers.at(i).powerSwitch);
		setupGPIO("ATX power", OUTPUT, _computers.at(i).atxSwitch);
		fprintf(stderr, "\n");
	}
}

RaspRPM::RaspRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf) :
	AbstractRPM(server, conf)
{
	prepareGPIOs();

	startPolling();
}


bool RaspRPM::gpioIsValid(const struct Gpio &gpio)
{
	return gpio.pin >= 0 && gpio.pin <= 20;
}

bool RaspRPM::readGPIO(const struct Gpio &gpio)
{
	int value = digitalRead(gpio.pin);

	if (gpio.inverted)
	{
		if (value == HIGH)
			value = LOW;
		else
			value = HIGH;
	}

	return value == HIGH;
}

void RaspRPM::writeGPIO(const struct Gpio &gpio, int value)
{
	if (!gpioIsValid(gpio)) {
		fprintf(stderr, "writeGPIO: Invalid write to GPIO %i\n", gpio.pin);
		return;
	}

	if (gpio.inverted)
	{
		if (value == HIGH)
			value = LOW;
		else
			value = HIGH;
	}

	digitalWrite(gpio.pin, value);
}

#endif
