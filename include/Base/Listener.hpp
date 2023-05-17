#ifndef _CLISTENER_H_
#define _CLISTENER_H_

#include <memory>
#include "EntryPoint.hpp"
#include "ExitPoint.hpp"
#include "ExitPointPtr.hpp"
#include <Common/CommonDefinitions.h>
#include <ServiceModules/HRTF.h>
#include <ServiceModules/ILD.hpp>

namespace BRTServices {
	class CHRTF;
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
			listenerPositionExitPoint	= std::make_shared<CExitPointTransform>("listenerTransform");
			hrtfExitPoint				= std::make_shared<CExitPointHRTFPtr>("listenerHRTF");
			ildExitPoint				= std::make_shared<CExitPointILDPtr>("listenerILD");
			listenerIDExitPoint			= std::make_shared<CExitPointID>("listenerID");
			
			// Init vars
			leftDataReady = false;
			rightDataReady = false;
			//Send this listener ID
			listenerIDExitPoint->sendData(listenerID);
		}

		void connectSamplesEntryTo(std::shared_ptr<CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
			if (entryPointID == "leftEar") { _exitPoint->attach(*leftEarEntryPoint.get()); }
			else if (entryPointID == "rightEar") { _exitPoint->attach(*rightEarEntryPoint.get()); }
			else { //TODO Notify error 
			}
		}

		void disconnectSamplesEntryFrom(std::shared_ptr<CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
			if (entryPointID == "leftEar") { _exitPoint->detach(leftEarEntryPoint.get()); }
			else if (entryPointID == "rightEar") { _exitPoint->detach(rightEarEntryPoint.get()); }
			else { //TODO Notify error 
			}
		}

		std::shared_ptr<CExitPointTransform> GetTransformExitPoint() {
			return listenerPositionExitPoint;
		}

		std::shared_ptr<CExitPointHRTFPtr> GetHRTFPtrExitPoint(){
			return hrtfExitPoint;
		}

		std::shared_ptr<CExitPointILDPtr> GetILDPtrExitPoint() {
			return ildExitPoint;
		}
		
		std::shared_ptr<CExitPointID> GetIDExitPoint() {
			return listenerIDExitPoint;
		}

		void updateFromEntryPoint(std::string id) {											
			if (id == "leftEar") { 								
				UpdateLeftBuffer();
			} else if (id == "rightEar") { 								
				UpdateRightBuffer();
			}											
		}

		bool isDataReady() { return leftDataReady && rightDataReady; }
				
		void GetBuffers(CMonoBuffer<float>& _leftBuffer, CMonoBuffer<float>& _rightBuffer) {						
			if (leftDataReady) {
				_leftBuffer = leftBuffer;
				leftDataReady = false;
			}
			else {
				_leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}
			
			if (rightDataReady) {
				_rightBuffer = rightBuffer;
				rightDataReady = false;
			}
			else {
				_rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}

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


		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		void SetHRTF(std::shared_ptr< BRTServices::CHRTF >& _listenerHRTF) {
			//listenerHRTF = std::move(_listenerHRTF);	
			listenerHRTF = _listenerHRTF;
			hrtfExitPoint->sendData(listenerHRTF);
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/		
		std::shared_ptr < BRTServices::CHRTF> GetHRTF() const
		{
			return listenerHRTF;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRTF() {
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// empty HRTF			
		}

		///
		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		void SetILD(std::shared_ptr< BRTServices::CILD >& _listenerILD) {			
			listenerILD = _listenerILD;
			ildExitPoint->sendData(listenerILD);			
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr <BRTServices::CILD> GetILD() const
		{
			return listenerILD;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveILD() {
			listenerILD = std::make_shared<BRTServices::CILD>();	// empty HRTF			
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
		std::shared_ptr<BRTServices::CILD> listenerILD;		// ILD of listener	
		Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  
		float listenerHeadRadius;							// Head radius of listener 

		Common::CGlobalParameters globalParameters;

		std::shared_ptr<CEntryPointSamplesVector >		leftEarEntryPoint;
		std::shared_ptr<CEntryPointSamplesVector >		rightEarEntryPoint;				
		std::shared_ptr<CExitPointTransform>			listenerPositionExitPoint;
		std::shared_ptr<CExitPointHRTFPtr>				hrtfExitPoint;
		std::shared_ptr<CExitPointILDPtr>				ildExitPoint;
		std::shared_ptr<CExitPointID>					listenerIDExitPoint;

		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;	

		// Private Methods
		

		void UpdateLeftBuffer() {
			if (!leftDataReady) {
				leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}
			CMonoBuffer<float> buffer = leftEarEntryPoint->GetData();
			if (buffer.size() != 0) {
				leftBuffer += buffer;
				leftDataReady = true;
			}
		}

		void UpdateRightBuffer() {
			if (!rightDataReady) {
				rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}
			CMonoBuffer<float> buffer = rightEarEntryPoint->GetData();
			if (buffer.size() != 0) {
				rightBuffer += buffer;
				rightDataReady = true;
			}
		}		
	};
}
#endif