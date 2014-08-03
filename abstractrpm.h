#ifndef ABSTRACTRPM_H
#define ABSTRACTRPM_H

#include <Wt/WObject>
#include <Wt/WString>
#include <Wt/WServer>
#include <Wt/Json/Object>

#include <boost/thread/mutex.hpp>

class ComputerView;
class View;

class AbstractRPM : public Wt::WObject
{
protected:
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

private:
	boost::mutex computerStateLock;
	std::map< Wt::WString, bool > _powerLedState;
	std::map< Wt::WString, Wt::WString > _computerLogs;
	std::shared_ptr<Wt::WServer> server;

	boost::mutex viewsLock;
	std::map< std::string, View* > views;

	bool parseConfiguration(Wt::Json::Object &conf);
	bool parseComputer(Wt::Json::Object &computer);
	Gpio parseGpio(Wt::Json::Object &gpio);

	std::string currentUser() const;

protected:
	const Computer *findComputer(const Wt::WString &computerName);

	void setPowerLedState(const Wt::WString &computerName, bool state);
	void consoleAddData(const Wt::WString &computerName, const Wt::WString &data);
	void setPingDelay(const Wt::WString &computerName, double delay);

public:
	AbstractRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf);

	void addView(View* view);
	bool deleteView(std::string sessionId);

	bool powerLedState(const Wt::WString &computerName);

	/* input events */
	virtual void atx_force_off(const Wt::WString &computerName) = 0;
	virtual void atx_force_on(const Wt::WString &computerName) = 0;
	virtual void atx_reset(const Wt::WString &computerName) = 0;
	virtual void pw_switch_press(const Wt::WString &computerName) = 0;
	virtual void pw_switch_force_off(const Wt::WString &computerName) = 0;
};

#endif // ABSTRACTRPM_H
