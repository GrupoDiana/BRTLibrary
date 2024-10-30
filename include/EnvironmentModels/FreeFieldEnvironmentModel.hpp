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

#ifndef _C_FREE_FIELD_ENVIRONMENT_MODEL_HPP_
#define _C_FREE_FIELD_ENVIRONMENT_MODEL_HPP_

#include <memory>
#include <Base/BRTManager.hpp>
#include <Base/EnvironmentModelBase.hpp>
#include <Base/ListenerModelBase.hpp>
#include <Base/SourceModelBase.hpp>
#include <EnvironmentModels/FreeFieldEnvironment/FreeFieldEnvironmentProcessor.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTEnvironmentModel {

	class CFreeFieldEnvironmentModel : public BRTBase::CEnviromentModelBase { 
	
		class CSourceProcessors {
		public:
			CSourceProcessors(std::string _sourceID, BRTBase::CBRTManager * brtManager)
				: sourceID { _sourceID } {				

				freeFieldProcessor = brtManager->CreateProcessor<BRTEnvironmentModel::CFreeFieldEnvironmentProcessor>(brtManager);
				freeFieldProcessor->Setup(_sourceID);
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
			bool ConnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listener) {
				return (freeFieldProcessor->ConnectToListenerModel(_listener));
			}

			/**
			 * @brief Disconnect SDN processor to a listener model
			 * @param _listener listener model to disconnect
			 * @return TRUE if the disconnection is successful
			 */
			bool DisconnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listener) {
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
				// do nothing
			}					
			std::string sourceID;
			std::shared_ptr<BRTEnvironmentModel::CFreeFieldEnvironmentProcessor> freeFieldProcessor;
		};
	
	public:
		CFreeFieldEnvironmentModel(const std::string & _environmentModelID, BRTBase::CBRTManager * _brtManager)
			: BRTBase::CEnviromentModelBase(_environmentModelID)
			, brtManager { _brtManager }
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
		 * @brief Connect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) override {
			return ConnectAnySoundSource(_source, false);
		};
		/**
		 * @brief Connect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) override {
			return ConnectAnySoundSource(_source, true);
		};

		
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) override { 
			return DisconnectAnySoundSource(_source, false);
		};
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) override { 
			return DisconnectAnySoundSource(_source, true);
		};


		/**
		 * @brief Connect a virtual source to this listener
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		/*bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CVirtualSourceModel> _source) override {
			return ConnectAnySoundSource(_source, false);
		}*/
						
		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() override {
			
			//BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
			//if (command.isNull() || command.GetCommand() == "") {
			//	return;
			//}

			//std::string listenerID = GetIDEntryPoint("listenerID")->GetData();
			////std::string listenerModelID = GetIDEntryPoint("listenerModelID")->GetData();

			//if (this->GetID() == command.GetStringParameter("environmentModelID")) {
			//	if (command.GetCommand() == "/environment/enableModel") {
			//		if (command.GetBoolParameter("enable")) {
			//			EnableModel();
			//		} else {
			//			DisableModel();
			//		}
			//	} else if (command.GetCommand() == "/environment/enableDirectPath") {
			//		if (command.GetBoolParameter("enable")) {
			//			EnableDirectPath();
			//		} else {
			//			DisableDirectPath();
			//		}
			//	} else if (command.GetCommand() == "/environment/enableReverbPath") {
			//		if (command.GetBoolParameter("enable")) {
			//			EnableReverbPath();
			//		} else {
			//			DisableReverbPath();
			//		}
			//	} else if (command.GetCommand() == "/environment/resetBuffers") {
			//		ResetProcessorBuffers();
			//	}
			//}

			//if (listenerID == command.GetStringParameter("listenerID")) {
			//	if (command.GetCommand() == "/listener/resetBuffers") {
			//		ResetProcessorBuffers();
			//	}
			//}
		}


	private:
		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		template <typename T>
		bool ConnectAnySoundSource(std::shared_ptr<T> _source, bool sourceNeedsListenerPosition) {
			std::lock_guard<std::mutex> l(mutex);

			// Get listener Model pointer
			std::shared_ptr<BRTBase::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTBase::CListenerModelBase>(GetIDEntryPoint("listenerModelID")->GetData());
			if (_listenerModel == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener Model.");
				return false;
			}

			// Get listener pointer
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(_listenerModel->GetListenerID());
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener.");
				return false;
			}

			// Make connections									
			CSourceProcessors _newSourceProcessors(_source->GetID(), brtManager);

			bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.freeFieldProcessor, "sourcePosition");
			//control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.distanceAttenuationProcessor, "sourceID");			

			if (sourceNeedsListenerPosition) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}

			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.freeFieldProcessor, "listenerPosition");						
			//control = control && brtManager->ConnectModuleID(_listener, _newSourceProcessors.distanceAttenuationProcessor, "listenerID"); 

			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.freeFieldProcessor, "inputSamples");						
			//control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.freeFieldProcessor, "outputSamples", this, "outputSamples");
			
			control = control && _newSourceProcessors.ConnectToListenerModel(_listenerModel);

			if (control) {
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
		template <typename T>
		bool DisconnectAnySoundSource(std::shared_ptr<T> _source, bool sourceNeedsListenerPosition) {
			
			std::lock_guard<std::mutex> l(mutex);

			// Get listener Model pointer
			std::shared_ptr<BRTBase::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTBase::CListenerModelBase>(GetIDEntryPoint("listenerModelID")->GetData());
			if (_listenerModel == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener Model.");
				return false;
			}

			// Get listener pointer
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(_listenerModel->GetListenerID());
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener.");
				return false;
			}

			// Make connections
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceProcessors & sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {				
				bool control = it->DisconnectToListenerModel(_listenerModel);
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->freeFieldProcessor, "inputSamples");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->freeFieldProcessor, "listenerPosition");
				if (sourceNeedsListenerPosition) {
					control = control && brtManager->DisconnectModuleTransform(_listener, _source, "listenerPosition");
				}
				control = brtManager->DisconnectModuleTransform(_source, it->freeFieldProcessor, "sourcePosition");
				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it);
				return true;
			}
			return false;			
		}

		/////////////////
		// Attributes
		/////////////////
		mutable std::mutex mutex;					// To avoid access collisions
		BRTBase::CBRTManager * brtManager;				

		std::vector<CSourceProcessors> sourcesConnectedProcessors;
	};	
}
#endif