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
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/
#ifndef _AMBISONIC_DOMAIN_CONVOLVER_PROCESSOR_
#define _AMBISONIC_DOMAIN_CONVOLVER_PROCESSOR_

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/UPCAnechoic.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/AmbisonicDomainConvolver.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
    class CAmbisonicDomainConvolverProcessor : public BRTBase::CProcessorBase, CAmbisonicDomainConvolver {
		
    public:
		CAmbisonicDomainConvolverProcessor() {
			CreateMultipleSamplesEntryPoint("inputChannels");

            //CreatePositionEntryPoint("sourcePosition");
			//CreatePositionEntryPoint("listenerPosition");           
			CreateABIRPtrEntryPoint("listenerAmbisonicBIR");

			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");

            CreateSamplesExitPoint("outSamples");
            
        }

        void Update(std::string _entryPointId) {            
			
			std::lock_guard<std::mutex> l(mutex);

			CMonoBuffer<float> outBuffer;			

			if (_entryPointId == "inputChannels") {
				std::vector<CMonoBuffer<float>> channels = GetMultipleSamplesVectorEntryPoint("inputChannels")->GetData();
				//Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				//Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();																
				std::weak_ptr<BRTServices::CAmbisonicBIR> listenerABIR = GetABIRPtrEntryPoint("listenerAmbisonicBIR")->GetData();
				if (channels.size() != 0) {
					Process(channels, outBuffer, listenerABIR);
					GetSamplesExitPoint("outSamples")->sendData(outBuffer);					
				}				
				this->resetUpdatingStack();				
			}            
        }

		void UpdateCommand() {					
			
			std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
															
			//if (IsToMyListener(command.GetStringParameter("listenerID"))) { 
				/*if (command.GetCommand() == "/listener/enableSpatialization") {					
					if (command.GetBoolParameter("enable")) { EnableSpatialization(); }
					else { DisableSpatialization(); }
				}
				else if (command.GetCommand() == "/listener/enableInterpolation") {					
					if (command.GetBoolParameter("enable")) { EnableInterpolation(); }
					else { DisableInterpolation(); }
				}
				else if (command.GetCommand() == "/listener/resetBuffers") {
					ResetSourceConvolutionBuffers();					
				}*/
			//}

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/AmbisonicDomainConvolver/resetBuffers") {
					ResetSourceConvolutionBuffers();
				}
			}
		} 

		void SetEar(Common::T_ear _ear) { CAmbisonicDomainConvolver::SetEar(_ear);	}
		void SetAmbisonicOrder(int _ambisonicOrder) { CAmbisonicDomainConvolver::SetAmbisonicOrder(_ambisonicOrder); }		

    private:
       
		mutable std::mutex mutex;

		bool IsToMySoundSource(std::string _sourceID) {
			std::string mySourceID = GetIDEntryPoint("sourceID")->GetData();
			return mySourceID == _sourceID;
		}
		
		bool IsToMyListener(std::string _listenerID) {
			std::string myListenerID = GetIDEntryPoint("listenerID")->GetData();
			return myListenerID == _listenerID;
		}
    };
}
#endif