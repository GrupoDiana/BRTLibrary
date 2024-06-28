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
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
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
																
		}
						
		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectListenerModel(std::shared_ptr<CListenerModelBase> _listenerModel, Common::T_ear _ear = Common::T_ear::BOTH) {
			
			if (_listenerModel == nullptr) return false;
			if (_listenerModel->IsConnectedToListener()) { return false; };
			
			bool control;
			control = brtManager->ConnectModuleID(this, _listenerModel, "listenerID");

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
						
			listenerModelsConnected.push_back(_listenerModel);
			return control;
		};

		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectListenerModel(const std::string& _listenerModelID, Common::T_ear _ear = Common::T_ear::BOTH) {
			
			std::shared_ptr<CListenerModelBase> _listenerModel = brtManager->GetListenerModel<CListenerModelBase>(_listenerModelID);
			if (_listenerModel == nullptr) return false;
			
			/*if (_listenerModel->IsConnectedToListener()) { return false; };
			bool control;
			control = brtManager->ConnectModuleID(this, _listenerModel, "listenerID");						
			control = control && brtManager->ConnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");			

			listenerModelsConnected.push_back(_listenerModel);		*/	

			return ConnectListenerModel(_listenerModel, _ear);			
		};

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
					SET_RESULT(RESULT_ERROR_NOTSET, "ERROR: Unknown error when trying to set the HRTF in the listener model with ID " + _listenerModel->GetID());
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
						SET_RESULT(RESULT_ERROR_NOTSET, "ERROR: Unknown error when trying to set the BRIR in the listener model with ID " + _listenerModel->GetID());
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
		bool SetNearFieldCompensationFilters(std::shared_ptr< BRTServices::CNearFieldCompensationFilters > _listenerNFC) {
			
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
		std::shared_ptr <BRTServices::CNearFieldCompensationFilters> GetNearFieldCompensationFilters() const
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

		Common::TAmbisonicNormalization GetAmbisonicNormalization() const { 			
			for (auto& it : listenerModelsConnected) {
				if (it->GetListenerModelCharacteristics().IsAmbisonic()) {
					return it->GetAmbisonicNormalization();
				}
			}							
			return Common::TAmbisonicNormalization::none; 
		}


	private:
				
		//////////////////////////
		// Private Methods
		/////////////////////////
		
		
		//////////////////////////
		// Private Attributes
		/////////////////////////
		CBRTManager* brtManager;							// Pointer to the BRT Manager			
		std::vector<std::shared_ptr<CListenerModelBase>> listenerModelsConnected;		// Listener models connected to the listener
		
	};
}
#endif