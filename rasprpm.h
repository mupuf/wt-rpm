#ifndef RASPRPM_H
#define RASPRPM_H

#include "abstractrpm.h"

#include <Wt/WTimer>

class RaspRPM : public AbstractRPM
{
	Wt::WTimer pollingTimer;

	void pollInputs();
public:
	RaspRPM(View *view);

	void atx_force_off(const Wt::WString &computerName);
	void atx_force_on(const Wt::WString &computerName);
	void atx_reset(const Wt::WString &computerName);
	void pw_switch_press(const Wt::WString &computerName);
	void pw_switch_force_off(const Wt::WString &computerName);
};

#endif // RASPRPM_H
