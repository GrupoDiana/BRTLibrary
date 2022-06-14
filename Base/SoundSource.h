#pragma once

#include "ExitPoint.h"
#include <vector>

namespace BRT_Base {

	class CSoundSource {
	public:
		CSoundSource() { exitPoint = std::make_shared<CExitPoint>(); }

		void SetBuffer(std::vector<float>& _buffer) { samplesBuffer = _buffer; }
		void SetDataReady() { exitPoint->sendData(samplesBuffer); }

		void operator()() {
			exitPoint->sendData(samplesBuffer);
		}

		//void SetSourceTransform(CTransform _sourceTransform);
		//const CTransform& GetCurrentSourceTransform() const;
		std::shared_ptr<CExitPoint> GetExitPoint() { return exitPoint; }
	private:
		//CTransform sourceTransform;
		std::vector<float> samplesBuffer;

		std::shared_ptr<CExitPoint> exitPoint;
	};

}