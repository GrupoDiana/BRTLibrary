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

			CreateIDEntryPoint("sourceID");
			CreateIDEntryPoint("listenerID");

            CreateSamplesExitPoint("leftEar");
            CreateSamplesExitPoint("rightEar");   									
        }

        void Update(std::string _entryPointId) {            
			
			std::lock_guard<std::mutex> l(_mtx);

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
			
			std::lock_guard<std::mutex> l(_mtx);
			BRTBase::CCommand command = GetCommandEntryPoint()->GetData();
															
			//if (IsToMyListener(command.GetStringParameter("listenerID"))) { 
				if (command.GetCommand() == "/listener/enableSpatialization") {					
					if (command.GetBoolParameter("enable")) { EnableSpatialization(); }
					else { DisableSpatialization(); }
				}
				else if (command.GetCommand() == "/listener/enableInterpolation") {					
					if (command.GetBoolParameter("enable")) { EnableInterpolation(); }
					else { DisableInterpolation(); }
				}
			//}

			if (IsToMySoundSource(command.GetStringParameter("sourceID"))) {
				if (command.GetCommand() == "/source/HRTFConvolver/resetBuffers") {
					ResetSourceConvolutionBuffers();
				}
			}
		} 
      
    private:
       
		mutable std::mutex _mtx;

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