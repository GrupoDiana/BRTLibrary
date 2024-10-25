/**
* \class CFreeFieldEnvironmentProcessor
*
* \brief This class implements the free field processor. Applies the effects of free space propagation to a single source
* \date	Oct 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo ||
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

#include <memory>
#include <Common/ErrorHandler.hpp>
#include <Base/BRTManager.hpp>
#include <Base/AdvancedEntryPointManager.hpp>
#include <EnvironmentModels/FreeFieldEnvironment/FreeFieldEnvironment.hpp>

#ifndef _C_FREE_FIELD_ENVIRONMENT_PROCESSOR_HPP_
#define _C_FREE_FIELD_ENVIRONMENT_PROCESSOR_HPP_
namespace BRTEnvironmentModel { 

	class CFreeFieldEnvironmentProcessor : public BRTBase::CAdvancedEntryPointManager, public BRTBase::CExitPointManager, public CFreeFieldEnvironment  {
	
	public:
		CFreeFieldEnvironmentProcessor(BRTBase::CBRTManager * _brtManager)			
			: brtManager { _brtManager }
			, initialized { false }	{

			CreateSamplesEntryPoint("inputSamples");
			CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");
			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");
			
			CreateSamplesExitPoint("outputSamples");
		}


		/**
		 * @brief Setup the environment processor
		 * @param _orinalSourceID ID of the original source
		 * @return True if the setup was successful
		 */
		bool Setup(std::string _orinalSourceID) {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (initialized) {
				SET_RESULT(RESULT_ERROR_NOTALLOWED, "The SDN environment processor is already initialized");
				return false;
			}

			if (_orinalSourceID == "") {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The source ID must be defined");
				return false;
			}

			originalSourceID = _orinalSourceID;
			
			/*std::shared_ptr<BRTBase::CSourceModelBase> _source = brtManager->GetSoundSource<BRTBase::CSourceModelBase>(originalSourceID);
			if (_source == nullptr) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The source ID does not exist");
				return false;
			}
			if (_source->GetSourceType() == BRTBase::CSourceModelBase::TSourceType::Simple) {
				source = 
			}*/			

			virtualSource = brtManager->CreateSoundSource<BRTSourceModel::CVirtualSourceModel>("FreeField_" + originalSourceID);						
			virtualSource->SetOriginSourceID(originalSourceID);
			initialized = true;
			return true;
		}

		/**
		 * @brief Connect the environment processor to a listener model
		 * @param _listenerModel Listener model to connect
		 * @return True if the connection was successful
		 */
		bool ConnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listenerModel) {			
			return _listenerModel->ConnectSoundSource(virtualSource);
		}

		/**
		 * @brief Disconnect the environment processor from a listener model
		 * @param _listenerModel Listener model to disconnect
		 * @return True if the disconnection was successful
		 */
		bool DisconnectToListenerModel(std::shared_ptr<BRTBase::CListenerModelBase> _listenerModel) {			
			return _listenerModel->DisconnectSoundSource(virtualSource);
		}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void AllEntryPointsAllDataReady() override {
			std::lock_guard<std::mutex> l(mutex); // Lock the mutex
			if (!initialized) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "The SDN environment processor is not initialized");
				return;
			}
			
			// Get data from entry points
			CMonoBuffer<float> inBuffer = GetSamplesEntryPoint("inputSamples")->GetData();
			Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
			Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();

			if (inBuffer.size() == 0) {
				std::cout << "Buffer Size = 0" << std::endl;
				SET_RESULT(RESULT_ERROR_BADSIZE, "The input buffer size is 0");
				return;
			
			}

			CMonoBuffer<float> outBuffer;
			Process(inBuffer, outBuffer, sourcePosition, listenerPosition);
			
			
			virtualSource->SetSourceTransform(sourcePosition);
			virtualSource->SetBuffer(outBuffer);													
		}

		void ResetProcessBuffers() {
		
		}

		/**
		 * @brief Implementation of CAdvancedEntryPointManager virtual method
		*/
		void UpdateCommand() override {
		}
	private:
		

		/////////////////
		// Attributes
		/////////////////

		mutable std::mutex mutex;	// To avoid access collisions		
		Common::CGlobalParameters globalParameters;		
		std::shared_ptr<BRTSourceModel::CVirtualSourceModel> virtualSource;
		BRTBase::CBRTManager * brtManager;
		
		//CWaveGuide waveGuide;
		std::shared_ptr<BRTProcessing::CDistanceAttenuation> distanceAttenuation;
		//CLongDistanceFilter longDistanceFilter;
		

		std::string originalSourceID;
		bool initialized;		
	};
}
#endif