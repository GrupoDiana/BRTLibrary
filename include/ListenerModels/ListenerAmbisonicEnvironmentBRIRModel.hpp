/**
* \class CListenerAmbisonicEnvironmentBRIRModel
*
* \brief Declaration of CListenerAmbisonicEnvironmentBRIRModel class
* \date	June 2024
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

#ifndef _CLISTENER_AMBISONIC_ENVIRONMENT_HRTF_MODEL_HPP_
#define _CLISTENER_AMBISONIC_ENVIRONMENT_HRTF_MODEL_HPP_

#include <memory>
#include <Base/ListenerModelBase.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/HRBRIR.hpp>
#include <ServiceModules/AmbisonicBIR.hpp>
#include <ProcessingModules/BilateralAmbisonicEncoderProcessor.hpp>
#include <ProcessingModules/AmbisonicDomainConvolverProcessor.hpp>
#include <Base/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>


namespace BRTListenerModel {

	class CListenerAmbisonicEnvironmentBRIRModel : public BRTBase::CListenerModelBase {

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
			*/
			void SetConfiguration(int _ambisonicOrder, Common::TAmbisonicNormalization _ambisonicNormalization) {

				bilateralAmbisonicEncoderProcessor->SetAmbisonicOrder(_ambisonicOrder);
				bilateralAmbisonicEncoderProcessor->SetAmbisonicNormalization(_ambisonicNormalization);
													
				bilateralAmbisonicEncoderProcessor->DisableITDSimulation();
				bilateralAmbisonicEncoderProcessor->DisableNearFieldEffect();
				bilateralAmbisonicEncoderProcessor->DisableParallaxCorrection(); 
				
			}

			/**
			 * @brief Set processor enable or disable
			 * @param _enableProcessor
			 */
			void SetEnableProcessor(bool _enableProcessor) {
				if (_enableProcessor) { bilateralAmbisonicEncoderProcessor->EnableProcessor(); }
				else { bilateralAmbisonicEncoderProcessor->DisableProcessor(); }
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
		CListenerAmbisonicEnvironmentBRIRModel(std::string _listenerID, BRTBase::CBRTManager* _brtManager) :
			brtManager{ _brtManager }, BRTBase::CListenerModelBase(_listenerID, BRTBase::TListenerModelcharacteristics(false, true, true, false, false, false, false, false)),
			ambisonicOrder{ 1 }, ambisonicNormalization{ Common::TAmbisonicNormalization::N3D }, enableNearFieldEffect{ false }, enableParallaxCorrection{ true }  {
			
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


		/** \brief SET HRBRIR of listener
		*	\param[in] pointer to HRBRIR to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		bool SetHRBRIR(std::shared_ptr< BRTServices::CHRBRIR > _listenerHRBRIR) {
			if (_listenerHRBRIR->GetSamplingRate() != globalParameters.GetSampleRate()) {
				SET_RESULT(RESULT_ERROR_NOTSET, "This HRBRIR has not been assigned to the listener. The sample rate of the HRBRIR does not match the one set in the library Global Parameters.");
				return false;
			}
			listenerHRBRIR = _listenerHRBRIR;
			InitListenerAmbisonicIR();
			GetHRBRIRExitPoint()->sendDataPtr(listenerHRBRIR);
			GetABIRExitPoint()->sendDataPtr(listenerAmbisonicIR);
			ResetProcessorBuffers();
		
			return true;
		}

		/** \brief Get HRBRIR of listener
		*	\retval HRBRIR pointer to current listener HRBRIR
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr < BRTServices::CHRBRIR> GetHRBRIR() const
		{
			return listenerHRBRIR;
		}

		/** \brief Remove the HRBRIR of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveHRBRIR() {
			listenerHRBRIR = nullptr;
		}

		
		/**
		 * @brief Set the ambisonic order to the listener
		 * @param _ambisonicOrder Ambisonic order to be set up. Currently only orders between 1 to 3 are allowed
		 * @return true if it is a valid order.
		*/
		bool SetAmbisonicOrder(int _ambisonicOrder) {
			
			if (_ambisonicOrder < 1 || _ambisonicOrder > 3) { return false; }
			if (ambisonicOrder == _ambisonicOrder) { return true; }			
			
			//std::lock_guard<std::mutex> l(mutex);
								
			ambisonicOrder = _ambisonicOrder;
			if (listenerHRBRIR->IsHRBRIRLoaded()) { InitListenerAmbisonicIR(); }
						
			SetConfigurationInALLSourcesProcessors();

			leftAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);
			rightAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);
			return true;
		}

		/**
		 * @brief Return current established ambisonic order
		 * @return order
		*/
		int GetAmbisonicOrder() {
			return ambisonicOrder;
		}

		/**
		 * @brief Set the ambisonin normalization to be used
		 * @param _ambisonicNormalization Normalization to be set up. 
		*/
		bool SetAmbisonicNormalization(Common::TAmbisonicNormalization _ambisonicNormalization) {
			
			if (ambisonicNormalization == _ambisonicNormalization) { return true; }
			
			//std::lock_guard<std::mutex> l(mutex);
			ambisonicNormalization = _ambisonicNormalization;
			if (listenerHRBRIR->IsHRBRIRLoaded()) {	InitListenerAmbisonicIR();	}
			SetConfigurationInALLSourcesProcessors();
			return true;
		}
		
		/**
		 * @brief Set the ambisonin normalization to be used.
		 * @param _ambisonicNormalization Normalization to be set up, valid strings are N3D, SN3D and maxN
		 * @return true if it is a valid normalization
		*/
		bool SetAmbisonicNormalization(std::string _ambisonicNormalization) {
			
			Common::TAmbisonicNormalization temp;
			if (_ambisonicNormalization == "N3D") {			temp = Common::TAmbisonicNormalization::N3D; }
			else if (_ambisonicNormalization == "SN3D") {	temp = Common::TAmbisonicNormalization::SN3D; }
			else if (_ambisonicNormalization == "maxN") {	temp = Common::TAmbisonicNormalization::maxN; }
			else { return false; }
			return SetAmbisonicNormalization(temp);					
		}

		/**
		 * @brief Return current established ambisonic normalization 
		 * @return current established ambisonic normalization
		*/
		Common::TAmbisonicNormalization GetAmbisonincNormalization() {
			return ambisonicNormalization;
		}
		

		/** \brief Enable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		/*void EnableNearFieldEffect() {
			enableNearFieldEffect = true;
			SetConfigurationInALLSourcesProcessors();			
		}*/

		/** \brief Disable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		/*void DisableNearFieldEffect() {
			enableNearFieldEffect = false;
			SetConfigurationInALLSourcesProcessors();			
		}*/

		/** \brief Get the flag for near field effect enabling
		*	\retval nearFieldEffectEnabled if true, near field effect is enabled for this listener
		*   \eh Nothing is reported to the error handler.
		*/
		/*bool IsNearFieldEffectEnabled() { return enableNearFieldEffect; }*/

		///** \brief Enable bilaterality for all source connected to this listener
		//*   \eh Nothing is reported to the error handler.
		//*/
		//void EnableITDSimulation() {
		//	enableITDSimulation = true;
		//	SetConfigurationInALLSourcesProcessors();
		//}

		///** \brief Disable bilaterality for all source connected to this listener
		//*   \eh Nothing is reported to the error handler.
		//*/
		//void DisableITDSimulation() {
		//	enableITDSimulation = false;
		//	SetConfigurationInALLSourcesProcessors();
		//}
		
		/**
		 * @brief Enable Parallax Correction
		*/
		//void EnableParallaxCorrection() { 
		//	enableParallaxCorrection = true; 
		//	SetConfigurationInALLSourcesProcessors();
		//}

		///**
		// * @brief Disable Parallax Correction
		//*/
		//void DisableParallaxCorrection() { 
		//	enableParallaxCorrection = false; 
		//	SetConfigurationInALLSourcesProcessors();
		//}

		///**
		//* @brief Get Parallax Correction state
		//*/
		//bool IsParallaxCorrectionEnabled() { return enableParallaxCorrection; }

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
				
		/**
		 * @brief Connecting to a specific listener transform
		 * @param _listenerID listener ID
		 * @return true if the connection success
		 */
		bool ConnectListenerTransform(const std::string _listenerID) {
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
		bool DisconnectListenerTransform(const std::string _listenerID) {
			std::shared_ptr<BRTBase::CListener> _listener = brtManager->GetListener(_listenerID);
			if (_listener != nullptr) {
				brtManager->DisconnectModuleTransform(_listener, leftAmbisonicDomainConvolverProcessor, "listenerPosition");
				brtManager->DisconnectModuleTransform(_listener, rightAmbisonicDomainConvolverProcessor, "listenerPosition");
				return true;
			}
			return false;
		}

		/**
		 * @brief Enable model
		 */
		void EnableModel() {
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
		void DisableModel() {
			std::lock_guard<std::mutex> l(mutex);
			enableModel = false;
			for (auto& it : sourcesConnectedProcessors) {
				it.SetEnableProcessor(false);
			}
			leftAmbisonicDomainConvolverProcessor->DisableProcessor();
			rightAmbisonicDomainConvolverProcessor->DisableProcessor();
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
		
		void InitListenerAmbisonicIR(){
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
			sourceProcessor.SetConfiguration(ambisonicOrder, ambisonicNormalization);
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
			CSourceToBeProcessed _newSourceProcessors(_source->GetID(), brtManager);
			_newSourceProcessors.bilateralAmbisonicEncoderProcessor->SetAmbisonicOrder(ambisonicOrder);
			_newSourceProcessors.bilateralAmbisonicEncoderProcessor->SetAmbisonicNormalization(ambisonicNormalization);

			bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourceID");			
			
			if (sourceNeedsListenerPosition) {
				control = control && brtManager->ConnectModuleTransform(this, _source, "listenerPosition");
			}

			control = control && brtManager->ConnectModuleTransform(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleHRBRIR(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerHRBRIR");
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
		template <typename T>
		bool DisconnectAnySoundSource(std::shared_ptr<T> _source, bool sourceNeedsListenerPosition) {
			std::lock_guard<std::mutex> l(mutex);
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceToBeProcessed& sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {

				bool control = brtManager->DisconnectModulesSamples(leftAmbisonicDomainConvolverProcessor, "outSamples", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(rightAmbisonicDomainConvolverProcessor, "outSamples", this, "rightEar");

				control = control && brtManager->DisconnectModulesMultipleSamplesVectors(it->bilateralAmbisonicEncoderProcessor, "leftAmbisonicChannels", leftAmbisonicDomainConvolverProcessor, "inputChannels");
				control = control && brtManager->DisconnectModulesMultipleSamplesVectors(it->bilateralAmbisonicEncoderProcessor, "rightAmbisonicChannels", rightAmbisonicDomainConvolverProcessor, "inputChannels");

				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->bilateralAmbisonicEncoderProcessor, "inputSamples");
				control = control && brtManager->DisconnectModuleID(this, it->bilateralAmbisonicEncoderProcessor, "listenerID");				
				control = control && brtManager->DisconnectModuleHRBRIR(this, it->bilateralAmbisonicEncoderProcessor, "listenerHRBRIR");

				control = control && brtManager->DisconnectModuleTransform(this, it->bilateralAmbisonicEncoderProcessor, "listenerPosition");

				if (sourceNeedsListenerPosition) {
					control = control && brtManager->DisconnectModuleTransform(this, _source, "listenerPosition");
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
		std::shared_ptr<BRTServices::CHRBRIR>	listenerHRBRIR;						// RBRIR of listener				
		std::shared_ptr<BRTServices::CAmbisonicBIR> listenerAmbisonicIR;			// AmbisonicIR related to the listener				

		int ambisonicOrder;															// Store the Ambisonic order
		Common::TAmbisonicNormalization ambisonicNormalization;						// Store the Ambisonic normalization
		bool enableNearFieldEffect;													// Enables/Disables the Near Field Effect
		bool enableParallaxCorrection;												// Enable parallax correction

		std::vector< CSourceToBeProcessed> sourcesConnectedProcessors;				// List of sources connected to this listener model

		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> leftAmbisonicDomainConvolverProcessor;
		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> rightAmbisonicDomainConvolverProcessor;

		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;
	};
}
#endif