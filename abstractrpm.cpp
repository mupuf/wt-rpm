#include "abstractrpm.h"
#include "computerview.h"
#include "view.h"

#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <Wt/Json/Array>
#include <Wt/WDate>
#include <Wt/WTime>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/Utils>

#include "confparser.h"

#define PING_MISSING -1000.0

const AbstractRPM::Computer *AbstractRPM::findComputer(const Wt::WString &computerName)
{
	for (size_t i = 0; i < _computers.size(); i++) {
		if (_computers.at(i).name == computerName)
			return &_computers.at(i);
	}

	return NULL;
}

bool AbstractRPM::parseConfiguration(Wt::Json::Object &conf)
{
	Wt::Json::Array computers = readJSONValue<Wt::Json::Array>(conf, "computers");

	if (computers.size() == 0) {
		std::cerr << "AbstractRPM: No computers found in the configuration file." << std::endl;
		return false;
	}

	for (size_t i = 0; i < computers.size(); i++) {
		parseComputer(computers[i]);
	}

	return true;
}

bool AbstractRPM::parseComputer(Wt::Json::Object &computer)
{
	Computer c;

	c.name = readJSONValue<Wt::WString>(computer, "name");
	c.ipAddress = readJSONValue<Wt::WString>(computer, "ip_address");

	Wt::Json::Object power_led = readJSONValue<Wt::Json::Object>(computer, "power_led_gpio");
	c.powerLed = parseGpio(power_led);

	Wt::Json::Object powerSwitch = readJSONValue<Wt::Json::Object>(computer, "power_switch_gpio");
	c.powerSwitch = parseGpio(powerSwitch);

	Wt::Json::Object atxSwitch = readJSONValue<Wt::Json::Object>(computer, "atx_switch_gpio");
	c.atxSwitch = parseGpio(atxSwitch);

	c.ping = PING_MISSING;

	if (c.name == Wt::WString())
		return false;

	_computers.push_back(c);

	return true;
}

AbstractRPM::Gpio AbstractRPM::parseGpio(Wt::Json::Object &gpio)
{
	Gpio g;

	g.pin = readJSONValue<int>(gpio, "pin", -1);
	g.inverted = readJSONValue<Wt::WString>(gpio, "inverted", "false") == "true";

	return g;
}

std::string AbstractRPM::currentUser() const
{
	if (!Wt::WApplication::instance())
		return "";

	std::string auth = Wt::WApplication::instance()->environment().headerValue("Authorization");
	if (auth.length() < 7) {
		std::cerr << "Auth too short";
		return "";
	}

	if (strncmp(auth.c_str(), "Basic ", 6) != 0) {
		std::cerr << "Auth does not start with 'Basic '";
		return "";
	}

	std::vector<std::string> strs;
	std::string credentials = Wt::Utils::base64Decode(std::string(auth.c_str() + 6));
	boost::split(strs, credentials, boost::is_any_of(":"));

	return strs[0];
}

AbstractRPM::AbstractRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf) : server(server)
{
	parseConfiguration(conf);
}

void AbstractRPM::setPowerLedState(const Wt::WString &computerName, bool state)
{
	computerStateLock.lock();
	bool last_state = this->_powerLedState[computerName];
	this->_powerLedState[computerName] = state;
	computerStateLock.unlock();

	if (last_state == state)
		return;

	viewsLock.lock();
	std::map< std::string, View* >::iterator it;
	for (it = views.begin(); it != views.end(); ++it) {
		server->post((*it).first, boost::bind(&View::powerLedStatusChanged,
						      (*it).second, computerName,
						      state));
	}
	viewsLock.unlock();

	/* add some logs */
	Wt::WString msg = "The power LED went " + Wt::WString(state?"ON":"OFF");
	consoleAddData(computerName, msg);
}

void AbstractRPM::consoleAddData(const Wt::WString &computerName, const Wt::WString &data)
{
	Wt::WString date = Wt::WDate::currentServerDate().toString("yyyy/MM/dd");
	Wt::WString time = Wt::WTime::currentServerTime().toString("hh:mm:ss");
	std::string user = currentUser();
	Wt::WString header = date + "-" + time;
	if (!user.empty())
		header += ": " + user;
	Wt::WString entry = header + ": " + data + "\n";

	computerStateLock.lock();
	Wt::WString logs = entry + _computerLogs[computerName];
	logs = logs.toUTF8().substr(0, 1000);	/* limit to 1k */
	_computerLogs[computerName] = logs;
	computerStateLock.unlock();

	viewsLock.lock();
	std::map< std::string, View* >::iterator it;
	for (it = views.begin(); it != views.end(); ++it) {
		server->post((*it).first, boost::bind(&View::consoleDataAdded,
						      (*it).second, computerName,
						      entry));
	}
	viewsLock.unlock();
}

void AbstractRPM::setPingDelay(const Wt::WString &computerName, double delay)
{
	viewsLock.lock();
	std::map< std::string, View* >::iterator it;
	for (it = views.begin(); it != views.end(); ++it) {
		server->post((*it).first, boost::bind(&View::setPingDelay,
						      (*it).second, computerName,
						      delay));
	}
	viewsLock.unlock();
}

void AbstractRPM::addView(View *view)
{
	bool alreadyExists = false;

	viewsLock.lock();
	if (views.find(view->sessionId()) == views.end())
		views[view->sessionId()] = view;
	else
		alreadyExists = true;
	viewsLock.unlock();

	if (alreadyExists)
		return;

	for (size_t i = 0; i < _computers.size(); i++) {
		Wt::WString computerName = _computers[i].name;

		std::shared_ptr<ComputerView> computer(new ComputerView(view, computerName));

		/* view --> backend */
		computer->sig_atxForceOff.connect(boost::bind(&AbstractRPM::atx_force_off, this, computerName));
		computer->sig_atxForceOn.connect(boost::bind(&AbstractRPM::atx_force_on, this, computerName));
		computer->sig_atxReset.connect(boost::bind(&AbstractRPM::atx_reset, this, computerName));
		computer->sig_pwSwitchPress.connect(boost::bind(&AbstractRPM::pw_switch_press, this, computerName));
		computer->sig_pwSwitchForceOff.connect(boost::bind(&AbstractRPM::pw_switch_force_off, this, computerName));

		/* send the current logs */
		Wt::WString logs = _computerLogs[computerName];
		server->post(view->sessionId(), boost::bind(&ComputerView::consoleDataAdded, computer.get(), logs));

		view->addComputer(computerName, computer);
	}
}

bool AbstractRPM::deleteView(std::string sessionId)
{
	bool ret = false;

	viewsLock.lock();
	std::map< std::string, View* >::iterator it;
	for (it = views.begin(); it != views.end(); ++it) {
		if ((*it).first == sessionId) {
			views.erase(it);
			ret = true;
		}
	}
	viewsLock.unlock();

	return ret;
}

bool AbstractRPM::powerLedState(const Wt::WString &computerName)
{
	return this->_powerLedState[computerName];
}
