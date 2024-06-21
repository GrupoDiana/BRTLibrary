/**
* \class CListener
*
* \brief Declaration of CListener class
* \date	June 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _CLISTENER_H_
#define _CLISTENER_H_

#include <memory>
#include <Base/EntryPointManager.hpp>
#include <Base/CommandEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>
#include <Base/ListenerModelBase.hpp>
#include <ListenerModels/ListenerHRTFModel.hpp>
#include <Base/ListenerBase.hpp>
#include <Base/BRTManager.hpp>
#include <Common/CommonDefinitions.hpp>
//#include <ServiceModules/HRTF.hpp>
//#include <ServiceModules/NFCFilters.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTBase {
	//class BRTManager;

	class CListener: public CListenerBase {
	public:
		
		CListener(std::string _listenerID, CBRTManager* _brtManager) : CListenerBase { _listenerID }, brtManager { _brtManager } {
												
			/*CreateSamplesEntryPoint("leftEar");
			CreateSamplesEntryPoint("rightEar");									
			CreateTransformExitPoint();			
			CreateIDExitPoint();
			GetIDExitPoint()->sendData(listenerID);						
			CreateCommandEntryPoint();*/			
		}
				

		///** \brief Set listener position and orientation
		//*	\param [in] _listenerTransform new listener position and orientation
		//*   \eh Nothing is reported to the error handler.
		//*/
		//void SetListenerTransform(Common::CTransform _transform) {
		//	listenerTransform = _transform;
		//	GetTransformExitPoint()->sendData(listenerTransform);	// Send to subscribers			
		//}

		///** \brief Get listener position and orientation
		//*	\retval transform current listener position and orientation
		//*   \eh Nothing is reported to the error handler.
		//*/
		//Common::CTransform GetListenerTransform() { return listenerTransform; }

		///**
		// * @brief Get listener ID
		// * @return Return listener identificator
		//*/
		//std::string GetID() { return listenerID; }		
		//					
		///**
		// * @brief Get output sample buffers from the listener
		// * @param _leftBuffer Left ear sample buffer
		// * @param _rightBuffer Right ear sample buffer
		//*/
		//void GetBuffers(CMonoBuffer<float>& _leftBuffer, CMonoBuffer<float>& _rightBuffer) {						
		//	if (leftDataReady) {
		//		_leftBuffer = leftBuffer;
		//		leftDataReady = false;
		//	}
		//	else {
		//		_leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		//	}
		//	
		//	if (rightDataReady) {
		//		_rightBuffer = rightBuffer;
		//		rightDataReady = false;
		//	}
		//	else {
		//		_rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		//	}

		//}

		///////////////////////		
		//// Update Callbacks
		///////////////////////
		//void UpdateEntryPointData(std::string id) {
		//	if (id == "leftEar") {
		//		UpdateLeftBuffer();
		//	}
		//	else if (id == "rightEar") {
		//		UpdateRightBuffer();
		//	}
		//}
		//
		//void updateFromCommandEntryPoint(std::string entryPointID) {
		//	BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
		//	if (!_command.isNull()) {
		//		//UpdateCommand();
		//	}
		//}
		//		

		/**
		 * @brief Connect listener model
		 * @param _listener Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectListenerModel(std::shared_ptr<CListenerModelBase> _listenerModel) {
			
			bool control;
			control = brtManager->ConnectModuleID(this, _listenerModel, "listenerID");
			control = control && brtManager->ConnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			//return ConnectAnySoundSource(_source, false);
			
			listenerModelsConnected.push_back(_listenerModel);
			return true;
		};

		bool ConnectListenerModel(const std::string& _listenerModelID) {
			
			std::shared_ptr<CListenerModelBase> _listenerModel = brtManager->GetListenerModel<CListenerModelBase>(_listenerModelID);
			if (_listenerModel == nullptr) return false;
			//TODO CHECK IF IT IS ALREADY CONNECTED
			bool control;
			control = brtManager->ConnectModuleID(this, _listenerModel, "listenerID");						
			control = control && brtManager->ConnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");			

			listenerModelsConnected.push_back(_listenerModel);
			return control;
		};

		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) {
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelType() == TListenerType::ListenerHRFTModel) {
					return _listenerModel->SetHRTF(_listenerHRTF);
				}				
			}			
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr < BRTServices::CHRTF> GetHRTF() const
		{
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelType() == TListenerType::ListenerHRFTModel) {
					return _listenerModel->GetHRTF();
				}
			}			
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRTF() {
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelType() == TListenerType::ListenerHRFTModel) {
					_listenerModel->RemoveHRTF();
				}
			}						
		}

		//** \brief Get the flag for HRTF spatialization
		//*	\retval IsSpatializationEnabled if true, run-time HRTF spatialization is enabled for this source
		//*   \eh Nothing is reported to the error handler.
		//*/
		bool IsSpatializationEnabled() { 
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelType() == TListenerType::ListenerHRFTModel) {					
					std::shared_ptr<BRTListenerModel::CListenerHRTFModel> listenerModel;
					listenerModel = std::dynamic_pointer_cast<BRTListenerModel::CListenerHRTFModel>(it);
					return listenerModel->IsSpatializationEnabled();
				}
			}			
		}
		//** \brief Get the flag for run-time HRTF interpolation
		//*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		//*   \eh Nothing is reported to the error handler.
		//*/
		bool IsInterpolationEnabled() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelType() == TListenerType::ListenerHRFTModel) {
					std::shared_ptr<BRTListenerModel::CListenerHRTFModel> listenerModel;
					listenerModel = std::dynamic_pointer_cast<BRTListenerModel::CListenerHRTFModel>(it);
					return listenerModel->IsInterpolationEnabled();
				}
			}
		}

	private:
				
		//////////////////////////
		// Private Methods
		/////////////////////////
		//
		///**
		// * @brief Mix the new buffer received for the left ear with the contents of the buffer.
		//*/
		//void UpdateLeftBuffer() {
		//	if (!leftDataReady) {
		//		leftBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		//	}			
		//	CMonoBuffer<float> buffer = GetSamplesEntryPoint("leftEar")->GetData();
		//	if (buffer.size() != 0) {
		//		leftBuffer += buffer;
		//		leftDataReady = true;
		//	}
		//}
		///**
		// * @brief Mix the new buffer received for the right ear with the contents of the buffer.
		//*/
		//void UpdateRightBuffer() {
		//	if (!rightDataReady) {
		//		rightBuffer = CMonoBuffer<float>(globalParameters.GetBufferSize());
		//	}			
		//	CMonoBuffer<float> buffer = GetSamplesEntryPoint("rightEar")->GetData();
		//	if (buffer.size() != 0) {
		//		rightBuffer += buffer;
		//		rightDataReady = true;
		//	}
		//}		


		//////////////////////////
		// Private Attributes
		/////////////////////////
		CBRTManager* brtManager;							// Pointer to the BRT Manager	
		//std::string listenerID;								// Store unique listener ID		
		//Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  

		std::vector<std::shared_ptr<CListenerModelBase>> listenerModelsConnected;		// Listener models connected to the listener

		/*Common::CGlobalParameters globalParameters;
		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;

		bool leftDataReady;
		bool rightDataReady;*/
	};
}
#endif