/**
* \class CListenerVirtualAmbisonicBasedModel
*
* \brief Declaration of CListenerVirtualAmbisonicBasedModel class
* \date	October 2023
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

#ifndef _CLISTENER_AMBISONIC_HRTF_MODEL_HPP_
#define _CLISTENER_AMBISONIC_HRTF_MODEL_HPP_

#include <memory>
#include <Common/ErrorHandler.hpp>
#include <ListenerModels/ListenerModelBase.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/AmbisonicBIR.hpp>
#include <ProcessingModules/BilateralAmbisonicEncoderProcessor.hpp>
#include <ProcessingModules/AmbisonicDomainConvolverProcessor.hpp>
#include <SourceModels/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTListenerModel {

	class CListenerAmbisonicHRTFModel : public CListenerModelBase {

		class CSourceToBeProcessed {
		public:

			CSourceToBeProcessed(std::string _sourceID, BRTBase::CBRTManager* brtManager) : sourceID{ _sourceID } {
				bilateralAmbisonicEncoderProcessor = brtManager->CreateProcessor <BRTProcessing::CBilateralAmbisonicEncoderProcessor>();				
			}
			
			/**
			 * @brief Remove processor from BRT
			 * @param brtManager brtManager pointer
			*/
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(bilateralAmbisonicEncoderProcessor);				
			}

			/**
			 * @brief Set proccesor configuration
			 * @param _ambisonicOrder 
			 * @param _ambisonicNormalization 
			 * @param enableBilateral 
			 * @param enableNearFieldEffect 
			*/
			void SetConfiguration(int _ambisonicOrder, BRTProcessing::TAmbisonicNormalization _ambisonicNormalization, bool enableNearFieldEffect, bool enableITDSimulation, bool enableParallaxCorrection) {

				bilateralAmbisonicEncoderProcessor->SetAmbisonicOrder(_ambisonicOrder);
				bilateralAmbisonicEncoderProcessor->SetAmbisonicNormalization(_ambisonicNormalization);
								
				if (enableITDSimulation) { bilateralAmbisonicEncoderProcessor->EnableITDSimulation(); }
				else { bilateralAmbisonicEncoderProcessor->DisableITDSimulation(); }

				if (enableNearFieldEffect) { bilateralAmbisonicEncoderProcessor->EnableNearFieldEffect(); }
				else { bilateralAmbisonicEncoderProcessor->DisableNearFieldEffect(); }		

				if (enableParallaxCorrection) { bilateralAmbisonicEncoderProcessor->EnableParallaxCorrection(); }
				else { bilateralAmbisonicEncoderProcessor->DisableParallaxCorrection(); }
			}

			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor 
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) {	bilateralAmbisonicEncoderProcessor->EnableProcessor(); }
				else {	bilateralAmbisonicEncoderProcessor->DisableProcessor();	}
			}

			/**		
			 * @brief Reset processor buffers
			*/
			void ResetBuffers() {
				bilateralAmbisonicEncoderProcessor->ResetBuffers();
			}

			std::string sourceID;
			std::shared_ptr <BRTProcessing::CBilateralAmbisonicEncoderProcessor> bilateralAmbisonicEncoderProcessor;			
		};

	public:
		CListenerAmbisonicHRTFModel(std::string _listenerID, BRTBase::CBRTManager* _brtManager) 
			: brtManager{ _brtManager }
			, CListenerModelBase(_listenerID, TListenerModelcharacteristics(true, false, true, true, true, true, false, false))
			, ambisonicOrder{ 1 }
			, ambisonicNormalization { BRTProcessing::TAmbisonicNormalization::N3D }
			, enableNearFieldEffect{ false }
			, enableParallaxCorrection{ true }
			, enableITDSimulation{ true } {
			
			// TODO Change to nullptr
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();					// Create a empty HRTF	class 
			listenerAmbisonicIR = std::make_shared<BRTServices::CAmbisonicBIR>();	// Create a empty AmbisonicIR class
			
			CreateHRTFExitPoint();
			CreateILDExitPoint();
			CreateABIRExitPoint();

			leftAmbisonicDomainConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CAmbisonicDomainConvolverProcessor>(Common::T_ear::LEFT);			
			brtManager->ConnectModuleABIR(this, leftAmbisonicDomainConvolverProcessor, "listenerAmbisonicBIR");
			rightAmbisonicDomainConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CAmbisonicDomainConvolverProcessor>(Common::T_ear::RIGHT);			
			brtManager->ConnectModuleABIR(this, rightAmbisonicDomainConvolverProcessor, "listenerAmbisonicBIR");			
		}


		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRTF(std::shared_ptr<BRTServices::CHRTF> _listenerHRTF) override {

			if (_listenerHRTF->GetSamplingRate() != globalParameters.GetSampleRate()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This HRTF has not been assigned to the listener. The sample rate of the HRTF does not match the one set in the library Global Parameters.");
				return false;
			}			
			listenerHRTF = _listenerHRTF;									
			InitListenerAmbisonicIR();

			GetHRTFExitPoint()->sendDataPtr(listenerHRTF);
			GetABIRExitPoint()->sendDataPtr(listenerAmbisonicIR);
						
			ResetProcessorBuffers();
			return true;
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr<BRTServices::CHRTF> GetHRTF() const override
		{
			return listenerHRTF;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRTF() override {
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Empty HRTF			
			listenerAmbisonicIR = std::make_shared<BRTServices::CAmbisonicBIR>();	// Empty AmbisonicIR class
		}

		///
		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetNearFieldCompensationFilters(std::shared_ptr< BRTServices::CSOSFilters > _listenerILD) override {
			listenerNFCFilters = _listenerILD;
			GetILDExitPoint()->sendDataPtr(listenerNFCFilters);
			return true;
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr <BRTServices::CSOSFilters> GetNearFieldCompensationFilters() const override
		{
			return listenerNFCFilters;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveNearFierldCompensationFilters() override {
			listenerNFCFilters = std::make_shared<BRTServices::CSOSFilters>();	// empty HRTF			
		}

		/**
		 * @brief Set the ambisonic order to the listener
		 * @param _ambisonicOrder Ambisonic order to be set up. Currently only orders between 1 to 3 are allowed
		 * @return true if it is a valid order.
		*/
		bool SetAmbisonicOrder(int _ambisonicOrder) override {
			
			if (_ambisonicOrder < 1 || _ambisonicOrder > 3) { return false; }
			if (ambisonicOrder == _ambisonicOrder) { return true; }			
			
			//std::lock_guard<std::mutex> l(mutex);
								
			ambisonicOrder = _ambisonicOrder;
			if (listenerHRTF->IsHRTFLoaded()) {	InitListenerAmbisonicIR();		}
						
			SetConfigurationInALLSourcesProcessors();

			leftAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);
			rightAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);
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
			
			//std::lock_guard<std::mutex> l(mutex);
			ambisonicNormalization = _ambisonicNormalization;
			if (listenerHRTF->IsHRTFLoaded()) {	InitListenerAmbisonicIR();	}			
			SetConfigurationInALLSourcesProcessors();
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
		*	\retval nearFieldEffectEnabled if true, near field effect is enabled for this listener
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsNearFieldEffectEnabled() override { return enableNearFieldEffect; }

		/** \brief Enable bilaterality for all source connected to this listener
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableITDSimulation() override {
			enableITDSimulation = true;
			SetConfigurationInALLSourcesProcessors();
		}

		/** \brief Disable bilaterality for all source connected to this listener
		*   \eh Nothing is reported to the error handler.
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
			leftAmbisonicDomainConvolverProcessor->EnableProcessor();
			rightAmbisonicDomainConvolverProcessor->EnableProcessor();
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
			leftAmbisonicDomainConvolverProcessor->DisableProcessor();
			rightAmbisonicDomainConvolverProcessor->DisableProcessor();
		};


		/**
		 * @brief Connect a new source to this listener
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
			
			leftAmbisonicDomainConvolverProcessor->ResetChannelsConvolutionBuffers();
			rightAmbisonicDomainConvolverProcessor->ResetChannelsConvolutionBuffers();

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
			if (command.isNull() || command.GetAddress() == "") {
				return;
			}

			if (listenerID == command.GetStringParameter("listenerID")) {
				if (command.GetCommand() == "/listener/setAmbisonicsOrder") {
					SetAmbisonicOrder(command.GetIntParameter("ambisonicsOrder"));
				} else if (command.GetCommand() == "/listener/setAmbisonicsNormalization") {
					SetAmbisonicNormalization(command.GetStringParameter("ambisonicsNormalization"));
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

		void InitListenerAmbisonicIR(){
			std::lock_guard<std::mutex> l(mutex);
			listenerAmbisonicIR->BeginSetup(ambisonicOrder, ambisonicNormalization);
			bool control = listenerAmbisonicIR->AddImpulseResponsesFromHRIR(listenerHRTF);
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
			sourceProcessor.SetConfiguration(ambisonicOrder, ambisonicNormalization, enableNearFieldEffect, enableITDSimulation, enableParallaxCorrection);
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

			// Make connection
			CSourceToBeProcessed _newSourceProcessors(_source->GetID(), brtManager);
			_newSourceProcessors.bilateralAmbisonicEncoderProcessor->SetAmbisonicOrder(ambisonicOrder);
			_newSourceProcessors.bilateralAmbisonicEncoderProcessor->SetAmbisonicNormalization(ambisonicNormalization);

			bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourceID");			
			
			if (_source->GetSourceType()==BRTSourceModel::TSourceType::Directivity) {
				control = control && brtManager->ConnectModuleTransform(_listener, _source, "listenerPosition");
			}

			control = control && brtManager->ConnectModuleTransform(_listener, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleHRTF(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerHRTF");
			control = control && brtManager->ConnectModuleILD(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerILD");
			control = control && brtManager->ConnectModuleID(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerID");
			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "inputSamples");

			control = control && brtManager->ConnectModulesMultipleSamplesVectors(_newSourceProcessors.bilateralAmbisonicEncoderProcessor, "leftAmbisonicChannels", leftAmbisonicDomainConvolverProcessor, "inputChannels");
			control = control && brtManager->ConnectModulesMultipleSamplesVectors(_newSourceProcessors.bilateralAmbisonicEncoderProcessor, "rightAmbisonicChannels", rightAmbisonicDomainConvolverProcessor, "inputChannels");

			control = control && brtManager->ConnectModulesSamples(leftAmbisonicDomainConvolverProcessor, "outSamples", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(rightAmbisonicDomainConvolverProcessor, "outSamples", this, "rightEar");

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
			// Make disconnection
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceToBeProcessed& sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {

				bool control = brtManager->DisconnectModulesSamples(leftAmbisonicDomainConvolverProcessor, "outSamples", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(rightAmbisonicDomainConvolverProcessor, "outSamples", this, "rightEar");

				control = control && brtManager->DisconnectModulesMultipleSamplesVectors(it->bilateralAmbisonicEncoderProcessor, "leftAmbisonicChannels", leftAmbisonicDomainConvolverProcessor, "inputChannels");
				control = control && brtManager->DisconnectModulesMultipleSamplesVectors(it->bilateralAmbisonicEncoderProcessor, "rightAmbisonicChannels", rightAmbisonicDomainConvolverProcessor, "inputChannels");

				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->bilateralAmbisonicEncoderProcessor, "inputSamples");
				control = control && brtManager->DisconnectModuleID(this, it->bilateralAmbisonicEncoderProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleILD(this, it->bilateralAmbisonicEncoderProcessor, "listenerILD");
				control = control && brtManager->DisconnectModuleHRTF(this, it->bilateralAmbisonicEncoderProcessor, "listenerHRTF");
				control = control && brtManager->DisconnectModuleTransform(_listener, it->bilateralAmbisonicEncoderProcessor, "listenerPosition");

				if (_source->GetSourceType() == BRTSourceModel::Directivity) {
					control = control && brtManager->DisconnectModuleTransform(_listener, _source, "listenerPosition");
				}

				control = control && brtManager->DisconnectModuleID(_source, it->bilateralAmbisonicEncoderProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->bilateralAmbisonicEncoderProcessor, "sourcePosition");

				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it);
				return true;
			}
			return false;
		}

		/////////////////
		// Attributes
		/////////////////		
		mutable std::mutex mutex;													// To avoid access collisions
		std::string listenerID;														// Store unique listener ID
		std::shared_ptr<BRTServices::CHRTF> listenerHRTF;							// HRTF of listener														
		std::shared_ptr<BRTServices::CSOSFilters> listenerNFCFilters;								// ILD of listener				
		std::shared_ptr<BRTServices::CAmbisonicBIR> listenerAmbisonicIR;			// AmbisonicIR related to the listener				

		int ambisonicOrder;															// Store the Ambisonic order
		BRTProcessing::TAmbisonicNormalization ambisonicNormalization;				// Store the Ambisonic normalization
		bool enableNearFieldEffect;													// Enables/Disables the Near Field Effect
		bool enableParallaxCorrection;												// Enable parallax correction
		bool enableITDSimulation;													// Enable ITD simulation 

		std::vector< CSourceToBeProcessed> sourcesConnectedProcessors;				// List of sources connected to this listener model

		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> leftAmbisonicDomainConvolverProcessor;
		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> rightAmbisonicDomainConvolverProcessor;

		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;

		std::vector<std::shared_ptr<BRTEnvironmentModel::CEnviromentModelBase>> environmentModelsConnected; // Listener models connected to the listener
	};
}
#endif
