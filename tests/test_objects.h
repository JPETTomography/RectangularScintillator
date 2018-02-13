// this file is distributed under 
// MIT license
#ifndef bIRWVXRF
#define bIRWVXRF
#include <gtest/gtest.h>
#include <math_h/error.h>
#include <RectScin/geometry.h>
#include <RectScin/scintillator.h>
#include <RectScin/sensitive.h>
#include <RectScin/lightguide.h>
#include <RectScin/photon2signal.h>
#include <RectScin/signal_processing.h>
#include <RectScin/signal_statistics.h>
using namespace std;
using namespace MathTemplates;
using namespace RectangularScintillator;
#define EXPECT_CLOSE_VALUES(A,B) EXPECT_TRUE(((A)>=((B)-(0.000001)))&&((A)<=((B)+(0.000001))));
class SignalSender{
public:
	SignalSender();
	~SignalSender();
	SignalSender& operator>>(shared_ptr<SignalAcceptor>);
	SignalSender& Connect2MultiInput(shared_ptr<AbstractMultiInput>input,size_t count);
	SignalSender& send(Vec&&signals);
private:
	vector<shared_ptr<SignalAcceptor>> m_slots;
};
class Out:public SignalAcceptor{
public:
	Out();
	virtual ~Out();
	virtual void AcceptEventStart()final;
	virtual void AcceptSignalValue(const double&s)override;
	virtual void AcceptEventEnd()final;
	double value();
private:
	double signal;
	bool event;
};
#endif 
