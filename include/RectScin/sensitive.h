// this file is distributed under 
// MIT license
#ifndef fXIVCFLg
#define fXIVCFLg
#include "scintillator.h"
namespace RectangularScintillator{
	class PhotonTimeAcceptor{
	public:
		virtual ~PhotonTimeAcceptor(){}
		virtual void AcceptEventStart()=0;
		//Photons come sorted by time
		virtual void AcceptPhotonTime(const double&time)=0;
		virtual void AcceptEventEnd()=0;
	};
	class PhotoSensitiveSurface:public IPhotonAbsorber,protected RectDimensions{
	public:
		PhotoSensitiveSurface(const std::vector<Pair>&dimensions,const double&glue,const Func efficiency);
		virtual ~PhotoSensitiveSurface();
		virtual void Start()override;
		virtual void AbsorbPhoton(Photon&photon)override;
		virtual void End()override;
		virtual const RectDimensions&Dimensions()const override;
		virtual double GlueEfficiency()const override;
		PhotoSensitiveSurface&operator>>(const std::shared_ptr<PhotonTimeAcceptor>);
	protected:
		//Photons come in not sorted by time, but are sorted here
		virtual void PhotonTimeAccepted(const double&time);
	private:
		double m_glue;
		Func m_efficiency;//depends on lambda
		std::vector<std::shared_ptr<PhotonTimeAcceptor>> m_signal;
		MathTemplates::RandomUniform<> P;
		Vec times;
	};
	//There's a problem with transfering vector<smth>&& parameter as {val1,val2,...} to make_Shared template function
	inline const std::shared_ptr<PhotoSensitiveSurface> Photosensor(const std::vector<Pair>&dimensions,const double&glue,const Func efficiency){
		return std::shared_ptr<PhotoSensitiveSurface>(new PhotoSensitiveSurface(dimensions,glue,efficiency));
	}
	class PhotoSensitiveSurfaceWithTTS:public PhotoSensitiveSurface{
	public:
		PhotoSensitiveSurfaceWithTTS(const std::vector<Pair>&dimensions,const double&glue,const Func efficiency,const double&tts);
		virtual ~PhotoSensitiveSurfaceWithTTS();
	protected:
		virtual void PhotonTimeAccepted(const double&time)override;
	private:
		MathTemplates::RandomGauss<> m_tts;;
	};
	//There's a problem with transfering vector<smth>&& parameter as {val1,val2,...} to make_Shared template function
	inline const std::shared_ptr<PhotoSensitiveSurface> Photosensor(
	    const std::vector<Pair>&dimensions,const double&glue,const Func efficiency,const double&tts
	){
		return std::shared_ptr<PhotoSensitiveSurfaceWithTTS>(new PhotoSensitiveSurfaceWithTTS(dimensions,glue,efficiency,tts));
	}
};
#endif
