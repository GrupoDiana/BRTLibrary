#pragma once
#include "EntryPoint.hpp"
#include <memory>


namespace BRT_Base {

	class CListener {
	public:
		CListener(float _listenerHeadRadius = 0.0875f) {
			std::shared_ptr<CEntryPoint<CListener> > _newEntryPoint = std::make_shared<CEntryPoint<CListener> >(*this);
			entryPoint = _newEntryPoint;
			dataReady = false;
			listenerHeadRadius = _listenerHeadRadius;
		}

		void connectEntryTo(std::shared_ptr<CExitPoint> _exitPoint) {
			_exitPoint->attach(*entryPoint.get());
		}

		void updateFromEntryPoint() {
			std::cout << "EndPoint Updating --> Recibing buffer" << std::endl;
			std::vector<float> temp = entryPoint->GetBuffer();

			leftBuffer = temp;
			dataReady = true;
		}

		bool isDataReady() { return dataReady; }

		std::vector<float> GetBuffer() {
			dataReady = false;
			return leftBuffer;
		}

		void GetBuffers(std::vector<float>& _leftBuffer, std::vector<float>& _rightBuffer) {
			_leftBuffer = leftBuffer;
			_rightBuffer = rightBuffer;
		}

	private:
		
		//std::unique_ptr<CHRTF> listenerHRTF;		// HRTF of listener														
		//std::unique_ptr<CILD> listenerILD;			// ILD of listener	
		//Common::CTransform listenerTransform;		// Transform matrix (position and orientation) of listener  
		float listenerHeadRadius;					// Head radius of listener 

		std::shared_ptr<CEntryPoint<CListener> > entryPoint;

		std::vector<float> leftBuffer;
		std::vector<float> rightBuffer;
		bool dataReady;
	};
}