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

#ifndef _CLISTENER_HRTF_MODEL_HPP_
#define _CLISTENER_HRTF_MODEL_HPP_

#include <memory>
#include <Base/ListenerModelBase.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/HRBRIR.hpp>
#include <ProcessingModules/HRTFConvolverProcessor.hpp>
#include <ProcessingModules/NearFieldEffectProcessor.hpp>
#include <Base/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTListenerModel {

	
	class CListenerHRTFModel: public BRTBase::CListenerModelBase {
		
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

				if (enableNearFieldEffect) { nearFieldEffectProcessor->EnableNearFieldEffect(); }
				else { nearFieldEffectProcessor->DisableNearFieldEffect(); }	

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
		CListenerHRTFModel(std::string _listenerModelID, BRTBase::CBRTManager* _brtManager) : 
			BRTBase::CListenerModelBase(_listenerModelID, BRTBase::TListenerModelcharacteristics(true, false, false, true, true, true, true, true)), 
			brtManager{ _brtManager }, enableSpatialization{ true }, enableInterpolation{ true }, enableNearFieldEffect{ false }, enableParallaxCorrection{ true },
			enableITDSimulation{ true }  {
			
			
			//listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Create a empty HRTF		
			listenerHRTF = nullptr;
			CreateHRTFExitPoint();
			CreateHRBRIRExitPoint();
			CreateILDExitPoint();									
		}

		
		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) {			
			
			if (_listenerHRTF->GetSamplingRate() != globalParameters.GetSampleRate()) { 
				SET_RESULT(RESULT_ERROR_NOTSET, "This HRTF has not been assigned to the listener. The sample rate of the HRTF does not match the one set in the library Global Parameters.");
				return false;
			}
			listenerHRTF = _listenerHRTF;			
			GetHRTFExitPoint()->sendDataPtr(listenerHRTF);	
			ResetProcessorBuffers();
						
			return true;
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/		
		std::shared_ptr < BRTServices::CHRTF> GetHRTF() const
		{
			return listenerHRTF;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRTF() {
			listenerHRTF = nullptr;						
		}
		
		///
		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetNearFieldCompensationFilters(std::shared_ptr< BRTServices::CNearFieldCompensationFilters > _listenerILD) {
			listenerNFCFilters = _listenerILD;
			GetILDExitPoint()->sendDataPtr(listenerNFCFilters);				
			return true;
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr <BRTServices::CNearFieldCompensationFilters> GetNearFieldCompensationFilters() const
		{
			return listenerNFCFilters;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveNearFierldCompensationFilters() {
			listenerNFCFilters = nullptr;
		}

		/**
		 * @brief Connect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel > _source) { 
			return ConnectAnySoundSource(_source, false);
		};
		/**
		 * @brief Connect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		bool ConnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel > _source) { 			
			return ConnectAnySoundSource(_source, true);
		};

		/**
		 * @brief Disconnect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the disconnection success
		*/
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceSimpleModel> _source) { 
			return DisconnectAnySoundSource(_source, false);
		};
		/**
		 * @brief Disconnect a new source to this listener
		 * @param _source Pointer to the source
		 * @return True if the disconnection success
		*/
		bool DisconnectSoundSource(std::shared_ptr<BRTSourceModel::CSourceDirectivityModel> _source) { 
			return DisconnectAnySoundSource(_source, true);
		};
	
		/** \brief Enable binaural spatialization based in HRTF convolution
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableSpatialization() { 
			enableSpatialization = true;	
			SetConfigurationInALLSourcesProcessors();
		}		

		/** \brief Disable binaural spatialization based in HRTF convolution
		*/
		void DisableSpatialization()
		{
			enableSpatialization = false;
			SetConfigurationInALLSourcesProcessors();			
		}

		///** \brief Get the flag for run-time HRTF interpolation
		//*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		//*   \eh Nothing is reported to the error handler.
		//*/
		bool IsSpatializationEnabled() { return enableSpatialization; }

		/** \brief Enable run-time HRTF interpolation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableInterpolation() {
			enableInterpolation = true;
			SetConfigurationInALLSourcesProcessors();			
		}

		/** \brief Disable run-time HRTF interpolation
		*/
		void DisableInterpolation() {
			enableInterpolation = false;
			SetConfigurationInALLSourcesProcessors();			
		}

		/** \brief Get the flag for run-time HRTF interpolation
		*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsInterpolationEnabled() { return enableInterpolation; }

		/** \brief Enable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableNearFieldEffect() {
			enableNearFieldEffect = true;
			SetConfigurationInALLSourcesProcessors();			
		}

		/** \brief Disable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableNearFieldEffect() {
			enableNearFieldEffect = false;
			SetConfigurationInALLSourcesProcessors();		
		}

		/** \brief Get the flag for near field effect enabling
		*	\retval nearFieldEffectEnabled if true, near field effect is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsNearFieldEffectEnabled() { return enableNearFieldEffect; }

		/**
		 * @brief Enable ITD simulation
		*/
		void EnableITDSimulation() { 
			enableITDSimulation = true; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		 * @brief Disable ITD simulation
		*/
		void DisableITDSimulation() { 
			enableITDSimulation = false; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		 * @brief Check if ITD simulation is enabled
		 * @return 
		 */
		bool IsITDSimulationEnabled() { return enableITDSimulation; }

		/**
		 * @brief Enable Parallax Correction
		*/
		void EnableParallaxCorrection() { 
			enableParallaxCorrection = true; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		 * @brief Disable Parallax Correction
		*/
		void DisableParallaxCorrection() { 
			enableParallaxCorrection = false; 
			SetConfigurationInALLSourcesProcessors();
		}

		/**
		* @brief Get Parallax Correction state
		*/
		bool IsParallaxCorrectionEnabled() { return enableParallaxCorrection; }

		/**
		 * @brief Enable model
		 */
		void EnableModel() {			
			std::lock_guard<std::mutex> l(mutex);
			enableModel = true;			
			for (auto& it : sourcesConnectedProcessors) {
				it.SetEnableProcessor(true);
			}
		};

		/**
		 * @brief Disable model
		 */
		void DisableModel() {
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
		 * @brief Implementation of the virtual method to process the data received by the entry points.
		 * @param entryPointID ID of the entry point
		*/
		void Update(std::string entryPointID) {
			// Nothing to do
		}

		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() {
			//std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();						
			if (command.isNull() || command.GetCommand() == "") { return; }

			std::string listenerID = GetIDEntryPoint("listenerID")->GetData();

			if (listenerID == command.GetStringParameter("listenerID")) {				
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
			}		
		}


	private:

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
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		template <typename T>
		bool ConnectAnySoundSource(std::shared_ptr<T> _source, bool sourceNeedsListenerPosition) {			
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
			
			if (sourceNeedsListenerPosition) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}

			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.binauralConvolverProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.nearFieldEffectProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleHRTF(this, _newSourceProcessors.binauralConvolverProcessor, "listenerHRTF");
			control = control && brtManager->ConnectModuleILD(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerILD");
			control = control && brtManager->ConnectModuleID(this, _newSourceProcessors.binauralConvolverProcessor, "listenerID");

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
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		*  @return True if the disconnection success
		*/
		template <typename T>
		bool DisconnectAnySoundSource(std::shared_ptr<T> _source, bool sourceNeedsListenerPosition) {
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

				control = control && brtManager->DisconnectModuleID(this, it->binauralConvolverProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleILD(this, it->nearFieldEffectProcessor, "listenerILD");
				control = control && brtManager->DisconnectModuleHRTF(this, it->binauralConvolverProcessor, "listenerHRTF");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->nearFieldEffectProcessor, "listenerPosition");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->binauralConvolverProcessor, "listenerPosition");

				if (sourceNeedsListenerPosition) {
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
		std::shared_ptr<BRTServices::CNearFieldCompensationFilters> listenerNFCFilters;		// ILD of listener						
		std::vector< CSourceProcessors> sourcesConnectedProcessors;
		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;

		bool enableSpatialization;		// Flags for independent control of processes
		bool enableInterpolation;		// Enables/Disables the interpolation on run time
		bool enableNearFieldEffect;     // Enables/Disables the Near Field Effect
		bool enableParallaxCorrection;	// Enable parallax correction
		bool enableITDSimulation;		// Enable ITD simulation 
				
	};
}
#endif
