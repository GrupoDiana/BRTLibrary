#pragma once

#include "ExitPoint.h"
#include <vector>

namespace BRTBase {

	class CSoundSource {
	public:
		CSoundSource() { 
			samplesExitPoint = std::make_shared<CExitPointSamplesVector>("samples");
			sourcePositionExitPoint = std::make_shared<CExitPointInt>("sourceTransform");
		}

		void SetBuffer(std::vector<float>& _buffer) { samplesBuffer = _buffer; }
		void SetDataReady() { samplesExitPoint->sendData(samplesBuffer); }

		void operator()() {
			samplesExitPoint->sendData(samplesBuffer);
		}

		void SetSourceTransform(int a) { sourcePositionExitPoint->sendData(a); }
		//void SetSourceTransform(CTransform _sourceTransform);
		//const CTransform& GetCurrentSourceTransform() const;

		
		std::shared_ptr<BRTBase::CExitPointSamplesVector> GetSamplesVectorExitPoint() {
			return samplesExitPoint;						
		}
	
		std::shared_ptr<BRTBase::CExitPointInt> GetTransformExitPoint() {
			return sourcePositionExitPoint;			
		}

	private:
		//CTransform sourceTransform;
		std::vector<float> samplesBuffer;
		
		std::shared_ptr<CExitPointSamplesVector> samplesExitPoint;
		std::shared_ptr<CExitPointInt> sourcePositionExitPoint;
	};

}