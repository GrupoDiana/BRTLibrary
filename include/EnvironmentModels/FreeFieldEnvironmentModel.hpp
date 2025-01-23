/**
* \class CFreeFieldEnvironmentModel
*
* \brief This class implements the free field propagation model.
* \date	Oct 2024
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

#ifndef _C_FREE_FIELD_ENVIRONMENT_MODEL_HPP_
#define _C_FREE_FIELD_ENVIRONMENT_MODEL_HPP_

#include <memory>
#include <Base/BRTManager.hpp>
#include <EnvironmentModels/EnvironmentModelBase.hpp>
#include <ListenerModels/ListenerModelBase.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <EnvironmentModels/FreeFieldEnvironment/FreeFieldEnvironmentProcessor.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTEnvironmentModel {

	class CFreeFieldEnvironmentModel : public CEnviromentModelBase { 
	
		class CSourceProcessors {
		public:
			CSourceProcessors(std::string _environmentModelID, std::string _sourceID, BRTBase::CBRTManager * brtManager)
				: sourceID { _sourceID } {				

				freeFieldProcessor = brtManager->CreateProcessor<BRTEnvironmentModel::CFreeFieldEnvironmentProcessor>(brtManager);
				freeFieldProcessor->Setup(_environmentModelID, _sourceID);								
			}

			/**
			 * @brief Remove processor from BRT
			 * @param brtManager brtManager pointer
			*/
			void Clear(BRTBase::CBRTManager * brtManager) {
				sourceID = "";
				freeFieldProcessor->Clear();
				brtManager->RemoveProcessor(freeFieldProcessor);				
			}
			
			/**
			 * @brief Connect SDN processor to a listener model
			 * @param _listener listener model to connect
			 * @return TRUE if the connection is successful
			 */
			bool ConnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listener) {
				return (freeFieldProcessor->ConnectToListenerModel(_listener));
			}

			/**
			 * @brief Disconnect SDN processor to a listener model
			 * @param _listener listener model to disconnect
			 * @return TRUE if the disconnection is successful
			 */
			bool DisconnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listener) {
				return (freeFieldProcessor->DisconnectToListenerModel(_listener));
			}

			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) {
					freeFieldProcessor->EnableProcessor();					

				} else {
					freeFieldProcessor->DisableProcessor();					
				}
			}

			/**
			 * @brief Reset processor buffers
			*/
			void ResetBuffers() {				
				freeFieldProcessor->ResetProcessBuffers();
			}		

			/**
			 * @brief Set the gain of the processor
			 * @param _gain Gain value
			 */
			void SetGain(float _gain) {
				freeFieldProcessor->SetGain(_gain);
			}
			
			/**
			 * @brief Set the enable distance attenuation
			 * @param _enableDistanceAttenuation
			 */
			void SetEnableDistanceAttenuation(bool _enableDistanceAttenuation) {
				if (_enableDistanceAttenuation) {
					freeFieldProcessor->EnableDistanceAttenuation();
				} else {
					freeFieldProcessor->DisableDistanceAttenuation();
				}
			}

			/**
			 * @brief Set the enable propagation delay
			 * @param _enablePropagationDelay
			 */
			void SetEnablePropagationDelay(bool _enablePropagationDelay) {
				if (_enablePropagationDelay) {
					freeFieldProcessor->EnablePropagationDelay();
				} else {
					freeFieldProcessor->DisablePropagationDelay();
				}
			}


			/**
			 * @brief Set proccesor configuration
			 * @param enableSpatialization Spatialization state
			 * @param enableInterpolation Interpolation state
			 * @param enableNearFieldEffect Nearfield state
			*/
			void SetConfiguration(bool _enableDistanceAttenuation, bool _enablePropagationDelay) {
				SetEnableDistanceAttenuation(_enableDistanceAttenuation);
				SetEnablePropagationDelay(_enablePropagationDelay);
			}

			// Attributes
			std::string sourceID;
			std::shared_ptr<BRTEnvironmentModel::CFreeFieldEnvironmentProcessor> freeFieldProcessor;
		};
	
	public:
		CFreeFieldEnvironmentModel(const std::string & _environmentModelID, BRTBase::CBRTManager * _brtManager)
			: CEnviromentModelBase(_environmentModelID)
			, brtManager { _brtManager }
			, enableDistanceAttenuation { true }
			, enablePropagationDelay { false }
		{ 
			
		}

		
		/**
		 * @brief Enable model
		 */
		void EnableModel() override {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = true;
			for (auto & it : sourcesConnectedProcessors) {
				it.SetEnableProcessor(true);
			}
		};

		/**
		 * @brief Disable model
		 */
		void DisableModel() override {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = false;
			for (auto & it : sourcesConnectedProcessors) {
				it.SetEnableProcessor(false);
			}
		};
				
		/**
		 * @brief Enable distance attenuation
		 */
		void EnableDistanceAttenuation() override {			
			enableDistanceAttenuation = true;
			SetConfigurationInALLSourcesProcessors();
		}
		/**
		 * @brief Disable distance attenuation
		 */
		void DisableDistanceAttenuation() override {			
			enableDistanceAttenuation = false;
			SetConfigurationInALLSourcesProcessors();
		}
		/**
		 * @brief Get the flag for distance attenuation enabling
		 * @return True if the distance attenuation is enabled
		 */
		bool IsDistanceAttenuationEnabled() override {
			return enableDistanceAttenuation;
		}

		/**
		 * @brief Enable propagation delay
		 */
		void EnablePropagationDelay() override {			
			enablePropagationDelay = true;
			SetConfigurationInALLSourcesProcessors();
		}
		/**
		 * @brief Disable propagation delay
		 */ 
		void DisablePropagationDelay() override {			
			enablePropagationDelay = false;
			SetConfigurationInALLSourcesProcessors();
		}
		/**
		 * @brief Get the flag for propagation delay enabling
		 * @return True if the propagation delay is enabled
		 */
		bool IsPropagationDelayEnabled() override {
			return enablePropagationDelay;
		}

		/**
		 * @brief Connect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) override {
			return ConnectAnySoundSource(_source);
		};
		
		/**
		 * @brief Connect a new source to this listener model
		 * @param _sourceID Source ID
		 * @return True if the connection success
		 */
		bool ConnectSoundSource(const std::string & _sourceID) override {
			std::shared_ptr<BRTSourceModel::CSourceModelBase> _source = brtManager->GetSoundSource(_sourceID);
			if (_source == nullptr) return false;
			return ConnectAnySoundSource(_source);
		}

		/**
		 * @brief Disconnect a new source to this listener
		 * @param _source Pointer to the source
		 */
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) override { 
			return DisconnectAnySoundSource(_source);
		};
		
		/**
		 * @brief Disconnect a new source to this listener model
		 * @param _sourceID Source ID
		 * @return True if the disconnection success
		 */
		bool DisconnectSoundSource(const std::string & _sourceID) override {
			std::shared_ptr<BRTSourceModel::CSourceModelBase> _source = brtManager->GetSoundSource(_sourceID);
			if (_source == nullptr) return false;
			return DisconnectAnySoundSource(_source);
		};

		/**
		 * @brief Reset all processor buffers
		*/
		void ResetProcessorBuffers() {
			std::lock_guard<std::mutex> l(mutex);
			for (auto & it : sourcesConnectedProcessors) {
				it.ResetBuffers();
			}
		}	

						
		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() override {
			
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetCommand() == "") {
				return;
			}
									
			std::string listenerModelID = GetIDEntryPoint("listenerModelID")->GetData();
			std::shared_ptr<BRTBase::CListener> listener = GetListenerPointer();
			if (listener == nullptr) return;
			std::string listenerID = GetListenerPointer()->GetID();

			if (this->GetModelID() == command.GetStringParameter("environmentModelID")) {
				if (command.GetCommand() == "/environment/enableModel") {
					if (command.GetBoolParameter("enable")) {
						EnableModel();
					} else {
						DisableModel();
					}
				} else if (command.GetCommand() == "/environment/enableDirectPath") {
					if (command.GetBoolParameter("enable")) {
						EnableDirectPath();
					} else {
						DisableDirectPath();
					}
				} else if (command.GetCommand() == "/environment/enableReverbPath") {
					if (command.GetBoolParameter("enable")) {
						EnableReverbPath();
					} else {
						DisableReverbPath();
					}
				} else if (command.GetCommand() == "/environment/resetBuffers") {
					ResetProcessorBuffers();
				}
			}

			if (listenerID == command.GetStringParameter("listenerID")) {
				if (command.GetCommand() == "/listener/resetBuffers") {
					ResetProcessorBuffers();
				}
			}
		}

	private:
		
		/**
		 * @brief Set the gain of the model
		 * @param _gain 
		 */
		void UpdateGain() override {
			std::lock_guard<std::mutex> l(mutex);
			for (auto & it : sourcesConnectedProcessors) {
				it.SetGain(gain);
			}
		}

		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/		
		bool ConnectAnySoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) {
			std::lock_guard<std::mutex> l(mutex);

			// Get listener Model pointer
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = GetListenerModelPointer();
			if (_listenerModel == nullptr) return false;
			// Get listener pointer
			std::shared_ptr<BRTBase::CListener> _listener = GetListenerPointer(_listenerModel);
			if (_listener == nullptr) return false;

			// Make connections									
			CSourceProcessors _newSourceProcessors(modelID,_source->GetID(), brtManager);
			bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.freeFieldProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.freeFieldProcessor, "sourceID");
			if (_source->GetSourceType() == BRTSourceModel::Directivity) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}

			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.freeFieldProcessor, "listenerPosition");									
			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.freeFieldProcessor, "inputSamples");											
			control = control && _newSourceProcessors.ConnectToListenerModel(_listenerModel);

			if (control) {
				SetSourceProcessorsConfiguration(_newSourceProcessors);
				sourcesConnectedProcessors.push_back(std::move(_newSourceProcessors));
				return true;
			}
			return false;
		}


		/**
		 * @brief Disconnect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		*  @return True if the disconnection success
		*/
		//template <typename T>
		bool DisconnectAnySoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) {
			
			std::lock_guard<std::mutex> l(mutex);

			// Get listener Model pointer
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = GetListenerModelPointer();
			if (_listenerModel == nullptr) return false;
			// Get listener pointer
			std::shared_ptr<BRTBase::CListener> _listener = GetListenerPointer(_listenerModel);
			if (_listener == nullptr) return false;
						
			// Make connections
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceProcessors & sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {				
				bool control = it->DisconnectToListenerModel(_listenerModel);
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->freeFieldProcessor, "inputSamples");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->freeFieldProcessor, "listenerPosition");
				if (_source->GetSourceType() == BRTSourceModel::Directivity) {
					control = control && brtManager->DisconnectModuleTransform(_listener, _source, "listenerPosition");
				}
				control = control && brtManager->DisconnectModuleID(_source, it->freeFieldProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->freeFieldProcessor, "sourcePosition");
				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it);
				return true;
			}
			return false;			
		}

		/**
		 * @brief Get listener Model pointer
		 * @return Listener Model pointer
		*/
		std::shared_ptr<BRTListenerModel::CListenerModelBase> GetListenerModelPointer() {
			// Get listener Model pointer
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(GetIDEntryPoint("listenerModelID")->GetData());
			if (_listenerModel == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener Model.");				
			}
			return _listenerModel;
		}

		/**
		 * @brief Get listener pointer
		 * @param _listenerModel Listener Model pointer
		 * @return Listener pointer
		*/
		std::shared_ptr<BRTBase::CListener> GetListenerPointer() {			
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = GetListenerModelPointer();
			if (_listenerModel == nullptr) return nullptr;
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(_listenerModel->GetListenerID());
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener.");
			}
			return _listener;
		} 
		/**
		 * @brief Get listener pointer
		 * @param _listenerModel Listener Model pointer
		 * @return Listener pointer
		*/
		std::shared_ptr<BRTBase::CListener> GetListenerPointer(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(_listenerModel->GetListenerID());
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener.");				
			}
			return _listener;
		}

		/**
		 * @brief Update Configuration in all source processor
		*/
		void SetConfigurationInALLSourcesProcessors() {
			std::lock_guard<std::mutex> l(mutex);
			for (auto & it : sourcesConnectedProcessors) {
				SetSourceProcessorsConfiguration(it);
			}
		}
		/**
		 * @brief Update configuration just in one source processor
		 * @param sourceProcessor 
		*/
		void SetSourceProcessorsConfiguration(CSourceProcessors & sourceProcessor) {
			sourceProcessor.SetConfiguration(enableDistanceAttenuation, enablePropagationDelay);
		}

		/////////////////
		// Attributes
		/////////////////
		mutable std::mutex mutex;					// To avoid access collisions
		BRTBase::CBRTManager * brtManager;						
		//float gain;

		bool enablePropagationDelay;
		bool enableDistanceAttenuation;

		std::vector<CSourceProcessors> sourcesConnectedProcessors;
	};	
}
#endif