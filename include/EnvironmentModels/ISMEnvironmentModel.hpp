/**
* \class CISMEnvironmentModel
*
* \brief This class implements the ISM environment model. It instantiates an ISM processor for each source that connects to it. 
* \date	Oct 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
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

#ifndef _C_ISM_ENVIRONMENT_MODEL_HPP_
#define _C_ISM_ENVIRONMENT_MODEL_HPP_

#include <memory>
#include <ListenerModels/ListenerModelBase.hpp>
#include <EnvironmentModels/EnvironmentModelBase.hpp>
#include <EnvironmentModels/ISMEnvironment/ISMEnvironmentProcessor.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTEnvironmentModel {
	
	class CISMEnvironmentModel : public CEnviromentModelBase {
		
		class CISMProcessors {
		public:
			
			CISMProcessors(std::string _sourceID, BRTBase::CBRTManager * brtManager)
				: sourceID { _sourceID } {
				
				ISMProcessor = brtManager->CreateProcessor<BRTEnvironmentModel::CISMEnvironmentProcessor>(brtManager);					
				ISMProcessor->Init(_sourceID);
			}	

			/**
			 * @brief Remove processor from BRT
			 * @param brtManager brtManager pointer
			 */
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(ISMProcessor);				
			}

			/**
			 * @brief Set Room dimensions and Room Centre
			 * @param _roomDimensions length, width and height of the room
			 * @param _roomCentre centre of the room
			 */
			/*bool Setup(const int & _reflectionOrder, const float & _maxDistanceSourcesToListener, const float & _windowSlopeDistance, const Common::CRoom & _room) {
				return ISMProcessor->Setup(_reflectionOrder, _maxDistanceSourcesToListener, _windowSlopeDistance, _room);
			}*/

			bool Setup(const int & _reflectionOrder, const float & _maxDistanceSourcesToListener, const float & _windowSlopeDistance, std::shared_ptr<BRTServices::CRoom> & _room, std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel) {
				return ISMProcessor->Setup(_reflectionOrder, _maxDistanceSourcesToListener, _windowSlopeDistance, _room, _listenerModel);
			}
									
			/**
			 * @brief Set Wall absortion coefficientes per band
			 * @param _wallIndex Wall where the coefficients are to be placed. 
			 * @param _wallAbsortions absortion coefficients per band
			 */
			void SetWallAbsortion(int _wallIndex, std::vector<float> _wallAbsortions) {									
				//ISMProcessor->SetWallFreqAbsorption(_wallIndex, _wallAbsortions);				
			}
			
			/**
			 * @brief Update the configuration of the processor
			 * @param _enableDirectPath enable or disable direct path
			 * @param _enableReverbPath enable or disable reverb path
			 */
			void SetConfiguration(/*bool _enableDirectPath,*/ bool _enableReverbPath) {
				/*if (_enableDirectPath) {
					ISMProcessor->MuteLOS(false);
				} else {
					ISMProcessor->MuteLOS(true);
				}*/

				if (_enableReverbPath) {
					ISMProcessor->MuteReverbPath(false);
				} else {
					ISMProcessor->MuteReverbPath(true);
				}										
			}
			
			/**
			 * @brief Connect SDN processor to a listener model
			 * @param _listener listener model to connect
			 * @return TRUE if the connection is successful
			 */
			bool ConnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listener) {
				return (ISMProcessor->ConnectToListenerModel(_listener));
			}
			
			/**
			 * @brief Disconnect SDN processor to a listener model
			 * @param _listener listener model to disconnect
			 * @return TRUE if the disconnection is successful
			 */
			bool DisconnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listener) {
				return (ISMProcessor->DisconnectToListenerModel(_listener));
			}
			
			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) { 
					ISMProcessor->EnableProcessor(); 					

				} else { 
					ISMProcessor->DisableProcessor(); 					
				}
			}

			/**
			 * @brief Set the gain of the processor
			 * @param _gain New gain value
			 */
			void SetGain(float _gain) {
				ISMProcessor->SetGain(_gain);
			}

			/**
			 * @brief Reset processor buffers
			*/
			void ResetBuffers() {			
				ISMProcessor->ResetProcessBuffers();
			}
			

			// Attributes
			std::string sourceID;
			std::shared_ptr<BRTEnvironmentModel::CISMEnvironmentProcessor> ISMProcessor;			
		};

	public:
		CISMEnvironmentModel(const std::string& _environmentModelID, BRTBase::CBRTManager * _brtManager)
			: CEnviromentModelBase(_environmentModelID)

			, brtManager { _brtManager }
			//, enableDirectPath { true }
			, enableReverbPath { true }
			, reflectionOrder {1}
			, maxDistanceSourcesToListener { 3.43 }
			, windowSlopeDistance { 2 * globalParameters.GetSoundSpeed() * 0.001f }			
		{ 			
			// Default room
			room = std::make_shared<BRTServices::CRoom>();
			//roomDefinition.SetupShoeBox(10, 10, 5); // TODO delete me after testing
			//roomDefinition.SetupShoeBox(8, 5, 3); // TODO delete me after testing		
			//roomDefinition.SetAllWallsAbsortion(std::vector<float>(9, 0.5f)); // TODO delete me after testing
			//reflectionOrder = 2; // TODO delete me after testing
			//maxDistanceSourcesToListener = 10; // TODO delete me after testing
		}

		/**
		 * @brief Destructor
		*/
		~CISMEnvironmentModel() {
			sourcesConnectedProcessors.clear();
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
		 * @brief Enable Direct Path
		 */
		//void EnableDirectPath() override {
		//	enableDirectPath = true;
		//	SetConfigurationInALLSourcesProcessors();
		//}
		///**
		// * @brief Disable Direct Path
		// */
		//void DisableDirectPath() override {
		//	enableDirectPath = false;
		//	SetConfigurationInALLSourcesProcessors();
		//}		
		///**
		// * @brief Check if Direct Path is enabled
		// * @return True if Direct Path is enabled
		// */
		//bool IsDirectPathEnabled() override {
		//	return enableDirectPath;
		//}

		/**
		 * @brief Enable Reverb Path
		 */
		void EnableReverbPath() override {
			enableReverbPath = true;
			SetConfigurationInALLSourcesProcessors();
		}
		/**
		 * @brief Disable Reverb Path
		 */
		void DisableReverbPath() override {
			enableReverbPath = false;
			SetConfigurationInALLSourcesProcessors();
		}
		/**
		 * @brief Check if Reverb Path is enabled
		 * @return True if Reverb Path is enabled
		 */
		bool IsReverbPathEnabled() override {
			return enableReverbPath;
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
		 * @return True if the disconnection success
		*/
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) override{ 
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
			for (auto& it : sourcesConnectedProcessors) {
				it.ResetBuffers();
			}
		}

		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() override{
			//std::lock_guard<std::mutex> l(mutex);
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();						
			if (command.isNull() || command.GetCommand() == "") { return; }
			

			if (this->GetModelID() == command.GetStringParameter("environmentModelID")) {
				if (command.GetCommand() == "/environment/enableModel") {
					if (command.GetBoolParameter("enable")) { EnableModel();} 
					else {	DisableModel();	}
				} else if (command.GetCommand() == "/environment/enableDirectPath") {
					if (command.GetBoolParameter("enable")) { EnableDirectPath(); } 
					else {	DisableDirectPath(); }
				} else if (command.GetCommand() == "/environment/enableReverbPath") {
					if (command.GetBoolParameter("enable")) {	EnableReverbPath();	} 
					else {	DisableReverbPath();}
				} else if (command.GetCommand() == "/environment/resetBuffers") {
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
		 * @brief Set the room and reset and setup again
		 * @param _room 
		 * @return 
		 */
		bool SetRoom(std::shared_ptr<BRTServices::CRoom> _room) override {
			std::lock_guard<std::mutex> l(mutex);
			room = _room;
			return ResetAndSetup();
		}
		
		/**
		 * @brief Called when the room is updated to reset and setup again
		 * @return t
		 */
		bool UpdateRoom() override { 
			std::lock_guard<std::mutex> l(mutex);
			if (room == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "Room is not set.");
				return false;
			}
			return ResetAndSetup();			
		}

		/**
		 * @brief Get the room object
		 * @return pointer to the room
		 */
		std::shared_ptr<BRTServices::CRoom> GetRoom() const override { 
			return room; 
		}	

		/**
		 * @brief Remove the room
		 */
		void RemoveRoom() override { 
			std::lock_guard<std::mutex> l(mutex);
			room = std::make_shared<BRTServices::CRoom>();
			ResetAndSetup();
		};
		
		
		/**
		 * @brief Set the reflection order and reset and setup again
		 * @param _reflectionOrder reflection order should be equal or greater than zero
		 * @return true if success
		 */
		bool SetReflectionOrder(int _reflectionOrder) override { 
			if (_reflectionOrder<0) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Reflection order must be equal or greater than zero.");
				return false;
			}
			if (_reflectionOrder == reflectionOrder) {
				return true; // No change
			}
			std::lock_guard<std::mutex> l(mutex);
			reflectionOrder = _reflectionOrder; 			
			return ResetAndSetup();	
		}

		/**
		 * @brief Get the reflection order
		 * @return reflection order
		 */
		int GetReflectionOrder() override { 
			return reflectionOrder; 
		}

		/**
		 * @brief Set the max distance from sources to listener and reset and setup again		 
		 * @param _maxDistanceSourcesToListener max distance from sources to listener. It must be greater than zero.
		 * This parameter it is used to limit the image sources generation.		 
		 * @return 
		 */
		bool SetMaxDistanceSourcesToListener(float _maxDistanceSourcesToListener) override	{ 
			if (_maxDistanceSourcesToListener <= 0) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Max distance from sources to listener must be greater than zero.");
				return false;
			}
			if (_maxDistanceSourcesToListener == maxDistanceSourcesToListener) {
				return true; // No change
			}
			std::lock_guard<std::mutex> l(mutex);
			maxDistanceSourcesToListener = _maxDistanceSourcesToListener;
			
			return ResetAndSetup(); 
		}

		/**
		 * @brief Returns the maximum distance from sources to the listener.
		 * @return The maximum distance between sources and the listener as a float.
		 */
		float GetMaxDistanceSourcesToListener() override { 
			return maxDistanceSourcesToListener; 
		}

		/**
		 * @brief Set the transition meters
		 * @param _transitionMeters 
		 * @return 
		*/ 
		bool SetTransitionMeters(float _transitionMeters) override { 
			if (_transitionMeters < 0) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Transition meters must be equal or greater than zero.");
				return false;
			}
			if (_transitionMeters == windowSlopeDistance) {
				return true; // No change
			}
			std::lock_guard<std::mutex> l(mutex);
			windowSlopeDistance = _transitionMeters;
			return ResetAndSetup();
		}

		/**
		 * @brief Get the transition meters
		 * @return 
		*/
		float GetTransitionMeters() override{ 
			return windowSlopeDistance; 
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
		void SetSourceProcessorsConfiguration(CISMProcessors & sourceProcessor) {
			sourceProcessor.SetConfiguration(enableReverbPath);
		}


		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		//template <typename T>
		bool ConnectAnySoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source/*, bool sourceNeedsListenerPosition*/) {			
			std::lock_guard<std::mutex> l(mutex);
			
			// Get listener Model pointer			
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(GetIDEntryPoint("listenerModelID")->GetData());			
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
			
			// Create a new SDN Processor			
			CISMProcessors _newISMProcessors(_source->GetID(), brtManager);

			// Make connections
			bool control = brtManager->ConnectModuleTransform(_source, _newISMProcessors.ISMProcessor, "sourcePosition");			
			control = control && brtManager->ConnectModuleID(_source, _newISMProcessors.ISMProcessor, "sourceID");						
			
			if (_source->GetSourceType() == BRTSourceModel::Directivity) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}
		
			control = control && brtManager->ConnectModuleTransform(_listener, _newISMProcessors.ISMProcessor, "listenerPosition");			
			control = control && brtManager->ConnectModuleID(this, _newISMProcessors.ISMProcessor, "listenerID");

			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newISMProcessors.ISMProcessor, "inputSamples");			
									
			if (control) {
				_newISMProcessors.Setup(reflectionOrder, maxDistanceSourcesToListener, windowSlopeDistance, room, _listenerModel);
				control = control && _newISMProcessors.ConnectToListenerModel(_listenerModel);
			}
			if (control) {
				SetSourceProcessorsConfiguration(_newISMProcessors);
				sourcesConnectedProcessors.push_back(std::move(_newISMProcessors));
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
		bool DisconnectAnySoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) {						
			std::lock_guard<std::mutex> l(mutex);

			// Get listener Model pointer
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(GetIDEntryPoint("listenerModelID")->GetData());
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
			
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CISMProcessors & sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {
				bool control = it->DisconnectToListenerModel(_listenerModel);
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->ISMProcessor, "inputSamples");
				control = control && brtManager->DisconnectModuleID(this, it->ISMProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->ISMProcessor, "listenerPosition");
				if (_source->GetSourceType() == BRTSourceModel::Directivity) {
					control = control && brtManager->DisconnectModuleTransform(_listener, _source, "listenerPosition");
				}
				control = control && brtManager->DisconnectModuleID(_source, it->ISMProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->ISMProcessor, "sourcePosition");
				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it);
				return true;
			}
			return false;
		}

		/**
		 * @brief Reset and setup all source processors
		 */
		bool ResetAndSetup() {												
			// Get listener Model pointer
			std::shared_ptr<BRTListenerModel::CListenerModelBase> _listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(GetIDEntryPoint("listenerModelID")->GetData());
			if (_listenerModel == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This environment has not been connected to a listener Model.");
				return false;
			}		
			
			brtManager->BeginSetup();			
			bool result = true;
			for (auto & it : sourcesConnectedProcessors) {				
				result = result && it.Setup(reflectionOrder, maxDistanceSourcesToListener, windowSlopeDistance, room, _listenerModel);
				result = result && it.ConnectToListenerModel(_listenerModel);
			}
			brtManager->EndSetup();			
			return result;
		}

		/////////////////
		// Attributes
		/////////////////
		mutable std::mutex mutex;									// To avoid access collisions		
		std::vector<CISMProcessors> sourcesConnectedProcessors; // A processor per connected source
		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;		

		std::shared_ptr<BRTServices::CRoom> room;

		//bool enableDirectPath; // Enable direct path
		bool enableReverbPath; // Enable reverb path

		int reflectionOrder; // Reflection order
		float maxDistanceSourcesToListener; // Maximum distance from sources to listener to consider reflections
		float windowSlopeDistance; // Distance for smoothing the window slope

	};
}
#endif
