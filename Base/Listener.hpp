#ifndef _CLISTENER_H_
#define _CLISTENER_H_

#include <memory>
#include "EntryPoint.hpp"
#include "ExitPoint.h"
#include "ExitPointPtr.hpp"
#include <Common/CommonDefinitions.h>
#include "ServiceModules/HRTF.h"

namespace BRTServices {
	class CHRTF;/* {
	public:
		void SetListener(BRTBase::CListener* _ownerListener);
	};*/
}

namespace BRTBase {

	class CListener {
	public:
		CListener(std::string _listenerID) : listenerHeadRadius{ DEFAULT_LISTENER_HEAD_RADIOUS } {
			
			listenerID = _listenerID;								// Save listenerID						
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Create a empty HRTF			

			// Create entry Points
			leftEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListener::updateFromEntryPoint, this, std::placeholders::_1), "leftEar", 1);
			rightEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListener::updateFromEntryPoint, this, std::placeholders::_1), "rightEar", 1);
			// Create exit point
			listenerPositionExitPoint = std::make_shared<CExitPointTransform>("listenerTransform");
			hrtfExitPoint = std::make_shared<CExitPointHRTFPtr>("listenerHRTF");
			// Init vars
			leftDataReady = false;
			rightDataReady = false;
		}

		void connectSamplesEntryTo(std::shared_ptr<CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
			if (entryPointID == "leftEar") { _exitPoint->attach(*leftEarEntryPoint.get()); }
			else if (entryPointID == "rightEar") { _exitPoint->attach(*rightEarEntryPoint.get()); }
			else { //TODO Notify error 
			}
		}

		std::shared_ptr<CExitPointTransform> GetTransformExitPoint() {
			return listenerPositionExitPoint;
		}


		std::shared_ptr<CExitPointHRTFPtr> GetHRTFPtrExitPoint(){
			return hrtfExitPoint;
		}

		void updateFromEntryPoint(std::string id) {									
			if (id == "leftEar") { 
				leftBuffer = leftEarEntryPoint->GetData();
				leftDataReady = true;
			
			} else if (id == "rightEar") { 
				rightBuffer = rightEarEntryPoint->GetData();
				rightDataReady = true;
			}											
		}

		bool isDataReady() { return leftDataReady && rightDataReady; }
				
		void GetBuffers(std::vector<float>& _leftBuffer, std::vector<float>& _rightBuffer) {
			_leftBuffer = leftBuffer;
			_rightBuffer = rightBuffer;
			leftDataReady = false;
			rightDataReady = false;
		}

		std::string GetListenerID() { return listenerID; }

		/** \brief Set listener position and orientation
		*	\param [in] _listenerTransform new listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		void SetListenerTransform(Common::CTransform _transform) {
			listenerTransform = _transform;
			listenerPositionExitPoint->sendData(listenerTransform);			// Send
		}

		/** \brief Get listener position and orientation
		*	\retval transform current listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CTransform GetListenerTransform() { return listenerTransform; }


		void SetHRTF(std::shared_ptr< BRTServices::CHRTF >& _listenerHRTF) {
			//listenerHRTF = std::move(_listenerHRTF);	
			listenerHRTF = _listenerHRTF;
			hrtfExitPoint->sendData(listenerHRTF);
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/		
		BRTServices::CHRTF* GetHRTF() const
		{
			return listenerHRTF.get();
		}

		///** \brief Get ILD Near Field effect of listener
		//*	\retval ILD pointer to current listener ILD Near Field effect
		//*   \eh Nothing is reported to the error handler.
		//*/
		//CILD* GetILD() const
		//{
		//	return listenerILD.get();
		//}

		/** \brief Set head radius of listener
		*	\param [in] _listenerHeadRadius new listener head radius, in meters
		*   \eh Nothing is reported to the error handler.
		*/
		void SetHeadRadius(float _listenerHeadRadius)
		{
			listenerHeadRadius = _listenerHeadRadius;
		}

		/** \brief Get head radius of listener
		*	\retval radius current listener head radius, in meters
		*   \eh Nothing is reported to the error handler.
		*/
		float GetHeadRadius() const {
			return listenerHeadRadius;
		}

	private:
		std::string listenerID;								// Store unique listener ID
		std::shared_ptr<BRTServices::CHRTF> listenerHRTF;	// HRTF of listener														
		//std::unique_ptr<CILD> listenerILD;				// ILD of listener	
		Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  
		float listenerHeadRadius;							// Head radius of listener 

		std::shared_ptr<CEntryPointSamplesVector >		leftEarEntryPoint;
		std::shared_ptr<CEntryPointSamplesVector >		rightEarEntryPoint;				
		std::shared_ptr<CExitPointTransform>			listenerPositionExitPoint;
		std::shared_ptr<CExitPointHRTFPtr>				hrtfExitPoint;

		std::vector<float> leftBuffer;
		std::vector<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;	

		//friend class BRTServices::CHRTF;							//Friend Class definition
	};
}
#endif