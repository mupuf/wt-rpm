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
#include <Wt/WTextArea>

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

	return new Wt::WFileResource(mime, path);
}

#include <signal.h>
ComputerView::ComputerView(Wt::WApplication *app, const Wt::WString &computerName, Wt::WContainerWidget *parent) :
	Wt::WContainerWidget(parent),
	app(app), _computerName(computerName), _img_led(NULL)
{
	signal(SIGSEGV, segv_handler);

	Wt::WBoxLayout *layout = new Wt::WBoxLayout(Wt::WBoxLayout::TopToBottom, this);

	Wt::WText *_title = new Wt::WText(computerName);
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

	grid->addWidget(new Wt::WImage(getImg("ping.png")), 1, 0, Wt::AlignCenter);
	grid->addWidget(new Wt::WText("Ping"), 1, 1);
	_ping_txt = new Wt::WText("N/A");
	grid->addWidget(_ping_txt, 1, 2, 0, 0, Wt::AlignCenter);

	grid->addWidget(new Wt::WImage(getImg("atx_power.png")), 2, 0, Wt::AlignCenter);
	grid->addWidget(new Wt::WText("ATX power"), 2, 1);
	grid->addWidget(_btn_atx_force_off, 2, 2);
	grid->addWidget(_btn_atx_force_on, 2, 3);
	grid->addWidget(_btn_atx_reset, 2, 4);

	grid->addWidget(new Wt::WImage(getImg("power-button.png")), 3, 0, Wt::AlignCenter);
	grid->addWidget(new Wt::WText("Power switch"), 3, 1);
	grid->addWidget(_btn_pw_switch_press, 3, 2);
	grid->addWidget(_btn_pw_switch_force_off, 3, 3);

	grid->setColumnStretch(5, 1);

	layout->addLayout(grid);

	layout->addSpacing(10);

	Wt::WLabel *label = new Wt::WLabel("Logs:");
	layout->addWidget(label);

	_logs_edit = new Wt::WTextArea("");
	_logs_edit->setHeight(150);
	_logs_edit->setMaximumSize(Wt::WLength::Auto, 150);
	label->setBuddy(_logs_edit);
	layout->addWidget(_logs_edit);

	layout->addSpacing(10);
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
	_logs_edit->setValueText(data + _logs_edit->valueText());
	app->triggerUpdate();
}

void ComputerView::setPingDelay(double delay)
{
	Wt::WString text;

	if (delay < 0)
		text = "Timeout";
	else
		text = Wt::WString("{1} ms").arg(floatToString(delay, 2));

	_ping_txt->setText(text);
	app->triggerUpdate();
}
