#ifndef _CLISTENER_MODEL_BASE_H_
#define _CLISTENER_MODEL_BASE_H_

#include <memory>
//#include "EntryPoint.hpp"
//#include "ExitPoint.hpp"
#include <Base/EntryPointManager.hpp>
#include <Base/CommandEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>

#include <Common/CommonDefinitions.h>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/ILD.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTBase {

	class CListenerModelBase: public CCommandEntryPointManager, public CExitPointManager, public CEntryPointManager {
	public:

		virtual ~CListenerModelBase() {}
		virtual void Update(std::string entryPointID) = 0;
		virtual void UpdateCommand() = 0;
		//virtual void ConnectSoundSource(std::shared_ptr<BRTBase::CSourceModelBase> _source, BRTBase::CBRTManager* brtManager) = 0;


		CListenerModelBase(std::string _listenerID) : listenerID{ _listenerID }, listenerHeadRadius{ DEFAULT_LISTENER_HEAD_RADIOUS }, leftDataReady{ false }, rightDataReady{false} {
			
			
			//listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Create a empty HRTF			

			// Create entry Points
			CreateSamplesEntryPoint("leftEar");
			CreateSamplesEntryPoint("rightEar");
			//leftEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListenerModelBase::updateFromEntryPoint, this, std::placeholders::_1), "leftEar", 1);
			//rightEarEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CListenerModelBase::updateFromEntryPoint, this, std::placeholders::_1), "rightEar", 1);
			
			// Create exit point
			CreateTransformExitPoint();
			//listenerPositionExitPoint	= std::make_shared<CExitPointTransform>("listenerTransform");
			//hrtfExitPoint				= std::make_shared<CExitPointHRTFPtr>("listenerHRTF");
			//ildExitPoint				= std::make_shared<CExitPointILDPtr>("listenerILD");
			//listenerIDExitPoint			= std::make_shared<CExitPointID>("listenerID");			
			CreateIDExitPoint();
			GetIDExitPoint()->sendData(listenerID);			
			
			//Send this listener ID
			//listenerIDExitPoint->sendData(listenerID);
			CreateCommandEntryPoint();
		}

		//void connectSamplesEntryTo(std::shared_ptr<CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
		//	if (entryPointID == "leftEar") { _exitPoint->attach(*leftEarEntryPoint.get()); }
		//	else if (entryPointID == "rightEar") { _exitPoint->attach(*rightEarEntryPoint.get()); }
		//	else { //TODO Notify error 
		//	}
		//}

		//void disconnectSamplesEntryFrom(std::shared_ptr<CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
		//	if (entryPointID == "leftEar") { _exitPoint->detach(leftEarEntryPoint.get()); }
		//	else if (entryPointID == "rightEar") { _exitPoint->detach(rightEarEntryPoint.get()); }
		//	else { //TODO Notify error 
		//	}
		//}

		/** \brief Set listener position and orientation
		*	\param [in] _listenerTransform new listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		void SetListenerTransform(Common::CTransform _transform) {
			listenerTransform = _transform;
			GetTransformExitPoint()->sendData(listenerTransform);	// Send to subscribers
			//listenerPositionExitPoint->sendData(listenerTransform);			// Send
		}

		/** \brief Get listener position and orientation
		*	\retval transform current listener position and orientation
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CTransform GetListenerTransform() { return listenerTransform; }

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

		/*std::shared_ptr<CExitPointTransform> GetTransformExitPoint() {
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
		}*/

		std::string GetID() { return listenerID; }

		//void ConnectSoundSource(BRTBase::CSourceModelBase _source) {

		//}

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

				
		// Update callback
		void updateFromEntryPoint(std::string id) {
			if (id == "leftEar") {
				UpdateLeftBuffer();
			}
			else if (id == "rightEar") {
				UpdateRightBuffer();
			}
		}
		void updateFromCommandEntryPoint(std::string entryPointID) {
			BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
		}

		///** \brief SET HRTF of listener
		//*	\param[in] pointer to HRTF to be stored
		//*   \eh On error, NO error code is reported to the error handler.
		//*/
		//void SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) {
		//	//listenerHRTF = std::move(_listenerHRTF);	
		//	listenerHRTF = _listenerHRTF;
		//	hrtfExitPoint->sendDataPtr(listenerHRTF);
		//}

		///** \brief Get HRTF of listener
		//*	\retval HRTF pointer to current listener HRTF
		//*   \eh On error, an error code is reported to the error handler.
		//*/		
		//std::shared_ptr < BRTServices::CHRTF> GetHRTF() const
		//{
		//	return listenerHRTF;
		//}

		///** \brief Remove the HRTF of thelistener
		//*   \eh Nothing is reported to the error handler.
		//*/
		//void RemoveHRTF() {
		//	listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// empty HRTF			
		//}

		/////
		///** \brief SET HRTF of listener
		//*	\param[in] pointer to HRTF to be stored
		//*   \eh On error, NO error code is reported to the error handler.
		//*/
		//void SetILD(std::shared_ptr< BRTServices::CILD > _listenerILD) {
		//	listenerILD = _listenerILD;
		//	ildExitPoint->sendDataPtr(listenerILD);			
		//}

		///** \brief Get HRTF of listener
		//*	\retval HRTF pointer to current listener HRTF
		//*   \eh On error, an error code is reported to the error handler.
		//*/
		//std::shared_ptr <BRTServices::CILD> GetILD() const
		//{
		//	return listenerILD;
		//}

		///** \brief Remove the HRTF of thelistener
		//*   \eh Nothing is reported to the error handler.
		//*/
		//void RemoveILD() {
		//	listenerILD = std::make_shared<BRTServices::CILD>();	// empty HRTF			
		//}

	


		///** \brief Get ILD Near Field effect of listener
		//*	\retval ILD pointer to current listener ILD Near Field effect
		//*   \eh Nothing is reported to the error handler.
		//*/
		//CILD* GetILD() const
		//{
		//	return listenerILD.get();
		//}



	private:
		std::string listenerID;								// Store unique listener ID
		//std::shared_ptr<BRTServices::CHRTF> listenerHRTF;	// HRTF of listener														
		//std::shared_ptr<BRTServices::CILD> listenerILD;		// ILD of listener	
		Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  
		float listenerHeadRadius;							// Head radius of listener 

		Common::CGlobalParameters globalParameters;

		/*std::shared_ptr<CEntryPointSamplesVector >		leftEarEntryPoint;
		std::shared_ptr<CEntryPointSamplesVector >		rightEarEntryPoint;				
		std::shared_ptr<CExitPointTransform>			listenerPositionExitPoint;
		std::shared_ptr<CExitPointHRTFPtr>				hrtfExitPoint;
		std::shared_ptr<CExitPointILDPtr>				ildExitPoint;
		std::shared_ptr<CExitPointID>					listenerIDExitPoint;*/

		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;	

		// Private Methods
		

		void UpdateLeftBuffer() {
			if (!leftDataReady) {
				leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}
			//CMonoBuffer<float> buffer = leftEarEntryPoint->GetData();
			CMonoBuffer<float> buffer = GetSamplesEntryPoint("leftEar")->GetData();
			if (buffer.size() != 0) {
				leftBuffer += buffer;
				leftDataReady = true;
			}
		}

		void UpdateRightBuffer() {
			if (!rightDataReady) {
				rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
			}
			//CMonoBuffer<float> buffer = rightEarEntryPoint->GetData();
			CMonoBuffer<float> buffer = GetSamplesEntryPoint("rightEar")->GetData();
			if (buffer.size() != 0) {
				rightBuffer += buffer;
				rightDataReady = true;
			}
		}		
	};
}
#endif