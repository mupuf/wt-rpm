#ifndef DUMMY_RPM_H
#define DUMMY_RPM_H

#ifdef USE_DUMMY_RPM

#include "abstractrpm.h"

class DummyRPM : public AbstractRPM
{
public:
	DummyRPM(std::shared_ptr<Wt::WServer> server, Wt::Json::Object conf);
};

#endif

#endif // DUMMY_RPM_H
