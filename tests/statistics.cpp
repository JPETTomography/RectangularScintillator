// this file is distributed under 
// MIT license
#include "test_objects.h"
using namespace std;
using namespace MathTemplates;
using namespace RectangularScintillator;
TEST(SignalStatictics,Base){
    RandomUniform<> Rand(-10,10);
	for(size_t i=0;i<10;i++){
		auto test=make_shared<SignalStatictics>();
		SignalSender s;
		s>>test;
		s.send({Rand()});
		for(size_t j=1;j<=i;j++){
			s.send({Rand()});
			EXPECT_TRUE(test->data().val()>=-10.);
			EXPECT_TRUE(test->data().val()<=10.);
		}
	}
}
