// this file is distributed under 
// MIT license
#include <random>
#include "test_objects.h"
using namespace std;
using namespace MathTemplates;
using namespace RectangularScintillator;
TEST(VecOpr,throwtest){
	RandomUniform<> Rand(-10,10);
	Vec A;
	Vec B;
	for(int i=0;i<10;i++){
		A.push_back(Rand());
		EXPECT_THROW((A)+(B),Exception<Vec>);
		EXPECT_THROW((A)-(B),Exception<Vec>);
		EXPECT_THROW(SqDistance((A),(B)),Exception<Vec>);
		EXPECT_THROW(Distance((A),(B)),Exception<Vec>);
		B.push_back(Rand());
		EXPECT_NO_THROW((A)+(B));
		EXPECT_NO_THROW((A)-(B));
		EXPECT_NO_THROW(SqDistance((A),(B)));
		EXPECT_NO_THROW(Distance((A),(B)));
	}
	A.clear();B.clear();
	for(int i=0;i<10;i++){
		B.push_back(Rand());
		EXPECT_THROW((A)+(B),Exception<Vec>);
		EXPECT_THROW((A)-(B),Exception<Vec>);
		A.push_back(Rand());
		EXPECT_NO_THROW((A)+(B));
		EXPECT_NO_THROW((A)-(B));
	}
}
TEST(VecOpr,Mul){
	RandomUniform<> Rand(-10,10);
	Vec A;
	for(int i=0;i<10;i++){
		for(double c=-10;c<=10;c+=1)
			EXPECT_TRUE(((A)*c).size()==A.size());
		A.push_back(Rand());
	}
	A.clear();
	for(int i=0;i<10;i++){
		for(double c=-9.5;c<10;c+=1){//escape zero
			Vec B=(A)*c;
			Vec C=(B)*(1.0/c);
			for(size_t i=0,n=A.size();i<n;i++){
				EXPECT_CLOSE_VALUES(C[i],A[i]);
				EXPECT_TRUE(B[i]==(A[i]*c));
			}
		}
		A.push_back(Rand());///////
	}
}
TEST(VecOpr,Add_Sub){
	RandomUniform<> Rand(-10,10);
	Vec A;
	Vec B;
	for(int i=0;i<10;i++){
		for(double c=-10;c<=10;c+=1){
			EXPECT_TRUE(((A)+(B)).size()==A.size());
			EXPECT_TRUE(((A)-(B)).size()==A.size());
		}
		A.push_back(Rand());
		B.push_back(Rand());
	}
	A.clear();
	B.clear();
	for(int i=0;i<10;i++){
		for(double c=-10;c<=10;c+=1){
			Vec C=(A)+(B);
			Vec D=(C)-(B);
			for(size_t i=0,n=A.size();i<n;i++){
				EXPECT_TRUE(D[i]==A[i]);
				EXPECT_TRUE(C[i]==(A[i]+B[i]));
			}
		}
		A.push_back(Rand());
		B.push_back(Rand());
	}
}
TEST(VecOpr,Distances){
	RandomUniform<> Rand(-10,10);
	Vec A;
	Vec B;
	for(int i=0;i<10;i++){
		for(double c=-10;c<=10;c+=1){
			double d=Distance((A),(B));
			EXPECT_TRUE(d>=0);
			EXPECT_TRUE(sqrt(SqDistance((A),(B)))==d);
		}
		A.push_back(Rand());
		B.push_back(Rand());
	}
	A.clear();
	B.clear();
	A.push_back(Rand());
	B.push_back(Rand());
	EXPECT_TRUE(SqDistance((A),(B))==pow(A[0]-B[0],2));
}

TEST(RectDimensions,Dimensions){
	RandomUniform<> Rand(-10,10);
	RectDimensions A;
	EXPECT_TRUE(A.NumberOfDimensions()==0);
	EXPECT_THROW(A.Dimension(0),Exception<RectDimensions>);
	EXPECT_THROW(A.Dimension(1),Exception<RectDimensions>);
	EXPECT_THROW(A.Dimension(2),Exception<RectDimensions>);
	for(size_t i=1;i<10;i++){
		EXPECT_TRUE(&A==&(A<<make_pair(i,2*i)));
		EXPECT_TRUE(A.NumberOfDimensions()==i);
		for(size_t j=1;j<=i;j++){
			EXPECT_TRUE(A.Dimension(j-1).first==j);
			EXPECT_TRUE(A.Dimension(j-1).second==2*j);
		}
		EXPECT_THROW(A.Dimension(i),Exception<RectDimensions>);
		EXPECT_THROW(A.Dimension(i+1),Exception<RectDimensions>);
		EXPECT_THROW(A.Dimension(i+2),Exception<RectDimensions>);
	}
}
TEST(RectDimensions,IsInside){
	RandomUniform<> Rand(-10,10);
	RectDimensions A;
	for(size_t i=1;i<10;i++){
		A<<make_pair(-10.0+i,double(i));
		for(size_t cnt=0;cnt<(500*i);cnt++){
			bool test=true;
			Vec v;
			for(size_t j=0,n=A.NumberOfDimensions();j<n;j++){
				double x=Rand();
				test&=(x>=A.Dimension(j).first)&&(x<=A.Dimension(j).second);
				EXPECT_THROW(A.IsInside((v)),Exception<RectDimensions>);
				v.push_back(x);
			}
			EXPECT_TRUE(A.IsInside((v))==test);
		}
	}
}
TEST(RectDimensions,WhereIntersects_throwing1){
	RandomUniform<> Rand(-10,10);
	RectDimensions A;
	Vec O;Vec D;
	for(size_t i=1;i<10;i++){
		A<<make_pair(-10.0+i,double(i));
		EXPECT_THROW(A.WhereIntersects((O),(D)),Exception<RectDimensions>);
		O.push_back(0);
		EXPECT_THROW(A.WhereIntersects((O),(D)),Exception<RectDimensions>);
		D.push_back(0);
		EXPECT_TRUE(A.WhereIntersects((O),(D)).surface==RectDimensions::None);
	}
}
TEST(RectDimensions,WhereIntersects_throwing2){
	RandomUniform<> Rand(-10,10);
	RectDimensions A;
	Vec O;Vec D;
	for(size_t i=1;i<10;i++){
		A<<make_pair(-10.0+i,double(i));
		EXPECT_THROW(A.WhereIntersects((O),(D)),Exception<RectDimensions>);
		D.push_back(0);
		EXPECT_THROW(A.WhereIntersects((O),(D)),Exception<RectDimensions>);
		O.push_back(0);
		EXPECT_TRUE(A.WhereIntersects((O),(D)).surface==RectDimensions::None);
	}
}
TEST(RectDimensions,WhereIntersects){
	RandomUniform<> Rand(-10,10);
	for(size_t cnt=0;cnt<20000;cnt++){
		RectDimensions A;
		Vec O, Outside,D;
		for(size_t i=1;i<10;i++){
			A<<make_pair(-10.0+i,double(i));
			RandomUniform<double> coord(-10.0+i,i);
			O.push_back(coord());
			Outside.push_back(100);
			D.push_back(Rand());
			EXPECT_TRUE(A.WhereIntersects((Outside),(D)).surface==RectDimensions::None);
			if(Abs((D))>0){
				RectDimensions::IntersectionSearchResults res=A.WhereIntersects((O),(D));
				if(res.surface!=RectDimensions::None){
					//Condition for found exit point
					EXPECT_TRUE(res.surfaceDimentionIndex<A.NumberOfDimensions());
					EXPECT_TRUE(res.k>=0);//path length/direction vector length
					double diff;
					if(res.surface==RectDimensions::Left)
						diff=pow(res.coordinates[res.surfaceDimentionIndex]-A.Dimension(res.surfaceDimentionIndex).first,2);
					else
						diff=pow(res.coordinates[res.surfaceDimentionIndex]-A.Dimension(res.surfaceDimentionIndex).second,2);
					EXPECT_TRUE(diff<0.00000001);
					EXPECT_FALSE(diff<0);
					double actual_dist=Distance((res.coordinates),(O));
					double expected_dist=Abs((D))*res.k;
					diff=pow(actual_dist-expected_dist,2);
					EXPECT_TRUE(diff<0.00000001);
				}else{
					//if source point is inside and direction is not zero there should be an exit point
					EXPECT_TRUE(false);
				}
			}else{
				EXPECT_TRUE(A.WhereIntersects((O),(D)).surface==RectDimensions::None);
			}
		}
	}
}
