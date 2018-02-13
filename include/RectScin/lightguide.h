// this file is distributed under 
// MIT license
#ifndef XSAYaAus
#define XSAYaAus
#include "scintillator.h"
namespace RectangularScintillator{
	class FlatLightguide:public virtual IPhotonAbsorber,protected virtual ScintillatorSurface{
	public:
		FlatLightguide(const std::vector<Pair>&dimensions,const double&glue_eff,const double&n,const double&H);
		virtual ~FlatLightguide();
		FlatLightguide&operator>>(const std::shared_ptr<IPhotonAbsorber>sensor);
		virtual void Start()override;
		virtual void AbsorbPhoton(Photon& photon)override;
		virtual void End()override;
		virtual double GlueEfficiency()const override;
		virtual const RectDimensions&Dimensions()const override;
	private:
		
		double g_eff,refr,height;
	};
	inline const std::shared_ptr<FlatLightguide> LightGuide(
	    const std::vector<Pair>&dimensions,const double&glue_eff,const double&n,const double&H
	){
		return std::shared_ptr<FlatLightguide>(new FlatLightguide(dimensions,glue_eff,n,H));
	}
};
#endif
