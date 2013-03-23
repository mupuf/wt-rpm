#include "view.h"

#include <Wt/WLength>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WPanel>
#include <Wt/WTextEdit>
#include <Wt/WLabel>
#include <Wt/WMessageBox>
#include <Wt/WApplication>
#include <Wt/WOverlayLoadingIndicator>

#include "util.h"

View::View(const Wt::WEnvironment& env, std::shared_ptr<Wt::WServer> /*server*/, std::shared_ptr<AbstractRPM> rpm) :
	Wt::WApplication(env), rpm(rpm)
{
	Wt::WString welcomeMsg = "Welcome to the Remote Power Manager (RPM)";

	this->enableUpdates(true);
	this->setLoadingIndicator(new Wt::WOverlayLoadingIndicator());
	this->styleSheet().addRule("body", "background-color: #eeeeee;");
	this->styleSheet().addRule(".computer", "background-color: #e0e0e0; border-style:solid; border-width: 2px; -moz-border-radius: 15px; border-radius: 15px;");

	/* Application root fixup */
	std::cerr << "Application root = '" << Wt::WApplication::appRoot()
		  << "' and doc root = '" << Wt::WApplication::docRoot()
		  << "'" << std::endl;

	/* Set the title */
	setTitle(welcomeMsg);

	Wt::WContainerWidget *w = new Wt::WContainerWidget(root());

	Wt::WBoxLayout *verticalLayout = new Wt::WBoxLayout(Wt::WBoxLayout::TopToBottom);

	Wt::WText *title = new Wt::WText(welcomeMsg);
	title->setStyleClass("WelcomeMsg");
	this->styleSheet().addRule(".WelcomeMsg", "font-size: 36px; text-transform: capitalize;");
	verticalLayout->addWidget(title, 0, Wt::AlignCenter);

	verticalLayout->addSpacing(50);

	_horizontalLayout = new Wt::WBoxLayout(Wt::WBoxLayout::LeftToRight);
	//_horizontalLayout->addStretch(1);
	verticalLayout->addLayout(_horizontalLayout, 0, Wt::AlignCenter);

	verticalLayout->addSpacing(50);

	Wt::WLabel *label = new Wt::WLabel("Copyrights 2013 - mupuf.org - This website has been created by mupuf.org developers and is released under a permissive license.");
	verticalLayout->addWidget(label, 0, Wt::AlignCenter);

	w->setLayout(verticalLayout);
}

View::~View()
{
	rpm->deleteView(this);
}

void View::addComputer(const Wt::WString &computerName, std::shared_ptr<ComputerView> view)
{
	_computers[computerName] = view;

	view->setStyleClass("computer");
	view->setMinimumSize(500, 150);

	_horizontalLayout->addWidget(view.get());
}

std::shared_ptr<ComputerView> View::getComputer(const Wt::WString &computerName)
{
	return _computers[computerName];
}

