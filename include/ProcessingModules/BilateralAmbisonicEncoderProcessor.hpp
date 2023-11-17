/**
* \class CAmbisonicEncoderProcessor
*
* \brief Declaration of CAmbisonicEncoderProcessor class
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
#ifndef _CBILATERAL_AMBISONIC_ENCODER_PROCESSOR_HPP
#define _CBILATERAL_AMBISONIC_ENCODER_PROCESSOR_HPP

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/UPCAnechoic.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/BilateralAmbisonicEncoder.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
    class CBilateralAmbisonicEncoderProcessor : public BRTBase::CProcessorBase, CBilateralAmbisonicEncoder {
		
    public:
		CBilateralAmbisonicEncoderProcessor() {
            CreateSamplesEntryPoint("inputSamples");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");           
			CreateHRTFPtrEntryPoint("listenerHRTF");
			CreateILDPtrEntryPoint("listenerILD");

			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");

			CreateMultipleSamplesExitPoint("leftAmbisonicChannels");
			CreateMultipleSamplesExitPoint("rightAmbisonicChannels");
        }

        void UpdateAllEntryPoints(std::string _entryPointId) {            
			
			std::lock_guard<std::mutex> l(mutex);

			std::vector<CMonoBuffer<float>> leftAmbisonicChannelsBuffers;
			std::vector<CMonoBuffer<float>> rightAmbisonicChannelsBuffers;
			CMonoBuffer<float> outRightBuffer;

			if (_entryPointId == "inputSamples") {
				CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
				Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();												
				std::weak_ptr<BRTServices::CHRTF> listenerHRTF = GetHRTFPtrEntryPoint("listenerHRTF")->GetData();
				std::weak_ptr<BRTServices::CILD> listenerILD = GetILDPtrEntryPoint("listenerILD")->GetData();
				if (buffer.size() != 0) {															
					Process(buffer, leftAmbisonicChannelsBuffers, rightAmbisonicChannelsBuffers, sourcePosition, listenerPosition, listenerHRTF, listenerILD);
					
					
					
					GetMultipleSamplesVectorExitPoint("leftAmbisonicChannels")->sendData(leftAmbisonicChannelsBuffers);
					GetMultipleSamplesVectorExitPoint("rightAmbisonicChannels")->sendData(rightAmbisonicChannelsBuffers);
				}				
				//this->ResetEntryPointWaitingList();				
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
			if (command.GetCommand() == "/listener/enableNearFieldEffect") {
				if (command.GetBoolParameter("enable")) { EnableNearFieldEffect(); }
				else { DisableNearFieldEffect(); }
			}
			/*if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/HRTFConvolver/resetBuffers") {
					ResetSourceConvolutionBuffers();
				}
			}*/
		} 
      		

		void SetAmbisonicOrder(int _ambisonicOrder) { CBilateralAmbisonicEncoder::SetAmbisonicOrder(_ambisonicOrder); }
		void SetAmbisonicNormalization(Common::TAmbisonicNormalization _ambisonicNormalization) { CBilateralAmbisonicEncoder::SetAmbisonicNormalization(_ambisonicNormalization); }


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