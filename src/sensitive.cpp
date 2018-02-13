// this file is distributed under 
// MIT license
#include <math_h/interpolate.h>
#include <math_h/error.h>
#include <RectScin/sensitive.h>
namespace RectangularScintillator{
	using namespace std;
	using namespace MathTemplates;
	PhotoSensitiveSurface::PhotoSensitiveSurface(const vector<Pair>&dimensions,const double&glue,const Func efficiency):RectDimensions(),P(0,1){
		for(const Pair&D:dimensions)
			RectDimensions::operator<<(D);
		m_efficiency=efficiency;
		m_glue=glue;
	}
	PhotoSensitiveSurface::~PhotoSensitiveSurface(){}
	const RectDimensions&PhotoSensitiveSurface::Dimensions()const{return *this;}
	double PhotoSensitiveSurface::GlueEfficiency()const{return m_glue;}
	PhotoSensitiveSurface& PhotoSensitiveSurface::operator>>(const shared_ptr<PhotonTimeAcceptor> sig){
		m_signal.push_back(sig);
		return *this;
	}
	void PhotoSensitiveSurface::Start(){
		for(const auto sig:m_signal)
			sig->AcceptEventStart();
		times.clear();
	}
	void PhotoSensitiveSurface::AbsorbPhoton(Photon& photon){
		if(IsInside(photon.coord))
			if(P()<m_efficiency(photon.lambda))
				PhotonTimeAccepted(photon.time);
	}
	void PhotoSensitiveSurface::PhotonTimeAccepted(const double&time){
		details::InsertSorted(time,times,field_size(times),field_insert(times,double));
	}
	void PhotoSensitiveSurface::End(){
		for(double time:times)
			for(auto sig:m_signal)
				sig->AcceptPhotonTime(time);
		for(auto sig:m_signal)
			sig->AcceptEventEnd();
	}
	
	PhotoSensitiveSurfaceWithTTS::PhotoSensitiveSurfaceWithTTS(
	    const vector<Pair>&dimensions,const double&glue,const Func efficiency,const double&tts
	):PhotoSensitiveSurface(dimensions,glue,efficiency),m_tts(5*tts,tts){}
	PhotoSensitiveSurfaceWithTTS::~PhotoSensitiveSurfaceWithTTS(){}
	void PhotoSensitiveSurfaceWithTTS::PhotonTimeAccepted(const double&time){
		double offs=m_tts();if(offs<0)offs=0;
		PhotoSensitiveSurface::PhotonTimeAccepted(time+offs);
	}
};
