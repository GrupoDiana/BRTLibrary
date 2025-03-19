/**
* \class CListenerHRTFModel
*
* \brief Declaration of CListenerHRTFModel class
* \date	June 2023
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

#ifndef _CLISTENER_HRTF_MODEL_HPP_
#define _CLISTENER_HRTF_MODEL_HPP_

#include <memory>
#include <ListenerModels/ListenerModelBase.hpp>
#include <EnvironmentModels/EnvironmentModelBase.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/HRBRIR.hpp>
#include <ProcessingModules/HRTFConvolverProcessor.hpp>
#include <ProcessingModules/NearFieldEffectProcessor.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTListenerModel {

	
	class CListenerHRTFModel : public CListenerModelBase {
		
		class CSourceProcessors {
		public:
			
			CSourceProcessors(std::string _sourceID, BRTBase::CBRTManager* brtManager) : sourceID{ _sourceID} {
				binauralConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CHRTFConvolverProcessor>();
				nearFieldEffectProcessor = brtManager->CreateProcessor<BRTProcessing::CNearFieldEffectProcessor>();
			}	

			/**
			 * @brief Remove processor from BRT
			 * @param brtManager brtManager pointer
			*/
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(nearFieldEffectProcessor);
				brtManager->RemoveProcessor(binauralConvolverProcessor);
			}

			/**
			 * @brief Set proccesor configuration
			 * @param enableSpatialization Spatialization state
			 * @param enableInterpolation Interpolation state
			 * @param enableNearFieldEffect Nearfield state
			*/
			void SetConfiguration(bool enableSpatialization, bool enableInterpolation, bool enableNearFieldEffect, bool enableITD, bool enableParallaxCorrection) {
				if (enableSpatialization) { binauralConvolverProcessor->EnableSpatialization(); }
				else { binauralConvolverProcessor->DisableSpatialization(); }

				if (enableInterpolation) { binauralConvolverProcessor->EnableInterpolation(); }
				else { binauralConvolverProcessor->DisableInterpolation(); }

				if (enableNearFieldEffect) {
					nearFieldEffectProcessor->EnableProcessor();
				} else {
					nearFieldEffectProcessor->DisableProcessor();
				}	

				if (enableITD) {binauralConvolverProcessor->EnableITDSimulation();}
				else {binauralConvolverProcessor->DisableITDSimulation();}

				if (enableParallaxCorrection) {binauralConvolverProcessor->EnableParallaxCorrection();}
				else {binauralConvolverProcessor->DisableParallaxCorrection();}
			}

			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) { 
					binauralConvolverProcessor->EnableProcessor(); 
					nearFieldEffectProcessor->EnableProcessor();

				} else { 
					binauralConvolverProcessor->DisableProcessor(); 
					nearFieldEffectProcessor->DisableProcessor();
				}
			}

			/**
			 * @brief Reset processor buffers
			*/
			void ResetBuffers() {
				binauralConvolverProcessor->ResetSourceConvolutionBuffers();
				nearFieldEffectProcessor->ResetProcessBuffers();
			}

			std::string sourceID;
			std::shared_ptr <BRTProcessing::CHRTFConvolverProcessor> binauralConvolverProcessor;
			std::shared_ptr <BRTProcessing::CNearFieldEffectProcessor> nearFieldEffectProcessor;
		};

	public:
		CListenerHRTFModel(std::string _listenerModelID, BRTBase::CBRTManager* _brtManager) 
			: CListenerModelBase(_listenerModelID, TListenerModelcharacteristics(true, false, false, true, true, true, true, true))
			, brtManager{ _brtManager }
			, enableSpatialization{ true }
			, enableInterpolation{ true }
			, enableNearFieldEffect{ false }
			, enableParallaxCorrection{ true }
			, enableITDSimulation{ true }  {
			
			
			//listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Create a empty HRTF		
			listenerHRTF = nullptr;
			CreateHRTFExitPoint();
			CreateHRBRIRExitPoint();
			CreateILDExitPoint();									
		}

		
		/** \brief SET HRTF to this listener model
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) override{			
			
			if (_listenerHRTF->GetSamplingRate() != globalParameters.GetSampleRate()) { 
				SET_RESULT(RESULT_ERROR_NOTSET, "This HRTF has not been assigned to the listener. The sample rate of the HRTF does not match the one set in the library Global Parameters.");
				return false;
			}
			listenerHRTF = _listenerHRTF;			
			GetHRTFExitPoint()->sendDataPtr(listenerHRTF);	
			ResetProcessorBuffers();
						
			return true;
		}

		/** \brief Get HRTF of this listener model
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/		
		std::shared_ptr<BRTServices::CHRTF> GetHRTF() const override
		{
			return listenerHRTF;
		}

		/** \brief Remove the HRTF of this listener model
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRTF() override {
			listenerHRTF = nullptr;						
		}
		
		///
		/** \brief SET Near field compesation filter of listener model
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetNearFieldCompensationFilters(std::shared_ptr<BRTServices::CSOSFilters> _listenerILD) override {
			listenerNFCFilters = _listenerILD;
			GetILDExitPoint()->sendDataPtr(listenerNFCFilters);				
			return true;
		}

		/** \brief Get Near field compesation filter of listener model
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr<BRTServices::CSOSFilters> GetNearFieldCompensationFilters() const override
		{
			return listenerNFCFilters;
		}

		/** \brief Remove the Near field compesation of this listener model
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveNearFierldCompensationFilters() override {
			listenerNFCFilters = nullptr;
		}
		
		/**
		 * @brief Connect a new source to this listener model
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) override { 
		 
			if (_source == nullptr) return false;
			return ConnectAnySoundSource(_source);
		}

		/**
		 * @brief Connect a new source to this listener model
		 * @param _sourceID Source ID
		 * @return True if the connection success
		 */
		bool ConnectSoundSource(const std::string & _sourceID) override{ 
			 std::shared_ptr<BRTSourceModel::CSourceModelBase> _source = brtManager->GetSoundSource(_sourceID);
			if (_source == nullptr) return false;
			 return ConnectAnySoundSource(_source);
		}

		/**
		 * @brief Disconnect a new source to this listener model
		 * @param _source Pointer to the source
		 * @return True if the disconnection success
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

		/** \brief Enable binaural spatialization based in HRTF convolution
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableSpatialization() override { 
			enableSpatialization = true;	
			SetConfigurationInALLSourcesProcessors();
		}		

		/** \brief Disable binaural spatialization based in HRTF convolution
		*/
		void DisableSpatialization() override
		{
			enableSpatialization = false;
			SetConfigurationInALLSourcesProcessors();			
		}

		///** \brief Get the flag for run-time HRTF interpolation
		//*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		//*   \eh Nothing is reported to the error handler.
		//*/
		bool IsSpatializationEnabled() override { return enableSpatialization; }

		/** \brief Enable run-time HRTF interpolation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableInterpolation() override {
			enableInterpolation = true;
			SetConfigurationInALLSourcesProcessors();			
		}

		/** \brief Disable run-time HRTF interpolation
		*/
		void DisableInterpolation() override {
			enableInterpolation = false;
			SetConfigurationInALLSourcesProcessors();			
		}

		/** \brief Get the flag for run-time HRTF interpolation
		*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsInterpolationEnabled() override { return enableInterpolation; }

		/** \brief Enable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableNearFieldEffect() override {
			enableNearFieldEffect = true;
			SetConfigurationInALLSourcesProcessors();			
		}

		/** \brief Disable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableNearFieldEffect() override {
			enableNearFieldEffect = false;
			SetConfigurationInALLSourcesProcessors();		
		}

		/** \brief Get the flag for near field effect enabling
		*	\retval nearFieldEffectEnabled if true, near field effect is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsNearFieldEffectEnabled() override { return enableNearFieldEffect; }

		/**
		 * @brief Enable ITD simulation
		*/
		void EnableITDSimulation() override { 
			enableITDSimulation = true; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		 * @brief Disable ITD simulation
		*/
		void DisableITDSimulation() override { 
			enableITDSimulation = false; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		 * @brief Check if ITD simulation is enabled
		 * @return 
		 */
		bool IsITDSimulationEnabled() override { return enableITDSimulation; }

		/**
		 * @brief Enable Parallax Correction
		*/
		void EnableParallaxCorrection() override { 
			enableParallaxCorrection = true; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		 * @brief Disable Parallax Correction
		*/
		void DisableParallaxCorrection() override { 
			enableParallaxCorrection = false; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		* @brief Get Parallax Correction state
		*/
		bool IsParallaxCorrectionEnabled() override { return enableParallaxCorrection; }

		/**
		 * @brief Enable model
		 */
		void EnableModel() override {			
			std::lock_guard<std::mutex> l(mutex);
			enableModel = true;			
			for (auto& it : sourcesConnectedProcessors) {
				it.SetEnableProcessor(true);
			}
		};

		/**
		 * @brief Disable model
		 */
		void DisableModel() override {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = false;			
			for (auto& it : sourcesConnectedProcessors) {
				it.SetEnableProcessor(false);
			}
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
		 * @brief Connect an environment to this listener model
		 * @param _environmentModel ID
		 * @return True if the connection success
		*/
		bool ConnectEnvironmentModel(const std::string & _environmentModelID) override {

			std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase> _environmentModel = brtManager->GetEnvironmentModel<BRTEnvironmentModel::CEnviromentModelBase>(_environmentModelID);
			if (_environmentModel == nullptr) return false;

			return ConnectEnvironmentModel(_environmentModel);
		};
		
		/**
		 * @brief Disconnect an environment from this listener model
		 * @param _environmentModel ID
		 * @return True if the disconnection success
		*/
		bool DisconnectEnvironmentModel(const std::string & _environmentModelID) override {

			std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase> _environmentModel = brtManager->GetEnvironmentModel<BRTEnvironmentModel::CEnviromentModelBase>(_environmentModelID);
			if (_environmentModel == nullptr) return false;

			return DisconnectEnvironmentModel(_environmentModel);
		};

		/**
		 * @brief Implementation of the virtual method to process the data received by the entry points.
		 * @param entryPointID ID of the entry point
		*/
		//void Update(std::string entryPointID) override {
		//	// Nothing to do
		//}

		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() override {
			//std::lock_guard<std::mutex> l(mutex);
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetCommand() == "") {
				return;
			}

			std::string listenerID = GetIDEntryPoint("listenerID")->GetData();

			if (listenerID == command.GetStringParameter("listenerID")) {
				if (command.GetCommand() == "/listener/enableSpatialization") {
					if (command.GetBoolParameter("enable")) {
						EnableSpatialization();
					} else {
						DisableSpatialization();
					}
				} else if (command.GetCommand() == "/listener/enableInterpolation") {
					if (command.GetBoolParameter("enable")) {
						EnableInterpolation();
					} else {
						DisableInterpolation();
					}
				} else if (command.GetCommand() == "/listener/enableNearFieldEffect") {
					if (command.GetBoolParameter("enable")) {
						EnableNearFieldEffect();
					} else {
						DisableNearFieldEffect();
					}
				} else if (command.GetCommand() == "/listener/enableITD") {
					if (command.GetBoolParameter("enable")) {
						EnableITDSimulation();
					} else {
						DisableITDSimulation();
					}
				} else if (command.GetCommand() == "/listener/enableParallaxCorrection") {
					if (command.GetBoolParameter("enable")) {
						EnableParallaxCorrection();
					} else {
						DisableParallaxCorrection();
					}
				} else if (command.GetCommand() == "/listener/resetBuffers") {
					ResetProcessorBuffers();
				}
			}
		}
	private:

		/**
		 * @brief Connect environment model to this listener model
		 * @param _environment model Pointer
		 * @return True if the connection success
		*/
		bool ConnectEnvironmentModel(std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase> _environmentModel) {

			if (_environmentModel == nullptr) return false;
			if (_environmentModel->IsConnectedToListenerModel()) {
				return false;
			};

			bool control;
			control = brtManager->ConnectModuleID(this, _environmentModel, "listenerModelID");
			SendMyID();

			environmentModelsConnected.push_back(_environmentModel);
			return control;
		};

		/**
		 * @brief Disconnect environment model to this listener model
		 * @param _environmentModel Pointer
		 * @return True if the disconnection success
		*/
		bool DisconnectEnvironmentModel(std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase> _environmentModel) {

			if (_environmentModel == nullptr) return false;
						
			auto it = find(environmentModelsConnected.begin(), environmentModelsConnected.end(), _environmentModel);	
			if (it == environmentModelsConnected.end()) return false;																										
			bool control;
			control = brtManager->DisconnectModuleID(this, _environmentModel, "listenerModelID");
			environmentModelsConnected.erase(it);
			return control;
		};

		/**
		 * @brief Find model in a shared_ptr list
		 * @tparam T base type
		 * @param _list list of shared_ptr of T objects
		 * @param _ID ID to find
		 * @return pointer to the model if found, otherwise nullptr
		 */
		template <typename T>
		std::shared_ptr<T> FindModel(std::vector<std::shared_ptr<T>> _list, const std::string & _ID) {
			auto it = std::find_if(_list.begin(), _list.end(), [&_ID](std::shared_ptr<T> & item) { return item->GetID() == _ID; });
			if (it != _list.end()) {
				return *it;
			}
			return nullptr;
		}

		/**
		 * @brief Update Configuration in all source processor
		*/
		void SetConfigurationInALLSourcesProcessors() {
			std::lock_guard<std::mutex> l(mutex);
			for (auto& it : sourcesConnectedProcessors) {								
				SetSourceProcessorsConfiguration(it);				
			}			
		}
		/**
		 * @brief Update configuration just in one source processor
		 * @param sourceProcessor 
		*/
		void SetSourceProcessorsConfiguration(CSourceProcessors& sourceProcessor) {			
			sourceProcessor.SetConfiguration(enableSpatialization, enableInterpolation, enableNearFieldEffect, enableITDSimulation, enableParallaxCorrection);
		}

		
		/**
		 * @brief Connect a new source to this listener		 
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/		
		bool ConnectAnySoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) {
			std::lock_guard<std::mutex> l(mutex);

			// Get listener pointer
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(GetIDEntryPoint("listenerID")->GetData());
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This listener Model has not been connected to a listener.");
				return false;
			}
			// Make connections
			CSourceProcessors _newSourceProcessors(_source->GetID(), brtManager);

			bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.binauralConvolverProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleTransform(_source, _newSourceProcessors.nearFieldEffectProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.binauralConvolverProcessor, "sourceID");
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.nearFieldEffectProcessor, "sourceID");

			if (_source->GetSourceType() == BRTSourceModel::TSourceType::Directivity) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}

			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.binauralConvolverProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.nearFieldEffectProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleHRTF(this, _newSourceProcessors.binauralConvolverProcessor, "listenerHRTF");
			control = control && brtManager->ConnectModuleILD(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerILD");
			control = control && brtManager->ConnectModuleID(_listener, _newSourceProcessors.binauralConvolverProcessor, "listenerID"); // this or listener??

			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.binauralConvolverProcessor, "inputSamples");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "leftEar", _newSourceProcessors.nearFieldEffectProcessor, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "rightEar", _newSourceProcessors.nearFieldEffectProcessor, "rightEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "leftEar", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "rightEar", this, "rightEar");

			if (control) {
				SetSourceProcessorsConfiguration(_newSourceProcessors);
				sourcesConnectedProcessors.push_back(std::move(_newSourceProcessors));
				return true;
			}
			return false;
		}
		/**
		 * @brief Disconnect a new source to this listener		 
		 * @param _source Pointer to the source
		*  @return True if the disconnection success
		*/		
		bool DisconnectAnySoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) {
			std::lock_guard<std::mutex> l(mutex);
			
			// Get listener pointer
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(GetIDEntryPoint("listenerID")->GetData());
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This listener Model has not been connected to a listener.");
				return false;
			}			
			// Get source			
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceProcessors& sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {
				bool control = brtManager->DisconnectModulesSamples(it->nearFieldEffectProcessor, "leftEar", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(it->nearFieldEffectProcessor, "rightEar", this, "rightEar");
				control = control && brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "leftEar", it->nearFieldEffectProcessor, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "rightEar", it->nearFieldEffectProcessor, "rightEar");
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->binauralConvolverProcessor, "inputSamples");

				control = control && brtManager->DisconnectModuleID(_listener, it->binauralConvolverProcessor, "listenerID"); // this or listener??
				control = control && brtManager->DisconnectModuleILD(this, it->nearFieldEffectProcessor, "listenerILD");
				control = control && brtManager->DisconnectModuleHRTF(this, it->binauralConvolverProcessor, "listenerHRTF");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->nearFieldEffectProcessor, "listenerPosition");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->binauralConvolverProcessor, "listenerPosition");

				if (_source->GetSourceType() == BRTSourceModel::Directivity) {
					control = control && brtManager->DisconnectModuleTransform(_listener, _source, "listenerPosition");
				}
				control = control && brtManager->DisconnectModuleID(_source, it->nearFieldEffectProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleID(_source, it->binauralConvolverProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->nearFieldEffectProcessor, "sourcePosition");
				control = control && brtManager->DisconnectModuleTransform(_source, it->binauralConvolverProcessor, "sourcePosition");

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
		std::string listenerID;										// Store unique listener ID
		std::shared_ptr<BRTServices::CHRTF>		listenerHRTF;		// HRTF of listener			
		std::shared_ptr<BRTServices::CSOSFilters> listenerNFCFilters;		// SOS Filter of listener						
		std::vector< CSourceProcessors> sourcesConnectedProcessors;
		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;

		bool enableSpatialization;		// Flags for independent control of processes
		bool enableInterpolation;		// Enables/Disables the interpolation on run time
		bool enableNearFieldEffect;     // Enables/Disables the Near Field Effect
		bool enableParallaxCorrection;	// Enable parallax correction
		bool enableITDSimulation;		// Enable ITD simulation 

		std::vector<std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase>> environmentModelsConnected; // Environment models connected to this
	};
}
#endif
