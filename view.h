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
	Wt::WLineEdit *_editAuthor;
	Wt::WTextEdit *_editMsg;
	Wt::WText *_noComments;
	Wt::WBoxLayout *_horizontalLayout;

	std::string getValueFromEnv(const Wt::WEnvironment& env,
				const std::string &key,
				const std::string &defaultValue = std::string()) const;
public:
	View(const Wt::WEnvironment &env, Wt::WServer &server);

	void addComputer(ComputerView *view);
};

#endif // VIEW_H
