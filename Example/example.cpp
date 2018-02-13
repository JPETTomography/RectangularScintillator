// this file is distributed under 
// MIT license
#include <iostream>
#include <RectScin/scintillator.h>
#include <RectScin/sensitive.h>
#include <RectScin/photon2signal.h>
#include <RectScin/signal_processing.h>
#include <RectScin/signal_statistics.h>
using namespace std;
using namespace MathTemplates;
using namespace RectangularScintillator;
int main(int , char **){
	auto scintillator=MakeScintillator({make_pair(-15,15),make_pair(-7,7),make_pair(-7,7)},1.58,TimeDistribution2(0.005,0.2,1.5));
	auto time_difference=make_shared<SignalStatictics>();
	{
		auto photosensor=[](){return Photosensor({make_pair(-7,7),make_pair(-7,7)},1,[](double){return 0.3;});};
		auto left=make_shared<Signal>(),right=make_shared<Signal>();
		scintillator->Surface(0,RectDimensions::Left)>>(photosensor()>>(TimeSignal({make_pair(0,1)})>>left));
		scintillator->Surface(0,RectDimensions::Right)>>(photosensor()>>(TimeSignal({make_pair(0,1)})>>(SignalInvert()>>right)));
		(make_shared<SignalSumm>()<<left<<right)>>time_difference;
	}
	cout<<"Simulation..."<<endl;
	for(unsigned int cnt=0;cnt<500;cnt++)
		scintillator->RegisterGamma({0,0,0},3000);
	cout<<"done."<<endl;
	cout<<"Time difference = "
	      <<time_difference->data().val()<<" +/- "
	      <<time_difference->data().uncertainty()<<" [ns]"<<endl;
	return 0;
}
