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

	for (size_t i = 0; i < views.size(); i++) {
		View *view = views[i];
		ComputerView *cview = view->getComputer(computerName).get();
		server->post(view->sessionId(), boost::bind(&ComputerView::powerLedStatusChanged, cview, state));
	}

}

void AbstractRPM::consoleAddData(const Wt::WString &computerName, const Wt::WString &data)
{
	Wt::WString date = Wt::WDate::currentServerDate().toString("yyyy/MM/dd");
	Wt::WString time = Wt::WTime::currentServerTime().toString("hh:mm:ss");
	Wt::WString entry = date + " - " + time + " : " + data + "\n";

	computerStateLock.lock();
	_computerLogs[computerName] += entry;
	computerStateLock.unlock();

	for (size_t i = 0; i < views.size(); i++) {
		View *view = views[i];
		ComputerView *cview = view->getComputer(computerName).get();
		server->post(view->sessionId(), boost::bind(&ComputerView::consoleDataAdded, cview, entry));
	}
}

bool AbstractRPM::addComputer(const Wt::WString &computerName)
{
	if (_computers.find(computerName) != _computers.end())
		return false;

	_computers.insert(computerName);

	return true;
}

void AbstractRPM::addView(View *view)
{
	views.push_back(view);

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
		computerStateLock.lock();
		Wt::WString logs = _computerLogs[computerName];
		computerStateLock.unlock();
		server->post(view->sessionId(), boost::bind(&ComputerView::consoleDataAdded, computer.get(), logs));

		view->addComputer(computerName, computer);
	}
}

bool AbstractRPM::deleteView(View* view)
{
	/* todo*/
	std::vector< View* >::iterator it;
	for (it = views.begin(); it != views.end(); ++it) {
		if (*it == view) {
			views.erase(it);
			return true;
		}
	}

	return false;
}
