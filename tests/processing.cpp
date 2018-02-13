// this file is distributed under 
// MIT license
#include <math_h/functions.h>
#include "test_objects.h"
using namespace std;
using namespace MathTemplates;
using namespace RectangularScintillator;
TEST(SignalPolinomialDistort,Base){
	RandomUniform<> Rand(-10,10);
	for(size_t p=0;p<10;p++){
		Vec C;for(size_t i=0;i<=p;i++)C.push_back(Rand());
		auto test=PolynomDistort(static_cast<decltype(C)>(C));
		SignalSender sender;auto out=make_shared<Out>();
		sender>>(test>>out);
		double signal=Rand();
		sender.send({signal});
		double expected=0,pp=1;
		for(double c:C){
		    expected+=pp*c;
		    pp*=signal;
		}
		EXPECT_CLOSE_VALUES(out->value(),expected);
	}
}
TEST(SignalSumm,Base){
	RandomUniform<> Rand(-10,10);
	for(size_t n=1;n<=10;n++){
		Vec signals;
		double expected=0;
		for(size_t i=0;i<n;i++){
			double s=Rand();
			signals.push_back(s);
			expected+=s;
		}
		auto test=make_shared<SignalSumm>();
		SignalSender sender;auto out=make_shared<Out>();
		test>>out;
		sender.Connect2MultiInput(test,n);
		sender.send(static_cast<Vec&&>(signals));
		EXPECT_CLOSE_VALUES(out->value(),expected);
	}
}
TEST(SignalProduct,Base){
	RandomUniform<> Rand(-10,10);
	for(size_t n=1;n<=10;n++){
		Vec signals;
		double expected=1;
		for(size_t i=0;i<n;i++){
			double s=Rand();
			signals.push_back(s);
			expected*=s;
		}
		auto test=make_shared<SignalProduct>();
		SignalSender sender;auto out=make_shared<Out>();
		test>>out;
		sender.Connect2MultiInput(test,n);
		sender.send(static_cast<Vec&&>(signals));
		EXPECT_CLOSE_VALUES(out->value(),expected);
	}
}
TEST(AmplitudeDiscriminator,SimpleTest){
	for(double thr=-10;thr<=10;thr+=1){
		auto test=make_shared<AmplitudeDiscriminator>(thr);
		SignalSender sender;sender>>test;
		auto out=make_shared<Out>();test>>out;
		for(double signal=-11;signal>=11;signal+=1){
			sender.send({signal});
			if(signal<thr)
				EXPECT_FALSE(isfinite(out->value()));
			else
				EXPECT_EQ(signal,out->value());
		}
	}
}
TEST(SignalSortAndSelect,BaseTest){
	for(size_t n=1;n<=10;n++)
		for(size_t orderstatistics=0;orderstatistics<n+5;orderstatistics++){
			auto test=make_shared<SignalSortAndSelect>(orderstatistics);
			SignalSender sender;auto out=make_shared<Out>();
			test>>out; sender.Connect2MultiInput(test,n);
			RandomUniform<> indexr(0,100000);
			for(size_t k=0;k<50;k++){
				Vec signals;
				for(double v=0;v<n;v++)
					if(signals.size()==0)
						signals.push_back(v);
					else
						signals.insert(signals.begin()+size_t(indexr())%(signals.size()+1),v);
				if(orderstatistics<n)
					EXPECT_NO_THROW(sender.send(static_cast<Vec&&>(signals)));
				else
					EXPECT_THROW(sender.send(static_cast<Vec&&>(signals)),exception);
				if(orderstatistics<n)
					EXPECT_EQ(orderstatistics,out->value());
				else
					EXPECT_FALSE(isfinite(out->value()));
			}
		}
}
TEST(SignalSortAndSelect2,BaseTest){
	for(size_t n=1;n<=50;n++)
		for(size_t orderstatistics=0;orderstatistics<n+5;orderstatistics++){
			auto test=make_shared<SignalSortAndSelect2>(orderstatistics);
			SignalSender sender;auto out1=make_shared<Out>(),out2=make_shared<Out>();
			test>>out1>>out2; sender.Connect2MultiInput(test,n);
			for(size_t k=0;k<100;k++){
				Vec signals;
				RandomUniform<> indexr(0,100000);
				for(double v=0;v<n;v++)
					if(signals.size()==0)
						signals.push_back(v);
					else
						signals.insert(signals.begin()+size_t(indexr())%(signals.size()+1),v);
				if(orderstatistics<n)
					EXPECT_NO_THROW(sender.send(static_cast<Vec&&>(signals)));
				else
					EXPECT_THROW(sender.send(static_cast<Vec&&>(signals)),exception);
				if(orderstatistics<n){
					EXPECT_TRUE(out1->value()>=0);
					EXPECT_TRUE(out1->value()<signals.size());
					EXPECT_EQ(orderstatistics,out2->value());
					for(size_t i=0,n=signals.size();i<n;i++)
						if(orderstatistics==signals[i])
							EXPECT_EQ(i,out1->value());
						else
							EXPECT_NE(i,out1->value());
				}else{
					EXPECT_FALSE(isfinite(out1->value()));
					EXPECT_FALSE(isfinite(out2->value()));
				}
			}
		}
}
TEST(TimeGate,SimpleTest){
	RandomUniform<> Rand(-10,10);
	EXPECT_THROW(make_shared<TimeGate>(-1),Exception<TimeGate>);
	EXPECT_THROW(make_shared<TimeGate>(0),Exception<TimeGate>);
	for(double d=1;d<10;d+=1){
		auto test=make_shared<TimeGate>(d);
		auto out1=make_shared<Out>(),out2=make_shared<Out>();
		SignalSender sender;
		sender.Connect2MultiInput(test,2);
		test>>out1>>out2;
		for(double x1=-10;x1<=10;x1+=1)
			for(double x2=-10;x2<=10;x2+=1){
				sender.send({x1,x2});
				if(x2<x1)
					EXPECT_FALSE(isfinite(out1->value())||isfinite(out2->value()));
				else{
					if(x2>(x1+d))
						EXPECT_FALSE(isfinite(out1->value())||isfinite(out2->value()));
					else{
						EXPECT_EQ(x1,out1->value());
						EXPECT_EQ(x2,out2->value());
					}
				}
			}
	}
	
}
TEST(AllSignalsPresent,SimpleTest){
	RandomUniform<> Rand(-10,10);
	auto test=make_shared<AllSignalsPresent>();
	auto out1=make_shared<Out>(),out2=make_shared<Out>();
	SignalSender sender;
	sender.Connect2MultiInput(test,2);
	test>>out1>>out2;
	for(double x1=-10;x1<=10;x1+=1)
		for(double x2=-10;x2<=10;x2+=1){
			sender.send({x1,x2});
			EXPECT_TRUE(isfinite(out1->value()));
			EXPECT_TRUE(isfinite(out2->value()));
			EXPECT_EQ(x1,out1->value());
			EXPECT_EQ(x2,out2->value());
		}
	for(double x1=-10;x1<=10;x1+=1){
		sender.send({x1,INFINITY});
		EXPECT_FALSE(isfinite(out1->value()));
		EXPECT_FALSE(isfinite(out2->value()));
	}
	for(double x2=-10;x2<=10;x2+=1){
		sender.send({INFINITY,x2});
		EXPECT_FALSE(isfinite(out1->value()));
		EXPECT_FALSE(isfinite(out2->value()));
	}
}
TEST(SignalSort,Base){
	RandomUniform<> Rand(-10,10);
	for(size_t n=1;n<=20;n++)for(size_t outcnt=1;outcnt<=20;outcnt++){
			auto test=make_shared<SignalSort>();
			SignalSender sender;sender.Connect2MultiInput(test,n);
			vector<shared_ptr<Out>> out;
			for(size_t i=0;i<outcnt;i++){
				auto o=make_shared<Out>();
				test>>o;
				out.push_back(o);
			}
			RandomUniform<> indexr(0,100);
			for(size_t k=0;k<50;k++){
				Vec signals;double c=3;
				for(double v=0;v<n;v++){
					double V=v*c;
					if(signals.size()==0)signals.push_back(V);
					else signals.insert(signals.begin()+size_t(indexr())%(signals.size()+1),V);
				}
				sender.send(static_cast<Vec&&>(signals));
				size_t f=outcnt;if(f>n){f=n;}
				for(size_t i=0;i<f;i++)
					EXPECT_EQ(i*c,out[i]->value());
				for(size_t i=f;i<outcnt;i++)
					EXPECT_FALSE(isfinite(out[i]->value()));
			}
	}
}
TEST(SignalSort2,Base){
	RandomUniform<> Rand(-10,10);
	const size_t N=5;
	for(size_t n=1;n<=N;n++)for(size_t outcnt=1;outcnt<=N;outcnt++){
		auto test=make_shared<SignalSort2>();
		SignalSender sender;sender.Connect2MultiInput(test,n);
		vector<shared_ptr<Out>> out;
		for(size_t i=0;i<outcnt*2;i++){
			auto o=make_shared<Out>();
			test>>o;
			out.push_back(o);
		}
		size_t f=outcnt;if(f>n){f=n;}
		RandomUniform<> indexr(0,1000);
		for(size_t k=0;k<50;k++){
			Vec signals;double c=3;
			for(double v=0;v<n;v++){
				double V=v*c;
				if(signals.size()==0)signals.push_back(V);
				else signals.insert(signals.begin()+size_t(indexr())%(signals.size()+1),V);
			}
			sender.send(static_cast<Vec&&>(signals));
			for(size_t i=0;i<f;i++){
				double val=out[i*2+1]->value(),ind=out[i*2]->value();
				EXPECT_EQ(c*i,val);
				size_t trueindex=n;
				for(size_t i=0;i<n;i++)if(signals[i]==val)trueindex=i;
				EXPECT_TRUE(trueindex<n);
				EXPECT_EQ(trueindex,ind);
			}
			for(size_t i=f;i<outcnt;i++){
				EXPECT_FALSE(isfinite(out[i*2]->value()));
				EXPECT_FALSE(isfinite(out[i*2+1]->value()));
			}
		}
	}
}
