#ifndef _NEAR_FIELD_EFFECT_PROCESSOR_HPP
#define _NEAR_FIELD_EFFECT_PROCESSOR_HPP

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/Buffer.h>
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
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
									
			std::cout << command.GetAddress() << std::endl;
			
			if (command.GetAddress() == "/brt/listener/enableNearFiedlEffect/") {
				//std::cout << "sourceID: " << command.GetID() << std::endl;
				//TODO check if its my source ID
				if (command.GetBoolParameter("boolParam")) { EnableNearFieldEffect(); }
				else { DisableNearFieldEffect(); }
			}							
		} 
      
    private:
       
		
		
    };
}
#endif