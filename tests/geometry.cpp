// this file is distributed under 
// GPL v 3.0 license
#include <random>
#include "test_objects.h"
TEST(VecOpr,throwtest){
	Vec A;
	Vec B;
	for(int i=0;i<10;i++){
		A.push_back(Rand(engine));
		EXPECT_THROW((A)+(B),math_h_error<Vec>);
		EXPECT_THROW((A)-(B),math_h_error<Vec>);
		EXPECT_THROW(SqDistance((A),(B)),math_h_error<Vec>);
		EXPECT_THROW(Distance((A),(B)),math_h_error<Vec>);
		B.push_back(Rand(engine));
		EXPECT_NO_THROW((A)+(B));
		EXPECT_NO_THROW((A)-(B));
		EXPECT_NO_THROW(SqDistance((A),(B)));
		EXPECT_NO_THROW(Distance((A),(B)));
	}
	A.clear();B.clear();
	for(int i=0;i<10;i++){
		B.push_back(Rand(engine));
		EXPECT_THROW((A)+(B),math_h_error<Vec>);
		EXPECT_THROW((A)-(B),math_h_error<Vec>);
		A.push_back(Rand(engine));
		EXPECT_NO_THROW((A)+(B));
		EXPECT_NO_THROW((A)-(B));
	}
}
TEST(VecOpr,Mul){
	Vec A;
	for(int i=0;i<10;i++){
		for(double c=-10;c<=10;c+=1)
			EXPECT_TRUE(((A)*c).size()==A.size());
		A.push_back(Rand(engine));
	}
	A.clear();
	for(int i=0;i<10;i++){
		for(double c=-9.5;c<10;c+=1){//escape zero
			Vec B=(A)*c;
			Vec C=(B)*(1.0/c);
			for(size_t i=0,n=A.size();i<n;i++){
				EXPECT_TRUE(C[i]==A[i]);
				EXPECT_TRUE(B[i]==(A[i]*c));
			}
		}
		A.push_back(iRand(engine));
	}
}
TEST(VecOpr,Add_Sub){
	Vec A;
	Vec B;
	for(int i=0;i<10;i++){
		for(double c=-10;c<=10;c+=1){
			EXPECT_TRUE(((A)+(B)).size()==A.size());
			EXPECT_TRUE(((A)-(B)).size()==A.size());
		}
		A.push_back(Rand(engine));
		B.push_back(Rand(engine));
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
		A.push_back(Rand(engine));
		B.push_back(Rand(engine));
	}
}
TEST(VecOpr,Distances){
	Vec A;
	Vec B;
	for(int i=0;i<10;i++){
		for(double c=-10;c<=10;c+=1){
			double d=Distance((A),(B));
			EXPECT_TRUE(d>=0);
			EXPECT_TRUE(sqrt(SqDistance((A),(B)))==d);
		}
		A.push_back(Rand(engine));
		B.push_back(Rand(engine));
	}
	A.clear();
	B.clear();
	A.push_back(Rand(engine));
	B.push_back(Rand(engine));
	EXPECT_TRUE(SqDistance((A),(B))==pow(A[0]-B[0],2));
}

TEST(RectDimensions,Dimensions){
	RectDimensions A;
	EXPECT_TRUE(A.NumberOfDimensions()==0);
	EXPECT_THROW(A.Dimension(0),math_h_error<RectDimensions>);
	EXPECT_THROW(A.Dimension(1),math_h_error<RectDimensions>);
	EXPECT_THROW(A.Dimension(2),math_h_error<RectDimensions>);
	for(size_t i=1;i<10;i++){
		EXPECT_TRUE(&A==&(A<<make_pair(i,2*i)));
		EXPECT_TRUE(A.NumberOfDimensions()==i);
		for(size_t j=1;j<=i;j++){
			EXPECT_TRUE(A.Dimension(j-1).first==j);
			EXPECT_TRUE(A.Dimension(j-1).second==2*j);
		}
		EXPECT_THROW(A.Dimension(i),math_h_error<RectDimensions>);
		EXPECT_THROW(A.Dimension(i+1),math_h_error<RectDimensions>);
		EXPECT_THROW(A.Dimension(i+2),math_h_error<RectDimensions>);
	}
}
TEST(RectDimensions,IsInside){
	RectDimensions A;
	for(size_t i=1;i<10;i++){
		A<<make_pair(-10.0+i,double(i));
		for(size_t cnt=0;cnt<(500*i);cnt++){
			bool test=true;
			Vec v;
			for(size_t j=0,n=A.NumberOfDimensions();j<n;j++){
				double x=Rand(engine);
				test&=(x>=A.Dimension(j).first)&&(x<=A.Dimension(j).second);
				EXPECT_THROW(A.IsInside((v)),math_h_error<RectDimensions>);
				v.push_back(x);
			}
			EXPECT_TRUE(A.IsInside((v))==test);
		}
	}
}
TEST(RectDimensions,WhereIntersects_throwing1){
	RectDimensions A;
	Vec O;Vec D;
	for(size_t i=1;i<10;i++){
		A<<make_pair(-10.0+i,double(i));
		EXPECT_THROW(A.WhereIntersects((O),(D)),math_h_error<RectDimensions>);
		O.push_back(0);
		EXPECT_THROW(A.WhereIntersects((O),(D)),math_h_error<RectDimensions>);
		D.push_back(0);
		EXPECT_TRUE(A.WhereIntersects((O),(D)).surface==RectDimensions::None);
	}
}
TEST(RectDimensions,WhereIntersects_throwing2){
	RectDimensions A;
	Vec O;Vec D;
	for(size_t i=1;i<10;i++){
		A<<make_pair(-10.0+i,double(i));
		EXPECT_THROW(A.WhereIntersects((O),(D)),math_h_error<RectDimensions>);
		D.push_back(0);
		EXPECT_THROW(A.WhereIntersects((O),(D)),math_h_error<RectDimensions>);
		O.push_back(0);
		EXPECT_TRUE(A.WhereIntersects((O),(D)).surface==RectDimensions::None);
	}
}
TEST(RectDimensions,WhereIntersects){
	for(size_t cnt=0;cnt<20000;cnt++){
		RectDimensions A;
		Vec O, Outside,D;
		for(size_t i=1;i<10;i++){
			A<<make_pair(-10.0+i,double(i));
			uniform_real_distribution<double> coord(-10.0+i,i);
			O.push_back(coord(engine));
			Outside.push_back(100);
			D.push_back(Rand(engine));
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