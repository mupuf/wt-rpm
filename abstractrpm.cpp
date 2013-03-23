#include "abstractrpm.h"
#include "computerview.h"
#include "view.h"

#include <boost/bind.hpp>

AbstractRPM::AbstractRPM(View *view) : view(view)
{

}

void AbstractRPM::setPowerLedState(const Wt::WString &computerName, bool state)
{
	if (this->powerLedState[computerName] != state) {
		_computers[computerName]->powerLedStatusChanged(state);
	}
	this->powerLedState[computerName] = state;
}

void AbstractRPM::consoleAddData(const Wt::WString &computerName, const Wt::WString &data)
{
	_computers[computerName]->consoleDataAdded(data);
}

bool AbstractRPM::addComputer(const Wt::WString &computerName)
{
	if (_computers.find(computerName) != _computers.end())
		return false;

	std::shared_ptr<ComputerView> computer(new ComputerView(computerName));

	/* view --> backend */
	computer->sig_atxForceOff.connect(boost::bind(&AbstractRPM::atx_force_off, this, computerName));
	computer->sig_atxForceOn.connect(boost::bind(&AbstractRPM::atx_force_on, this, computerName));
	computer->sig_atxReset.connect(boost::bind(&AbstractRPM::atx_reset, this, computerName));
	computer->sig_pwSwitchPress.connect(boost::bind(&AbstractRPM::pw_switch_press, this, computerName));
	computer->sig_pwSwitchForceOff.connect(boost::bind(&AbstractRPM::pw_switch_force_off, this, computerName));

	_computers[computerName] = computer;

	view->addComputer(computer.get());

	return true;
}
