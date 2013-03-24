#ifndef ABSTRACTRPM_H
#define ABSTRACTRPM_H

#include <Wt/WObject>
#include <Wt/WString>
#include <Wt/WServer>
#include <boost/signal.hpp>
#include <boost/thread/mutex.hpp>

class ComputerView;
class View;

class AbstractRPM : public Wt::WObject
{
private:
	boost::mutex computerStateLock;
	std::map< Wt::WString, bool > _powerLedState;
	std::map< Wt::WString, Wt::WString > _computerLogs;

	std::set<Wt::WString> _computers;

	std::shared_ptr<Wt::WServer> server;
	std::vector< View* > views;

protected:
	void setPowerLedState(const Wt::WString &computerName, bool state);
	void consoleAddData(const Wt::WString &computerName, const Wt::WString &data);
	void setPingDelay(const Wt::WString &computerName, double delay);

	bool addComputer(const Wt::WString &computerName);

public:
	AbstractRPM(std::shared_ptr<Wt::WServer> server);

	void addView(View* view);
	bool deleteView(View* view);

	/* input events */
	virtual void atx_force_off(const Wt::WString &computerName) = 0;
	virtual void atx_force_on(const Wt::WString &computerName) = 0;
	virtual void atx_reset(const Wt::WString &computerName) = 0;
	virtual void pw_switch_press(const Wt::WString &computerName) = 0;
	virtual void pw_switch_force_off(const Wt::WString &computerName) = 0;
};

#endif // ABSTRACTRPM_H
