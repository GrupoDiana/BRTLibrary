#pragma once
#include "EntryPoint.hpp"
#include "ExitPoint.h"
#include <Common/CommonDefinitions.h>
#include <Common/Vector3.h>
#include <memory>


namespace BRTBase {

	class CListener {
	public:
		CListener(float _listenerHeadRadius = 0.0875f) {			
			
			leftEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListener::updateFromEntryPoint, this, std::placeholders::_1), "leftEar", 1);
			rightEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListener::updateFromEntryPoint, this, std::placeholders::_1), "rightEar", 1);
						
			listenerPositionExitPoint		= std::make_shared<CExitPointTransform>("listenerTransform");
			listenerEarsPositionExitPoint	= std::make_shared<CExitPointEarsTransform>("listenerEarsTransform");

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

		std::shared_ptr<CExitPointEarsTransform> GetEarsTransformExitPoint() {
			return listenerEarsPositionExitPoint;
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

		/** \brief Set listener position and orientation
		*	\param [in] _listenerTransform new listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		void SetListenerTransform(Common::CTransform _transform) {
			listenerTransform = _transform;
			listenerPositionExitPoint->sendData(listenerTransform);			// Send

			// Calculate Ears Transform						
			listenerEarsTransforms.leftEarListenerTransform	= GetListenerEarTransform(Common::T_ear::LEFT);
			listenerEarsTransforms.rightEarListenerTransform	= GetListenerEarTransform(Common::T_ear::RIGHT);
			listenerEarsTransforms.leftEarListenerLocalPosition = GetListenerEarLocalPosition(Common::T_ear::LEFT);
			listenerEarsTransforms.rightEarListenerLocalPosition	= GetListenerEarLocalPosition(Common::T_ear::RIGHT);
			
			//Send
			listenerEarsPositionExitPoint->sendData(listenerEarsTransforms);
		}

		/** \brief Get listener position and orientation
		*	\retval transform current listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CTransform GetListenerTransform() { return listenerTransform; }


		/** \brief Get position and orientation of one listener ear
		*	\param [in] ear listener ear for wich we want to get transform
		*	\retval transform current listener ear position and orientation
		*   \eh On error, an error code is reported to the error handler.
		*/
		Common::CTransform GetListenerEarTransform(Common::T_ear ear) const
		{
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
				return Common::CTransform();
			}

			Common::CVector3 earLocalPosition = Common::CVector3::ZERO();
			if (ear == Common::T_ear::LEFT) {
				earLocalPosition.SetAxis(RIGHT_AXIS, -listenerHeadRadius);
			}
			else
				earLocalPosition.SetAxis(RIGHT_AXIS, listenerHeadRadius);

			return listenerTransform.GetLocalTranslation(earLocalPosition);
		}

		/** \brief Get EarPosition local to the listenerr
		*   \param [in] ear indicates the ear which you want to knowthe position
		*	\retval ear local position
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CVector3 GetListenerEarLocalPosition(Common::T_ear ear) const
		{
			if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
			{
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
				return Common::CVector3();
			}

			Common::CVector3 earLocalPosition = Common::CVector3::ZERO();
			if (ear == Common::T_ear::LEFT) {
				earLocalPosition.SetAxis(RIGHT_AXIS, -listenerHeadRadius);
			}
			else
				earLocalPosition.SetAxis(RIGHT_AXIS, listenerHeadRadius);


			return earLocalPosition;
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/		
		//CHRTF* GetHRTF() const
		//{
		//	return listenerHRTF.get();
		//}

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
		
		//std::unique_ptr<CHRTF> listenerHRTF;				// HRTF of listener														
		//std::unique_ptr<CILD> listenerILD;				// ILD of listener	
		Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  
		Common::CEarsTransforms listenerEarsTransforms;
		float listenerHeadRadius;							// Head radius of listener 

		std::shared_ptr<CEntryPointSamplesVector >		leftEarEntryPoint;
		std::shared_ptr<CEntryPointSamplesVector >		rightEarEntryPoint;				
		std::shared_ptr<CExitPointTransform>			listenerPositionExitPoint;
		std::shared_ptr<CExitPointEarsTransform>		listenerEarsPositionExitPoint;

		std::vector<float> leftBuffer;
		std::vector<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;		
	};
}