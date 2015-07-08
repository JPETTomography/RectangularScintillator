#ifndef fXIVCFLg
#define fXIVCFLg
#include "rectscin.h"
class ISignal{
public:
	virtual ~ISignal(){}
	virtual void Start()=0;
	virtual void Photon(double time)=0;
	virtual void End()=0;
};
class Photosensor:public virtual IPhotoSensitive,protected virtual RectDimensions{
public:
	Photosensor(std::vector<Pair> dimensions,Func efficiency,double tts);
    virtual ~Photosensor();
	virtual void Start()override;
	virtual void RegisterPhoton(Photon&photon)override;
	virtual void End()override;
    Photosensor&operator<<(std::shared_ptr<ISignal>);
private:
	std::normal_distribution<double> m_tts;
	Func m_efficiency;//depends on lambda
	std::vector<std::shared_ptr<ISignal>> m_signal;
	std::default_random_engine rand;
};
#endif