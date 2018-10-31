// this file is distributed under 
// MIT license
#include <mutex>
#include <math.h>
#include <math_h/interpolate.h>
#include <math_h/randomfunc.h>
#include <math_h/error.h>
#include <RectScin/geometry.h>
namespace RectangularScintillator{
	using namespace std;
	using namespace MathTemplates;
	Vec operator*(const Vec&p,const double&c){
		Vec res;
		for(double x:p)
			res.push_back(x*c);
		return res;
	}
	Vec operator+(const Vec&p1,const Vec&p2){
		if(p1.size()!=p2.size())
			throw Exception<Vec>("A+B: vector sizes differ");
		Vec res;
		for(size_t i=0,n=p1.size();i<n;i++)
			res.push_back(p1[i]+p2[i]);
		return res;
	}
	Vec operator-(const Vec&p1,const Vec&p2){
		if(p1.size()!=p2.size())
			throw Exception<Vec>("A-B: vector sizes differ");
		Vec res;
		for(size_t i=0,n=p1.size();i<n;i++)
			res.push_back(p1[i]-p2[i]);
		return res;
	}
	double SqAbs(const Vec&p){
		double res=0;
		for(double x:p)res+=x*x;
		return res;
	}
	double Abs(const Vec&p){return sqrt(SqAbs(p));}
	double SqDistance(const Vec&p1,const Vec&p2){return SqAbs(p1-p2);}
	double Distance(const Vec&p1,const Vec&p2){return Abs(p1-p2);}
	RectDimensions::Side& inc(RectDimensions::Side& val){
		val=static_cast<RectDimensions::Side>(val+1);
		return val;
	}
	RectDimensions::RectDimensions(){}
	RectDimensions::~RectDimensions(){}
	RectDimensions& RectDimensions::operator<<(const Pair&dimension){
		if(dimension.first>dimension.second)
			throw Exception<RectDimensions>("RectDimensions: wrong dimension left>right");
		Lock lock(geom_mutex);
		m_dimensions.push_back(dimension);
		return *this;
	}
	const size_t RectDimensions::NumberOfDimensions()const{return m_dimensions.size();}
	const Pair&RectDimensions::Dimension(size_t i)const{
		if(i>=NumberOfDimensions())
			throw Exception<RectDimensions>("RectDimensions: dimension index out of range");
		return m_dimensions[i];
	}
	const bool RectDimensions::IsInside(const Vec&point)const{
		if(point.size()!=NumberOfDimensions())
			throw Exception<RectDimensions>("RectDimensions::IsInside: wrong point size");
		for(size_t i=0,n=NumberOfDimensions();i<n;i++)
			if((point[i]<Dimension(i).first)||(point[i]>Dimension(i).second))
				return false;
			return true;
	}
	const RectDimensions::IntersectionSearchResults RectDimensions::WhereIntersects(const Vec&point,const Vec&dir)const{
		if(NumberOfDimensions()!=dir.size())
			throw Exception<RectDimensions>("RectDimensions trace: wrong direction vector size");
		if(!IsInside(point)){
			IntersectionSearchResults res;
			res.surface=None;
			return res;
		}
		// Distance to the corresponding edge, dimension index
		typedef pair<double,size_t> dist_dim;
		vector<dist_dim> dim_order;
		for(size_t i=0,n=NumberOfDimensions();i<n;i++){
			dist_dim newdim;
			Lock lock(const_cast<mutex&>(geom_mutex));
			newdim.second=i;
			if(dir[i]<0)
				newdim.first=point[i]-m_dimensions[i].first;
			if(dir[i]>0)
				newdim.first=m_dimensions[i].second-point[i];
			if(dir[i]!=0)
				table_data_details::InsertSorted(newdim,dim_order,std_size(dim_order),std_insert(dim_order,dist_dim));
		}
		if(dim_order.size()==0){
			IntersectionSearchResults res;
			res.surface=None;
			return res;
		}
		for(size_t i=0,n=dim_order.size();i<n;i++){
			size_t dimension=dim_order[i].second;
			double k=dim_order[i].first/dir[dimension];
			if(k<0) k=-k;
			Vec endpoint=point+(dir*k);
			bool Belong_to_surface=true;
			for(size_t i=0,n=NumberOfDimensions();i<n;i++)
				if(i!=dimension){
					Belong_to_surface &= endpoint[i]>=Dimension(i).first;
					Belong_to_surface &= endpoint[i]<=Dimension(i).second;
				}
				if(Belong_to_surface){
					IntersectionSearchResults res;
					if(dir[dimension]<0)
						res.surface=Left;
					else
						res.surface=Right;
					res.surfaceDimentionIndex=dimension;
					res.k=k;
					res.coordinates=endpoint;
					return res;
				}
		}
		IntersectionSearchResults res;
		res.surface=None;
		return res;
	}
};
