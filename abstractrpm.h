#ifndef ABSTRACTRPM_H
#define ABSTRACTRPM_H

#include <Wt/WObject>
#include <Wt/WString>
#include <Wt/WServer>
#include <Wt/Json/Object>

#include <boost/thread.hpp>
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

	boost::thread gpioPollingThread;
	boost::thread pingPollingThread;
	boost::mutex pollingThreadsExitLock;
	bool shouldExit;

	bool parseConfiguration(Wt::Json::Object &conf);
	bool parseComputer(Wt::Json::Object &computer);
	Gpio parseGpio(Wt::Json::Object &gpio);

	void setPingDelay(const Wt::WString &computerName, double delay);
	std::string currentUser() const;
	void pollInputs();
	void pollPings();

protected:
	const Computer *findComputer(const Wt::WString &computerName);

	void setPowerLedState(const Wt::WString &computerName, bool state);
	void consoleAddData(const Wt::WString &computerName, const Wt::WString &data);

	void startPolling();

public:
	AbstractRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf);

	void addView(View* view);
	bool deleteView(std::string sessionId);

	bool powerLedState(const Wt::WString &computerName);

	/* input events */
	void atx_force_off(const Wt::WString &computerName);
	void atx_force_on(const Wt::WString &computerName);
	void atx_reset(const Wt::WString &computerName);
	void pw_switch_press(const Wt::WString &computerName);
	void pw_switch_force_off(const Wt::WString &computerName);

	/* to be re-implemented by the backend */
	virtual bool gpioIsValid(const struct Gpio &gpio);
	virtual bool readGPIO(const struct Gpio &gpio);
	virtual void writeGPIO(const struct Gpio &gpio, int value);
};

#endif // ABSTRACTRPM_H
