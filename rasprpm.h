#ifndef RASPRPM_H
#define RASPRPM_H

#include "abstractrpm.h"

#include <Wt/WTimer>

class RaspRPM : public AbstractRPM
{
	static void pollInputs(int signal);
public:
	RaspRPM(std::shared_ptr<Wt::WServer> server);

	void atx_force_off(const Wt::WString &computerName);
	void atx_force_on(const Wt::WString &computerName);
	void atx_reset(const Wt::WString &computerName);
	void pw_switch_press(const Wt::WString &computerName);
	void pw_switch_force_off(const Wt::WString &computerName);
};

#endif // RASPRPM_H
