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
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/
#ifndef _AMBISONIC_DOMAIN_CONVOLVER_PROCESSOR_
#define _AMBISONIC_DOMAIN_CONVOLVER_PROCESSOR_

#include <memory>
#include <vector>
#include <algorithm>
#include <Connectivity/BRTConnectivity.hpp>
#include <ProcessingModules/UniformPartitionedConvolution.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/AmbisonicDomainConvolver.hpp>


namespace BRTProcessing {
	class CAmbisonicDomainConvolverProcessor : public BRTConnectivity::CBRTConnectivity, public CAmbisonicDomainConvolver {
		
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
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
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
			std::shared_ptr<BRTConnectivity::CEntryPointID> _listenerIDEntryPoint = GetIDEntryPoint("listenerID");
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