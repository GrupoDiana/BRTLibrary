/**
* \class CSOSBilateralFilterModel
*
* \brief This class implements a bilateral filter.
* \date	Nov 2024
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

#ifndef _SOS_BILATERAL_FILTER_MODEL_HPP_
#define _SOS_BILATERAL_FILTER_MODEL_HPP_

#include <Base/BRTManager.hpp>
#include <Base/Listener.hpp>
#include <ServiceModules/SOSCoefficients.hpp>
#include <BilateralFilterModels/BilateralFilterModelBase.hpp>
#include <Filters/FilterBase.hpp>
#include <Filters/SOSFilter.hpp>
#include <ListenerModels/ListenerModelBase.hpp>

#define NUMBER_OF_COEFFICIENTS_IN_STAGE_SOS 6

namespace BRTBilateralFilter { 

	class CSOSBilateralFilterModel : public CBilateralFilterModelBase { 
	public:

		CSOSBilateralFilterModel(const std::string & _binauraFilterID, BRTBase::CBRTManager * _brtManager)
			: CBilateralFilterModelBase(_binauraFilterID)
			, brtManager { _brtManager }
		{						
		}

		/**
		 * @brief Enable model
		 */
		void EnableModel() override {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = true;	
			//binauralFilter.EnableProcessor();
			sosFilter.Enable();
		};

		/**
		 * @brief Disable model
		 */
		void DisableModel() override {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = false;	
			//binauralFilter.DisableProcessor();
			sosFilter.Disable();
		};
		
		/** \brief SET SOS filters of the Binaural Filter
		*	\param[in] pointer to SOS filter to be stored		
		*/
		bool SetSOSFilter(std::shared_ptr<BRTServices::CSOSCoefficients> _SOSFilterCoefficientsTable) override {
			SOSFilterCoefficientsTable = _SOSFilterCoefficientsTable;			
			FiltersSetup();						
			return true;
		}

		/** \brief Get the SOS filter of the Binaural Filter
		*	\retval SOS filter of the listener
		*/
		std::shared_ptr<BRTServices::CSOSCoefficients> GetSOSFilter() const override {
			return SOSFilterCoefficientsTable;
		}

		/** \brief Remove the SOS filter of the Binaural Filter		
		*/
		void RemoveSOSFilter() override {
			SOSFilterCoefficientsTable = nullptr;
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
						
			sosFilter.Process(leftBuffer, outLeftBuffer, Common::T_ear::LEFT);
			sosFilter.Process(rightBuffer, outRightBuffer, Common::T_ear::RIGHT);
			
			outLeftBuffer.ApplyGain(gain);
			outRightBuffer.ApplyGain(gain);

			GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
			GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);			
		}	

		
		void UpdateCommand() override { 
		
		}
		

		void FiltersSetup() {
			std::vector<float> coefficientsLeft = SOSFilterCoefficientsTable->GetSOSFilterCoefficients(Common::T_ear::LEFT, 0.1, 0);
			std::vector<float> coefficientsRight = SOSFilterCoefficientsTable->GetSOSFilterCoefficients(Common::T_ear::RIGHT, 0.1, 0);

			int numberOfStages = coefficientsLeft.size() / NUMBER_OF_COEFFICIENTS_IN_STAGE_SOS;			

			sosFilter.Setup(2, numberOfStages); //2 ears
			sosFilter.SetCoefficients(Common::T_ear::LEFT, coefficientsLeft);
			sosFilter.SetCoefficients(Common::T_ear::RIGHT, coefficientsRight);
			UpdatedEnabledDisabledSOSFilter();			
		}

		void UpdatedEnabledDisabledSOSFilter() {
			if (enableModel) {
				sosFilter.Enable();
			} else {
				sosFilter.Disable();
			}
		}

		/////////////////
		// Attributes
		/////////////////
		mutable std::mutex mutex;												// To avoid access collisions
		BRTBase::CBRTManager * brtManager;										// Pointer to the BRT Manager
		std::shared_ptr<BRTServices::CSOSCoefficients> SOSFilterCoefficientsTable; // SOS Filter of listener
		
		BRTFilters::CSOSFilter sosFilter; // Biquad chain table
	};
}

#endif