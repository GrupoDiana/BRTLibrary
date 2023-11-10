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

#ifndef _CLISTENER_VIRTUALAMBISONICBASED_MODEL_HPP_
#define _CLISTENER_VIRTUALAMBISONICBASED_MODEL_HPP_

#include <memory>
#include <Base/ListenerModelBase.hpp>
#include <ServiceModules/HRTFDefinitions.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/ILD.hpp>
#include <ServiceModules/AmbisonicBIR.hpp>
#include <ProcessingModules/BilateralAmbisonicEncoderProcessor.hpp>
#include <ProcessingModules/AmbisonicDomainConvolverProcessor.hpp>
#include <Base/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>
#include <third_party_libraries/nlohmann/json.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTListenerModel {

	class CListenerVirtualAmbisonicBasedModel : public BRTBase::CListenerModelBase {

		class CSourceToBeProcessed {
		public:

			CSourceToBeProcessed(std::string _sourceID, BRTBase::CBRTManager* brtManager) : sourceID{ _sourceID } {
				bilateralAmbisonicEncoderProcessor = brtManager->CreateProcessor <BRTProcessing::CBilateralAmbisonicEncoderProcessor>();
				//nearFieldEffectProcessor = brtManager->CreateProcessor<BRTProcessing::CNearFieldEffectProcessor>();
			}

			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(bilateralAmbisonicEncoderProcessor);
				//brtManager->RemoveProcessor(binauralConvolverProcessor);
			}
			std::string sourceID;
			std::shared_ptr <BRTProcessing::CBilateralAmbisonicEncoderProcessor> bilateralAmbisonicEncoderProcessor;
			//std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> ambiconicDomainConvolverProcessor;
		};

	public:
		CListenerVirtualAmbisonicBasedModel(std::string _listenerID, BRTBase::CBRTManager* _brtManager) : brtManager{ _brtManager }, BRTBase::CListenerModelBase(_listenerID),
			ambisonicOrder{ 1 }, ambisonicNormalization{ Common::TAmbisonicNormalization::N3D } {
			
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();					// Create a empty HRTF	class
			listenerAmbisonicIR = std::make_shared<BRTServices::CAmbisonicBIR>();	// Create a empty AmbisonicIR class
			
			CreateHRTFExitPoint();
			CreateILDExitPoint();
			CreateABIRExitPoint();

			leftAmbisonicDomainConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CAmbisonicDomainConvolverProcessor>();
			leftAmbisonicDomainConvolverProcessor->SetEar(Common::T_ear::LEFT);			
			brtManager->ConnectModuleABIR(this, leftAmbisonicDomainConvolverProcessor, "listenerAmbisonicBIR");
			rightAmbisonicDomainConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CAmbisonicDomainConvolverProcessor>();
			rightAmbisonicDomainConvolverProcessor->SetEar(Common::T_ear::RIGHT);
			brtManager->ConnectModuleABIR(this, rightAmbisonicDomainConvolverProcessor, "listenerAmbisonicBIR");
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
			
			listenerAmbisonicIR->Setup(globalParameters.GetBufferSize(), listenerHRTF->GetHRIRLength(), listenerHRTF->GetHRIRNumberOfSubfilters(), listenerHRTF->GetHRIRSubfilterLength(), ambisonicOrder, ambisonicNormalization);
			listenerAmbisonicIR->AddImpulseResponsesFromHRTF(listenerHRTF);

			GetHRTFExitPoint()->sendDataPtr(listenerHRTF);
			GetABIRExitPoint()->sendDataPtr(listenerAmbisonicIR);
			ResetConvolutionsBuffers();
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
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Empty HRTF			
			listenerAmbisonicIR = std::make_shared<BRTServices::CAmbisonicBIR>();	// Empty AmbisonicIR class
		}

		///
		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		void SetILD(std::shared_ptr< BRTServices::CILD > _listenerILD) {
			listenerILD = _listenerILD;
			GetILDExitPoint()->sendDataPtr(listenerILD);
		}

		/** \brief Get HRTF of listener
		*	\retval HRTF pointer to current listener HRTF
		*   \eh On error, an error code is reported to the error handler.
		*/
		std::shared_ptr <BRTServices::CILD> GetILD() const
		{
			return listenerILD;
		}

		/** \brief Remove the HRTF of thelistener
		*   \eh Nothing is reported to the error handler.
		*/
		void RemoveILD() {
			listenerILD = std::make_shared<BRTServices::CILD>();	// empty HRTF			
		}


		void SetAmbisonicOrder(int _ambisonicOrder) {
			ambisonicOrder = _ambisonicOrder;
			if (listenerHRTF->IsHRTFLoaded()) {
				listenerAmbisonicIR->Reset();
				listenerAmbisonicIR->Setup(globalParameters.GetBufferSize(), listenerHRTF->GetHRIRLength(), listenerHRTF->GetHRIRNumberOfSubfilters(), listenerHRTF->GetHRIRSubfilterLength(), ambisonicOrder, ambisonicNormalization);
				listenerAmbisonicIR->AddImpulseResponsesFromHRTF(listenerHRTF);
			}
			for (int nSource = 0; nSource < sourcesConnectedProcessors.size(); nSource++) {
				sourcesConnectedProcessors[nSource].bilateralAmbisonicEncoderProcessor->SetAmbisonicOrder(_ambisonicOrder);
			}
			leftAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);
			rightAmbisonicDomainConvolverProcessor->SetAmbisonicOrder(_ambisonicOrder);
		}

		int GetAmbisonicOrder() {
			return ambisonicOrder;
		}


		void SetAmbisonicNormalization(Common::TAmbisonicNormalization _ambisonicNormalization) {
			ambisonicNormalization = _ambisonicNormalization;
			if (listenerHRTF->IsHRTFLoaded()) {
				listenerAmbisonicIR->Reset();
				listenerAmbisonicIR->Setup(globalParameters.GetBufferSize(), listenerHRTF->GetHRIRLength(), listenerHRTF->GetHRIRNumberOfSubfilters(), listenerHRTF->GetHRIRSubfilterLength(), ambisonicOrder, ambisonicNormalization);
				listenerAmbisonicIR->AddImpulseResponsesFromHRTF(listenerHRTF);
			}			
			for (int nSource = 0; nSource < sourcesConnectedProcessors.size(); nSource++) {
				sourcesConnectedProcessors[nSource].bilateralAmbisonicEncoderProcessor->SetAmbisonicNormalization(_ambisonicNormalization);
			}
		}
		
		Common::TAmbisonicNormalization GetAmbisonincNormalization() {
			return ambisonicNormalization;
		}
		
		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		template <typename T>
		bool ConnectSoundSource(std::shared_ptr<T> _source) {
			CSourceToBeProcessed _newSourceProcessors(_source->GetID(), brtManager);
			_newSourceProcessors.bilateralAmbisonicEncoderProcessor->SetAmbisonicOrder(ambisonicOrder);
			_newSourceProcessors.bilateralAmbisonicEncoderProcessor->SetAmbisonicNormalization(ambisonicNormalization);

			bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourcePosition");			
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "sourceID");

			control = control && brtManager->ConnectModuleTransform(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerPosition");			
			control = control && brtManager->ConnectModuleHRTF(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerHRTF");			
			control = control && brtManager->ConnectModuleILD(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerILD");
			control = control && brtManager->ConnectModuleID(this, _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "listenerID");
			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.bilateralAmbisonicEncoderProcessor, "inputSamples");
			
			control = control && brtManager->ConnectModulesMultipleSamplesVectors(_newSourceProcessors.bilateralAmbisonicEncoderProcessor, "leftAmbisonicChannels", leftAmbisonicDomainConvolverProcessor, "inputChannels");
			control = control && brtManager->ConnectModulesMultipleSamplesVectors(_newSourceProcessors.bilateralAmbisonicEncoderProcessor, "rightAmbisonicChannels", rightAmbisonicDomainConvolverProcessor, "inputChannels");
			
			
			control = control && brtManager->ConnectModulesSamples(leftAmbisonicDomainConvolverProcessor, "outSamples", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(rightAmbisonicDomainConvolverProcessor, "outSamples", this, "rightEar");

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
		bool DisconnectSoundSource(std::shared_ptr<T> _source) {
			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceToBeProcessed& sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {
				/*bool control = brtManager->DisconnectModulesSamples(it->nearFieldEffectProcessor, "leftEar", this, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(it->nearFieldEffectProcessor, "rightEar", this, "rightEar");
				control = control && brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "leftEar", it->nearFieldEffectProcessor, "leftEar");
				control = control && brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "rightEar", it->nearFieldEffectProcessor, "rightEar");
				control = control && brtManager->DisconnectModulesSamples(_source, "samples", it->binauralConvolverProcessor, "inputSamples");

				control = control && brtManager->DisconnectModuleID(this, it->binauralConvolverProcessor, "listenerID");
				control = control && brtManager->DisconnectModuleILD(this, it->nearFieldEffectProcessor, "listenerILD");
				control = control && brtManager->DisconnectModuleHRTF(this, it->binauralConvolverProcessor, "listenerHRTF");
				control = control && brtManager->DisconnectModuleTransform(this, it->nearFieldEffectProcessor, "listenerPosition");
				control = control && brtManager->DisconnectModuleTransform(this, it->binauralConvolverProcessor, "listenerPosition");

				control = control && brtManager->DisconnectModuleID(_source, it->binauralConvolverProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->nearFieldEffectProcessor, "sourcePosition");
				control = control && brtManager->DisconnectModuleTransform(_source, it->binauralConvolverProcessor, "sourcePosition");*/

				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it);
				return true;
			}
			return false;
		}

		/** \brief Enable binaural spatialization based in HRTF convolution
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableSpatialization() {
			nlohmann::json j;
			j["command"] = "/listener/enableSpatialization";
			j["listenerID"] = listenerID;
			j["enable"] = true;
			brtManager->ExecuteCommand(j.dump());
		}

		/** \brief Disable binaural spatialization based in HRTF convolution
		*/
		void DisableSpatialization()
		{
			nlohmann::json j;
			j["command"] = "/listener/enableSpatialization";
			j["listenerID"] = listenerID;
			j["enable"] = false;
			brtManager->ExecuteCommand(j.dump());
		}

		///** \brief Get the flag for run-time HRTF interpolation
		//*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		//*   \eh Nothing is reported to the error handler.
		//*/
		//bool IsSpatializationEnabled();

		/** \brief Enable run-time HRTF interpolation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableInterpolation() {
			nlohmann::json j;
			j["command"] = "/listener/enableInterpolation";
			j["listenerID"] = listenerID;
			j["enable"] = true;
			brtManager->ExecuteCommand(j.dump());
		}

		/** \brief Disable run-time HRTF interpolation
		*/
		void DisableInterpolation() {
			nlohmann::json j;
			j["command"] = "/listener/enableInterpolation";
			j["listenerID"] = listenerID;
			j["enable"] = false;
			brtManager->ExecuteCommand(j.dump());
		}

		/** \brief Get the flag for run-time HRTF interpolation
		*	\retval IsInterpolationEnabled if true, run-time HRTF interpolation is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		//bool IsInterpolationEnabled();

		/** \brief Enable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableNearFieldEffect() {
			nlohmann::json j;
			j["command"] = "/listener/enableNearFiedlEffect";
			j["listenerID"] = listenerID;
			j["enable"] = true;
			brtManager->ExecuteCommand(j.dump());
		}

		/** \brief Disable near field effect for this source
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableNearFieldEffect() {
			nlohmann::json j;
			j["command"] = "/listener/enableNearFiedlEffect";
			j["listenerID"] = listenerID;
			j["enable"] = false;
			brtManager->ExecuteCommand(j.dump());
		}

		/** \brief Get the flag for near field effect enabling
		*	\retval nearFieldEffectEnabled if true, near field effect is enabled for this source
		*   \eh Nothing is reported to the error handler.
		*/
		//bool IsNearFieldEffectEnabled();

		void ResetConvolutionsBuffers() {
			nlohmann::json j;
			j["command"] = "/listener/resetBuffers";
			j["listenerID"] = listenerID;
			brtManager->ExecuteCommand(j.dump());
		}

		void Update(std::string entryPointID) {
			// Nothing to do
		}
		void UpdateCommand() {
			// Nothing to do
		}


	private:

		/////////////////
		// Attributes
		/////////////////		
		std::string listenerID;														// Store unique listener ID
		std::shared_ptr<BRTServices::CHRTF> listenerHRTF;							// HRTF of listener														
		std::shared_ptr<BRTServices::CILD> listenerILD;								// ILD of listener				
		std::shared_ptr<BRTServices::CAmbisonicBIR> listenerAmbisonicIR;			// AmbisonicIR related to the listener				
		int ambisonicOrder;															// Store the Ambisonic order
		Common::TAmbisonicNormalization ambisonicNormalization;						// Store the Ambisonic normalization
		
		std::vector< CSourceToBeProcessed> sourcesConnectedProcessors;				// List of sources connected to this listener model
		
		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> leftAmbisonicDomainConvolverProcessor;
		std::shared_ptr <BRTProcessing::CAmbisonicDomainConvolverProcessor> rightAmbisonicDomainConvolverProcessor;
		
		BRTBase::CBRTManager* brtManager;
		Common::CGlobalParameters globalParameters;


		/////////////////
		// Methods
		/////////////////
					
	};
}
#endif