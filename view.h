#ifndef VIEW_H
#define VIEW_H

#include <Wt/WApplication>
#include <Wt/WVBoxLayout>
#include <Wt/WEnvironment>

#include "abstractrpm.h"
#include "computerview.h"

#include <memory>
#include <vector>
#include <map>

class View : public Wt::WApplication
{
private:
	std::shared_ptr<AbstractRPM> rpm;
	Wt::WBoxLayout *_horizontalLayout;

	std::map< Wt::WString, std::shared_ptr<ComputerView> > _computers;

	std::shared_ptr<ComputerView> getComputer(const Wt::WString &computerName);
public:
	View(const Wt::WEnvironment &env, std::shared_ptr<Wt::WServer> server, std::shared_ptr<AbstractRPM> rpm);

	void addComputer(const Wt::WString &computerName, std::shared_ptr<ComputerView> view);

	/* slots */
	void powerLedStatusChanged(const Wt::WString &computerName, bool status);
	void consoleDataAdded(const Wt::WString &computerName, const Wt::WString &data);
	void setPingDelay(const Wt::WString &computerName, double delay);
};

#endif // VIEW_H
