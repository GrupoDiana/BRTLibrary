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
#ifndef _CBILATERAL_AMBISONIC_ENCODER_PROCESSOR_HPP
#define _CBILATERAL_AMBISONIC_ENCODER_PROCESSOR_HPP

#include <memory>
#include <vector>
#include <algorithm>
#include <Connectivity/AdvancedEntryPointManager.hpp>
#include <Connectivity/ExitPointManager.hpp>
#include <ProcessingModules/UniformPartitionedConvolution.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/BilateralAmbisonicEncoder.hpp>


namespace BRTProcessing {
class CBilateralAmbisonicEncoderProcessor : public BRTConnectivity::CBRTConnectivity, public CBilateralAmbisonicEncoder {
		
    public:
		CBilateralAmbisonicEncoderProcessor() {
            CreateSamplesEntryPoint("inputSamples");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");           
			CreateHRTFPtrEntryPoint("listenerHRTF");
			CreateHRBRIRPtrEntryPoint("listenerHRBRIR");
			CreateILDPtrEntryPoint("listenerILD");

			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");

			CreateMultipleSamplesExitPoint("leftAmbisonicChannels");
			CreateMultipleSamplesExitPoint("rightAmbisonicChannels");
        }

		/**
		 * @brief Implementation of CProcessorBase virtual method
		*/
        void AllEntryPointsAllDataReady() {
			
			//std::lock_guard<std::mutex> l(mutex);

			std::vector<CMonoBuffer<float>> leftAmbisonicChannelsBuffers;
			std::vector<CMonoBuffer<float>> rightAmbisonicChannelsBuffers;
			CMonoBuffer<float> outRightBuffer;
			
			CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
			if (buffer.size() == 0) { return; }

			Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
			Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();												
			std::weak_ptr<BRTServices::CServicesBase> listenerHRTF = GetHRTFPtrEntryPoint("listenerHRTF")->GetData();
			std::weak_ptr<BRTServices::CServicesBase> listenerHRBRIR = GetHRBRIRPtrEntryPoint("listenerHRBRIR")->GetData();
			std::weak_ptr<BRTServices::CSOSFilters> listenerNFCFilters = GetILDPtrEntryPoint("listenerILD")->GetData();
			
			if (listenerHRTF.lock() != nullptr) {
				Process(buffer, leftAmbisonicChannelsBuffers, rightAmbisonicChannelsBuffers, sourcePosition, listenerPosition, listenerHRTF, listenerNFCFilters);
			}
			else if (listenerHRBRIR.lock() != nullptr) {
				Process(buffer, leftAmbisonicChannelsBuffers, rightAmbisonicChannelsBuffers, sourcePosition, listenerPosition, listenerHRBRIR, listenerNFCFilters);
			}
			else {
				SET_RESULT(RESULT_ERROR_NOTSET, "Bilateral Ambisonic Encoder Processor ERROR: No HRTF or HRBRIR data available");
				return;
			}								
			GetMultipleSamplesVectorExitPoint("leftAmbisonicChannels")->sendData(leftAmbisonicChannelsBuffers);
			GetMultipleSamplesVectorExitPoint("rightAmbisonicChannels")->sendData(rightAmbisonicChannelsBuffers);										
        }

		/**
		 * @brief Implementation of CProcessorBase virtual method
		*/
		void UpdateCommand() {					
						
			BRTConnectivity::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull() || command.GetCommand() == "") { return; }

			if (IsToMyListener(command.GetStringParameter("listenerID"))) {
				
				if (command.GetCommand() == "/bilateralAmbisonicsEncoder/enableNearFieldEffect") {
					if (command.GetBoolParameter("enable")) { EnableNearFieldEffect(); }
					else { DisableNearFieldEffect(); }
				}
				else if (command.GetCommand() == "/bilateralAmbisonicsEncoder/enableBilateralAmbisonics") {
					if (command.GetBoolParameter("enable")) { EnableITDSimulation(); }
					else { DisableITDSimulation(); }
				}
				else if (command.GetCommand() == "/bilateralAmbisonicsEncoder/resetBuffers") {
					ResetBuffers();
				}
			}
			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/resetBuffers") {
					ResetBuffers();
				}
			}
		} 
      		
		///**
		// * @brief Set the ambisonic order for the spatialization process
		// * @param _ambisonicOrder 
		//*/
		//void SetAmbisonicOrder(int _ambisonicOrder) { 			
		//	CBilateralAmbisonicEncoder::SetAmbisonicOrder(_ambisonicOrder); 
		//}
		///**
		// * @brief Set the ambisonic order for the spatialization process
		// * @param _ambisonicNormalization 
		//*/
		//void SetAmbisonicNormalization(Common::TAmbisonicNormalization _ambisonicNormalization) { 			
		//	CBilateralAmbisonicEncoder::SetAmbisonicNormalization(_ambisonicNormalization); 
		//}


    private:       				
		/// Check Source ID
		bool IsToMySoundSource(std::string _sourceID) {
			std::string mySourceID = GetIDEntryPoint("sourceID")->GetData();
			return mySourceID == _sourceID;
		}
		
		/// Check Listener ID
		bool IsToMyListener(std::string _listenerID) {
			std::string myListenerID = GetIDEntryPoint("listenerID")->GetData();
			return myListenerID == _listenerID;
		}
    };
}
#endif