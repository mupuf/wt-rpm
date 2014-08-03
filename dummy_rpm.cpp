#ifdef USE_DUMMY_RPM

#include "dummy_rpm.h"

DummyRPM::DummyRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf) :
	AbstractRPM(server, conf)
{
	startPolling();
}
#endif
