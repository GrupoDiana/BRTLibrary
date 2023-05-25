
#ifndef _SERVICE_INTERFACES_H_
#define _SERVICE_INTERFACES_H_

#include <cstdint>
//#include <ServiceModules/HRTF.h>
//#include <ServiceModules/ILD.hpp>

namespace BRTServices {

	/** \brief Type definition for a left-right pair of impulse response with the ITD removed and stored in a specific struct field
*/
	struct THRIRStruct {
		uint64_t leftDelay;				///< Left delay, in number of samples
		uint64_t rightDelay;			///< Right delay, in number of samples
		CMonoBuffer<float> leftHRIR;	///< Left impulse response data
		CMonoBuffer<float> rightHRIR;	///< Right impulse response data
	};

	struct TILDStruct {
		CMonoBuffer<float> leftCoefs;	///< Left filters coefs
		CMonoBuffer<float> rightCoefs;	///< Right filters coefs
	};

	struct TDirectivityTFStruct {
		CMonoBuffer<float> dataReal;	
		CMonoBuffer<float> dataImag;	
	};

	class CServicesBase {
		
	public:
		CServicesBase() {}
		virtual ~CServicesBase() {}		
		
		virtual void BeginSetup() {}
		virtual void BeginSetup(int32_t _SRTFLength) {}
		virtual void BeginSetup(int32_t _HRIRLength, float _distance) {}
		virtual bool EndSetup() = 0;

		virtual void SetResamplingStep(int _resamplingStep) {};
		virtual void SetTitle(std::string _title) = 0;
		virtual void SetDatabaseName(std::string _databaseName) =0;
		virtual void SetListenerShortName(std::string _listenerShortName) {};
		virtual void SetFilename(std::string _fileName) = 0;

		virtual void SetNumberOfEars(int _numberOfEars) {}
		virtual void SetEarPosition(Common::T_ear _ear, Common::CVector3 _earPosition) {};

		virtual void AddHRIR(float _azimuth, float _elevation, THRIRStruct&& newHRIR) {}
		virtual void AddCoefficients(float azimuth, float distance, TILDStruct&& newCoefs) {}
		virtual void AddDirectivityTF(float _azimuth, float _elevation, TDirectivityTFStruct&& DirectivityTF){}
		
	};}

#endif