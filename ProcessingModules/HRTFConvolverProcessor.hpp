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

#define EPSILON 0.0001f
#define ELEVATION_SINGULAR_POINT_UP 90.0
#define ELEVATION_SINGULAR_POINT_DOWN 270.0

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
			
			if (command.GetCommand() == "/brt/ConvolverEnableSpatialization/") {
				std::cout << command.GetBoolParameter() << std::endl;
			}
			else if (command.GetCommand() == "/brt/flotante/") { 

				std::cout << command.GetFloatParameter() << std::endl;
				DisableSpatialization(); 
			}
			else if (command.GetCommand() == "/brt/cadena/") { EnableSpatialization(); }
		}
      
    private:
       
 
		
    };
}
#endif