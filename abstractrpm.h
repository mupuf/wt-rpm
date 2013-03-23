#ifndef ABSTRACTRPM_H
#define ABSTRACTRPM_H

#include <Wt/WObject>
#include <Wt/WString>
#include <boost/signal.hpp>

class View;
class ComputerView;

class AbstractRPM : public Wt::WObject
{
private:
	std::map< Wt::WString, bool > powerLedState;
	std::map< Wt::WString, std::shared_ptr<ComputerView> > _computers;

	View *view;

protected:
	void setPowerLedState(const Wt::WString &computerName, bool state);
	void consoleAddData(const Wt::WString &computerName, const Wt::WString &data);

	bool addComputer(const Wt::WString &computerName);

public:
	AbstractRPM(View *view);

	/* input events */
	virtual void atx_force_off(const Wt::WString &computerName) = 0;
	virtual void atx_force_on(const Wt::WString &computerName) = 0;
	virtual void atx_reset(const Wt::WString &computerName) = 0;
	virtual void pw_switch_press(const Wt::WString &computerName) = 0;
	virtual void pw_switch_force_off(const Wt::WString &computerName) = 0;
};

#endif // ABSTRACTRPM_H
