#pragma once

#include "ExitPoint.h"
#include <vector>

namespace BRTBase {

	class CSoundSource {
	public:
		CSoundSource() { 
			samplesExitPoint = std::make_shared<CExitPointSamplesVector>("samples");
			sourcePositionExitPoint = std::make_shared<CExitPointTransform>("sourceTransform");
		}

		void SetBuffer(CMonoBuffer<float>& _buffer) { samplesBuffer = _buffer; }
		void SetDataReady() { samplesExitPoint->sendData(samplesBuffer); }

		void operator()() {
			samplesExitPoint->sendData(samplesBuffer);
		}

		void SetSourceTransform(Common::CTransform _transform) { 
			sourceTransform = _transform;
			sourcePositionExitPoint->sendData(sourceTransform);
		}
		//void SetSourceTransform(CTransform _sourceTransform);
		//const CTransform& GetCurrentSourceTransform() const;

		
		std::shared_ptr<BRTBase::CExitPointSamplesVector> GetSamplesVectorExitPoint() {
			return samplesExitPoint;						
		}
	
		std::shared_ptr<BRTBase::CExitPointTransform> GetTransformExitPoint() {
			return sourcePositionExitPoint;			
		}

	private:
		Common::CTransform sourceTransform;
		CMonoBuffer<float> samplesBuffer;
		
		std::shared_ptr<CExitPointSamplesVector> samplesExitPoint;
		std::shared_ptr<CExitPointTransform> sourcePositionExitPoint;
	};

}