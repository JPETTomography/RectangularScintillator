// this file is distributed under 
// MIT license
#ifndef BYeAoSgG
#define BYeAoSgG
#include <random>
#include "photon2signal.h"
namespace RectangularScintillator{
	class Single2SingleSignal:public SignalAcceptor,public SignalProducent{
		virtual void AcceptEventStart()override;
		virtual void AcceptEventEnd()override;
	};
	class Signal:public Single2SingleSignal{
	public:
		Signal();
		virtual ~Signal();
		virtual void AcceptSignalValue(const double&signal)override;
	};
	class SignalPolinomialDistort:public Single2SingleSignal{
	public:
		SignalPolinomialDistort(const Vec&coefs);
		virtual ~SignalPolinomialDistort();
		virtual void AcceptSignalValue(const double&signal)override;
	private:
		Vec m_coefs;
	};
	//There's a problem with transfering vector<smth>&& parameter as {val1,val2,...} to make_Shared template function
	inline const std::shared_ptr<SignalPolinomialDistort> PolynomDistort(const Vec&coefs){
		return std::shared_ptr<SignalPolinomialDistort>(new SignalPolinomialDistort(coefs));
	}
	inline const std::shared_ptr<SignalPolinomialDistort> SignalAdd(const double&v){return PolynomDistort({v,1});}
	inline const std::shared_ptr<SignalPolinomialDistort> SignalMultiply(const double&c){return PolynomDistort({0,c});}
	inline const std::shared_ptr<SignalPolinomialDistort> SignalInvert(){return SignalMultiply(-1);}
	class AmplitudeDiscriminator:public Single2SingleSignal{
	public:
		AmplitudeDiscriminator(const double&thr);
		virtual ~AmplitudeDiscriminator();
		virtual void AcceptSignalValue(const double&signal)override;
	private:
		double threshold;
	};
	class AbstractMultiInput:public std::enable_shared_from_this<AbstractMultiInput>{
		class Slot;
		friend class Slot;
	public:
		AbstractMultiInput();
		virtual ~AbstractMultiInput();
		AbstractMultiInput&operator<<(const std::shared_ptr<SignalProducent> input);
	protected:
		virtual void Start()=0;
		virtual void Process(const Vec&signals)=0;
		virtual void Finish()=0;
		void OneChannelBegin();
		void OneChannelEnd();
	private:
		class Slot:public SignalAcceptor{
		public:
			Slot(std::shared_ptr<AbstractMultiInput>father);
			virtual ~Slot();
			virtual void AcceptEventStart()override;
			virtual void AcceptSignalValue(const double&signal)override;
			virtual void AcceptEventEnd()override;
			const double Value()const;
		private:
			std::shared_ptr<AbstractMultiInput>master;
			double m_value;
		};
		std::vector<std::shared_ptr<Slot>> m_input_slots;
		int m_state;
	};
	class Multi2SingleSignal:public AbstractMultiInput,public SignalProducent{
	public:
		Multi2SingleSignal();
		virtual ~Multi2SingleSignal();
	protected:
		virtual void Start()override;
		virtual void Finish()override;
	};
	class SignalSumm:public Multi2SingleSignal{
	public:
		SignalSumm();
		virtual ~SignalSumm();
	protected:
		virtual void Process(const Vec&signals)override;
	};
	class SignalProduct:public Multi2SingleSignal{
	public:
		SignalProduct();
		virtual ~SignalProduct();
	protected:
		virtual void Process(const Vec&signals)override;
	};
	class SignalSortAndSelect:public Multi2SingleSignal{
	public:
		SignalSortAndSelect(const size_t number);
		virtual ~SignalSortAndSelect();
	protected:
		virtual void Process(const Vec&signals)override;
	private:
		size_t m_number;
	};
	class AbstractMultiOutput{
	public:
		AbstractMultiOutput();
		virtual ~AbstractMultiOutput();
		AbstractMultiOutput&operator>>(const std::shared_ptr<SignalAcceptor>output);
	protected:
		void SendEventStart();
		const size_t GetOutSlotsCount();
		void SendSignalValue(const size_t i,const double&signal);
		void SendEventEnd();
	private:
		class Slot:public SignalProducent{
		public:
			Slot();
			virtual ~Slot();
			void Start();
			void Value(const double&signal);
			void End();
		};
		std::vector<std::shared_ptr<Slot>> m_output_slots;
	};
	class Single2MultiSignal:public SignalAcceptor,public AbstractMultiOutput{
	public:
		Single2MultiSignal();
		virtual ~Single2MultiSignal();
		virtual void AcceptEventStart()override;
		//virtual void AcceptSignalValue(const double&time)=0;
		virtual void AcceptEventEnd()override;
	};
	class Multi2MultiSignal:public AbstractMultiInput,public AbstractMultiOutput{
	public:
		Multi2MultiSignal();
		virtual ~Multi2MultiSignal();
	protected:
		virtual void Start()override;
		virtual void Finish()override;
	};
	class TimeGate:public Multi2MultiSignal{
	public:
		TimeGate(const double&width);
		virtual ~TimeGate();
	protected:
		virtual void Process(const Vec&signals)override;
	private:
		double m_width;
	};
	class AllSignalsPresent:public Multi2MultiSignal{
	public:
		AllSignalsPresent(){}
		virtual ~AllSignalsPresent(){}
	protected:
		virtual void Process(const Vec&signals);
	};
	
	class SignalSortAndSelect2:public Multi2MultiSignal{
	public:
		SignalSortAndSelect2(const size_t number);
		virtual ~SignalSortAndSelect2();
	protected:
		virtual void Process(const Vec&signals)override;
	private:
		size_t m_number;
	};
	class SignalSort:public Multi2MultiSignal{
	public:
		SignalSort(){}
		virtual ~SignalSort(){}
	protected:
		virtual void Process(const Vec&signals)override;
	};
	class SignalSort2:public Multi2MultiSignal{
	public:
		SignalSort2(){}
		virtual ~SignalSort2(){}
	protected:
		virtual void Process(const Vec&signals)override;
	};
};
#endif 
