#include "abstractrpm.h"
#include "computerview.h"
#include "view.h"

#include <boost/bind.hpp>

#include <Wt/WDate>
#include <Wt/WTime>

AbstractRPM::AbstractRPM(std::shared_ptr<Wt::WServer> server) : server(server)
{

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
	Wt::WString entry = date + "-" + time + ": " + data + "\n";

	computerStateLock.lock();
	_computerLogs[computerName] = entry + _computerLogs[computerName];
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

bool AbstractRPM::addComputer(const Wt::WString &computerName)
{
	bool ret = true;

	computerStateLock.lock();
	if (_computers.find(computerName) != _computers.end())
		ret = false;
	else
		_computers.insert(computerName);
	computerStateLock.unlock();

	return ret;
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

	computerStateLock.lock();
	std::set<Wt::WString>::iterator it;
	for (it = _computers.begin(); it != _computers.end(); ++it) {
		Wt::WString computerName = *it;

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
	computerStateLock.unlock();
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
