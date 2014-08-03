#include "abstractrpm.h"
#include "computerview.h"
#include "view.h"

#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <Wt/Json/Array>
#include <Wt/WDate>
#include <Wt/WTime>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/Utils>

#ifdef USE_PING
	#include "oping.h"
#endif
#define PING_MISSING -1000.0

#include "confparser.h"

const AbstractRPM::Computer *AbstractRPM::findComputer(const Wt::WString &computerName) const
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

	Wt::Json::Array read_acl = readJSONValue<Wt::Json::Array>(computer, "read_acl");
	for (size_t i = 0; i < read_acl.size(); i++)
		c.read_ACL.push_back(read_acl[i].toString());

	Wt::Json::Array write_acl = readJSONValue<Wt::Json::Array>(computer, "write_acl");
	for (size_t i = 0; i < write_acl.size(); i++)
		c.write_ACL.push_back(write_acl[i].toString());

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

void AbstractRPM::pollInputs()
{
	while (1)
	{
		{
			boost::lock_guard<boost::mutex> lock(pollingThreadsExitLock);
			if (shouldExit)
				return;
		}

		for (size_t i = 0; i < _computers.size(); i++)
		{
			if (gpioIsValid(_computers[i].powerLed)) {
				bool powerLedOn = readGPIO(_computers[i].powerLed);
				setPowerLedState(_computers[i].name, powerLedOn);
			}
		}

		usleep(1000000);
	}
}

void AbstractRPM::pollPings()
{
#ifdef USE_PING
	pingobj_t *pingContext = ping_construct();

	std::map<Wt::WString, Wt::WString> ipToComputer;

	double timeout = 0.1;
	if (ping_setopt(pingContext, PING_OPT_TIMEOUT, (void *)&timeout))
		std::cerr << "ping_setopt failed: " << ping_get_error(pingContext) << std::endl;

	for (size_t i = 0; i < _computers.size(); i++) {
		if (_computers[i].ipAddress != Wt::WString()) {
			if (ping_host_add(pingContext, _computers[i].ipAddress.toUTF8().c_str()))
				std::cerr << "ping_host_add failed: " << ping_get_error(pingContext) << std::endl;
			ipToComputer[_computers[i].ipAddress] = _computers[i].name;
		}
	}

	while (1)
	{
		{
			boost::lock_guard<boost::mutex> lock(pollingThreadsExitLock);
			if (shouldExit)
				break;
		}

		if (ping_send(pingContext) < 0)
			std::cerr << "ping_send failed: " << ping_get_error(pingContext) << std::endl;

		pingobj_iter_t *iter;
		for (iter = ping_iterator_get(pingContext); iter != NULL; iter = ping_iterator_next(iter)) {
			double ping;
			size_t size = sizeof(double);
			if (ping_iterator_get_info(iter, PING_INFO_LATENCY, (void*)&ping, &size))
				std::cerr << "ping_iterator_get_info/latency failed" << std::endl;

			char hostname[101];
			size_t hostname_len = sizeof(hostname);
			if (ping_iterator_get_info(iter, PING_INFO_USERNAME, (void*)hostname, &hostname_len))
				std::cerr << "ping_iterator_get_info/hostname failed" << std::endl;

			setPingDelay(ipToComputer[hostname], ping);
		}

		sleep(1);
	}

	ping_destroy(pingContext);
#endif
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

bool AbstractRPM::currentUserIsInAccessList(const Wt::WString &computerName,
					    AccesssType type) const
{
	const AbstractRPM::Computer *c = findComputer(computerName);
	if (!c)
		return false;

	std::string cu = currentUser();

	std::vector<Wt::WString> list;
	if (type == READ)
		list = c->read_ACL;
	else if (type == WRITE)
		list = c->write_ACL;
	else
		return false;

	for (size_t i = 0; i < list.size(); i++)
		if (list[i] == cu || list[i] == "all")
			return true;

	return list.size() == 0;
}

void AbstractRPM::startPolling()
{
	gpioPollingThread = boost::thread(&AbstractRPM::pollInputs, this);
	pingPollingThread = boost::thread(&AbstractRPM::pollPings, this);
}

AbstractRPM::AbstractRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf) :
		server(server), shouldExit(false)
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

		if (!currentUserIsInAccessList(computerName, READ))
			continue;

		bool writeAccess = currentUserIsInAccessList(computerName, WRITE);

		std::shared_ptr<ComputerView> computer(new ComputerView(view, computerName, writeAccess));

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

void AbstractRPM::atx_force_off(const Wt::WString &computerName)
{
	const Computer *computer = findComputer(computerName);
	if (!computer) {
		consoleAddData(computerName, "Unknown computer '" + computerName + "'");
		return;
	}

	if (!currentUserIsInAccessList(computerName, WRITE))
		return;

	writeGPIO(computer->atxSwitch, 1);

	consoleAddData(computerName, "ATX force off");
}

void AbstractRPM::atx_force_on(const Wt::WString &computerName)
{
	const Computer *computer = findComputer(computerName);
	if (!computer) {
		consoleAddData(computerName, "Unknown computer '" + computerName + "'");
		return;
	}

	if (!currentUserIsInAccessList(computerName, WRITE))
		return;

	writeGPIO(computer->atxSwitch, 0);

	consoleAddData(computerName, "ATX force on");
}

void AbstractRPM::atx_reset(const Wt::WString &computerName)
{
	const Computer *computer = findComputer(computerName);
	if (!computer) {
		consoleAddData(computerName, "Unknown computer '" + computerName + "'");
		return;
	}

	if (!currentUserIsInAccessList(computerName, WRITE))
		return;

	writeGPIO(computer->atxSwitch, 1);
	sleep(3);
	writeGPIO(computer->atxSwitch, 0);

	consoleAddData(computerName, "ATX reset");
}

void AbstractRPM::pw_switch_press(const Wt::WString &computerName)
{
	const Computer *computer = findComputer(computerName);
	if (!computer) {
		consoleAddData(computerName, "Unknown computer '" + computerName + "'");
		return;
	}

	if (!currentUserIsInAccessList(computerName, WRITE))
		return;

	writeGPIO(computer->powerSwitch, 1);
	usleep(250000);
	writeGPIO(computer->powerSwitch, 0);

	consoleAddData(computerName, "Power switch pressed");
}

void AbstractRPM::pw_switch_force_off(const Wt::WString &computerName)
{
	const Computer *computer = findComputer(computerName);
	if (!computer) {
		consoleAddData(computerName, "Unknown computer '" + computerName + "'");
		return;
	}

	if (!currentUserIsInAccessList(computerName, WRITE))
		return;

	writeGPIO(computer->powerSwitch, 1);
	sleep(10);
	writeGPIO(computer->powerSwitch, 0);

	consoleAddData(computerName, "Power switch forced off");
}

bool AbstractRPM::powerLedState(const Wt::WString &computerName)
{
	return this->_powerLedState[computerName];
}

bool AbstractRPM::gpioIsValid(const struct Gpio &/*gpio*/)
{
	return false;
}

bool AbstractRPM::readGPIO(const struct Gpio &/*gpio*/)
{
	return false;
}

void AbstractRPM::writeGPIO(const struct Gpio &/*gpio*/, int /*value*/)
{

}
