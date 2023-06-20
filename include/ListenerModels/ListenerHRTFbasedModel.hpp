#ifndef _CLISTENER_HRTFBASED_MODEL_HPP_
#define _CLISTENER_HRTFBASED_MODEL_HPP_

#include <memory>
//#include "EntryPoint.hpp"
//#include "ExitPoint.hpp"
#include "Base/ListenerModelBase.hpp"
#include <Common/CommonDefinitions.h>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/ILD.hpp>
#include <ProcessingModules/HRTFConvolverProcessor.hpp>
#include <ProcessingModules/NearFieldEffectProcessor.hpp>
#include <Base/SourceModelBase.hpp>
#include <SourceModels/SourceDirectivityModel.hpp>
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
		CListenerHRTFbasedModel(std::string _listenerID) : BRTBase::CListenerModelBase(_listenerID){
			
			
			listenerHRTF = std::make_shared<BRTServices::CHRTF>();	// Create a empty HRTF						
			// Create exit point			
			//hrtfExitPoint				= std::make_shared<CExitPointHRTFPtr>("listenerHRTF");
			//ildExitPoint				= std::make_shared<CExitPointILDPtr>("listenerILD");
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
			//hrtfExitPoint->sendDataPtr(listenerHRTF);
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

		template <typename T>
		void ConnectSoundSource(std::shared_ptr<T> _source, BRTBase::CBRTManager* brtManager) {

			
			CSourceProcessors _newSourceProcessors(_source->GetID(), brtManager);

			brtManager->ConnectModuleTransform(_source, _newSourceProcessors.binauralConvolverProcessor, "sourcePosition");
			brtManager->ConnectModuleTransform(_source, _newSourceProcessors.nearFieldEffectProcessor, "sourcePosition");
			brtManager->ConnectModuleID(_source, _newSourceProcessors.binauralConvolverProcessor, "sourceID");

			brtManager->ConnectModuleTransform(this, _newSourceProcessors.binauralConvolverProcessor, "listenerPosition");
			brtManager->ConnectModuleTransform(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerPosition");
			brtManager->ConnectModuleHRTF(this, _newSourceProcessors.binauralConvolverProcessor, "listenerHRTF");
			brtManager->ConnectModuleILD(this, _newSourceProcessors.nearFieldEffectProcessor, "listenerILD");
			brtManager->ConnectModuleID(this, _newSourceProcessors.binauralConvolverProcessor, "listenerID");

			brtManager->ConnectModulesSamples(_source, "samples", _newSourceProcessors.binauralConvolverProcessor, "inputSamples");
			brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "leftEar", _newSourceProcessors.nearFieldEffectProcessor, "leftEar");
			brtManager->ConnectModulesSamples(_newSourceProcessors.binauralConvolverProcessor, "rightEar", _newSourceProcessors.nearFieldEffectProcessor, "rightEar");
			brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "leftEar", this, "leftEar");
			brtManager->ConnectModulesSamples(_newSourceProcessors.nearFieldEffectProcessor, "rightEar", this, "rightEar");

			sourcesConnectedProcessors.push_back(std::move(_newSourceProcessors));
		}

		template <typename T>
		void DisconnectSoundSource(std::shared_ptr<T> _source, BRTBase::CBRTManager* brtManager) {

			std::string _sourceID = _source->GetID();
			auto it = std::find_if(sourcesConnectedProcessors.begin(), sourcesConnectedProcessors.end(), [&_sourceID](CSourceProcessors& sourceProcessorItem) { return sourceProcessorItem.sourceID == _sourceID; });
			if (it != sourcesConnectedProcessors.end()) {
				
			
				brtManager->DisconnectModulesSamples(it->nearFieldEffectProcessor, "leftEar", this, "leftEar");
				brtManager->DisconnectModulesSamples(it->nearFieldEffectProcessor, "rightEar", this, "rightEar");
				brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "leftEar", it->nearFieldEffectProcessor, "leftEar");
				brtManager->DisconnectModulesSamples(it->binauralConvolverProcessor, "rightEar", it->nearFieldEffectProcessor, "rightEar");
				brtManager->DisconnectModulesSamples(_source, "samples", it->binauralConvolverProcessor, "inputSamples");

				brtManager->DisconnectModuleID(this, it->binauralConvolverProcessor, "listenerID");
				brtManager->DisconnectModuleILD(this, it->nearFieldEffectProcessor, "listenerILD");
				brtManager->DisconnectModuleHRTF(this, it->binauralConvolverProcessor, "listenerHRTF");
				brtManager->DisconnectModuleTransform(this, it->nearFieldEffectProcessor, "listenerPosition");
				brtManager->DisconnectModuleTransform(this, it->binauralConvolverProcessor, "listenerPosition");

				brtManager->DisconnectModuleID(_source, it->binauralConvolverProcessor, "sourceID");
				brtManager->DisconnectModuleTransform(_source, it->nearFieldEffectProcessor, "sourcePosition");
				brtManager->DisconnectModuleTransform(_source, it->binauralConvolverProcessor, "sourcePosition");
				
				/*brtManager->RemoveProcessor(nearFieldEffectProcessor);
				brtManager->RemoveProcessor(binauralConvolverProcessor);*/

				it->Clear(brtManager);
				sourcesConnectedProcessors.erase(it); 					
			}			
		}


		void Update(std::string entryPointID) {
		
		}
		void UpdateCommand() {
		
		}


	private:

		/////////////////
		// Methods
		/////////////////





		/////////////////
		// Attributes
		/////////////////
		std::string listenerID;								// Store unique listener ID
		std::shared_ptr<BRTServices::CHRTF> listenerHRTF;	// HRTF of listener														
		std::shared_ptr<BRTServices::CILD> listenerILD;		// ILD of listener	
		//Common::CTransform listenerTransform;				// Transform matrix (position and orientation) of listener  
		//float listenerHeadRadius;							// Head radius of listener 

		Common::CGlobalParameters globalParameters;

		/*std::shared_ptr<CEntryPointSamplesVector >		leftEarEntryPoint;
		std::shared_ptr<CEntryPointSamplesVector >		rightEarEntryPoint;				
		std::shared_ptr<CExitPointTransform>			listenerPositionExitPoint;
		std::shared_ptr<CExitPointHRTFPtr>				hrtfExitPoint;
		std::shared_ptr<CExitPointILDPtr>				ildExitPoint;
		std::shared_ptr<CExitPointID>					listenerIDExitPoint;*/

		CMonoBuffer<float> leftBuffer;
		CMonoBuffer<float> rightBuffer;
		
		bool leftDataReady;
		bool rightDataReady;	


		std::vector< CSourceProcessors> sourcesConnectedProcessors;

		
		
	};
}
#endif