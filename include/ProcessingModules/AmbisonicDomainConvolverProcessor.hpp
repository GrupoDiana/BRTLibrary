/**
* \class CAmbisonicDomainConvolverProcessor
*
* \brief Declaration of CAmbisonicDomainConvolverProcessor class
* \date	November 2023
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
#ifndef _AMBISONIC_DOMAIN_CONVOLVER_PROCESSOR_
#define _AMBISONIC_DOMAIN_CONVOLVER_PROCESSOR_

#include <memory>
#include <vector>
#include <algorithm>
#include <Base/AdvancedEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>
#include <Common/UPCAnechoic.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/AmbisonicDomainConvolver.hpp>


namespace BRTProcessing {
    class CAmbisonicDomainConvolverProcessor : public BRTBase::CAdvancedEntryPointManager, public BRTBase::CExitPointManager, public CAmbisonicDomainConvolver {
		
    public:
		CAmbisonicDomainConvolverProcessor(Common::T_ear _earToProcess) : CAmbisonicDomainConvolver(_earToProcess) {
			CreateMultipleChannelsEntryPoint("inputChannels", 1);            
			CreateABIRPtrEntryPoint("listenerAmbisonicBIR");
			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");
			CreatePositionEntryPoint("listenerPosition");
            CreateSamplesExitPoint("outSamples");			            
        }
		
		/**
		 * @brief Implementation of CProcessorBase virtual method
		 * @param _entryPointId entryPoint ID
		*/
		void OneEntryPointOneDataReceived(std::string _entryPointId) {
			std::lock_guard<std::mutex> l(mutex);
			if (_entryPointId == "inputChannels") {				
				std::vector<CMonoBuffer<float>> inputChannels = GetMultipleSamplesVectorEntryPoint("inputChannels")->GetData();
				if (inputChannels.size() != 0) { MixChannelsBuffer(inputChannels); }
			}
		}
        
		/**
		 * @brief Implementation of CProcessorBase virtual method
		*/
		void AllEntryPointsAllDataReady() {
			
			std::lock_guard<std::mutex> l(mutex);
			if (channelsBuffer.size() == 0) { return;	}
			CMonoBuffer<float> outBuffer;			
							
			std::weak_ptr<BRTServices::CAmbisonicBIR> listenerABIR = GetABIRPtrEntryPoint("listenerAmbisonicBIR")->GetData();
			Common::CTransform _listenerTransform = GetPositionEntryPoint("listenerPosition")->GetData();
			Process(channelsBuffer, outBuffer, listenerABIR, _listenerTransform);
			GetSamplesExitPoint("outSamples")->sendData(outBuffer);					
			channelsBuffer.clear();
			
			
        }

		void UpdateCommand() {					
			
			std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetCommand() == "") { return; }

			if (IsToMyListener(command.GetStringParameter("listenerID"))) { 				
				if (command.GetCommand() == "/ambisonicsDomianConvoler/resetBuffers") {
					ResetChannelsConvolutionBuffers();
				}
			}

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/resetBuffers") {
					ResetChannelsConvolutionBuffers();
				}
			}
		} 

		////void SetEar(Common::T_ear _ear) { CAmbisonicDomainConvolver::SetEar(_ear);	}
		//void SetAmbisonicOrder(int _ambisonicOrder) { CAmbisonicDomainConvolver::SetAmbisonicOrder(_ambisonicOrder); }		

    private:
       
		mutable std::mutex mutex;				
		std::vector<CMonoBuffer<float>> channelsBuffer;		// To store the mix of the ambisonic channels before doing the process.
				
		/**
		 * @brief Mix new channes with buffer channesl
		 * @param inputChannels Vector of CMonoBuffer to be mixed with the buffer
		*/
		void MixChannelsBuffer(std::vector<CMonoBuffer<float>> inputChannels) {
			
			if (channelsBuffer.size() != inputChannels.size()) {
				channelsBuffer = std::vector<CMonoBuffer<float>>(inputChannels.size(), CMonoBuffer<float>(inputChannels[0].size()));
			}

			for (int nChannel = 0; nChannel < inputChannels.size(); nChannel++) {
				channelsBuffer[nChannel] += inputChannels[nChannel];
			}
		}

		/**
		 * @brief Check if the indicated source is the one this module is connected to.
		 * @param _sourceID Source ID
		 * @return true if yes
		*/
		bool IsToMySoundSource(std::string _sourceID) {
			std::string mySourceID = GetIDEntryPoint("sourceID")->GetData();
			return mySourceID == _sourceID;
		}
		/**
		 * @brief Check if this module is connected to the indicated listener.
		 * @param _listenerID Listener ID
		 * @return true if yes
		*/
		bool IsToMyListener(std::string _listenerID) {						
			std::shared_ptr<BRTBase::CEntryPointID> _listenerIDEntryPoint= GetIDEntryPoint("listenerID");
			if (_listenerIDEntryPoint != nullptr) {
				std::string myListenerID = _listenerIDEntryPoint->GetData();
				return myListenerID == _listenerID;
			}
			else {
				return false;
			}			
		}
    };
}
#endif