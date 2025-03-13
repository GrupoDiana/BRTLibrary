/**
* \class CSDNEnvironmentModel
*
* \brief This class implements the SDN environment model. It instantiates an SDN processor for each source that connects to it. 
* \date	Sep 2024
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

#ifndef _C_SDN_ENVIRONMENT_MODEL_HPP_
#define _C_SDN_ENVIRONMENT_MODEL_HPP_

#include <memory>
#include <ListenerModels/ListenerModelBase.hpp>
#include <EnvironmentModels/EnvironmentModelBase.hpp>
#include <EnvironmentModels/SDNEnvironment/SDNEnvironmentProcessor.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTEnvironmentModel {
	
	class CSDNEnvironmentModel : public CEnviromentModelBase {
		
		class CSDNProcessors {
		public:
			
			CSDNProcessors(std::string _sourceID, BRTBase::CBRTManager * brtManager)
				: sourceID { _sourceID } {
				
				SDNProcessor = brtManager->CreateProcessor<BRTEnvironmentModel::CSDNEnvironmentProcessor>(brtManager);					
				SDNProcessor->Setup(_sourceID);
			}	

			/**
			 * @brief Remove processor from BRT
			 * @param brtManager brtManager pointer
			 */
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(SDNProcessor);				
			}

			/**
			 * @brief Set Room dimensions and Room Centre
			 * @param _roomDimensions length, width and height of the room
			 * @param _roomCentre centre of the room
			 */
			void SetupRoom(Common::CVector3 _roomDimensions, Common::CVector3 _roomCentre) {
				SDNProcessor->SetupRoom(_roomDimensions, _roomCentre);
			}
			
			/**
			 * @brief Set Wall absortion coefficientes per band
			 * @param _wallIndex Wall where the coefficients are to be placed. 
			 * @param _wallAbsortions absortion coefficients per band
			 */
			void SetWallAbsortion(int _wallIndex, std::vector<float> _wallAbsortions) {									
				SDNProcessor->SetWallFreqAbsorption(_wallIndex, _wallAbsortions);				
			}
			
			/**
			 * @brief Update the configuration of the processor
			 * @param _enableDirectPath enable or disable direct path
			 * @param _enableReverbPath enable or disable reverb path
			 */
			void SetConfiguration(bool _enableDirectPath, bool _enableReverbPath) {
				if (_enableDirectPath) {
					SDNProcessor->MuteLOS(false);
				} else {
					SDNProcessor->MuteLOS(true);
				}

				if (_enableReverbPath) {
					SDNProcessor->MuteReverbPath(false);
				} else {
					SDNProcessor->MuteReverbPath(true);
				}										
			}
			
			/**
			 * @brief Connect SDN processor to a listener model
			 * @param _listener listener model to connect
			 * @return TRUE if the connection is successful
			 */
			bool ConnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listener) {
				return (SDNProcessor->ConnectToListenerModel(_listener));
			}
			
			/**
			 * @brief Disconnect SDN processor to a listener model
			 * @param _listener listener model to disconnect
			 * @return TRUE if the disconnection is successful
			 */
			bool DisconnectToListenerModel(std::shared_ptr<BRTListenerModel::CListenerModelBase> _listener) {
				return (SDNProcessor->DisconnectToListenerModel(_listener));
			}
			
			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) { 
					SDNProcessor->EnableProcessor(); 					

				} else { 
					SDNProcessor->DisableProcessor(); 					
				}
			}

			/**
			 * @brief Set the gain of the processor
			 * @param _gain New gain value
			 */
			void SetGain(float _gain) {
				SDNProcessor->SetGain(_gain);
			}

			/**
			 * @brief Reset processor buffers
			*/
			void ResetBuffers() {			
				SDNProcessor->ResetProcessBuffers();
			}

			// Attributes
			std::string sourceID;
			std::shared_ptr<BRTEnvironmentModel::CSDNEnvironmentProcessor> SDNProcessor;			
		};

	public:
		CSDNEnvironmentModel(const std::string& _environmentModelID, BRTBase::CBRTManager * _brtManager)
			: CEnviromentModelBase(_environmentModelID)
			, brtManager { _brtManager }
			, enableDirectPath { true }
			, enableReverbPath { true } { }

		/**
		 * @brief Destructor
		*/
		~CSDNEnvironmentModel() {
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
		void EnableDirectPath() override {
			enableDirectPath = true;
			SetConfigurationInALLSourcesProcessors();
		}
		/**
		 * @brief Disable Direct Path
		 */
		void DisableDirectPath() override {
			enableDirectPath = false;
			SetConfigurationInALLSourcesProcessors();
		}		
		/**
		 * @brief Check if Direct Path is enabled
		 * @return True if Direct Path is enabled
		 */
		bool IsDirectPathEnabled() override {
			return enableDirectPath;
		}

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
		 * @brief Update room geometry. Called from father class
		*/
		void UpdateRoomGeometry() override {
			std::lock_guard<std::mutex> l(mutex);

			//Get Room dimensions and Room Centre from CRoom object
			Common::CVector3 roomDimensions = GetRoom().GetShoeBoxRoomSize();
			Common::CVector3 roomCentre = GetRoom().GetCenter();
			// Update Room dimensions and Room Centre in all sources processors
			for (auto & it : sourcesConnectedProcessors) {
				it.SetupRoom(roomDimensions, roomCentre);
			}
		}

		/**
		 * @brief Update room wall absortion. Called from father class
		 * @param _wallIndex Pointer to the source
		*/
		void UpdateRoomWallAbsortion(int _wallIndex) override {
			std::lock_guard<std::mutex> l(mutex);			
			std::vector<float> absortionBands = GetRoom().GetWalls().at(_wallIndex).GetAbsortionBand();			
			
			//The SDN has one less band, it does not have the lowest frequency band.
			std::vector<float> _sdnWallAbsortion(absortionBands.begin() + 1, absortionBands.end());
			for (auto& it : sourcesConnectedProcessors) {
				it.SetWallAbsortion(ToSDNWallIndex(_wallIndex), _sdnWallAbsortion);
			}
		}
		
		/**
		 * @brief Update room all walls absortion. Called from father class
		 */
		void UpdateRoomAllWallsAbsortion() override {			
			std::vector<Common::CWall> walls = GetRoom().GetWalls();
			for (int _wallIndex = 0; _wallIndex < walls.size(); _wallIndex++) {
				UpdateRoomWallAbsortion(ToSDNWallIndex(_wallIndex));				
			}
		}

		/**
		 * @brief 
		 * @param _wallIndex 
		 * @return 
		 */
		int ToSDNWallIndex(int _wallIndex) {
			// BRT [front, left, right, back, floor, ceiling]
			// SDN [X0, XSize, Y0, YSize, Z0, ZSize]
						
			switch (_wallIndex) {
			case 0:
				return 1; // front -> XSize
			case 1:
				return 3; // left -> YSize
			case 2:
				return 2; // right -> Y0
			case 3:
				return 0; // back -> X0
			case 4:
				return 4; // floor -> Z0
			case 5:
				return 5; // ceiling -> ZSize
			default:
				return -1;
			}
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
		void SetSourceProcessorsConfiguration(CSDNProcessors & sourceProcessor) {
			sourceProcessor.SetConfiguration(enableDirectPath, enableReverbPath);
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
			CSDNProcessors _newSDNProcessors(_source->GetID(), brtManager);

			// Make connections
			bool control = brtManager->ConnectModuleTransform(_source, _newSDNProcessors.SDNProcessor, "sourcePosition");			
			control = control && brtManager->ConnectModuleID(_source, _newSDNProcessors.SDNProcessor, "sourceID");						
			
			if (_source->GetSourceType() == BRTSourceModel::Directivity/*sourceNeedsListenerPosition*/) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}
		
			control = control && brtManager->ConnectModuleTransform(_listener, _newSDNProcessors.SDNProcessor, "listenerPosition");			
			control = control && brtManager->ConnectModuleID(this, _newSDNProcessors.SDNProcessor, "listenerID");

			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSDNProcessors.SDNProcessor, "inputSamples");
			control = control && _newSDNProcessors.ConnectToListenerModel(_listenerModel);
									
			if (control) {	
				Common::CVector3 roomDimensions = GetRoom().GetShoeBoxRoomSize();
				Common::CVector3 roomCentre = GetRoom().GetCenter();
				if (roomDimensions == Common::CVector3::ZERO()) {
					roomDimensions = Common::CVector3(1.0f, 1.0f, 1.0f);				
				}
				_newSDNProcessors.SetupRoom(roomDimensions, roomCentre);
				SetSourceProcessorsConfiguration(_newSDNProcessors);
				sourcesConnectedProcessors.push_back(std::move(_newSDNProcessors));
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
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSDNProcessors & sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {
				bool control = it->DisconnectToListenerModel(_listenerModel);
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->SDNProcessor, "inputSamples");
				control = control && brtManager->DisconnectModuleID(this, it->SDNProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->SDNProcessor, "listenerPosition");
				if (_source->GetSourceType() == BRTSourceModel::Directivity) {
					control = control && brtManager->DisconnectModuleTransform(_listener, _source, "listenerPosition");
				}
				control = control && brtManager->DisconnectModuleID(_source, it->SDNProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->SDNProcessor, "sourcePosition");
				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it);
				return true;
			}
			return false;
		}

		/////////////////
		// Attributes
		/////////////////
		mutable std::mutex mutex;									// To avoid access collisions		
		std::vector< CSDNProcessors> sourcesConnectedProcessors;
		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;		

		bool enableDirectPath; // Enable direct path
		bool enableReverbPath; // Enable reverb path

	};
}
#endif
