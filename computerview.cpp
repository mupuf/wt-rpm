#include "computerview.h"
#include "abstractrpm.h"

#include <Wt/WApplication>
#include <Wt/WBoxLayout>
#include <Wt/WGridLayout>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WPushButton>
#include <Wt/WFileResource>
#include <Wt/WLabel>

#include "util.h"

void ComputerView::btn_atx_force_off_clicked()
{
	sig_atxForceOff();
}

void ComputerView::btn_atx_force_on_clicked()
{
	sig_atxForceOn();
}

void ComputerView::btn_atx_reset_clicked()
{
	sig_atxReset();
}

void ComputerView::btn_pw_switch_press_clicked()
{
	sig_pwSwitchPress();
}

void ComputerView::btn_pw_switch_force_off_clicked()
{
	sig_pwSwitchForceOff();
}

Wt::WFileResource *ComputerView::getImg(const Wt::WString &name)
{
	std::string path = getExeDirectory() + "/../imgs/" + name.toUTF8();
	std::string mime = "image/png";

	size_t ext_found = name.toUTF8().find_last_of('.');
	if (ext_found != std::string::npos)
		mime = "image/" + name.toUTF8().substr(ext_found + 1);

	std::cerr << "path = " << path << ", " << "extension = '" << mime << '"' << std::endl;

	return new Wt::WFileResource(mime, path);
}

ComputerView::ComputerView(Wt::WApplication *app, const Wt::WString &computerName, Wt::WContainerWidget *parent) :
	Wt::WContainerWidget(parent),
	app(app), _computerName(computerName), _img_led(NULL)
{
	Wt::WBoxLayout *layout = new Wt::WBoxLayout(Wt::WBoxLayout::TopToBottom, this);

	_title = new Wt::WText(computerName);
	_title->setStyleClass("ComputerName");
	app->styleSheet().addRule(".ComputerName", "font-size: 36px; text-transform: capitalize;");
	layout->addWidget(_title, 0, Wt::AlignCenter);

	/* create the buttons and connect them to their slots */
	_btn_atx_force_off = new Wt::WPushButton("Force off");
	_btn_atx_force_on = new Wt::WPushButton("Force on");
	_btn_atx_reset = new Wt::WPushButton("Reset");
	_btn_pw_switch_press = new Wt::WPushButton("Press");
	_btn_pw_switch_force_off = new Wt::WPushButton("Force off");

	_btn_atx_force_off->clicked().connect(this, &ComputerView::btn_atx_force_off_clicked);
	_btn_atx_force_on->clicked().connect(this, &ComputerView::btn_atx_force_on_clicked);
	_btn_atx_reset->clicked().connect(this, &ComputerView::btn_atx_reset_clicked);
	_btn_pw_switch_press->clicked().connect(this, &ComputerView::btn_pw_switch_press_clicked);
	_btn_pw_switch_force_off->clicked().connect(this, &ComputerView::btn_pw_switch_force_off_clicked);

	Wt::WGridLayout *grid = new Wt::WGridLayout();
	powerLedStatusChanged(false);
	grid->addWidget(_img_led, 0, 0, Wt::AlignCenter);
	grid->addWidget(new Wt::WText("Power Led state"), 0, 1);
	grid->addWidget(new Wt::WLabel(""), 0, 5);

	grid->addWidget(new Wt::WImage(getImg("atx_power.png")), 1, 0, Wt::AlignCenter);
	grid->addWidget(new Wt::WText("ATX power"), 1, 1);
	grid->addWidget(_btn_atx_force_off, 1, 2);
	grid->addWidget(_btn_atx_force_on, 1, 3);
	grid->addWidget(_btn_atx_reset, 1, 4);

	grid->addWidget(new Wt::WImage(getImg("power-button.png")), 2, 0, Wt::AlignCenter);
	grid->addWidget(new Wt::WText("Power switch"), 2, 1);
	grid->addWidget(_btn_pw_switch_press, 2, 2);
	grid->addWidget(_btn_pw_switch_force_off, 2, 3);

	grid->setColumnStretch(5, 1);

	layout->addLayout(grid);

	layout->addSpacing(20);
}

void ComputerView::powerLedStatusChanged(bool status)
{
	Wt::WString file = "green_light.png";

	if (!status)
		file = "off_light.png";

	if (_img_led == NULL) {
		_img_led = new Wt::WImage(getImg(file));
		_img_led->setHeight(Wt::WLength(16));
	}
	else {
		_img_led->setImageLink(getImg(file));
		app->triggerUpdate();
	}
}

void ComputerView::consoleDataAdded(const Wt::WString &data)
{
	std::cerr << computerName().toUTF8() << " : " << data.toUTF8() << std::endl;

	Wt::WApplication::instance()->triggerUpdate();
}
