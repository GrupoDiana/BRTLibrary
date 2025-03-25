/**
* \class CListenerEnviromentBRIRModel
*
* \brief Declaration of CListenerEnviromentBRIRModel class
* \date	June 2024
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

#ifndef _CLISTENER_ENVIRONMENT_BRIR_MODEL_HPP_
#define _CLISTENER_ENVIRONMENT_BRIR_MODEL_HPP_

#include <memory>
#include <ListenerModels/ListenerModelBase.hpp>
#include <ServiceModules/HRBRIR.hpp>
#include <ProcessingModules/HRTFConvolverProcessor.hpp>
#include <ProcessingModules/NearFieldEffectProcessor.hpp>
#include <ProcessingModules/DistanceAttenuatorProcessor.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTListenerModel {

	
	class CListenerEnvironmentBRIRModel : public CListenerModelBase {
		
		class CSourceProcessors {
		public:
			
			CSourceProcessors(std::string _sourceID, BRTBase::CBRTManager* brtManager) : sourceID{ _sourceID} {
				binauralConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CHRTFConvolverProcessor>();
				binauralConvolverProcessor->DisableParallaxCorrection();
				distanceAttenuatorProcessor = brtManager->CreateProcessor<BRTProcessing::CDistanceAttenuatorProcessor>();
			}	

			/**
			 * @brief Remove processor from BRT
			 * @param brtManager brtManager pointer
			*/
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";				
				brtManager->RemoveProcessor(binauralConvolverProcessor);
			}

			/**
			 * @brief Set proccesor configuration
			 * @param enableSpatialization Spatialization state
			 * @param enableInterpolation Interpolation state
			 * @param enableNearFieldEffect Nearfield state
			*/
			void SetConfiguration(bool _enableSpatialization, bool _enableInterpolation
				, bool _enableDistanceAttenuation, float _distanceAttenuationFactorDB, float _referenceAttenuationDistance) {
				
				if (_enableSpatialization) { binauralConvolverProcessor->EnableSpatialization(); }
				else { binauralConvolverProcessor->DisableSpatialization(); }

				if (_enableInterpolation) { binauralConvolverProcessor->EnableInterpolation(); }
				else { binauralConvolverProcessor->DisableInterpolation(); }
						
				if (_enableDistanceAttenuation) { distanceAttenuatorProcessor->EnableProcessor();				
				} else { distanceAttenuatorProcessor->DisableProcessor();
				}				
				distanceAttenuatorProcessor->SetDistanceAttenuationFactor(_distanceAttenuationFactorDB);
				distanceAttenuatorProcessor->SetReferenceAttenuationDistance(_referenceAttenuationDistance);

				binauralConvolverProcessor->DisableITDSimulation();								
				binauralConvolverProcessor->DisableParallaxCorrection();				
			}

			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) {
					binauralConvolverProcessor->EnableProcessor();					

				}
				else {
					binauralConvolverProcessor->DisableProcessor();					
				}
			}

			/**
			* @brief Set the distance attenuation factor in decibels
			* @param _distanceAttenuationFactorDB distance attenuation factor in decibels
			*/
			void SetDistanceAttenuationFactor(float _distanceAttenuationFactorDB) {
				distanceAttenuatorProcessor->SetDistanceAttenuationFactor(_distanceAttenuationFactorDB);
			}

			/**
			 * @brief Reset processor buffers
			*/
			void ResetBuffers() {
				binauralConvolverProcessor->ResetSourceConvolutionBuffers();				
			}

			std::string sourceID;
			std::shared_ptr <BRTProcessing::CHRTFConvolverProcessor> binauralConvolverProcessor;
			std::shared_ptr<BRTProcessing::CDistanceAttenuatorProcessor> distanceAttenuatorProcessor;
		};

	public:
		CListenerEnvironmentBRIRModel(std::string _listenerID, BRTBase::CBRTManager* _brtManager) 
			: CListenerModelBase(_listenerID, TListenerModelcharacteristics(false, true, false, false, false, false, true, true, true))
			, brtManager{ _brtManager }
			, enableSpatialization{ true }
			, enableInterpolation{ true }
			, enableDistanceAttenuation { false }
			, distanceAttenuationFactorDB { globalParameters.reverbDistanceAttenuationFactorDB }
			, referenceAttenuationDistance { globalParameters.referenceAttenuationDistance } 
		{
									
			listenerHRBRIR = nullptr;
			CreateHRBRIRExitPoint();
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

		/** \brief Enable binaural spatialization based in HRTF convolution
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableSpatialization() override {
			enableSpatialization = true;
			SetConfigurationInALLSourcesProcessors();
		}

		/** \brief Disable binaural spatialization based in HRTF convolution
		*/
		void DisableSpatialization() override {
			enableSpatialization = false;
			SetConfigurationInALLSourcesProcessors();
		}

		/** \brief Get the flag for run-time HRTF interpolation
		/*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		/*   \eh Nothing is reported to the error handler.
		/*/
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

		/** \brief SET HRBRIR of listener
		*	\param[in] pointer to HRBRIR to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRBRIR(std::shared_ptr<BRTServices::CHRBRIR> _listenerBRIR) override {			    
			if (_listenerBRIR->GetSamplingRate() != globalParameters.GetSampleRate()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This HRTF has not been assigned to the listener. The sample rate of the HRTF does not match the one set in the library Global Parameters.");
				return false;
			}
			listenerHRBRIR = _listenerBRIR;
			GetHRBRIRExitPoint()->sendDataPtr(_listenerBRIR);
			ResetProcessorBuffers();
						
			return true;
		}

		/** \brief Get HRBRIR of listener
		*	\retval HRBRIR pointer to current listener HRBRIR
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr<BRTServices::CHRBRIR> GetHRBRIR() const override
		{
			return listenerHRBRIR;
		}

		/** \brief Remove the HRBRIR of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRBRIR() override {						
			listenerHRBRIR = nullptr;
		}

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
		 * @brief Set the distance attenuation factor in dB
		 * @param _distanceAttenuationFactorDB Attenuation factor in dB
		 */
		bool SetDistanceAttenuationFactor(float _distanceAttenuationFactorDB) override {
			std::lock_guard<std::mutex> l(mutex);
			if (_distanceAttenuationFactorDB > 0) {
				SET_RESULT(RESULT_ERROR_PHYSICS, "Attenuation factor in decibels must be a negative value");
				return false;
			}
			distanceAttenuationFactorDB = _distanceAttenuationFactorDB;
			for (auto & it : sourcesConnectedProcessors) {
				it.SetDistanceAttenuationFactor(_distanceAttenuationFactorDB);
			}
			return true;
		}

		/**
		 * @brief Get the distance attenuation factor in dB
		 * @return Distance attenuation factor in decibels
		 */
		float GetDistanceAttenuationFactor() override {
			return distanceAttenuationFactorDB;
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
			for (auto& it : sourcesConnectedProcessors) {
				it.ResetBuffers();
			}
		}
		
		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() override {
			std::lock_guard<std::mutex> l(mutex);
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();						
			if (command.isNull() || command.GetCommand() == "") { return; }

			if (listenerID == command.GetStringParameter("listenerID")) {				
				if (command.GetCommand() == "/listener/enableSpatialization") {
						if (command.GetBoolParameter("enable")) { EnableSpatialization(); }
						else { DisableSpatialization(); }
				}
				else if (command.GetCommand() == "/listener/enableInterpolation") {
					if (command.GetBoolParameter("enable")) { EnableInterpolation(); }
					else { DisableInterpolation(); }
				}
				
				else if (command.GetCommand() == "/listener/enableITD") {
					if (command.GetBoolParameter("enable")) { EnableITDSimulation(); }
					else { DisableITDSimulation(); }
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
			sourceProcessor.SetConfiguration(enableSpatialization, enableInterpolation, enableDistanceAttenuation, distanceAttenuationFactorDB, referenceAttenuationDistance);
		}

		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
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

			bool control = true;
			// Connect soundsource to listener, just in case it is a directivity source		
			if (_source->GetSourceType() == BRTSourceModel::Directivity) {
				control = control && brtManager->ConnectModuleTransform(this, _source, "listenerPosition");
			}			
			// Create new set of processors
			CSourceProcessors _newSourceProcessors(_source->GetID(), brtManager);			
			//_newSourceProcessors.distanceAttenuatorProcessor->Setup(distanceAttenuationFactorDB, globalParameters.referenceAttenuationDistance);
			//_newSourceProcessors.distanceAttenuatorProcessor->DisableProcessor();

			// Connect source and listener to processors
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.distanceAttenuatorProcessor, "sourceID");						
			control = control && brtManager->ConnectModuleTransform(_source, _newSourceProcessors.distanceAttenuatorProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_listener, _newSourceProcessors.distanceAttenuatorProcessor, "listenerID");
			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.distanceAttenuatorProcessor, "listenerPosition");			

			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.binauralConvolverProcessor, "sourceID");						
			control = control && brtManager->ConnectModuleTransform(_source, _newSourceProcessors.binauralConvolverProcessor, "sourcePosition");			
			control = control && brtManager->ConnectModuleID(_listener, _newSourceProcessors.binauralConvolverProcessor, "listenerID");
			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.binauralConvolverProcessor, "listenerPosition");			
			control = control && brtManager->ConnectModuleHRBRIR(this, _newSourceProcessors.binauralConvolverProcessor, "listenerHRBRIR");			
			
			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.distanceAttenuatorProcessor, "inputSamples");			
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.distanceAttenuatorProcessor, "outputSamples", _newSourceProcessors.binauralConvolverProcessor, "inputSamples");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "leftEar", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "rightEar", this, "rightEar");

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
		bool DisconnectAnySoundSource(std::shared_ptr<BRTSourceModel::CSourceModelBase> _source) {
			std::lock_guard<std::mutex> l(mutex);
			
			// Get listener pointer
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(GetIDEntryPoint("listenerID")->GetData());
			if (_listener == nullptr) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This listener Model has not been connected to a listener.");
				return false;
			}

			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceProcessors& sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {
				bool control = brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "leftEar", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "rightEar", this, "rightEar");				
				control = control && brtManager->DisconnectModulesSamples(it->distanceAttenuatorProcessor, "outputSamples", it->binauralConvolverProcessor, "inputSamples");
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->distanceAttenuatorProcessor, "inputSamples");			
							
				control = control && brtManager->DisconnectModuleID(_source, it->binauralConvolverProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->binauralConvolverProcessor, "sourcePosition");
				control = control && brtManager->DisconnectModuleID(_listener, it->binauralConvolverProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->binauralConvolverProcessor, "listenerPosition");
				control = control && brtManager->DisconnectModuleHRBRIR(this, it->binauralConvolverProcessor, "listenerHRBRIR");			

				control = control && brtManager->DisconnectModuleID(_source, it->distanceAttenuatorProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->distanceAttenuatorProcessor, "sourcePosition");
				control = control && brtManager->DisconnectModuleID(_listener, it->distanceAttenuatorProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->distanceAttenuatorProcessor, "listenerPosition");			
				
				if (_source->GetSourceType() == BRTSourceModel::Directivity) {
					control = control && brtManager->DisconnectModuleTransform(this, _source, "listenerPosition");								
				}				
				
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
		std::shared_ptr<BRTServices::CHRBRIR>	listenerHRBRIR;		// RBRIR of listener		
		std::vector< CSourceProcessors> sourcesConnectedProcessors;	// Store the sources connected to this listener		

		bool enableSpatialization;			// Flags for independent control of processes
		bool enableInterpolation;			// Enables/Disables the interpolation on run time						
		bool enableDistanceAttenuation;		// Enable distance attenuation
		float distanceAttenuationFactorDB;	// Distance attenuation factor in decibels
		float referenceAttenuationDistance; // Reference distance for distance attenuation

		BRTBase::CBRTManager * brtManager;
		Common::CGlobalParameters globalParameters;
	};
}
#endif