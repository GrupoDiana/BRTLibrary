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
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _NEAR_FIELD_EFFECT_PROCESSOR_HPP
#define _NEAR_FIELD_EFFECT_PROCESSOR_HPP

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/NearFieldEffect.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
    class CNearFieldEffectProcessor : public BRTBase::CProcessorBase, CNearFieldEffect {
		
    public:
		CNearFieldEffectProcessor() {
            CreateSamplesEntryPoint("leftEar");
			CreateSamplesEntryPoint("rightEar");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");           			
			CreateILDPtrEntryPoint("listenerILD");

            CreateSamplesExitPoint("leftEar");
            CreateSamplesExitPoint("rightEar");   									
        }

        void Update(std::string _entryPointId) {            
			std::lock_guard<std::mutex> l(mutex);
			CMonoBuffer<float> outLeftBuffer;
			CMonoBuffer<float> outRightBuffer;

			if (_entryPointId == "leftEar" || _entryPointId == "rightEar") {
				CMonoBuffer<float> leftBuffer = GetSamplesEntryPoint("leftEar")->GetData();
				CMonoBuffer<float> rightBuffer = GetSamplesEntryPoint("rightEar")->GetData();

				Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();												
				std::weak_ptr<BRTServices::CILD> listenerILD = GetILDPtrEntryPoint("listenerILD")->GetData();
				
				if (leftBuffer.size() != 0  || rightBuffer.size() !=0)  {
					Process(leftBuffer, rightBuffer, outLeftBuffer, outRightBuffer, sourcePosition, listenerPosition, listenerILD);
					GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
					GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);
				}				
				this->resetUpdatingStack();				
			}            
        }

		void UpdateCommand() {
			std::lock_guard<std::mutex> l(mutex);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
			
			//if (IsToMyListener(command.GetStringParameter("listenerID"))) { 
				if (command.GetCommand() == "/listener/enableNearFiedlEffect") {
					if (command.GetBoolParameter("enable")) { EnableNearFieldEffect(); }
					else { DisableNearFieldEffect(); }
				}
			//}
		} 
      
    private:
		mutable std::mutex mutex;
		
		
    };
}
#endif