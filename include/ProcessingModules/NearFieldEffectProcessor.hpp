/**
* \class CNearFieldEffectProcessor
*
* \brief Declaration of CNearFieldEffectProcessor class
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

#ifndef _NEAR_FIELD_EFFECT_PROCESSOR_HPP
#define _NEAR_FIELD_EFFECT_PROCESSOR_HPP

#include <memory>
#include <vector>
#include <algorithm>
#include <Base/AdvancedEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/NearFieldEffect.hpp>


namespace BRTProcessing {
    class CNearFieldEffectProcessor : public BRTBase::CAdvancedEntryPointManager, public BRTBase::CExitPointManager, public CNearFieldEffect {
		
    public:
		CNearFieldEffectProcessor() {
            CreateSamplesEntryPoint("leftEar");
			CreateSamplesEntryPoint("rightEar");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");           			
			
			CreateIDEntryPoint("sourceID");
			CreateILDPtrEntryPoint("listenerILD");

            CreateSamplesExitPoint("leftEar");
            CreateSamplesExitPoint("rightEar");   									
        }

        void AllEntryPointsAllDataReady() {
			std::lock_guard<std::mutex> l(mutex);
			CMonoBuffer<float> outLeftBuffer;
			CMonoBuffer<float> outRightBuffer;

			//if (_entryPointId == "leftEar" || _entryPointId == "rightEar") {
				CMonoBuffer<float> leftBuffer = GetSamplesEntryPoint("leftEar")->GetData();
				CMonoBuffer<float> rightBuffer = GetSamplesEntryPoint("rightEar")->GetData();

				Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();												
				std::weak_ptr<BRTServices::CNearFieldCompensationFilters> listenerNFCFilters = GetILDPtrEntryPoint("listenerILD")->GetData();
				
				if (leftBuffer.size() != 0  || rightBuffer.size() !=0)  {
					Process(leftBuffer, rightBuffer, outLeftBuffer, outRightBuffer, sourcePosition, listenerPosition, listenerNFCFilters);
					GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
					GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);
				}				
				//this->ResetEntryPointWaitingList();				
			//}            
        }

		void UpdateCommand() {
			std::lock_guard<std::mutex> l(mutex);
			
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
			if (command.isNull()) { return; }			
			
			if (IsToMyListener(command.GetStringParameter("listenerID"))) { 
				if (command.GetCommand() == "/nearFieldProcessor/enable") {
					if (command.GetBoolParameter("enable")) { EnableNearFieldEffect(); }
					else { DisableNearFieldEffect(); }
				}
				else if (command.GetCommand() == "/nearFieldProcessor/resetBuffers") {
					ResetProcessBuffers();
				}
			}
			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/resetBuffers") {
					ResetProcessBuffers();
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
			std::shared_ptr<BRTBase::CEntryPointID> _listenerIDEntryPoint = GetIDEntryPoint("listenerID");
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