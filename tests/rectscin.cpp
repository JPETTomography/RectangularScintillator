// this file is distributed under 
// MIT license
#include <math_h/functions.h>
#include "test_objects.h"
using namespace std;
using namespace MathTemplates;
using namespace RectangularScintillator;
TEST(Scintillator,reflection){
	auto rsc=MakeScintillator({make_pair(0,1),make_pair(0,1),make_pair(0,1)},1.0+0.1*(rand()%10),TimeDistribution1(0.5,1.5));
	for(const auto&p:rsc->ReflectionProbabilityFunction())
		EXPECT_TRUE((p.Y()>=0)&&(p.Y()<=1));
}
TEST(Scintillator,geometry){
	auto rsc=MakeScintillator({make_pair(0,1),make_pair(0,1),make_pair(0,1)},1.0,TimeDistribution1(0.5,1.5));
	EXPECT_NO_THROW(rsc->RegisterGamma({0.5,0.5,0.5},10));
	EXPECT_THROW(rsc->RegisterGamma({2,2,2},1),Exception<Scintillator>);
}
TEST(Scintillator, Isotropic){
	auto rsc=MakeScintillator({make_pair(0,1),make_pair(0,1),make_pair(0,1)},1.6,TimeDistribution1(0.5,1.5));
	vector<shared_ptr<SignalStatictics>> counts,times;
	for(size_t dimension=0;dimension<3;dimension++)
		for(auto side=RectDimensions::Left;side<=RectDimensions::Right;inc(side)){
			auto ampl=make_shared<SignalStatictics>(),time=make_shared<SignalStatictics>();
			rsc->Surface(dimension,side)>>(
				Photosensor({make_pair(0,1),make_pair(0,1)},1.0,[](double){return 1.0;})
					>>(make_shared<AmplitudeSignal>()>>ampl)
					>>(TimeSignal({make_pair(0,1)})>>time)
			);
			counts.push_back(ampl);
			times.push_back(time);
		}
	for(size_t cnt=0;cnt<300;cnt++)
		rsc->RegisterGamma({0.5,0.5,0.5},3000);
	auto check_close=[](vector<shared_ptr<SignalStatictics>>&vec){
		value<double> for_cmp=vec[0]->data();
		for(shared_ptr<SignalStatictics>one:vec)
			EXPECT_TRUE(one->data().Contains(for_cmp));
	};
	check_close(counts);
	check_close(times);
}
TEST(Scintillator,Glue){
	auto rsc=MakeScintillator({make_pair(-5,5),make_pair(-5,5),make_pair(-5,5)},1.6,TimeDistribution1(0.5,1.5));
	auto ideal=make_shared<SignalStatictics>(),
		middle=make_shared<SignalStatictics>(),
		worst=make_shared<SignalStatictics>();
	rsc->Surface(0,RectDimensions::Left)>>(
		Photosensor({make_pair(-5,5),make_pair(-5,5)},1.0,[](double){return 1.0;})
		>>(make_shared<AmplitudeSignal>()>>ideal)
	);
	rsc->Surface(1,RectDimensions::Left)>>(
		Photosensor({make_pair(-5,5),make_pair(-5,5)},0.5,[](double){return 1.0;})
		>>(make_shared<AmplitudeSignal>()>>middle)
	);
	rsc->Surface(2,RectDimensions::Left)>>(
		Photosensor({make_pair(-5,5),make_pair(-5,5)},0.0,[](double){return 1.0;})
		>>(make_shared<AmplitudeSignal>()>>worst)
	);
	for(size_t cnt=0;cnt<200;cnt++)
		rsc->RegisterGamma({0,0,0},3000);
	EXPECT_TRUE(worst->data().val()<ideal->data().val());
	EXPECT_TRUE(worst->data().val()<middle->data().val());
	EXPECT_TRUE(middle->data().val()<ideal->data().val());
}
TEST(Scintillator, oneD_symmetry_plus_concurrency){
	vector<shared_ptr<Scintillator>> rsc{
		MakeScintillator({make_pair(-50,50),make_pair(-5,5),make_pair(-5,5)},1.6,TimeDistribution1(0.5,1.5)),
		MakeScintillator({make_pair(-50,50),make_pair(-5,5),make_pair(-5,5)},1.6,TimeDistribution1(0.5,1.5))
	};
	vector<shared_ptr<SignalStatictics>>
		timestat{make_shared<SignalStatictics>(2),make_shared<SignalStatictics>(2)},
		amplstat{make_shared<SignalStatictics>(2),make_shared<SignalStatictics>(2)};
	for(size_t index=0;index<2;index++){
		auto timediff=make_shared<SignalSumm>(),ampldiff=make_shared<SignalSumm>();
		{
			auto time=make_shared<Signal>(),ampl=make_shared<Signal>();
			rsc[index]->Surface(0,RectDimensions::Left)>>(
				Photosensor({make_pair(0,1),make_pair(0,1)},1.0,[](double){return 1.0;})
					>>(make_shared<AmplitudeSignal>()>>ampl)
					>>(TimeSignal({make_pair(0,1)})>>time)
			);
			timediff<<time;ampldiff<<ampl;
		}
		{
			auto time=make_shared<Signal>(),ampl=make_shared<Signal>();
			rsc[index]->Surface(0,RectDimensions::Right)>>(
				Photosensor({make_pair(0,1),make_pair(0,1)},1.0,[](double){return 1.0;})
					>>(make_shared<AmplitudeSignal>()>>(SignalInvert()>>ampl))
					>>(TimeSignal({make_pair(0,1)})>>(SignalInvert()>>time))
			);
			timediff<<time;ampldiff<<ampl;
		}
		timediff>>(timestat[index]);ampldiff>>(amplstat[index]);
	}
	for(size_t threads=1;threads<5;threads++){
		for(size_t index=0;index<2;index++)rsc[index]->Configure(Scintillator::Options(threads,5));
		cout<< threads<<" threads"<<endl;
		for(size_t cnt=0;cnt<1000;cnt++)
			rsc[0]->RegisterGamma({-30,0,0},3000);
		for(size_t cnt=0;cnt<1000;cnt++)
			rsc[1]->RegisterGamma({+30,0,0},3000);
		EXPECT_TRUE(value<>(amplstat[0]->data()).make_wider(2).Contains(amplstat[1]->data()));
		EXPECT_TRUE(timestat[0]->data().Contains(timestat[1]->data()));
	}
}
