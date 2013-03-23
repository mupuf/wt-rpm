#ifndef VIEW_H
#define VIEW_H

#include <Wt/WApplication>
#include <Wt/WVBoxLayout>
#include <Wt/WEnvironment>

#include "computerview.h"

#include <memory>
#include <vector>

class View : public Wt::WApplication
{
private:
	Wt::WBoxLayout *_horizontalLayout;

public:
	View(const Wt::WEnvironment &env, Wt::WServer &server);

	void addComputer(ComputerView *view);
};

#endif // VIEW_H
