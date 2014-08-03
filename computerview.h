#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include <Wt/WContainerWidget>
#include <Wt/WString>
#include <boost/signal.hpp>

class ComputerView : public Wt::WContainerWidget
{
private:
	Wt::WApplication *app;
	Wt::WString _computerName;
	bool _ledStatus;
	bool _powerStatus;
	bool _atxStatus;

	Wt::WPushButton *_btn_atx_force_off;
	Wt::WPushButton *_btn_atx_force_on;
	Wt::WPushButton *_btn_atx_reset;

	Wt::WPushButton *_btn_pw_switch_press;
	Wt::WPushButton *_btn_pw_switch_force_off;

	Wt::WImage *_img_led;
	Wt::WText *_ping_txt;
	Wt::WTextArea *_logs_edit;

	std::shared_ptr<Wt::WFileResource> _ico_led_on_file;
	std::shared_ptr<Wt::WFileResource> _ico_led_off_file;
	std::shared_ptr<Wt::WFileResource> _ico_ping_file;
	std::shared_ptr<Wt::WFileResource> _ico_atx_pwr_file;
	std::shared_ptr<Wt::WFileResource> _ico_pwr_switch_file;

	void btn_atx_force_off_clicked();
	void btn_atx_force_on_clicked();
	void btn_atx_reset_clicked();
	void btn_pw_switch_press_clicked();
	void btn_pw_switch_force_off_clicked();

	Wt::WFileResource *getImg(const Wt::WString &name);
	void setPowerLedStatus(bool status);
public:
	ComputerView(Wt::WApplication *app,
		     const Wt::WString &computerName,
		     bool writeAccess,
		     Wt::WContainerWidget *parent = NULL);

	Wt::WString computerName() const { return _computerName; };

	/* slots */
	void powerLedStatusChanged(bool status);
	void consoleDataAdded(const Wt::WString &data);
	void setPingDelay(double delay);

	/* signals */
	boost::signal<void ()> sig_atxForceOff;
	boost::signal<void ()> sig_atxForceOn;
	boost::signal<void ()> sig_atxReset;
	boost::signal<void ()> sig_pwSwitchPress;
	boost::signal<void ()> sig_pwSwitchForceOff;
};

#endif // COMPUTERVIEW_H
