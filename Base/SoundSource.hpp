#ifndef _SOUND_SOURCE_HPP
#define _SOUND_SOURCE_HPP

#include "ExitPoint.hpp"
#include <vector>

namespace BRTBase {

	class CSoundSource {
	public:
		CSoundSource(std::string _sourceID) : dataReady{ false }, sourceID{ _sourceID} {			
			samplesExitPoint = std::make_shared<CExitPointSamplesVector>("samples");
			sourcePositionExitPoint = std::make_shared<CExitPointTransform>("sourceTransform");
			sourceIDExitPoint = std::make_shared<CExitPointID>("sourceID");
			sourceIDExitPoint->sendData(sourceID);
		}

		void SetBuffer(CMonoBuffer<float>& _buffer) { 
			samplesBuffer = _buffer; 
			dataReady = true;
		}
		void SetDataReady() {
			if (!dataReady) { return; }
			samplesExitPoint->sendData(samplesBuffer); 
			dataReady = false;
		}

		void operator()() {
			samplesExitPoint->sendData(samplesBuffer);
		}

		void SetSourceTransform(Common::CTransform _transform) { 
			sourceTransform = _transform;
			sourcePositionExitPoint->sendData(sourceTransform);
		}
		
		const Common::CTransform& GetCurrentSourceTransform() const { return sourceTransform; };		
		
		std::string GetSourceID() { return sourceID; }

		std::shared_ptr<BRTBase::CExitPointSamplesVector> GetSamplesVectorExitPoint() {
			return samplesExitPoint;						
		}
	
		std::shared_ptr<BRTBase::CExitPointTransform> GetTransformExitPoint() {
			return sourcePositionExitPoint;			
		}

		std::shared_ptr<BRTBase::CExitPointID> GetIDExitPoint() {
			return sourceIDExitPoint;
		}

	private:
		std::string sourceID;
		bool dataReady;

		Common::CTransform sourceTransform;
		CMonoBuffer<float> samplesBuffer;
		
		std::shared_ptr<CExitPointSamplesVector> samplesExitPoint;
		std::shared_ptr<CExitPointTransform> sourcePositionExitPoint;
		std::shared_ptr<CExitPointID> sourceIDExitPoint;
	};
}
#endif