/**
* \class CHRTFConvolverProcessor
*
* \brief Declaration of CHRTFConvolverProcessor class
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
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/
#ifndef HRTF_CONVOLVER_PROCESSOR_
#define HRTF_CONVOLVER_PROCESSOR_

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/UPCAnechoic.hpp>
#include <Common/Buffer.hpp>
#include <ServiceModules/ServiceModuleInterfaces.hpp>
#include <ServiceModules/HRBRIR.hpp>
#include <ProcessingModules/HRTFConvolver.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
    class CHRTFConvolverProcessor : public BRTBase::CProcessorBase, public CHRTFConvolver {
		
    public:
		CHRTFConvolverProcessor() {
            CreateSamplesEntryPoint("inputSamples");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");           
			CreateHRTFPtrEntryPoint("listenerHRTF");
			CreateHRBRIRPtrEntryPoint("listenerHRBRIR");

			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");

            CreateSamplesExitPoint("leftEar");
            CreateSamplesExitPoint("rightEar");   									
        }

		/**
		 * @brief Implementation of CProcessorBase virtual method
		*/
		void AllEntryPointsAllDataReady() {

			std::lock_guard<std::mutex> l(mutex);

			CMonoBuffer<float> outLeftBuffer;
			CMonoBuffer<float> outRightBuffer;

			//if (_entryPointId == "inputSamples") {
			CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();

			if (buffer.size() == 0) { return; }

			Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
			Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();
			//std::weak_ptr<BRTServices::CHRTF> listenerHRTF = GetHRTFPtrEntryPoint("listenerHRTF")->GetData();			 						
			std::weak_ptr<BRTServices::CServicesBase> listenerHRTF = GetHRTFPtrEntryPoint("listenerHRTF")->GetData();
			std::weak_ptr<BRTServices::CServicesBase> listenerHRBRIR = GetHRBRIRPtrEntryPoint("listenerHRBRIR")->GetData();

			if (listenerHRTF.lock() != nullptr) { 
				Process(buffer, outLeftBuffer, outRightBuffer, sourcePosition, listenerPosition, listenerHRTF);
			}							
			else if (listenerHRBRIR.lock() != nullptr) {				
				Process(buffer, outLeftBuffer, outRightBuffer, sourcePosition, listenerPosition, listenerHRBRIR);
			} else {
				SET_RESULT(RESULT_ERROR_NOTSET, "HRTF Convolver error: No HRTF or HRBRIR data available");
				return;
			}

								
			GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
			GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);
				

        }

		void UpdateCommand() {					
			
			std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetCommand() == "") { return; }

			if (IsToMyListener(command.GetStringParameter("listenerID"))) { 
				if (command.GetCommand() == "/HRTFConvolver/enableSpatialization") {					
					if (command.GetBoolParameter("enable")) { EnableSpatialization(); }
					else { DisableSpatialization(); }
				}
				else if (command.GetCommand() == "/HRTFConvolver/enableInterpolation") {					
					if (command.GetBoolParameter("enable")) { EnableInterpolation(); }
					else { DisableInterpolation(); }
				}
				else if (command.GetCommand() == "/HRTFConvolver/resetBuffers") {
					ResetSourceConvolutionBuffers();					
				}
			}

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/resetBuffers") {
					ResetSourceConvolutionBuffers();
				}
			}
		} 
      
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