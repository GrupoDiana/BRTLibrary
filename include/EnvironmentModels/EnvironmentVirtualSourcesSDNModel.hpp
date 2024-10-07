/**
* \class CEnvironmentVirtualSourcesSDNModel
*
* \brief Declaration of CEnvironmentVirtualSourcesSDNModel class
* \date	Sep 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
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

#ifndef _C_ENVIRONMENT_VIRTUALSOURCES_SDN_MODEL_HPP_
#define _C_ENVIRONMENT_VIRTUALSOURCES_SDN_MODEL_HPP_

#include <memory>
#include <Base/ListenerModelBase.hpp>
#include <Base/EnvironmentModelBase.hpp>
#include <EnvironmentModels/SDNenv/SDNEnvironmentProcessor.hpp>
#include <Base/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTEnvironmentModel {
	
	class CEnvironmentVirtualSourcesSDNModel : public BRTBase::CEnviromentModelBase {
		
		class CSDNProcessors {
		public:
			
			CSDNProcessors(std::string _sourceID, BRTBase::CBRTManager * brtManager)
				: sourceID { _sourceID } {
				
				SDNProcessor = brtManager->CreateProcessor<BRTEnvironmentModel::CSDNEnvironmentProcessor>(brtManager);					
				SDNProcessor->Setup(_sourceID);
			}	

			///**
			// * @brief Remove processor from BRT
			// * @param brtManager brtManager pointer
			//*/
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(SDNProcessor);				
			}
			
			void SetupRoom(Common::CVector3 _roomDimensions, Common::CVector3 _roomCentre) {
				SDNProcessor->SetupRoom(_roomDimensions, _roomCentre);
			}
			
			void SetWallAbsortion(int _wallIndex, std::vector<float> _wallAbsortions) {									
				SDNProcessor->SetWallFreqAbsorption(_wallIndex, _wallAbsortions);				
			}
			

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
			
			bool ConnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listener) {
				return (SDNProcessor->ConnectToListenerModel(_listener));
			}

			bool DisconnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listener) {
				return (SDNProcessor->DisconnectToListenerModel(_listener));
			}
			
			///**
			// * @brief Set processor enable or disable
			// * @param _enableProcessor
			// */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) { 
					SDNProcessor->EnableProcessor(); 					

				} else { 
					SDNProcessor->DisableProcessor(); 					
				}
			}

			///**
			// * @brief Reset processor buffers
			//*/
			//void ResetBuffers() {
			//	binauralConvolverProcessor->ResetSourceConvolutionBuffers();
			//	nearFieldEffectProcessor->ResetProcessBuffers();
			//}

			std::string sourceID;
			std::shared_ptr<BRTEnvironmentModel::CSDNEnvironmentProcessor> SDNProcessor;			
		};

	public:
		CEnvironmentVirtualSourcesSDNModel(const std::string& _environmentModelID, BRTBase::CBRTManager * _brtManager)
			: 
			BRTBase::CEnviromentModelBase(_environmentModelID), 
			brtManager{ _brtManager }  {								
		}

		/**
		 * @brief Destructor
		*/
		~CEnvironmentVirtualSourcesSDNModel() {
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
				
		void EnableDirectPath() override {
			enableDirectPath = true;
			SetConfigurationInALLSourcesProcessors();
		}

		void DisableDirectPath() override {
			enableDirectPath = false;
			SetConfigurationInALLSourcesProcessors();
		}

		bool IsDirectPathEnabled() override {
			return enableDirectPath;
		}

		void EnableReverbPath() override {
			enableReverbPath = true;
			SetConfigurationInALLSourcesProcessors();
		}

		void DisableReverbPath() override {
			enableReverbPath = false;
			SetConfigurationInALLSourcesProcessors();
		}
		
		bool IsReverbPathEnabled() override {
			return enableReverbPath;
		}

		/**
		 * @brief Connect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel > _source) override { 
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

		/**
		 * @brief Disconnect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the disconnection success
		*/
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) override{ 
			return DisconnectAnySoundSource(_source, false);
		};
		/**
		 * @brief Disconnect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the disconnection success
		*/
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) override { 
			return DisconnectAnySoundSource(_source, true);
		};
	
		/**
		 * @brief Reset all processor buffers
		*/
		void ResetProcessorBuffers() {
			//std::lock_guard<std::mutex> l(mutex);
			//for (auto& it : sourcesConnectedProcessors) {
			//	//it.ResetBuffers();
			//}
		}

		/**
		 * @brief Implementation of the virtual method to process the data received by the entry points.
		 * @param entryPointID ID of the entry point
		*/
		void Update(std::string entryPointID) override {
			// Nothing to do
		}

		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() override{
			//std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();						
			if (command.isNull() || command.GetCommand() == "") { return; }

			std::string listenerID = GetIDEntryPoint("listenerID")->GetData();

			/*if (listenerID == command.GetStringParameter("listenerID")) {				
				if (command.GetCommand() == "/listener/enableSpatialization") {
						if (command.GetBoolParameter("enable")) { EnableSpatialization(); }
						else { DisableSpatialization(); }
				}
				else if (command.GetCommand() == "/listener/enableInterpolation") {
					if (command.GetBoolParameter("enable")) { EnableInterpolation(); }
					else { DisableInterpolation(); }
				}
				else if (command.GetCommand() == "/listener/enableNearFieldEffect") {
					if (command.GetBoolParameter("enable")) { EnableNearFieldEffect(); }
					else { DisableNearFieldEffect(); }
				}
				else if (command.GetCommand() == "/listener/enableITD") {
					if (command.GetBoolParameter("enable")) { EnableITDSimulation(); }
					else { DisableITDSimulation(); }
				}
				else if (command.GetCommand() == "/listener/enableParallaxCorrection") {
					if (command.GetBoolParameter("enable")) { EnableParallaxCorrection(); }
					else { DisableParallaxCorrection(); }
				}
				else if (command.GetCommand() == "/listener/resetBuffers") {
					ResetProcessorBuffers();
				}
			}	*/	
		}


	private:

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
			
			// Create a new SDN Processor
			Common::CVector3 roomDimensions = (GetRoom().GetShoeBoxRoomSize());
			if (roomDimensions == Common::CVector3::ZERO()) {
				roomDimensions = Common::CVector3(1.0f, 1.0f, 1.0f);
				SET_RESULT(RESULT_ERROR_NOTSET, "Room dimensions are not set.");				
			}
			CSDNProcessors _newSDNProcessors(_source->GetID(), brtManager);

			// Make connections
			bool control = brtManager->ConnectModuleTransform(_source, _newSDNProcessors.SDNProcessor, "sourcePosition");			
			control = control && brtManager->ConnectModuleID(_source, _newSDNProcessors.SDNProcessor, "sourceID");						
			
			if (sourceNeedsListenerPosition) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}
		
			control = control && brtManager->ConnectModuleTransform(_listener, _newSDNProcessors.SDNProcessor, "listenerPosition");			
			control = control && brtManager->ConnectModuleID(this, _newSDNProcessors.SDNProcessor, "listenerID");

			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSDNProcessors.SDNProcessor, "inputSamples");
			control = control && _newSDNProcessors.ConnectToListenerModel(_listenerModel);
									
			if (control) {				
				_newSDNProcessors.SetupRoom(GetRoom().GetShoeBoxRoomSize(), GetRoom().GetCenter());
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
			
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSDNProcessors & sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {
				bool control = it->DisconnectToListenerModel(_listenerModel);
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->SDNProcessor, "inputSamples");
				control = control && brtManager->DisconnectModuleID(this, it->SDNProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->SDNProcessor, "listenerPosition");
				if (sourceNeedsListenerPosition) {
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
