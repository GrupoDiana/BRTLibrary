#pragma once
#include "EntryPoint.hpp"
#include "ExitPoint.h"
#include <memory>


namespace BRTBase {

	class CListener {
	public:
		CListener(float _listenerHeadRadius = 0.0875f) {			
			
			leftEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListener::updateFromEntryPoint, this, std::placeholders::_1), "leftEar", 1);
			rightEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListener::updateFromEntryPoint, this, std::placeholders::_1), "rightEar", 1);
						
			listenerPositionExitPoint = std::make_shared<CExitPointTransform>("listenerTransform");

			leftDataReady = false;
			rightDataReady = false;			
			listenerHeadRadius = _listenerHeadRadius;
		}
		
		void connectSamplesEntryTo(std::shared_ptr<CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
			if (entryPointID == "leftEar")		 { _exitPoint->attach(*leftEarEntryPoint.get());	}
			else if (entryPointID == "rightEar") { _exitPoint->attach(*rightEarEntryPoint.get());	}
			else { //TODO Notify error 
			}
		}

		std::shared_ptr<CExitPointTransform> GetTransformExitPoint() {
			return listenerPositionExitPoint;
		}


		void updateFromEntryPoint(std::string id) {									
			if (id == "leftEar") { 
				leftBuffer = leftEarEntryPoint->getAttr();
				leftDataReady = true;
			
			} else if (id == "rightEar") { 
				rightBuffer = rightEarEntryPoint->getAttr();
				rightDataReady = true;
			}											
		}

		bool isDataReady() { return leftDataReady & rightDataReady; }

		void SetListenerTransform(Common::CTransform _transform) {
			listenerTransform = _transform;
			listenerPositionExitPoint->sendData(listenerTransform);
		}

		void GetBuffers(std::vector<float>& _leftBuffer, std::vector<float>& _rightBuffer) {
			_leftBuffer = leftBuffer;
			_rightBuffer = rightBuffer;
			leftDataReady = false;
			rightDataReady = false;

		}

	private:
		
		//std::unique_ptr<CHRTF> listenerHRTF;		// HRTF of listener														
		//std::unique_ptr<CILD> listenerILD;			// ILD of listener	
		Common::CTransform listenerTransform;		// Transform matrix (position and orientation) of listener  
		float listenerHeadRadius;					// Head radius of listener 

		std::shared_ptr<CEntryPointSamplesVector > leftEarEntryPoint;
		std::shared_ptr<CEntryPointSamplesVector > rightEarEntryPoint;				
		std::shared_ptr<CExitPointTransform> listenerPositionExitPoint;

		std::vector<float> leftBuffer;
		std::vector<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;		
	};
}