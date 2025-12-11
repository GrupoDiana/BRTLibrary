
#include <memory>
#include <string>
#include <vector>
#include <Common/CommonDefinitions.hpp>
#include <Common/Buffer.hpp>
#include <ServiceModules/SOSCoefficients.hpp>
#include <ServiceModules/GeneralFIR.hpp>

#ifndef _FILTER_BASE_HPP
#define _FILTER_BASE_HPP

namespace BRTFilters { 
	
	enum TFilterType { 
		FIR = 0, 
		IIR_SOS_SOFA = 1,
		IIR_CASCADE_GRAPHIC_EQ = 2,
		IIR_LP = 3,
		IIR_GAMMATONE = 4
	};

	class CFilterBase 
	{
	public:
		CFilterBase(TFilterType _filterType)
			: filterType { _filterType }
			, enable { false }
			, generalGain { 1.0f } { }
		
		// Public Methods		
		TFilterType GetFilterType() { return filterType; }
		
		// Virtual methods
		virtual void Enable() { enable = true; };
		virtual void Disable() { enable = false; };
		virtual bool IsEnabled() { return enable; }
		void SetGeneralGain(float _gain) { generalGain = _gain; }
		float GetGeneralGain() { return generalGain; }
		
		virtual bool Setup(int _numberOfChannels) { return false; }
		virtual bool Setup(int _numberOfChannels, int _numberOfBiquadSectionsPerChannel) { return false; }
		virtual bool SetCommandGains(const std::vector<float> & gains) { return false; }
		virtual bool SetCoefficients(const int & _channel, const std::vector<float> & _coefficients) { return false; }
		virtual bool SetFIRTable(std::shared_ptr<BRTServices::CGeneralFIR> _firTable) { return false; }

		virtual void Process(CMonoBuffer<float> & buffer) { }	
		virtual void Process(const CMonoBuffer<float> & buffer, CMonoBuffer<float> & output) { } 
		
		virtual void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, const int & _channel) { }
		virtual void Process(const CMonoBuffer<float> & _inLeftBuffer, CMonoBuffer<float> & _outLeftBuffer, const CMonoBuffer<float> & _inRightBuffer, CMonoBuffer<float> & _outRightBuffer) { }
		
		virtual void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, const int & _channel, const Common::CTransform & sourceTransform, const Common::CTransform & listenerTransform) { }		
		virtual void Process(const CMonoBuffer<float> & _inLeftBuffer, CMonoBuffer<float> & _outLeftBuffer, const CMonoBuffer<float> & _inRightBuffer, CMonoBuffer<float> & _outRightBuffer, const Common::CTransform & sourceTransform, const Common::CTransform & listenerTransform) { }
		
		virtual void Process(const int & _channel, const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer
			, const Common::CTransform & sourceTransform, const Common::CTransform & listenerTransform
			, const Common::T_ear _ear, std::weak_ptr<BRTServices::CServicesBase> & _irTableWeakPtr) { }

		virtual void ResetBuffers() { }

	private:
		TFilterType filterType;
			
	protected:
		bool enable;		// Enable or disable the filter
		float generalGain; // Output audio samples gain
	};
}
#endif