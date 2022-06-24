#pragma once
#include "EntryPoint.hpp"
#include <memory>


namespace BRT_Base {

	class CListener {
	public:
		CListener(float _listenerHeadRadius = 0.0875f) {
			std::shared_ptr<CEntryPoint<CListener> > _leftEntryPoint = std::make_shared<CEntryPoint<CListener> >(*this, "leftEar");
			leftEarEntryPoint = _leftEntryPoint;
									
			std::shared_ptr<CEntryPoint<CListener> > _rightEntryPoint = std::make_shared<CEntryPoint<CListener> >(*this, "rightEar");
			rightEarEntryPoint = _rightEntryPoint;

			leftDataReady = false;
			rightDataReady = false;			
			listenerHeadRadius = _listenerHeadRadius;
		}
		
		void connectEntryTo(std::shared_ptr<CExitPoint> _exitPoint, std::string entryPointID) {

			if (entryPointID == "leftEar")		 { _exitPoint->attach(*leftEarEntryPoint.get());	}
			else if (entryPointID == "rightEar") { _exitPoint->attach(*rightEarEntryPoint.get());	}
			else { //TODO Notify error 
			}
		}

		void updateFromEntryPoint(std::string id) {
			std::cout << "EndPoint Updating --> Recibing buffer" << std::endl;
			
			
			if (id == "leftEar") { 
				leftBuffer = leftEarEntryPoint->GetData(); 
				leftDataReady = true;
			
			} else if (id == "rightEar") { 
				rightBuffer = rightEarEntryPoint->GetData();
				rightDataReady = true;
			}											
		}

		bool isDataReady() { return leftDataReady & rightDataReady; }

		//std::vector<float> GetBuffer() {
		//	leftDataReady = false;
		//	return leftBuffer;
		//}

		void GetBuffers(std::vector<float>& _leftBuffer, std::vector<float>& _rightBuffer) {
			_leftBuffer = leftBuffer;
			_rightBuffer = rightBuffer;
			leftDataReady = false;
			rightDataReady = false;

		}

	private:
		
		//std::unique_ptr<CHRTF> listenerHRTF;		// HRTF of listener														
		//std::unique_ptr<CILD> listenerILD;			// ILD of listener	
		//Common::CTransform listenerTransform;		// Transform matrix (position and orientation) of listener  
		float listenerHeadRadius;					// Head radius of listener 

		std::shared_ptr<CEntryPoint<CListener> > leftEarEntryPoint;
		std::shared_ptr<CEntryPoint<CListener> > rightEarEntryPoint;

		std::vector<float> leftBuffer;
		std::vector<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;		
	};
}