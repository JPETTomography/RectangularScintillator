// this file is distributed under 
// MIT license
#include <thread>
#include <mutex>
#include <math_h/integrate.h>
#include <math_h/functions.h>
#include <math_h/error.h>
#include <RectScin/scintillator.h>
namespace RectangularScintillator{
	using namespace std;
	using namespace MathTemplates;
	ScintillatorSurface::ScintillatorSurface(){}
	ScintillatorSurface::~ScintillatorSurface(){}
	void ScintillatorSurface::Start(){
		Lock lock(surface_mutex);
		for(auto handler:m_handlers)
			handler->Start();
	}
	void ScintillatorSurface::RegisterPhoton(Photon&photon){
		if(IsInside(photon.coord)){
			Lock lock(surface_mutex);
			for(auto handler:m_handlers)
				handler->AbsorbPhoton(photon);
		}
	}
	void ScintillatorSurface::End(){
		Lock lock(surface_mutex);
		for(auto handler:m_handlers)
			handler->End();
	}
	ScintillatorSurface& ScintillatorSurface::operator>>(const shared_ptr<IPhotonAbsorber> sensor){
		Lock lock(surface_mutex);
		m_handlers.push_back(sensor);
		return *this;
	}
	const double ScintillatorSurface::ReflectionProbabilityCoeff(const Vec&point)const{
		for(auto sensor:m_handlers)
		    if(sensor->Dimensions().IsInside(point)){
			double eff=sensor->GlueEfficiency();
			if((eff<0)||(eff>1))
			    throw Exception<ScintillatorSurface>("Optical glue efficiency must be in the range [0;1]");
			return 1.0-eff;
		    }
		return 1.0;
	}
	const RectDimensions&ScintillatorSurface::Dimensions()const{return *this;}
	
	const double ReflectionProbability(const double&refraction,const double&cos_){
		double cos_phi=cos_;
		if(cos_phi<0.0)cos_phi=-cos_phi;
		if(cos_phi>1.0)cos_phi=1;
		double sin_phi=sqrt(1.0-cos_phi*cos_phi);
		double squrt=sqrt(1-refraction*refraction*sin_phi*sin_phi);
		double T_ort=(refraction*squrt-cos_phi)/(refraction*squrt+cos_phi);
		double T_par=(refraction*cos_phi-squrt)/(refraction*cos_phi+squrt);
		auto SubInt=[T_par,T_ort](double pol){
			return sqrt(pow(sin(pol)*T_ort,2)+pow(cos(pol)*T_par,2));
		};
		double r=Sympson(SubInt,0.0,3.1415926*0.5,0.001);
		if(r<1.0)return r;
		return 1.0;
	}
	Scintillator::Scintillator(
		const vector<Pair>&dimensions,
		const double refraction,
		const shared_ptr<Distrib>time_distribution,
		const shared_ptr<Distrib>lambda_distribution,
		const Func absorption
	):RectDimensions(),
		m_config(Defaults()),
		m_time_distribution(time_distribution),
		m_lambda_distribution(lambda_distribution)
	{
		trace_mutex=make_shared<mutex>();
		if(dimensions.size()==0)
			throw Exception<Scintillator>("Cannot create scintillator with no dimensions");
		for(const Pair&D:dimensions)RectDimensions::operator<<(D);
		m_refraction=refraction;
		m_absorption=absorption;
		for(size_t i=0,n=NumberOfDimensions();i<n;i++){
		    SurfPair surfaces=make_pair(make_shared<ScintillatorSurface>(),make_shared<ScintillatorSurface>());
		    for(size_t j=0,n=NumberOfDimensions();j<n;j++)
			if(j!=i){
			    static_cast<RectDimensions&>(*(surfaces.first))<<Dimension(i);
			    static_cast<RectDimensions&>(*(surfaces.second))<<Dimension(i);
			}
		    m_edges.push_back(surfaces);
		}
		for(double x=-0.01;x<=1.01;x+=0.01)
			reflection_probability<<point<double>(x,ReflectionProbability(m_refraction,x));
	}
	Scintillator::~Scintillator(){}
	const LinearInterpolation<double>&Scintillator::ReflectionProbabilityFunction()const{
		return reflection_probability;
	}
	Scintillator::Options::Options(const size_t c,const unsigned long refl):
	concurrency(c),max_reflections(refl){}
	Scintillator::Options::Options(const Scintillator::Options&source):
	concurrency(source.concurrency),max_reflections(source.max_reflections){}
	void Scintillator::Options::operator=(const Scintillator::Options& source){
		concurrency=source.concurrency;
		max_reflections=source.max_reflections;
	}
	Scintillator::Options Scintillator::Defaults(){return Options(1,0);}
	Scintillator::Options Scintillator::Concurrency(const size_t c){return Options(c,0);}
	Scintillator::Options Scintillator::Reflections(const long unsigned int r){return Options(1,r);}
	
	
	void Scintillator::Configure(const Scintillator::Options&conf){
		m_config=conf;
	}
	const Scintillator::Options& Scintillator::CurrentConfig()const{
		return m_config;
	}
	ScintillatorSurface& Scintillator::Surface(size_t dimension, Side side)const{
		if(dimension>=NumberOfDimensions())
			throw Exception<Scintillator>("Scintillator: dimension index out of range");
		Lock lock(*trace_mutex);
		if(side==Left)return *(m_edges[dimension].first);
		if(side==Right)return *(m_edges[dimension].second);
		throw Exception<Scintillator>("Scintillator: surface index out of range");
	}
	void Scintillator::RegisterGamma(const Vec&&coord,const size_t N)const{
		for(const SurfPair&sp:m_edges){
			sp.first->Start();
			sp.second->Start();
		}
		if(coord.size()!=NumberOfDimensions())
			throw Exception<Scintillator>("Scintillator: wrong gamma interaction point vector size");
		if(!IsInside(coord))
			throw Exception<Scintillator>("Scintillator: gamma interaction point is outside");
		auto process=[this,&coord](size_t n){
			for(size_t i=0;i<n;i++){
				Photon ph=GeneratePhoton(coord);
				IntersectionSearchResults trace=TraceGeometry(ph);
				if(trace.surface!=None){
					Surface(trace.surfaceDimentionIndex,trace.surface).RegisterPhoton(ph);
				}
			}
		};
		{
			size_t threads=m_config.concurrency;
			if(threads==0)
				throw Exception<Scintillator>("Threads count cannot be zero");
			size_t part=N/threads,rest=N%threads;
			vector<shared_ptr<thread>> thread_vector;
			for(size_t i=1;i<threads;i++)
				thread_vector.push_back(make_shared<thread>(process,part));
			process(part+rest);
			for(auto thr:thread_vector)thr->join();
		}
		for(const SurfPair&sp:m_edges){
			sp.first->End();
			sp.second->End();
		}
	}
	Photon Scintillator::GeneratePhoton(const Vec&coord)const{
		Photon res;
		res.coord=coord;
		if(NumberOfDimensions()==1){
			RandomUniform<> get_bin(0,1);
			if(get_bin()<0.5)
				res.dir.push_back(-1);
			else
				res.dir.push_back(1);
		}
		if(NumberOfDimensions()==2){
			RandomUniform<> get_angle(0,2.0*3.1415926);
			double phi=get_angle();
			res.dir.push_back(sin(phi));
			res.dir.push_back(cos(phi));
		}
		if(NumberOfDimensions()==3){
			RandomUniform<> get_angle(0,2.0*3.1415926);
			RandomUniform<> get_secondangle(-1,1);
			double cos_theta,phi;
			{Lock lock(*trace_mutex);
				phi=get_angle();
				cos_theta=get_secondangle();
			}
			double sin_theta=sqrt(1-cos_theta*cos_theta);
			res.dir.push_back(cos_theta);
			res.dir.push_back(sin_theta*sin(phi));
			res.dir.push_back(sin_theta*cos(phi));
		}
		if(NumberOfDimensions()>=4)
			throw Exception<Scintillator>("Scintillator: isotropic 4D and higher random directions not implemented");
		{
			Lock lock(*trace_mutex);
			res.time=m_time_distribution->operator()();
			res.lambda=m_lambda_distribution->operator()();
		}
		return res;
	}
	RectDimensions::IntersectionSearchResults Scintillator::TraceGeometry(Photon&ph)const{
		RandomUniform<> prob_(0,1);
		double absorption;{
			Lock lock(*trace_mutex);
			absorption=m_absorption(ph.lambda);
		}
		double refl_p[NumberOfDimensions()];
		for(size_t dimension=0,n=NumberOfDimensions();dimension<n;dimension++){
			double cos_angle=ph.dir[dimension];
			if(cos_angle<0.0)
				cos_angle=-cos_angle;
			if(cos_angle>1.0)
				throw Exception<Scintillator>("Trace: cosine error");
			Lock lock(*trace_mutex);
			refl_p[dimension]=reflection_probability(cos_angle);
		}
		unsigned long refl_counter=0;
		while(true){
			IntersectionSearchResults res=WhereIntersects(ph.coord,ph.dir);
			if(res.surface==None)
				return res;
			size_t dimension=res.surfaceDimentionIndex;
			double path_length=res.k;
			//update kinematic parameters
			ph.coord=res.coordinates;
			ph.time+=path_length*m_refraction/speed_of_light;
			//correct if small errors caused that were a little bit outside
			if(ph.coord[dimension]<Dimension(dimension).first)
				ph.coord[dimension]=Dimension(dimension).first;
			if(ph.coord[dimension]>Dimension(dimension).second)
				ph.coord[dimension]=Dimension(dimension).second;
			//check for other effects
			double absorption_prob=1.0-exp(-path_length*absorption);
			{Lock lock(*trace_mutex);
				if(prob_()<absorption_prob){
					//Photon is absopbed
					res.surface=None;
					return res;
				}
			}
			double refl_prob=refl_p[dimension];
			{//check if photon reached a glued area of surface
				Vec point=ph.coord;
				point.erase(point.begin()+dimension);
				refl_prob*=Surface(dimension,res.surface).ReflectionProbabilityCoeff(point);
			}
			{Lock lock(*trace_mutex);
				if(prob_()<refl_prob){
					//Photon is reflected back
					ph.dir[dimension]=-ph.dir[dimension];
				}else{
					//Photon leaves the scintillator
					ph.dir=ph.dir*m_refraction;
					ph.coord.erase(ph.coord.begin()+dimension);
					ph.dir.erase(ph.dir.begin()+dimension);
					return res;
				}
			}
			if(m_config.max_reflections>0){
				refl_counter++;
				if(refl_counter>m_config.max_reflections){
					res.surface=None;
					return res;
				}
			}
		}
	}
	const std::shared_ptr<Distrib> TimeDistribution1(const double& sigma, const double& decay,const SortedChain<double>&chain){
		if((sigma<=0)||(decay<=0))throw Exception<Distrib>("wrong distribution parameters");
		double min=chain[0],max=chain[chain.size()-1],dt=(max-min)/double(chain.size());
		auto func=[sigma,decay,max,min,dt](const double&t){
			auto A=[&sigma](const double& th){if(th<0)return 0.0;return Gaussian(th,2.5*sigma,sigma);};
			auto B=[&decay](const double& th){if(th<0)return 0.0;return exp(-th/decay);};
			return Sympson([&t,&sigma,&decay,&A,&B](double ksi){return A(ksi)*B(t-ksi);},min-sigma,max+sigma,dt);
		};
		return make_shared<DistribTable>(func,chain);
	}
	const std::shared_ptr<Distrib> TimeDistribution2(const double&rize,const double&sigma,const double&decay,const SortedChain<double>&chain){
		if((sigma<=0)||(decay<=0))throw Exception<Distrib>("wrong distribution parameters");
		double min=chain[0],max=chain[chain.size()-1],dt=(max-min)/double(chain.size());
		auto func=[rize,sigma,decay,min,max,dt](const double&t){
			auto A=[&sigma](const double& th){if(th<0)return 0.0;return Gaussian(th,2.5*sigma,sigma);};
			auto B=[&rize,&decay](const double& th){if(th<0)return 0.0;return exp(-th/decay)-exp(-th/rize);};
			return Sympson([&t,&sigma,&decay,&A,&B](const double& ksi){return A(ksi)*B(t-ksi);},min-sigma,max+sigma,dt);
		};
		return make_shared<DistribTable>(func,chain);
	}
};
