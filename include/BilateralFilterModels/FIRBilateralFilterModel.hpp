/**
* \class CFIRBilateralFilterModel
*
* \brief This class implements a FIR bilateral filter.
* \date	Dec 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo ||
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

#ifndef _FIR_BILATERAL_FILTER_MODEL_HPP_
#define _FIR_BILATERAL_FILTER_MODEL_HPP_

#include <Base/BRTManager.hpp>
#include <Base/Listener.hpp>
#include <BilateralFilterModels/BilateralFilterModelBase.hpp>
#include <ListenerModels/ListenerModelBase.hpp>
#include <Filters/FIRFilter.hpp>
#include <Filters/FilterBase.hpp>

namespace BRTBilateralFilter { 

	class CFIRBilateralFilterModel : public CBilateralFilterModelBase { 
	public:

		CFIRBilateralFilterModel(const std::string & _binauraFilterID, BRTBase::CBRTManager * _brtManager)
			: CBilateralFilterModelBase(_binauraFilterID)
			, brtManager { _brtManager }
		{
			filterType = T_BilateralFilterType::FIR_FILTER;
		}

		/**
		 * @brief Enable model
		 */
		void EnableModel() override {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = true;	
			firFilter.Enable();			
		};

		/**
		 * @brief Disable model
		 */
		void DisableModel() override {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = false;				
			firFilter.Disable();
		};
		

		/** \brief SET SOS filters of the Binaural Filter
		*	\param[in] pointer to SOS filter to be stored		
		*/
		bool SetFIRTable(std::shared_ptr<BRTServices::CServicesBase> _firTable) override {			
			bool result = firFilter.SetFIRTable(_firTable);
			if (result) {				
				UpdatedEnabledDisabledFIRFilter();			
			}
			return result;		
		}

		std::shared_ptr<BRTServices::CServicesBase> GetFIRTable() const override { 
			return firFilter.GetFIRTable(); 
		}
		
		void RemoveFIRTable() override {
			firFilter.RemoveFIRTable();			
		}	

		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @param _ear Ear to connect, both by default
		 * @return True if the connection success
		*/
		bool ConnectListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel, Common::T_ear _ear = Common::T_ear::BOTH) {

			if (_listenerModel == nullptr) return false;
			if (_listenerModel->IsAlreadyConnected()) return false;
			
			// Get listener pointer
			std::string listenerID = GetIDEntryPoint("listenerID")->GetData();		// The ID of the listener I am connected to.			
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(listenerID);
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This Binaural Filter has not been connected to a listener.");
				return false;
			}

			//Make connections
			bool control;
			control = brtManager->ConnectModuleID(this, _listenerModel, "binauralFilterID");
			control = control && brtManager->ConnectModuleID(_listener, _listenerModel, "listenerID");	

			_listenerModel->ConnectListenerTransform(listenerID);

			if (_ear == Common::T_ear::LEFT) {
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
			} else if (_ear == Common::T_ear::RIGHT) {
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			} else if (_ear == Common::T_ear::BOTH) {
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
				control = control && brtManager->ConnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			} else {
				return false;
			}

			//listenerModelsConnected.push_back(_listenerModel); //TO solved
			_listener->AddListenerModelConnected(_listenerModel);
			AddInputConnection(_listenerModel->GetModelID());

			SendMyID();
			return control;
		};

		/**
		 * @brief Connect listener model to this listener
		 * @param _listener Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectListenerModel(const std::string & _listenerModelID, Common::T_ear _ear = Common::T_ear::BOTH) override {

			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(_listenerModelID);
			if (_listenerModel == nullptr) return false;

			return ConnectListenerModel(_listenerModel, _ear);
		};
		
		/**
		 * @brief Disconnect listener model from this listener
		 * @param _listener Pointer to the source
		 * @return True if the disconnection success
		*/
		bool DisconnectListenerModel(const std::string & _listenerModelID, Common::T_ear _ear = Common::T_ear::BOTH) override { 
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(_listenerModelID);
			if (_listenerModel == nullptr) return false;

			return DisconnectListenerModel(_listenerModel, _ear);		
		
		};

		/**
		 * @brief Disconnect listener model from this listener
		 * @param _listener Pointer to the source
		 * @return True if the disconnection success
		*/
		bool DisconnectListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel, Common::T_ear _ear = Common::T_ear::BOTH) {
			if (_listenerModel == nullptr) return false;
			

			// Get listener pointer
			std::string listenerID = GetIDEntryPoint("listenerID")->GetData(); // The ID of the listener I am connected to.
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(listenerID);
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This Binaural Filter has not been connected to a listener.");
				return false;
			}

			//Make disconnections
			bool control;
			
			_listener->RemoveListenerModelConnected(_listenerModel);

			if (_ear == Common::T_ear::LEFT) {
				control = brtManager->DisconnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
			} else if (_ear == Common::T_ear::RIGHT) {
				control = brtManager->DisconnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			} else if (_ear == Common::T_ear::BOTH) {
				control =  brtManager->DisconnectModulesSamples(_listenerModel, "leftEar", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(_listenerModel, "rightEar", this, "rightEar");
			} else {
				return false;
			}
			
			_listenerModel->DisconnectListenerTransform(listenerID);

			control = control && brtManager->DisconnectModuleID(this, _listenerModel, "binauralFilterID");
			control = control && brtManager->DisconnectModuleID(_listener, _listenerModel, "listenerID");

			RemoveInputConnection(_listenerModel->GetModelID());

			return control;
		}

	private:
		
		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {

			CMonoBuffer<float> outLeftBuffer;
			CMonoBuffer<float> outRightBuffer;

			CMonoBuffer<float> leftBuffer = leftChannelMixer.GetMixedBuffer();
			CMonoBuffer<float> rightBuffer = rightChannelMixer.GetMixedBuffer();
			
			if (leftBuffer.size() == 0 || rightBuffer.size() == 0) return;
														
			firFilter.Process(leftBuffer, outLeftBuffer, rightBuffer, outRightBuffer);

			if (enableModel) {
				outLeftBuffer.ApplyGain(gain);
				outRightBuffer.ApplyGain(gain);
			}
			
			GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
			GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);			
		}	

		
		void UpdateCommand() override { 
		
		}

		void UpdatedEnabledDisabledFIRFilter() {
			if (enableModel) {
				firFilter.Enable();
			} else {
				firFilter.Disable();
			}
		}
				
		/////////////////
		// Attributes
		/////////////////
		mutable std::mutex mutex;			// To avoid access collisions
		BRTBase::CBRTManager * brtManager;	// Pointer to the BRT Manager			
		BRTFilters::CFIRFilter firFilter; 
	};
}

#endif