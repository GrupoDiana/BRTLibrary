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
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _CLISTENER_H_
#define _CLISTENER_H_

#include <memory>
#include <Connectivity/EntryPointManager.hpp>
#include <Connectivity/CommandEntryPointManager.hpp>
#include <Connectivity/ExitPointManager.hpp>
#include <ListenerModels/ListenerModelBase.hpp>
#include <EnvironmentModels/EnvironmentModelBase.hpp>
#include <ListenerModels/ListenerHRTFModel.hpp>
#include <Base/ListenerBase.hpp>
#include <Base/BRTManager.hpp>
#include <Common/CommonDefinitions.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTBase {
	//class BRTManager;

	class CListener: public CListenerBase {
	public:
		
		CListener(std::string _listenerID, CBRTManager* _brtManager) : CListenerBase { _listenerID }, brtManager { _brtManager } {
																
		}
		
		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectListenerModel(const std::string & _listenerModelID, Common::T_ear _ear = Common::T_ear::BOTH) {

			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(_listenerModelID);
			if (_listenerModel == nullptr) return false;

			return ConnectListenerModel(_listenerModel, _ear);
		};

		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @param _ear Ear to connect, both by default
		 * @return True if the connection success
		*/		
		bool ConnectListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel, Common::T_ear _ear = Common::T_ear::BOTH) {
			
			if (_listenerModel == nullptr) return false;
			if (_listenerModel->IsAlreadyConnected()) return false;
			
			bool control;
			control = brtManager->ConnectModuleID(this, _listenerModel, "listenerID");
			_listenerModel->ConnectListenerTransform(GetID());					


			if (_ear == Common::T_ear::LEFT) {
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
			}
			else if (_ear == Common::T_ear::RIGHT) {
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			}
			else if (_ear == Common::T_ear::BOTH){
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			}
			else {
				return false;
			}
									
			AddListenerModelConnected(_listenerModel);
			SendMyID();			
			return control;
		};

		/**
		 * @brief Disconnect listener model to this listener
		 * @param _listener Pointer to the source
		 * @return True if the disconnection success
		*/
		bool DisconnectListenerModel(const std::string & _listenerModelID, Common::T_ear _ear = Common::T_ear::BOTH) {

			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(_listenerModelID);
			if (_listenerModel == nullptr) return false;

			return DisconnectListenerModel(_listenerModel, _ear);
		};

		/**
		 * @brief Disconnect listener model to this listener
		 * @param _listener Pointer to the source
		 * @param _ear Ear to disconnect, both by default
		 * @return True if the disconnection success
		*/
		bool DisconnectListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel, Common::T_ear _ear = Common::T_ear::BOTH) {

			if (_listenerModel == nullptr) return false;
			if (_listenerModel->IsAlreadyConnected()) { return false; };

			bool control;
			control = RemoveListenerModelConnected(_listenerModel);

			control =  control && brtManager->DisconnectModuleID(this, _listenerModel, "listenerID");
			_listenerModel->DisconnectListenerTransform(GetID());


			if (_ear == Common::T_ear::LEFT) {
				control = control && brtManager->DisconnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
			}
			else if (_ear == Common::T_ear::RIGHT) {
				control = control && brtManager->DisconnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			}
			else if (_ear == Common::T_ear::BOTH) {
				control = control && brtManager->DisconnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			}
			else {
				return false;
			}
						
			return control;
		};


		/**
		 * @brief Add listener model to the list of connected listener models
		 * @param _listenerModel listener model to add	
		 */
		void AddListenerModelConnected(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {
			listenerModelsConnected.push_back(_listenerModel);
		}

		/**
		 * @brief Remove listener model from the list of connected listener models
		 * @param _listenerModel listener model to remove	
		 */
		bool RemoveListenerModelConnected(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {			
			auto it = std::find(listenerModelsConnected.begin(), listenerModelsConnected.end(), _listenerModel);
			if (it != listenerModelsConnected.end()) {				
				listenerModelsConnected.erase(it);
				return true;
			}
			return false;
		}

		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectBinauralFilter(const std::string & _binauralFilterID, Common::T_ear _ear = Common::T_ear::BOTH) {

			std::shared_ptr<BRTBinauralFilter::CBinauralFilterBase> _binauralFilter = brtManager->GetBinauralFilter<BRTBinauralFilter::CBinauralFilterBase>(_binauralFilterID);
			if (_binauralFilter == nullptr) return false;

			return ConnectBinauralFilter(_binauralFilter, _ear);
		};

		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @param _ear Ear to connect, both by default
		 * @return True if the connection success
		*/
		bool ConnectBinauralFilter(std::shared_ptr<BRTBinauralFilter::CBinauralFilterBase> _binauralFilter, Common::T_ear _ear = Common::T_ear::BOTH) {

			if (_binauralFilter == nullptr) return false;
			if (_binauralFilter->IsConnectedToListener()) {
				return false;
			};

			bool control;
			control = brtManager->ConnectModuleID(this, _binauralFilter, "listenerID");
			//_binauralFilterID->ConnectListenerTransform(GetID());

			if (_ear == Common::T_ear::LEFT) {
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "leftEar", this, "leftEar");
			} else if (_ear == Common::T_ear::RIGHT) {
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "rightEar", this, "rightEar");
			} else if (_ear == Common::T_ear::BOTH) {
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "leftEar", this, "leftEar");
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "rightEar", this, "rightEar");
			} else {
				return false;
			}

			binauralFiltersConnected.push_back(_binauralFilter);
			SendMyID();
			return control;
		};


		/**
		 * @brief Disconnect listener model to this listener
		 * @param _listener Pointer to the source
		 * @return True if the connection success
		*/
		bool DisconnectBinauralFilter(const std::string & _binauralFilterID, Common::T_ear _ear = Common::T_ear::BOTH) {

			std::shared_ptr<BRTBinauralFilter::CBinauralFilterBase> _binauralFilter = brtManager->GetBinauralFilter<BRTBinauralFilter::CBinauralFilterBase>(_binauralFilterID);
			if (_binauralFilter == nullptr) return false;

			return DisconnectBinauralFilter(_binauralFilter, _ear);
		};

		/**
		 * @brief Disconnect listener model to this listener
		 * @param _listener Pointer to the source
		 * @param _ear Ear to connect, both by default
		 * @return True if the connection success
		*/
		bool DisconnectBinauralFilter(std::shared_ptr<BRTBinauralFilter::CBinauralFilterBase> _binauralFilter, Common::T_ear _ear = Common::T_ear::BOTH) {

			if (_binauralFilter == nullptr) return false;			

			bool control;
			control = RemoveBinauralFiltersConnected(_binauralFilter);
					
			if (_ear == Common::T_ear::LEFT) {
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "leftEar", this, "leftEar");
			} else if (_ear == Common::T_ear::RIGHT) {
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "rightEar", this, "rightEar");
			} else if (_ear == Common::T_ear::BOTH) {
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "leftEar", this, "leftEar");
				control = control && brtManager->ConnectModulesSamples(_binauralFilter, "rightEar", this, "rightEar");
			} else {
				return false;
			}

			control = brtManager->DisconnectModuleID(this, _binauralFilter, "listenerID");			
			return control;
		};


		/**
		 * @brief Remove listener model from the list of connected listener models
		 * @param _listenerModel listener model to remove	
		 */
		bool RemoveBinauralFiltersConnected(std::shared_ptr<BRTBinauralFilter::CBinauralFilterBase> _binauralFilter) {
			auto it = std::find(binauralFiltersConnected.begin(), binauralFiltersConnected.end(), _binauralFilter);
			if (it != binauralFiltersConnected.end()) {
				binauralFiltersConnected.erase(it);
				return true;
			}
			return false;
		} 

		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) {
			
			bool control = false;
			for (auto& _listenerModel : listenerModelsConnected) {
				bool result = _listenerModel->SetHRTF(_listenerHRTF);
				if (result) {
					control = true;
				}
				else {
					SET_RESULT(RESULT_ERROR_NOTSET, "ERROR: Unknown error when trying to set the HRTF in the listener model with ID " + _listenerModel->GetModelID());
				}				
			}
			return control;
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr < BRTServices::CHRTF> GetHRTF() const
		{
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportHRTF()) {
					return _listenerModel->GetHRTF();				
				}
			}			
			return nullptr;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		bool RemoveHRTF() {			
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportHRTF()) {
					_listenerModel->RemoveHRTF();	
					return true;
				}
			}	
			return false;
		}

		/** \brief SET HRBRIR of listener
		*	\param[in] pointer to HRBRIR to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRBRIR(std::shared_ptr< BRTServices::CHRBRIR > _listenerBRIR) {			
			
			bool control = false;
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportBRIR()) {					
					bool result = _listenerModel->SetHRBRIR(_listenerBRIR);
					if (result) {
						control = true;
					}
					else {
						SET_RESULT(RESULT_ERROR_NOTSET, "ERROR: Unknown error when trying to set the BRIR in the listener model with ID " + _listenerModel->GetModelID());
					}
				}
			}
			return control;
		}

		/** \brief Get HRBRIR of listener
		*	\retval HRBRIR pointer to current listener HRBRIR
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr < BRTServices::CHRBRIR> GetHRBRIR() const
		{
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportBRIR()) {
					return _listenerModel->GetHRBRIR();
				}
			}
			return nullptr;
		}

		/** \brief Remove the HRBRIR of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		bool RemoveHRBRIR() {
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportBRIR()) {
					_listenerModel->RemoveHRBRIR();
					return true;
				}			
			}
			return false;
		}

		/** \brief SET NFCFilters of listener
		*	\param[in] pointer to NFCFilters to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetSOSFilter(std::shared_ptr<BRTServices::CSOSFilters> _SOSFilter) {

			for (auto & _binauralFilter : binauralFiltersConnected) {
				//if (_binauralFilter->GetListenerModelCharacteristics().SupportNearFieldCompensation()) {
				return _binauralFilter->SetSOSFilter(_SOSFilter);
				//}
			}
			return false;
		}


		/** \brief SET NFCFilters of listener
		*	\param[in] pointer to NFCFilters to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetNearFieldCompensationFilters(std::shared_ptr< BRTServices::CSOSFilters > _listenerNFC) {
			
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportNearFieldCompensation()) {
					return _listenerModel->SetNearFieldCompensationFilters(_listenerNFC);
				}				
			}
			return false;						
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr <BRTServices::CSOSFilters> GetNearFieldCompensationFilters() const
		{
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportNearFieldCompensation()) {
					return _listenerModel->GetNearFieldCompensationFilters();
				}
			}
			return nullptr;			
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveNearFierldCompensationFilters() {
			for (auto& _listenerModel : listenerModelsConnected) {
				if (_listenerModel->GetListenerModelCharacteristics().SupportNearFieldCompensation()) {
					_listenerModel->RemoveNearFierldCompensationFilters();
				}
			}
		}

		void EnableSpatialization(){			
			for (auto& it : listenerModelsConnected) {				
				it->EnableSpatialization();								
			}			
		}

		void DisableSpatialization() {			 
			 for (auto& it : listenerModelsConnected) {
				 it->DisableSpatialization();			
			 }			
		}

		//** \brief Get the flag for HRTF spatialization
		//*	\retval IsSpatializationEnabled if true, run-time HRTF spatialization is enabled for this source
		//*   \eh Nothing is reported to the error handler.
		//*/
		bool IsSpatializationEnabled() { 
			bool control = false;
			for (auto& it : listenerModelsConnected) {				
				if (it->IsSpatializationEnabled()) 
				{
					control = true;
				}									
			}
			return control;
		}

		void EnableInterpolation() {
			for (auto& it : listenerModelsConnected) {
				it->EnableInterpolation();
			}
		}

		void DisableInterpolation() {
			for (auto& it : listenerModelsConnected) {
				it->DisableInterpolation();
			}
		}

		//** \brief Get the flag for run-time HRTF interpolation
		//*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		//*   \eh Nothing is reported to the error handler.
		//*/
		bool IsInterpolationEnabled() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportConfigurableInterpolation()) {					
					return it->IsInterpolationEnabled();					
				}
			}
			return false;
		}

		void EnableITDSimulation() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportITDSimulation()) {
					it->EnableITDSimulation();
				}
			}
		}

		void DisableITDSimulation() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportITDSimulation()) {
					it->DisableITDSimulation();
				}
			}
		}

		/**
		 * @brief 
		 * @return 
		 */
		bool IsITDSimulationEnabled() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportITDSimulation()) {
					return it->IsITDSimulationEnabled();
				}
			}
			return false;
		}

		void EnableNearFieldEffect() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportNearFieldCompensation()) {
					it->EnableNearFieldEffect();
				}
			}
		}

		void DisableNearFieldEffect() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportNearFieldCompensation()) {
					it->DisableNearFieldEffect();
				}
			}
		}

		/**
		 * @brief 
		 * @return 
		 */
		bool IsNearFieldEffectEnabled() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportNearFieldCompensation()) {
					return it->IsNearFieldEffectEnabled();
				}
			}
			return false;
		}

		void EnableParallaxCorrection() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportParallaxCorrection()) {
					it->EnableParallaxCorrection();
				}
			}
		}

		void DisableParallaxCorrection() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportParallaxCorrection()) {
					it->DisableParallaxCorrection();
				}
			}
		}

		/**
		 * @brief 
		 * @return 
		 */
		bool IsParallaxCorrectionEnabled() {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().SupportParallaxCorrection()) {
					return it->IsParallaxCorrectionEnabled();
				}
			}
			return false;
		}

		bool SetAmbisonicOrder(int _ambisonicOrder) {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().IsAmbisonic()) {
					return it->SetAmbisonicOrder(_ambisonicOrder);
				}
			}
			return false;
		}
		int GetAmbisonicOrder() const { 
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().IsAmbisonic()) {
					return it->GetAmbisonicOrder();
				}
			}
			return 0;
		}
		
		template <typename T>
		bool SetAmbisonicNormalization(T _ambisonicNormalization) {
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().IsAmbisonic()) {
					return it->SetAmbisonicNormalization(_ambisonicNormalization);
				}
			}
			return false;
		}

		/*bool SetAmbisonicNormalization(Common::TAmbisonicNormalization _ambisonicNormalization) { 
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().IsAmbisonic()) {
					return it->SetAmbisonicNormalization(_ambisonicNormalization);
				}
			}
			return false;
		}*/

		BRTProcessing::TAmbisonicNormalization GetAmbisonicNormalization() const { 			
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().IsAmbisonic()) {
					return it->GetAmbisonicNormalization();
				}
			}							
			return BRTProcessing::TAmbisonicNormalization::none; 
		}


	private:
				
		//////////////////////////
		// Private Methods
		/////////////////////////
		
		
		//////////////////////////
		// Private Attributes
		/////////////////////////
		CBRTManager* brtManager;							// Pointer to the BRT Manager			
		std::vector<std::shared_ptr<BRTListenerModel::CListenerModelBase>> listenerModelsConnected;						// Listener models connected to the listener
		std::vector<std::shared_ptr<BRTBinauralFilter::CBinauralFilterBase>> binauralFiltersConnected; // Binaural filters connected to the listener
	};
}
#endif