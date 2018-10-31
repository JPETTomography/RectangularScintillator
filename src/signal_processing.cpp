// this file is distributed under 
// MIT license
#include <math_h/error.h>
#include <math_h/functions.h>
#include <math_h/interpolate.h>
#include <RectScin/signal_processing.h>
namespace RectangularScintillator{
	using namespace std;
	using namespace MathTemplates;
	void Single2SingleSignal::AcceptEventStart(){SendEventStart();}
	void Single2SingleSignal::AcceptEventEnd(){SendEventEnd();}
	Signal::Signal(){}
	Signal::~Signal(){}
	void Signal::AcceptSignalValue(const double&signal){SendSignalValue(signal);}
	
	SignalPolinomialDistort::SignalPolinomialDistort(const Vec& coefs){
		if(coefs.size()==0)
			throw Exception<SignalPolinomialDistort>("Distortion: empty coefficients");
		m_coefs=coefs;
	}
	SignalPolinomialDistort::~SignalPolinomialDistort(){}
	void SignalPolinomialDistort::AcceptSignalValue(const double&signal){
	    if(m_coefs.size()==0){
		SendSignalValue(0);
	    }else{
		double res=m_coefs[0],p=1;
		for(size_t i=1;i<m_coefs.size();i++){
		    p*=signal;
		    res+=p*m_coefs[i];
		}
		SendSignalValue(res);
	    }
	}
	
	AmplitudeDiscriminator::AmplitudeDiscriminator(const double& thr){
		threshold=thr;
	}
	AmplitudeDiscriminator::~AmplitudeDiscriminator(){}
	void AmplitudeDiscriminator::AcceptSignalValue(const double& signal){
		if(isfinite(signal))
			if(signal>=threshold)
				SendSignalValue(signal);
	}
	
	
	AbstractMultiInput::AbstractMultiInput(){m_state=0;}
	AbstractMultiInput::~AbstractMultiInput(){}
	AbstractMultiInput& AbstractMultiInput::operator<<(const shared_ptr<SignalProducent> input){
		auto slot=make_shared<Slot>(shared_from_this());
		input>>slot;
		m_input_slots.push_back(slot);
		m_state=0;
		return *this;
	}
	void AbstractMultiInput::OneChannelBegin(){
		if(m_state<0)
			throw Exception<AbstractMultiInput>("MultiInput error: state changed to an invalid value.");
		if(m_state==0)
			Start();
		m_state++;
	}
	void AbstractMultiInput::OneChannelEnd(){
		m_state--;
		if(m_state==0){
			Vec values;
			for(auto slot:m_input_slots)
				values.push_back(slot->Value());
			Process(values);
			Finish();
		}
		if(m_state<0)
			throw Exception<AbstractMultiInput>("MultiInput error: state changed to an invalid value.");
	}
	AbstractMultiInput::Slot::Slot(std::shared_ptr<AbstractMultiInput>father){master=father;}
	AbstractMultiInput::Slot::~Slot(){}
	void AbstractMultiInput::Slot::AcceptEventStart(){master->OneChannelBegin();m_value=INFINITY;}
	void AbstractMultiInput::Slot::AcceptSignalValue(const double& signal){m_value=signal;}
	void AbstractMultiInput::Slot::AcceptEventEnd(){master->OneChannelEnd();}
	const double AbstractMultiInput::Slot::Value()const{return m_value;}
	
	Multi2SingleSignal::Multi2SingleSignal(){}
	Multi2SingleSignal::~Multi2SingleSignal(){}
	void Multi2SingleSignal::Start(){SendEventStart();}
	void Multi2SingleSignal::Finish(){SendEventEnd();}
	
	SignalSumm::SignalSumm(){}
	SignalSumm::~SignalSumm(){}
	void SignalSumm::Process(const Vec&signals){
		double val=0;
		for(double signal:signals)
		    if(isfinite(signal))
			val+=signal;
		    else
			return;
		SendSignalValue(val);
	}
	SignalProduct::SignalProduct(){}
	SignalProduct::~SignalProduct(){}
	void SignalProduct::Process(const Vec&signals){
		double val=1;
		for(double signal:signals)
		    if(isfinite(signal))
			val*=signal;
		    else
			return;
		SendSignalValue(val);
	}
	SignalSortAndSelect::SignalSortAndSelect(size_t number){m_number=number;}
	SignalSortAndSelect::~SignalSortAndSelect(){}
	void SignalSortAndSelect::Process(const Vec&signals){
		if(signals.size()<=m_number)
			throw Exception<SignalSortAndSelect>("SignalSortAndSelect: selected order statistics is greater than input slots count");
		Vec out;
		for(double v:signals)if(isfinite(v))
			table_data_details::InsertSorted(v,out,std_size(out),std_insert(out,double));
		if(out.size()>m_number)
			SendSignalValue(out[m_number]);
	}
	
	AbstractMultiOutput::AbstractMultiOutput(){}
	AbstractMultiOutput::~AbstractMultiOutput(){}
	AbstractMultiOutput& AbstractMultiOutput::operator>>(shared_ptr< SignalAcceptor > output){
		m_output_slots.push_back(make_shared<Slot>()>>output);
		return *this;
	}
	void AbstractMultiOutput::SendEventStart(){
		for(auto slot:m_output_slots)
			slot->Start();
	}
	const size_t AbstractMultiOutput::GetOutSlotsCount(){
		return m_output_slots.size();
	}
	void AbstractMultiOutput::SendSignalValue(const size_t i,const double& signal){
		if(i>=m_output_slots.size())
			throw Exception<AbstractMultiOutput>("Mutli output range check error.");
		m_output_slots[i]->Value(signal);
	}
	void AbstractMultiOutput::SendEventEnd(){
		for(auto slot:m_output_slots)
			slot->End();
	}
	AbstractMultiOutput::Slot::Slot(){}
	AbstractMultiOutput::Slot::~Slot(){}
	void AbstractMultiOutput::Slot::Start(){SendEventStart();}
	void AbstractMultiOutput::Slot::Value(const double& signal){SendSignalValue(signal);}
	void AbstractMultiOutput::Slot::End(){SendEventEnd();}
	
	Single2MultiSignal::Single2MultiSignal(){}
	Single2MultiSignal::~Single2MultiSignal(){}
	void Single2MultiSignal::AcceptEventStart(){SendEventStart();}
	void Single2MultiSignal::AcceptEventEnd(){SendEventEnd();}
	
	Multi2MultiSignal::Multi2MultiSignal(){}
	Multi2MultiSignal::~Multi2MultiSignal(){}
	void Multi2MultiSignal::Start(){SendEventStart();}
	void Multi2MultiSignal::Finish(){SendEventEnd();}
	
	TimeGate::TimeGate(const double& width){
		if(width<=0)
			throw Exception<TimeGate>("Wrong time threshold value for TimeGate: it should be positive");
		m_width=width;
	}
	TimeGate::~TimeGate(){}
	void TimeGate::Process(const Vec&signals){
		if(signals.size()>0){
			bool condition=true;
			for(double signal:signals)
				condition&=isfinite(signal);
			if(condition){
				double start=signals[0];
				for(double v:signals){
					if(v<start)return;
					if((v-start)>m_width)return;
				}
				for(size_t i=0;i<signals.size();i++)
					SendSignalValue(i,signals[i]);
			}
		}
	}
	void AllSignalsPresent::Process(const Vec&signals){
		bool condition=true;
		for(double signal:signals)
			condition&=isfinite(signal);
		if(condition)for(size_t i=0;i<signals.size();i++)
			SendSignalValue(i,signals[i]);
	}
	
	SignalSortAndSelect2::SignalSortAndSelect2(const size_t number){m_number=number;}
	SignalSortAndSelect2::~SignalSortAndSelect2(){}
	inline bool operator>(const Pair&a,const Pair&b){return a.first>b.first;}
	inline bool operator<(const Pair&a,const Pair&b){return a.first<b.first;}
	void SignalSortAndSelect2::Process(const Vec&signals){
		if(signals.size()<=m_number)
			throw Exception<SignalSortAndSelect2>("SignalSortAndSelect: selected order statistics is greater than input slots count");
		vector<Pair> out;
		for(size_t i=0,n=signals.size();i<n;i++)if(isfinite(signals[i])){
			table_data_details::InsertSorted(make_pair(signals[i],double(i)),out,std_size(out),std_insert(out,Pair));
		}
		if(out.size()>m_number){
			SendSignalValue(0,out[m_number].second);
			SendSignalValue(1,out[m_number].first);
		}
	}
	
	void SignalSort::Process(const Vec&signals){
		Vec out;
		for(double signal:signals)
		    if(isfinite(signal))
			table_data_details::InsertSorted(signal,out,std_size(out),std_insert(out,double));
		size_t i=0;
		for(double signal:out){
		    if(i<GetOutSlotsCount())
			SendSignalValue(i,signal);
		    i++;
		}
	}
	void SignalSort2::Process(const Vec&signals){
		vector<Pair> out;
		size_t i=0;
		for(double signal:signals){
			if(isfinite(signal))
				table_data_details::InsertSorted(make_pair(signal,double(i)),out,std_size(out),std_insert(out,Pair));
			i++;
		}
		i=0;
		for(const Pair&signal:out){
			if(i<(GetOutSlotsCount()/2)){
				SendSignalValue(i*2,signal.second);
				SendSignalValue(i*2+1,signal.first);
			}
			i++;
		}
	}
};
