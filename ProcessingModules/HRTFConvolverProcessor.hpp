#ifndef HRTF_CONVOLVER_PROCESSOR_
#define HRTF_CONVOLVER_PROCESSOR_

#include <Base/ProcessorBase.hpp>
#include <Base/EntryPoint.hpp>
#include <Common/UPCAnechoic.h>
#include <Common/Buffer.h>
#include <ProcessingModules/HRTFConvolver.hpp>

#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
    class CHRTFConvolverProcessor : public BRTBase::CProcessorBase, CHRTFConvolver {
		
    public:
		CHRTFConvolverProcessor() {
            CreateSamplesEntryPoint("inputSamples");

            CreatePositionEntryPoint("sourcePosition");
			CreatePositionEntryPoint("listenerPosition");           
			CreateHRTFPtrEntryPoint("listenerHRTF");

            CreateSamplesExitPoint("leftEar");
            CreateSamplesExitPoint("rightEar");   									
        }

        void Update(std::string _entryPointId) {            

			CMonoBuffer<float> outLeftBuffer;
			CMonoBuffer<float> outRightBuffer;

			if (_entryPointId == "inputSamples") {
				CMonoBuffer<float> buffer = GetSamplesEntryPoint("inputSamples")->GetData();
				Common::CTransform sourcePosition = GetPositionEntryPoint("sourcePosition")->GetData();
				Common::CTransform listenerPosition = GetPositionEntryPoint("listenerPosition")->GetData();												
				std::weak_ptr<BRTServices::CHRTF> listenerHRTF = GetHRTFPtrEntryPoint("listenerHRTF")->GetData();
				if (buffer.size() != 0) {
					Process(buffer, outLeftBuffer, outRightBuffer, sourcePosition, listenerPosition, listenerHRTF);
					GetSamplesExitPoint("leftEar")->sendData(outLeftBuffer);
					GetSamplesExitPoint("rightEar")->sendData(outRightBuffer);
				}				
				this->resetUpdatingStack();				
			}            
        }

		void UpdateCommand() {					
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
									
			std::cout << command.GetCommand() << std::endl;
			
			if (command.GetCommand() == "/brt/listener/enableSpatialization/") {
				//std::cout << "sourceID: " << command.GetID() << std::endl;
				//TODO check if its my source ID
				if (command.GetBoolParameter()) { EnableSpatialization(); }
				else { DisableSpatialization(); }
			}
			else if (command.GetCommand() == "/brt/listener/enableInterpolation/") {
				//std::cout << "sourceID: " << command.GetID() << std::endl;
				//TODO check if its my source ID
				if (command.GetBoolParameter()) { EnableInterpolation(); }
				else { DisableInterpolation(); }
			}
			else if (command.GetCommand() == "/brt/source/HRTFConvolver/resetBuffers/") {
				//std::cout << "sourceID: " << command.GetSourceID() << std::endl;
				//TODO check if its my source ID
				ResetSourceConvolutionBuffers();
			}			
		} 
      
    private:
       
		
		
    };
}
#endif