#pragma once

#include "ExitPoint.h"
#include <vector>

namespace BRTBase {

	class CSoundSource {
	public:
		CSoundSource() { 
			samplesExitPoint = std::make_shared<CExitPoint>("samples"); 
			sourcePositionExitPoint = std::make_shared<CExitPoint>("sourceTransform");
		}

		void SetBuffer(std::vector<float>& _buffer) { samplesBuffer = _buffer; }
		void SetDataReady() { samplesExitPoint->sendData(samplesBuffer); }

		void operator()() {
			samplesExitPoint->sendData(samplesBuffer);
		}

		//void SetSourceTransform(CTransform _sourceTransform);
		//const CTransform& GetCurrentSourceTransform() const;
		std::shared_ptr<CExitPoint> GetExitPoint(std::string id) { 
		
			if (id == "samples") { return samplesExitPoint; }
			else if (id == "sourceTransform") { return sourcePositionExitPoint; }
			else return nullptr;			 		
		}
	
	private:
		//CTransform sourceTransform;
		std::vector<float> samplesBuffer;
		
		std::shared_ptr<CExitPoint> samplesExitPoint;
		std::shared_ptr<CExitPoint> sourcePositionExitPoint;
	};

}