/**
* \class CListenerAmbisonicReverberantVirtualLoudspeakersModel
*
* \brief Declaration of CListenerAmbisonicReverberantVirtualLoudspeakersModel class
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

#ifndef _CLISTENER_AMBISONIC_REVERBERANT_VIRTUAL_LOUDSPEAKERS_MODEL_HPP_
#define _CLISTENER_AMBISONIC_REVERBERANT_VIRTUAL_LOUDSPEAKERS_MODEL_HPP_

#include <memory>
#include <ListenerModels/ListenerModelBase.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/HRBRIR.hpp>
#include <ServiceModules/AmbisonicBIR.hpp>
#include <ProcessingModules/BilateralAmbisonicEncoderProcessor.hpp>
#include <ProcessingModules/AmbisonicDomainConvolverProcessor.hpp>
#include <ProcessingModules/DistanceAttenuatorProcessor.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>


namespace BRTListenerModel {

	class CListenerAmbisonicReverberantVirtualLoudspeakersModel : public CListenerModelBase {

		class CSourceToBeProcessed {
		public:

			CSourceToBeProcessed(std::string _sourceID, BRTBase::CBRTManager* brtManager) : sourceID{ _sourceID } {
				distanceAttenuatorProcessor = brtManager->CreateProcessor<BRTProcessing::CDistanceAttenuatorProcessor>();
				bilateralAmbisonicEncoderProcessor = brtManager->CreateProcessor <BRTProcessing::CBilateralAmbisonicEncoderProcessor>();
			}
			
			/**
			 * @brief Remove processor from BRT
			 * @param brtManager brtManager pointer
			*/
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(distanceAttenuatorProcessor);
				brtManager->RemoveProcessor(bilateralAmbisonicEncoderProcessor);				
			}

			/**
			 * @brief Set proccesor configuration
			 * @param _ambisonicOrder 
			 * @param _ambisonicNormalization 
			 * @param enableBilateral 			 
			*/
			void SetConfiguration(int _ambisonicOrder, BRTProcessing::TAmbisonicNormalization _ambisonicNormalization
				, bool enableDistanceAttenuation, float _distanceAttenuationFactorDB, float _referenceAttenuationDistance) {

				bilateralAmbisonicEncoderProcessor->SetAmbisonicOrder(_ambisonicOrder);
				bilateralAmbisonicEncoderProcessor->SetAmbisonicNormalization(_ambisonicNormalization);
													
				bilateralAmbisonicEncoderProcessor->DisableITDSimulation();
				bilateralAmbisonicEncoderProcessor->DisableNearFieldEffect();
				bilateralAmbisonicEncoderProcessor->DisableParallaxCorrection(); 
				
				if (enableDistanceAttenuation) {
					distanceAttenuatorProcessor->EnableProcessor();
				} else {
					distanceAttenuatorProcessor->DisableProcessor();
				}								
				distanceAttenuatorProcessor->SetDistanceAttenuationFactor(_distanceAttenuationFactorDB);
				distanceAttenuatorProcessor->SetReferenceAttenuationDistance(_referenceAttenuationDistance);
			}

			/**
			* @brief Set the distance attenuation factor in decibels
			* @param _distanceAttenuationFactorDB distance attenuation factor in decibels
			*/
			void SetDistanceAttenuationFactor(float _distanceAttenuationFactorDB) {
				distanceAttenuatorProcessor->SetDistanceAttenuationFactor(_distanceAttenuationFactorDB);
			}

			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) { 
					bilateralAmbisonicEncoderProcessor->EnableProcessor(); 
					distanceAttenuatorProcessor->EnableProcessor();
				}
				else { 
					bilateralAmbisonicEncoderProcessor->DisableProcessor(); 
					distanceAttenuatorProcessor->DisableProcessor();
				}
			}

			/**		
			 * @brief Reset processor buffers
			*/
			void ResetBuffers() {
				bilateralAmbisonicEncoderProcessor->ResetBuffers();
			}

			std::string sourceID;
			std::shared_ptr<BRTProcessing::CBilateralAmbisonicEncoderProcessor> bilateralAmbisonicEncoderProcessor;
			std::shared_ptr<BRTProcessing::CDistanceAttenuatorProcessor> distanceAttenuatorProcessor;
		};

	public:
		CListenerAmbisonicReverberantVirtualLoudspeakersModel(std::string _listenerID, BRTBase::CBRTManager * _brtManager) 
			: brtManager{ _brtManager }
			, CListenerModelBase(_listenerID, TListenerModelcharacteristics(false, true, true, false, false, false, false, false, true))
			, ambisonicOrder{ 1 }
			, ambisonicNormalization { BRTProcessing::TAmbisonicNormalization::N3D }
			, enableNearFieldEffect{ false }
			, enableParallaxCorrection{ true }
			, enableDistanceAttenuation { false }
			, distanceAttenuationFactorDB {globalParameters.reverbDistanceAttenuationFactorDB}
			, referenceAttenuationDistance { globalParameters.referenceAttenuationDistance } {
			
			listenerHRBRIR = nullptr;												// Create a empty HRTF	class
			listenerAmbisonicIR = std::make_shared<BRTServices::CAmbisonicBIR>();	// Create a empty AmbisonicIR class //TODO CHANGE TO NULLPTR						

			CreateHRBRIRExitPoint();
			CreateILDExitPoint();
			CreateABIRExitPoint();

			leftAmbisonicDomainConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CAmbisonicDomainConvolverProcessor>(Common::T_ear::LEFT);			
			brtManager->ConnectModuleABIR(this, leftAmbisonicDomainConvolverProcessor, "listenerAmbisonicBIR");

			rightAmbisonicDomainConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CAmbisonicDomainConvolverProcessor>(Common::T_ear::RIGHT);
			brtManager->ConnectModuleABIR(this, rightAmbisonicDomainConvolverProcessor, "listenerAmbisonicBIR");			
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
			leftAmbisonicDomainConvolverProcessor->EnableProcessor();
			rightAmbisonicDomainConvolverProcessor->EnableProcessor();
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
			leftAmbisonicDomainConvolverProcessor->DisableProcessor();
			rightAmbisonicDomainConvolverProcessor->DisableProcessor();
		};

		/** \brief SET HRBRIR of listener
		*	\param[in] pointer to HRBRIR to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRBRIR(std::shared_ptr<BRTServices::CHRBRIR> _listenerHRBRIR) override {
			
			if (!_listenerHRBRIR->IsHRBRIRLoaded()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "The HRBRIR has not been assigned becaused it is empty.");
				return false;
			}	
			if (_listenerHRBRIR->GetSamplingRate() != globalParameters.GetSampleRate()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This HRBRIR has not been assigned to the listener. The sample rate of the HRBRIR does not match the one set in the library Global Parameters.");
				return false;
			}
						
			EnableAmbisonicDomainConvolvers(false);		// Stop the convolvers
						
			listenerHRBRIR = _listenerHRBRIR;			
			InitListenerAmbisonicIR();
			GetHRBRIRExitPoint()->sendDataPtr(listenerHRBRIR);
			GetABIRExitPoint()->sendDataPtr(listenerAmbisonicIR);			
			ResetProcessorBuffers();
			
			EnableAmbisonicDomainConvolvers(true);		// Start again
			
			return true;
		}

		/** \brief Get HRBRIR of listener
		*	\retval HRBRIR pointer to current listener HRBRIR
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr<BRTServices::CHRBRIR> GetHRBRIR() const override {
			return listenerHRBRIR;
		}

		/** \brief Remove the HRBRIR of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRBRIR() override {
			listenerHRBRIR = nullptr;
		}

		
		/**
		 * @brief Set the ambisonic order to the listener
		 * @param _ambisonicOrder Ambisonic order to be set up. Currently only orders between 1 to 3 are allowed
		 * @return true if it is a valid order.
		*/
		bool SetAmbisonicOrder(int _ambisonicOrder) override {
			
			if (_ambisonicOrder < 1 || _ambisonicOrder > 3) { return false; }
			if (ambisonicOrder == _ambisonicOrder) { return true; }			
			
			EnableAmbisonicDomainConvolvers(false); // Stop the convolvers
								
			ambisonicOrder = _ambisonicOrder;
			if (listenerHRBRIR->IsHRBRIRLoaded()) { InitListenerAmbisonicIR(); }						
			SetConfigurationInALLSourcesProcessors();
			leftAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);
			rightAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);

			EnableAmbisonicDomainConvolvers(true); // Start again
			return true;
		}

		/**
		 * @brief Return current established ambisonic order
		 * @return order
		*/
		int GetAmbisonicOrder() override {
			return ambisonicOrder;
		}

		/**
		 * @brief Set the ambisonin normalization to be used
		 * @param _ambisonicNormalization Normalization to be set up. 
		*/
		bool SetAmbisonicNormalization(BRTProcessing::TAmbisonicNormalization _ambisonicNormalization) override {
			
			if (ambisonicNormalization == _ambisonicNormalization) { return true; }
			
			EnableAmbisonicDomainConvolvers(false); // Stop the convolvers
			
			ambisonicNormalization = _ambisonicNormalization;
			if (listenerHRBRIR->IsHRBRIRLoaded()) {	InitListenerAmbisonicIR();	}
			SetConfigurationInALLSourcesProcessors();
			
			EnableAmbisonicDomainConvolvers(true); // Start again
			return true;
		}
		
		/**
		 * @brief Set the ambisonin normalization to be used.
		 * @param _ambisonicNormalization Normalization to be set up, valid strings are N3D, SN3D and maxN
		 * @return true if it is a valid normalization
		*/
		bool SetAmbisonicNormalization(std::string _ambisonicNormalization) override {
			
			BRTProcessing::TAmbisonicNormalization temp;
			if (_ambisonicNormalization == "N3D") {			temp = BRTProcessing::TAmbisonicNormalization::N3D; }
			else if (_ambisonicNormalization == "SN3D") {	temp = BRTProcessing::TAmbisonicNormalization::SN3D; }
			else if (_ambisonicNormalization == "maxN") {	temp = BRTProcessing::TAmbisonicNormalization::maxN; }
			else { return false; }
			return SetAmbisonicNormalization(temp);					
		}

		/**
		 * @brief Return current established ambisonic normalization 
		 * @return current established ambisonic normalization
		*/
		BRTProcessing::TAmbisonicNormalization GetAmbisonicNormalization() override {
			return ambisonicNormalization;
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
		 * @brief Connecting to a specific listener transform
		 * @param _listenerID listener ID
		 * @return true if the connection success
		 */
		bool ConnectListenerTransform(const std::string _listenerID) override {
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(_listenerID);
			if (_listener != nullptr) {
				brtManager->ConnectModuleTransform(_listener, leftAmbisonicDomainConvolverProcessor, "listenerPosition");
				brtManager->ConnectModuleTransform(_listener, rightAmbisonicDomainConvolverProcessor, "listenerPosition");
				return true;
			}			
			return false;			
		}

		/**
		 * @brief Disconnecting to a specific listener transform
		 * @param _listenerID listener ID
		 * @return true if the diconnection success
		 */
		bool DisconnectListenerTransform(const std::string _listenerID) override {
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(_listenerID);
			if (_listener != nullptr) {
				brtManager->DisconnectModuleTransform(_listener, leftAmbisonicDomainConvolverProcessor, "listenerPosition");
				brtManager->DisconnectModuleTransform(_listener, rightAmbisonicDomainConvolverProcessor, "listenerPosition");
				return true;
			}
			return false;
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
		 * @brief Reset all processor buffers
		*/
		void ResetProcessorBuffers() {		
			std::lock_guard<std::mutex> l(mutex);
			
			leftAmbisonicDomainConvolverProcessor->ResetChannelsConvolutionBuffers();
			rightAmbisonicDomainConvolverProcessor->ResetChannelsConvolutionBuffers();

			for (auto& it : sourcesConnectedProcessors) {
				it.ResetBuffers();
			}
		}
		
		/**
		 * @brief Implementation of the virtual method for processing the received commands
		*/
		void UpdateCommand() override {
			//std::lock_guard<std::mutex> l(mutex);
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetAddress() == "") { return; }

			if (listenerID == command.GetStringParameter("listenerID")) {
				if (command.GetCommand() == "/listener/setAmbisonicsOrder") {
					SetAmbisonicOrder(command.GetIntParameter("ambisonicsOrder"));					
				}
				else if (command.GetCommand() == "/listener/setAmbisonicsNormalization") {
					SetAmbisonicNormalization(command.GetStringParameter("ambisonicsNormalization"));					
				}
				else if (command.GetCommand() == "/listener/enableNearFieldEffect") {
					if (command.GetBoolParameter("enable")) { EnableNearFieldEffect(); }
					else { DisableNearFieldEffect(); }
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
		
		/////////////////
		// Methods
		/////////////////
		
		/**
		 * @brief Initialize the ambisonic IR of the listener
		 */
		void InitListenerAmbisonicIR() {
			
			std::lock_guard<std::mutex> l(mutex);												
			
			listenerAmbisonicIR->BeginSetup(ambisonicOrder, ambisonicNormalization);			
			bool control = listenerAmbisonicIR->AddImpulseResponsesFromHRIR(listenerHRBRIR);
			if (control) {				
				listenerAmbisonicIR->EndSetup();
			}
			else {
				ASSERT(false, RESULT_ERROR_UNKNOWN, "It has not been possible to initialise the ambisonic IR of the associated listener.", "");
			}
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
		void SetSourceProcessorsConfiguration(CSourceToBeProcessed& sourceProcessor) {
			sourceProcessor.SetConfiguration(ambisonicOrder, ambisonicNormalization, enableDistanceAttenuation, distanceAttenuationFactorDB, referenceAttenuationDistance);
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
			
			bool control = true;			
			// Connect soundsource to listener, just in case it is a directivity source
			if (_source->GetSourceType() == BRTSourceModel::Directivity) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}

			// Create new set of processors and configure them
			CSourceToBeProcessed _newSourceProcessors(_source->GetID(), brtManager);			
						
			// Connect source and listener to processors
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.distanceAttenuatorProcessor, "sourceID");			
			control = control && brtManager->ConnectModuleTransform(_source, _newSourceProcessors.distanceAttenuatorProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_listener, _newSourceProcessors.distanceAttenuatorProcessor, "listenerID");
			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.distanceAttenuatorProcessor, "listenerPosition");			
			
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourceID");												
			control = control && brtManager->ConnectModuleTransform(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_listener, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerID");
			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerPosition");			
			control = control && brtManager->ConnectModuleHRBRIR(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerHRBRIR");
			
			
			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.distanceAttenuatorProcessor, "inputSamples");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.distanceAttenuatorProcessor, "outputSamples", _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "inputSamples");

			control = control && brtManager->ConnectModulesMultipleSamplesVectors(_newSourceProcessors.bilateralAmbisonicEncoderProcessor, "leftAmbisonicChannels", leftAmbisonicDomainConvolverProcessor, "inputChannels");
			control = control && brtManager->ConnectModulesMultipleSamplesVectors(_newSourceProcessors.bilateralAmbisonicEncoderProcessor, "rightAmbisonicChannels", rightAmbisonicDomainConvolverProcessor, "inputChannels");

			control = control && brtManager->ConnectModulesSamples(leftAmbisonicDomainConvolverProcessor, "outputSamples", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(rightAmbisonicDomainConvolverProcessor, "outputSamples", this, "rightEar");

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
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceToBeProcessed& sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {

				bool control = brtManager->DisconnectModulesSamples(leftAmbisonicDomainConvolverProcessor, "outputSamples", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(rightAmbisonicDomainConvolverProcessor, "outputSamples", this, "rightEar");

				control = control && brtManager->DisconnectModulesMultipleSamplesVectors(it->bilateralAmbisonicEncoderProcessor, "leftAmbisonicChannels", leftAmbisonicDomainConvolverProcessor, "inputChannels");
				control = control && brtManager->DisconnectModulesMultipleSamplesVectors(it->bilateralAmbisonicEncoderProcessor, "rightAmbisonicChannels", rightAmbisonicDomainConvolverProcessor, "inputChannels");
				
				control = control && brtManager->DisconnectModulesSamples(it->distanceAttenuatorProcessor, "outputSamples", it->bilateralAmbisonicEncoderProcessor, "inputSamples");
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->distanceAttenuatorProcessor, "inputSamples");
				
				control = control && brtManager->DisconnectModuleID(_source, it->bilateralAmbisonicEncoderProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->bilateralAmbisonicEncoderProcessor, "sourcePosition");
				control = control && brtManager->DisconnectModuleID(_listener, it->bilateralAmbisonicEncoderProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->bilateralAmbisonicEncoderProcessor, "listenerPosition");
				control = control && brtManager->DisconnectModuleHRBRIR(this, it->bilateralAmbisonicEncoderProcessor, "listenerHRBRIR");

				control = control && brtManager->DisconnectModuleID(_source, it->distanceAttenuatorProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->distanceAttenuatorProcessor, "sourcePosition");
				control = control && brtManager->DisconnectModuleID(_listener, it->distanceAttenuatorProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->distanceAttenuatorProcessor, "listenerPosition");			

				if (_source->GetSourceType() == BRTSourceModel::Directivity) {
					control = control && brtManager->DisconnectModuleTransform(_listener, _source, "listenerPosition");
				}
				
				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it);
				return true;
			}
			return false;
		}

		/**
		 * @brief Enable or disable left and right ambisonic domain convolvers
		 * @param _enable true to enable, false to disable
		 */
		void EnableAmbisonicDomainConvolvers(bool _enable) {
			if (_enable) {
				leftAmbisonicDomainConvolverProcessor->EnableProcessor();
				rightAmbisonicDomainConvolverProcessor->EnableProcessor();
			} else {
				leftAmbisonicDomainConvolverProcessor->DisableProcessor();
				rightAmbisonicDomainConvolverProcessor->DisableProcessor();
			}	
		}

		/////////////////
		// Attributes
		/////////////////		
		mutable std::mutex mutex;													// To avoid access collisions
		std::string listenerID;														// Store unique listener ID
		std::shared_ptr<BRTServices::CHRBRIR>	listenerHRBRIR;						// RBRIR of listener				
		std::shared_ptr<BRTServices::CAmbisonicBIR> listenerAmbisonicIR;			// AmbisonicIR related to the listener				

		int ambisonicOrder;															// Store the Ambisonic order
		BRTProcessing::TAmbisonicNormalization ambisonicNormalization;				// Store the Ambisonic normalization
		bool enableNearFieldEffect;													// Enables/Disables the Near Field Effect
		bool enableParallaxCorrection;												// Enable parallax correction
		bool enableDistanceAttenuation;												// Enable distance attenuation
		float distanceAttenuationFactorDB;											// Distance attenuation factor in decibels
		float referenceAttenuationDistance;											// Reference distance for distance attenuation

		std::vector< CSourceToBeProcessed> sourcesConnectedProcessors;				// List of sources connected to this listener model

		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> leftAmbisonicDomainConvolverProcessor;
		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> rightAmbisonicDomainConvolverProcessor;

		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;
	};
}
#endif