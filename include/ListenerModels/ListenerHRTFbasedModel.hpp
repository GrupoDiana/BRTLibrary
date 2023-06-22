/**
* \class CBuffer
*
* \brief Declaration of CBuffer class
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
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _CLISTENER_HRTFBASED_MODEL_HPP_
#define _CLISTENER_HRTFBASED_MODEL_HPP_

#include <memory>
#include "Base/ListenerModelBase.hpp"
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/ILD.hpp>
#include <ProcessingModules/HRTFConvolverProcessor.hpp>
#include <ProcessingModules/NearFieldEffectProcessor.hpp>
#include <Base/SourceModelBase.hpp>
#include <Base/BRTManager.hpp>

namespace BRTServices {
	class CHRTF;
}

namespace BRTListenerModel {

	class CListenerHRTFbasedModel: public BRTBase::CListenerModelBase {
		
		class CSourceProcessors {
		public:
			
			CSourceProcessors(std::string _sourceID, BRTBase::CBRTManager* brtManager) : sourceID{ _sourceID} {
				binauralConvolverProcessor = brtManager->CreateProcessor <BRTProcessing::CHRTFConvolverProcessor>();
				nearFieldEffectProcessor = brtManager->CreateProcessor<BRTProcessing::CNearFieldEffectProcessor>();
			}	
			
			void Clear(BRTBase::CBRTManager* brtManager) {
				sourceID = "";
				brtManager->RemoveProcessor(nearFieldEffectProcessor);
				brtManager->RemoveProcessor(binauralConvolverProcessor);
			}
			std::string sourceID;
			std::shared_ptr <BRTProcessing::CHRTFConvolverProcessor> binauralConvolverProcessor;
			std::shared_ptr <BRTProcessing::CNearFieldEffectProcessor> nearFieldEffectProcessor;
		};

	public:
		CListenerHRTFbasedModel(std::string _listenerID, BRTBase::CBRTManager* _brtManager) : brtManager{_brtManager}, BRTBase::CListenerModelBase(_listenerID) {						
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Create a empty HRTF									
			CreateHRTFExitPoint();
			CreateILDExitPoint();									
		}

		
		/** \brief SET HRTF of listener
		*	\param[in] pointer to HRTF to be stored
		*   \eh On error, NO error code is reported to the error handler.
		*/
		void SetHRTF(std::shared_ptr< BRTServices::CHRTF > _listenerHRTF) {			
			listenerHRTF = _listenerHRTF;			
			GetHRTFExitPoint()->sendDataPtr(listenerHRTF);			
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
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// empty HRTF			
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

		/**
		 * @brief Connect a new source to this listener
		 * @tparam T It must be a source model, i.e. a class that inherits from the CSourceModelBase class.
		 * @param _source Pointer to the source
		 * @return True if the connection success
		*/
		template <typename T>
		bool ConnectSoundSource(std::shared_ptr<T> _source) {						
			CSourceProcessors _newSourceProcessors(_source->GetID(), brtManager);
			
			bool control = brtManager->ConnectModuleTransform(_source, _newSourceProcessors.binauralConvolverProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleTransform(_source, _newSourceProcessors.nearFieldEffectProcessor, "sourcePosition");
			control = control && brtManager->ConnectModuleID(_source, _newSourceProcessors.binauralConvolverProcessor, "sourceID");
							  
			control = control && brtManager->ConnectModuleTransform(this, _newSourceProcessors.binauralConvolverProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleTransform(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerPosition");
			control = control && brtManager->ConnectModuleHRTF(this, _newSourceProcessors.binauralConvolverProcessor, "listenerHRTF");
			control = control && brtManager->ConnectModuleILD(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerILD");
			control = control && brtManager->ConnectModuleID(this, _newSourceProcessors.binauralConvolverProcessor, "listenerID");
							  
			control = control && brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.binauralConvolverProcessor, "inputSamples");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "leftEar", _newSourceProcessors.nearFieldEffectProcessor, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "rightEar", _newSourceProcessors.nearFieldEffectProcessor, "rightEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "leftEar", this, "leftEar");
			control = control && brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "rightEar", this, "rightEar");

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
				control = control && brtManager->DisconnectModuleTransform(this, it->nearFieldEffectProcessor, "listenerPosition");
				control = control && brtManager->DisconnectModuleTransform(this, it->binauralConvolverProcessor, "listenerPosition");
				
				control = control && brtManager->DisconnectModuleID(_source, it->binauralConvolverProcessor, "sourceID");
				control = control && brtManager->DisconnectModuleTransform(_source, it->nearFieldEffectProcessor, "sourcePosition");
				control = control && brtManager->DisconnectModuleTransform(_source, it->binauralConvolverProcessor, "sourcePosition");

				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it); 	
				return true;
			}			
			return false;
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
		std::string listenerID;								// Store unique listener ID
		std::shared_ptr<BRTServices::CHRTF> listenerHRTF;	// HRTF of listener														
		std::shared_ptr<BRTServices::CILD> listenerILD;		// ILD of listener						
		std::vector< CSourceProcessors> sourcesConnectedProcessors;
		BRTBase::CBRTManager* brtManager;				
	};
}
#endif